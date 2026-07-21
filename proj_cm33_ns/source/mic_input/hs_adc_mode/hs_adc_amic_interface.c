/*******************************************************************************
 * File Name:   hs_adc_amic_interface.c
 *
 * Description: This file contains the high speed analog mic interface
 *
 * Related Document : See README.md
 *
*******************************************************************************
* (c) 2025-2026, Infineon Technologies AG, or an affiliate of Infineon
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
#include "hs_adc_amic_config.h"
#include "cy_tcpwm_counter.h"

#include "audio_input_configuration.h"
#include "app_logger.h"

#include "hs_adc_amic_interface.h"
/*******************************************************************************
* Macros
*******************************************************************************/
#define ANALOG_FIFO_IDX        (0U)
#define ANALOG_FIFO_BUF0_IDX   (0U)
#define MIC_MODE                                        (1)
#define MIC_RX_SAMPLES_COUNT                            (160*MIC_MODE)


#define AMIC_Timer             tcpwm_amic_timer_16kHz_HW, tcpwm_amic_timer_16kHz_NUM
#define AMIC_Timer_cfg         &tcpwm_amic_timer_16kHz_config

#define RMS_NOISE_FLOOR        (150U)
#define QUIET_WINDOW_FRAMES    (10) /* Number of 10mS frames */

const uint32_t noise_square = RMS_NOISE_FLOOR*RMS_NOISE_FLOOR;


/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool intr_fifo_flag        = false;

int32_t audio_buffer_left[256U];

uint32_t quiet_window_frames = QUIET_WINDOW_FRAMES;
uint32_t quiet_count = 0;
bool muted = false;

int16_t mic_audio_app_buffer_ping[MIC_RX_SAMPLES_COUNT] = {0};
int16_t mic_audio_app_buffer_pong[MIC_RX_SAMPLES_COUNT] = {0};
int16_t* ping_pong_local_pointer = NULL;
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
cy_rslt_t hs_adc_analog_init(void);
void hs_adc_analog_interrupt_init(void);
void hs_adc_analog_isr(void);
void hs_adc_analog_fifo_isr(void);

void hs_adc_microphone_mute_process(int32_t* samples, size_t sample_count);
extern void audio_mic_data_feed_cm33(int16_t *audio_data);


void switch_to_hs_adc_AA_config(void)
{
    uint32_t status;
    /* Deinitialize Low Power SAR ADC configuration of Autonomous Analog and Initialize High Speed SAR ADC configuration */
    Cy_AutAnalog_Disable();
    /* Switch to high speed ADC config of AA */
    status = hs_adc_analog_mic_interface_init();
    if(CY_AUTANALOG_SUCCESS != status)
    {
        app_log_print("Analog mic initialization failed\r\n");
    }
    else
    {
        cy_svc_lp_low_noise_config_t aad_config;
        aad_config.timeout_ms = SOD_TO_AAD_TIMEOUT_MS; //7 sec
        aad_config.low_noise_threshold = SOD_TO_AAD_THRESHOLD;
        aad_config.enable_feature = false;
        cy_svc_lp_low_noise_config(&aad_config);
        mtb_hal_syspm_lock_deepsleep();
    }
}

/*******************************************************************************
 * Function Name: hs_adc_analog_mic_interface_init
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
cy_rslt_t hs_adc_analog_mic_interface_init(void)
{
    uint32_t status;

    /* HS_AMIC_PUMP_CLK_DIV_NUM */
    Cy_SysClk_PeriPclkAssignDivider(PCLK_PASS_CLOCK_PUMP_PERI, CY_SYSCLK_DIV_8_BIT, HS_AMIC_PUMP_CLK_DIV_NUM);

    /* Configure the 16kHz timer */
    if (CY_TCPWM_SUCCESS != Cy_TCPWM_Counter_Init(AMIC_Timer, AMIC_Timer_cfg))
    {
        app_log_print("TCPWM Counter Configuration Failed!");
        CY_ASSERT(0);

    }

    /* Initialize the Autonomous Analog and its interrupts */
    status = hs_adc_analog_init();

     /* Enable the 16kHz timer */
    Cy_TCPWM_Counter_Enable(AMIC_Timer);
    /* Start the 16kHz counter */
    Cy_TCPWM_TriggerStart_Single(AMIC_Timer);

    return status;
}

cy_rslt_t hs_adc_analog_init(void)
{
    /* Variables to capture return value of functions */
    uint32_t status;

    /* Initialize Autonomous Analog  */
    status = Cy_AutAnalog_Init(&amic_hq_mono_init);

    if (CY_AUTANALOG_SUCCESS == status)
    {
        /* Enable the required interrupts */
        hs_adc_analog_interrupt_init();

        /* Start the AC */
        Cy_AutAnalog_StartAutonomousControl();
    }
    return status;
}

