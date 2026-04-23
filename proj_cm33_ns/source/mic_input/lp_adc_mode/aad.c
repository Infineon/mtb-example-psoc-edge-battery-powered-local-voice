/*******************************************************************************
 * File Name:   aad.c
 *
 * Description: This file contains the initialization routine for the
 *              Acoustic Activity Detection (AAD) and the functions to set the 
 *              parameters for the AAD.
 *
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
#include "aad.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define AAD_WAIT_FOR_TRIGGER_STATE  3u
#define AAD_DIVISION_FACTOR         10u
#define AAD_THRESHOLD_HIGH_MASK     0xF000FFFF

/*******************************************************************************
* Global Variables
*******************************************************************************/
static aad_trigger_count_t add_internal_count = AAD_DEFAULT_TRIGGER_COUNT;

/*******************************************************************************
* Function Name: aad_init
********************************************************************************
* Summary:
* Overwrites the AAD parameters. This function is optional. 
* 
* Parameters:
*  none
*
* Return:
*  AAD_RSLT_SUCCESS if successful, otherwise AAD_RSLT_FAIL.
*
*******************************************************************************/
aad_rslt_t aad_init(void)
{
    aad_rslt_t result;
    
    result = aad_set_window_size(AAD_DEFAULT_WINDOW_SIZE);
    if (result != AAD_RSLT_SUCCESS)
    {
        return result;
    }

    result = aad_set_trigger_count(AAD_DEFAULT_TRIGGER_COUNT);
    if (result != AAD_RSLT_SUCCESS)
    {
        return result;
    }

    result = aad_set_trigger_threshold(AAD_DEFAULT_TRIGGER_THRESHOLD);
    if (result != AAD_RSLT_SUCCESS)
    {
        return result;
    }

    return result;
}

/*******************************************************************************
* Function Name: aad_restart
********************************************************************************
* Summary:
* If the AAD is disabled, it re-starts. If already enabled, don't do anything.
* 
* Note that on AAD event, the SAR ADC will be automatically started.
* You should call this function after processing the SAR ADC samples for a 
* while and determine that the environment is silent or very low noise.
* 
* Parameters:
*  none
*
* Return:
*  AAD_RSLT_SUCCESS if successful, otherwise AAD_RSLT_FAIL.
*
*******************************************************************************/
aad_rslt_t aad_restart(void)
{
    cy_stc_autanalog_state_t aa_state;

    Cy_AutAnalog_GetControllerState(&aa_state);

    if (aa_state.ac.state > AAD_WAIT_FOR_TRIGGER_STATE)
    {
        Cy_AutAnalog_FwTrigger(CY_AUTANALOG_FW_TRIGGER1);
    }

    return AAD_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: aad_set_window_size
********************************************************************************
* Summary:
* This parameter determines how long the system listens for a sound before
* triggering an AAD event. A shorter window size can lead to more rapid 
* detection but may also increase false positives.
* 
* Parameters:
*  window_size: define the window size in microseconds (use enum)
*
* Return:
*  AAD_RSLT_SUCCESS if successful, otherwise AAD_RSLT_FAIL.
*
*******************************************************************************/
aad_rslt_t aad_set_window_size(aad_window_size_t window_size)
{
    aad_rslt_t result;

    if (window_size < AAD_WINDOW_SIZE_200_US || window_size > AAD_WINDOW_SIZE_12800_US)
    {
        result = AAD_RSLT_FAIL;
    }
    else
    {
        PTC->PP[0].WINDOW = window_size;

        result = aad_set_trigger_count(add_internal_count);
    }
        
    return result;
}

/*******************************************************************************
* Function Name: aad_set_trigger_count
********************************************************************************
* Summary:
* This parameter determines how many times the analog microphone signal has to 
* be above the threshold within a certain time period (based on window size) to
* trigger an AAD event.
* 
* Parameters:
*  trigger_count: the percentage the signal should be above threshold (use enum)
*
* Return:
*  AAD_RSLT_SUCCESS if successful, otherwise AAD_RSLT_FAIL.
*
*******************************************************************************/
aad_rslt_t aad_set_trigger_count(aad_trigger_count_t trigger_count)
{
    aad_window_size_t window_size = PTC->PP[0].WINDOW;
    uint16_t window_size_samples = 0;
    uint16_t threshold;

    if (trigger_count < AAD_TRIGGER_COUNT_10_PERCENT || trigger_count > AAD_TRIGGER_COUNT_90_PERCENT)
    {
        return AAD_RSLT_FAIL;
    }

    add_internal_count = trigger_count;

    /* Determine the window size in number of samples */
    switch (window_size)
    {
        case AAD_WINDOW_SIZE_200_US: window_size_samples = 2; break;
        case AAD_WINDOW_SIZE_400_US: window_size_samples = 4; break;
        case AAD_WINDOW_SIZE_800_US: window_size_samples = 8; break;
        case AAD_WINDOW_SIZE_1600_US: window_size_samples = 16; break;
        case AAD_WINDOW_SIZE_3200_US: window_size_samples = 32; break;
        case AAD_WINDOW_SIZE_6400_US: window_size_samples = 64; break;
        case AAD_WINDOW_SIZE_12800_US: window_size_samples = 128; break;
        default: break;
    }

    threshold = (window_size_samples * trigger_count) / AAD_DIVISION_FACTOR;

    PTC->PP[0].THRESHOLD &= AAD_THRESHOLD_HIGH_MASK;
    PTC->PP[0].THRESHOLD |= _VAL2FLD(PTC_PP_THRESHOLD_HIGH, threshold);

    return AAD_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: aad_set_trigger_threshold
********************************************************************************
* Summary:
* This parameter determines how sensitive the system is to detecting sounds. 
* It defines the difference between the SAR ADC ground (0.9V) and the analog
* microphone output signal (after amplification) to be considered a trigger.
*
* Note that it assumes the analog microphone will output 0.9V in complete silent
* environment.
* 
* Parameters:
*  trigger_threshold: threshold in millivolts (use enum)
*
* Return:
*  AAD_RSLT_SUCCESS if successful, otherwise AAD_RSLT_FAIL.
*
*******************************************************************************/
aad_rslt_t aad_set_trigger_threshold(aad_trigger_threshold_t trigger_threshold)
{
    switch (trigger_threshold)
    {
        case AAD_TRIGGER_THRESHOLD_112_5_MV:
            Cy_AutAnalog_PRB_Write(0, CY_AUTANALOG_PRB_TAP_8);
            Cy_AutAnalog_PRB_Write(1, CY_AUTANALOG_PRB_TAP_6);
            break;
        case AAD_TRIGGER_THRESHOLD_225_0_MV:
            Cy_AutAnalog_PRB_Write(0, CY_AUTANALOG_PRB_TAP_9);
            Cy_AutAnalog_PRB_Write(1, CY_AUTANALOG_PRB_TAP_5);
            break;
        case AAD_TRIGGER_THRESHOLD_337_5_MV:
            Cy_AutAnalog_PRB_Write(0, CY_AUTANALOG_PRB_TAP_10);
            Cy_AutAnalog_PRB_Write(1, CY_AUTANALOG_PRB_TAP_4);
            break;
        default:
            return AAD_RSLT_FAIL;
    }

    return AAD_RSLT_SUCCESS;
}

/* [] END OF FILE */