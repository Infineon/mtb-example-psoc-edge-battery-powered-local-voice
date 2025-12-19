/*****************************************************************************
* File Name        : main.c
*
* Description      : This source file contains the main routine for non-secure
*                    application in the CM33 CPU
*
* Related Document : See README.md
*
*******************************************************************************
 * (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"
#include "app_logger.h"
#include "amic_interface.h"
#include "audio_input_configuration.h"

#include "ppudbg.h"
#include "aad.h"

#define MODE                                            CYHAL_PDM_PCM_MODE_LEFT
#define MIC_MODE                                        (1)

#define MIC_MODE                                        (1)

#define MIC_RX_SAMPLES_COUNT                            (160*MIC_MODE)

/* Task Priority for Autonomous Analog FIFO */
#define AUTONOMOUS_ANALOG_INTERRUPT_PRIORITY            (2U)

/*******************************************************************************
* Global Variables
*******************************************************************************/
int32_t databuffer[256U];

int16_t mic_audio_app_buffer_ping[MIC_RX_SAMPLES_COUNT] = {0};
int16_t mic_audio_app_buffer_pong[MIC_RX_SAMPLES_COUNT] = {0};
int16_t* ping_pong_local_pointer = NULL;

static uint32_t sod_to_add_timeout = 0;
/*******************************************************************************
* Functions Prototypes
*******************************************************************************/
extern void audio_mic_data_feed_cm33(int16_t *audio_data);

/*******************************************************************************
 * Function Name: analog_mic_interface_init
 *******************************************************************************
 * Summary:
 * Function to enable the analog mic interface
 *
 * Parameters:
 *  none
 *
 * Return:
 *  Status of mic interface initialization
 *
 ******************************************************************************/
cy_rslt_t analog_mic_interface_init(void)
{
    uint32_t status;

    status = analog_init();

    /* Wait for the PRBs and the Preamps/Filters to stabilize */
    Cy_SysLib_Delay(100u);

    /* Send a firmware trigger to the Autonomous Analog to start the state machine */
    Cy_AutAnalog_FwTrigger(CY_AUTANALOG_FW_TRIGGER0);
    return status;
}

/*******************************************************************************
* Function Name: analog_init
********************************************************************************
* Summary:
* Function to initialize the Autonomous Analog. 
*
* Parameters:
*  Result of Autonomous analog initialisation
*
* Return:
*  Status of initialization
*
*******************************************************************************/
cy_rslt_t analog_init(void)
{
    /* Variables to capture return value of functions */
    uint32_t status;

    /* Initialize Autonomous Analog  */
    status = Cy_AutAnalog_Init(&autonomous_analog_init);

    if (CY_AUTANALOG_SUCCESS == status)
    {

        /* Advanced power controls:
         * Autonomous Controller power cycling:    disabled (default)
         * LP Oscillator power cycling        : disabled (default)
         * SAR Core and Buffer power cycling  : enabled */
        Cy_AutAnalog_AdvPowerControl(false,false,true);

        analog_interrupt_init();

        /* Start the AC */
        Cy_AutAnalog_StartAutonomousControl();

    }
    return status;
}

/*******************************************************************************
* Function Name: analog_fifo_isr
********************************************************************************
* Summary:
* Interrupt for analog fifo data. 
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void analog_fifo_isr(void)
{
    uint32_t intr_status;
    static bool ping_pong = true;
    bool all_below_threshold = true;

    /* Read interrupt status register */
    intr_status = Cy_AutAnalog_FIFO_GetInterruptStatus(ANALOG_FIFO_IDX);

    /* Disable the FIFO level interrupt. This needs to be re-enabled after reading the FIFO */
    Cy_AutAnalog_FIFO_ClearInterruptMask(ANALOG_FIFO_IDX,CY_AUTANALOG_INT_FIFO_LEVEL0);

    /* Clear handled interrupt */
    Cy_AutAnalog_FIFO_ClearInterrupt(ANALOG_FIFO_IDX, intr_status);

    /* Check FIFO0 interrupt */
    if ((intr_status & (uint32_t)(CY_AUTANALOG_INT_FIFO_LEVEL0)) != 0UL)
    {
        uint16_t fifoSize = Cy_AutAnalog_FIFO_ReadData(ANALOG_FIFO_IDX, ANALOG_FIFO_BUF0_IDX, MIC_RX_SAMPLES_COUNT, databuffer);
        if (ping_pong)
        {
            ping_pong_local_pointer = mic_audio_app_buffer_ping;
        }
        else
        {
            ping_pong_local_pointer = mic_audio_app_buffer_pong;
        }

        for(uint8_t z = 0; z<MIC_RX_SAMPLES_COUNT; z++)
        {
            /* convert 32bit signed value representing 12bit ADC value into 16bit integer value */
            *(ping_pong_local_pointer + z) = (int16_t)((*(databuffer + z))<<4); 
        }
        /* Copy data at ping_pong_local_pointer in to global buffer for dump */
        audio_mic_data_feed_cm33(ping_pong_local_pointer);
        ping_pong = !ping_pong;

        /* Re-enable the FIFO0 level interrupt */
        Cy_AutAnalog_FIFO_SetInterruptMask(ANALOG_FIFO_IDX,CY_AUTANALOG_INT_FIFO_LEVEL0);
    }
}


/*******************************************************************************
* Function Name: analog_interrupt_init
********************************************************************************
* Summary:
* Function to init the analog interrupt routine. 
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void analog_interrupt_init(void)
{
    /* Variables to capture return value of functions */
    cy_en_sysint_status_t sysIntStatus;

    /* Configuration of Autonomous Analog FIFO Interrupt */
    const cy_stc_sysint_t AUT_ANALOG_FIFO_IRQ_cfg =
    {
        .intrSrc      = pass_interrupt_fifo_IRQn,
        .intrPriority = AUTONOMOUS_ANALOG_INTERRUPT_PRIORITY,
    };
    /* Register FIFO interrupt service routine */
    sysIntStatus = Cy_SysInt_Init(&AUT_ANALOG_FIFO_IRQ_cfg, &analog_fifo_isr);
    if (sysIntStatus == CY_SYSINT_SUCCESS)
    {
        /* Enable interrupt in NVIC */
        NVIC_ClearPendingIRQ(pass_interrupt_fifo_IRQn);
        NVIC_EnableIRQ(pass_interrupt_fifo_IRQn);
    }
    else
    {
        app_log_print("Registring ISR for Autonomous analog failed\r\n");
        CY_ASSERT(0);
    }

    /* Enable the FIFO0 level interrupt */
    Cy_AutAnalog_FIFO_SetInterruptMask(ANALOG_FIFO_IDX,CY_AUTANALOG_INT_FIFO_LEVEL0);
}

/* [] END OF FILE */