void hs_adc_analog_fifo_isr(void)
{
    uint32_t intr_status;
    static bool ping_pong = true;

    /* Read interrupt status register */
    intr_status = Cy_AutAnalog_FIFO_GetInterruptStatus(ANALOG_FIFO_IDX);

    /* Disable the FIFO level interrupt. This needs to be re-enabled after reading the FIFO */
    Cy_AutAnalog_FIFO_ClearInterruptMask(ANALOG_FIFO_IDX,CY_AUTANALOG_INT_FIFO_LEVEL0);

    /* Clear handled interrupt */
    Cy_AutAnalog_FIFO_ClearInterrupt(ANALOG_FIFO_IDX, intr_status);

    /* Check FIFO0 interrupt */
    if ((intr_status & (uint32_t)(CY_AUTANALOG_INT_FIFO_LEVEL0)) != 0UL)
    {
        Cy_AutAnalog_FIFO_ReadAllData(ANALOG_FIFO_IDX, ANALOG_FIFO_BUF0_IDX, audio_buffer_left);
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
            /* Copy 32bit signed value representing 16bit ADC value into 16bit integer value array */
            *(ping_pong_local_pointer + z) = (int16_t)(*(audio_buffer_left + z));
        }

        if(muted)
        {
            app_log_print("Microphone muted \r\n");
        }

        /* Copy data at ping_pong_local_pointer in to global buffer for dump */
        audio_mic_data_feed_cm33(ping_pong_local_pointer);
        ping_pong = !ping_pong;

        /* Re-enable the FIFO0 level interrupt */
        Cy_AutAnalog_FIFO_SetInterruptMask(ANALOG_FIFO_IDX,CY_AUTANALOG_INT_FIFO_LEVEL0);
    }
}

void hs_adc_analog_interrupt_init(void)
{

    /* Variables to capture return value of functions */
    cy_en_sysint_status_t sysIntStatus;

    /* Configuration of Autonomous Analog FIFO Interrupt */
    const cy_stc_sysint_t AUT_ANALOG_FIFO_IRQ_cfg =
    {
        .intrSrc      = pass_interrupt_fifo_IRQn,
        .intrPriority = 3U,
    };
    /* Register FIFO interrupt service routine */
    sysIntStatus = Cy_SysInt_Init(&AUT_ANALOG_FIFO_IRQ_cfg, &hs_adc_analog_fifo_isr);
    if (sysIntStatus == CY_SYSINT_SUCCESS)
    {
        /* Enable interrupt in NVIC */
        NVIC_ClearPendingIRQ(pass_interrupt_fifo_IRQn);
        NVIC_EnableIRQ(pass_interrupt_fifo_IRQn);
    }
    else
    {
        CY_ASSERT(0);
    }

    /* Enable the FIFO0 level interrupt */
    Cy_AutAnalog_FIFO_SetInterruptMask(ANALOG_FIFO_IDX,CY_AUTANALOG_INT_FIFO_LEVEL0);
}

uint32_t mean_square(int32_t* data_arr, uint16_t arr_size)
{
    if (data_arr == NULL || arr_size == 0) return 0;

    uint64_t sum = 0;

    for (uint16_t i = 0; i < arr_size; ++i) {
        /* Upper 16 bits are zero; truncation to int16_t restores the signed value. */
        int16_t s = (int16_t)data_arr[i];
        int32_t sq = (int32_t)s * (int32_t)s;   /* exact in 32-bit */
        sum += (uint32_t)sq;                    /* widen-add to 64-bit */
    }

    /* Floor mean: sum / n */
    return (uint32_t)(sum / (uint64_t)arr_size);
}

void hs_adc_microphone_mute_process(int32_t* samples, size_t sample_count)
{
    /* If muting is disabled, just pass through */
    if (quiet_window_frames == 0) {
        return;
    }

    bool quiet = (bool)(mean_square(samples,sample_count) < noise_square);

    if (muted) {
        if (quiet) {
            /* Stay muted; clear the current frame */
            memset(samples, 0, sample_count * sizeof(int32_t));
            /* Keep quiet_count saturated at window size while muted (optional) */
            if (quiet_count < quiet_window_frames) {
                quiet_count++;
            }
        } else {
            /* Non-quiet frame: immediately unmute and pass-through */
            muted = false;
            quiet_count = 0;
            /* Do not clear samples; deliver them as-is */
        }
    } else {
        if (quiet) {
            /* Increment consecutive quiet frame count  */
            if (quiet_count + 1 >= quiet_window_frames)
            {
                /* This frame reaches X consecutive quiet frames:
                 Enter mute and clear the current frame */
                muted = true;
                /* optional saturation  */
                quiet_count = quiet_window_frames;
                memset(samples, 0, sample_count * sizeof(int32_t));
            }
            else
            {
                /* Pass-through until we reach X consecutive quiet frames  */
                quiet_count++;

            }
        }
        else
        {
            /* Noise detected; reset quiet streak and remain unmuted */
            quiet_count = 0;
        }
    }
}
/* [] END OF FILE */
