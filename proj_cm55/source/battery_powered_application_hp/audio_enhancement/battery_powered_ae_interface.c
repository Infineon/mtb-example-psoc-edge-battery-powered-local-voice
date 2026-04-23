/******************************************************************************
* File Name : battery_powered_ae_interface.c
*
* Description :
* Wrapper for Audio Enhancement (Battery Powered)
********************************************************************************
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

#include "battery_powered_ae_interface.h"
#include "app_logger.h"
#include "battery_powered_inferencing.h"

#include "FreeRTOS.h"
#include "queue.h"

#ifdef PROFILER_ENABLE
#include "cy_afe_profiler.h"
#include "cy_profiler.h"
#endif /*PROFILER_ENABLE */

/*******************************************************************************
* Macros
*******************************************************************************/

#define APP_BUF_INFO_QUEUE_COUNT                       (200u)

/*******************************************************************************
* Global Variables
*******************************************************************************/

cy_afe_t afe_handle = NULL;
QueueHandle_t app_buf_info_queue = NULL;

/*******************************************************************************
* Function Name: license_limitation_exit
********************************************************************************
* Summary:
*  This function is blocking call after Audio Front End license timeout
* 
* Parameters:
*  None
* 
* Return:
*  None
*
*******************************************************************************/
void license_limitation_exit()
{
    while(1){}
}

/*******************************************************************************
* Function Name: audio_enhancement_process_output
********************************************************************************
* Summary:
* Process AE output
*
* Parameters:
*  output_buffer - structure variable from AFE MW. 
* 
* Return:
*  None
*
*******************************************************************************/


void audio_enhancement_process_output(ae_buffer_info_t *output_buffer)
{
    cy_svc_buffer_info_t buffer_info = 0;

    if(pdTRUE == xQueueReceive(app_buf_info_queue, &buffer_info, portMAX_DELAY))
    {
        battery_powered_inference_feed((short*)output_buffer->output_buf, buffer_info);
    }
    
#if AE_APP_PROFILE
    cy_afe_profile(AFE_PROFILE_CMD_PRINT_STATS_1SEC, NULL);
    cy_afe_profile(AFE_PROFILE_CMD_RESET, NULL);
#endif /* AE_APP_PROFILE */        

}



/*******************************************************************************
* Function Name: battery_powered_ae_init
********************************************************************************
* Summary:
* Initialize Audio Enhancement for battery powered application
*
* Parameters:
*  channels - Number of channels
* 
* Return:
*  Result of AE initialization.
*
*******************************************************************************/

int battery_powered_ae_init(uint8_t channels)
{
   ae_rslt_t result = AE_RSLT_SUCCESS;


#if AE_APP_PROFILE
    cy_profiler_init();
    cy_afe_profile(AFE_PROFILE_CMD_ENABLE,NULL);
#endif /* AE_APP_PROFILE */

    app_buf_info_queue = xQueueCreate(APP_BUF_INFO_QUEUE_COUNT, sizeof(int32_t));
    if (app_buf_info_queue == NULL)
    {
        app_log_print("Failed to initialize queue to store buffer info for Audio Enhancement + SVC \r\n");
        return AE_APP_FAILURE;
    }

    result = audio_enhancement_init(channels);
    
    if (result != AE_RSLT_SUCCESS) 
    {
        app_log_print("DEEPCRAFT Audio Enhancement initialization failed \r\n");
    }

    return result;
}

/*******************************************************************************
* Function Name: battery_powered_ae_feed
********************************************************************************
* Summary:
*  Feed audio data to Audio Enhancement.
*
* Parameters:
*  audio_input: Audio Data
*  aec_buffer: AEC Buffer
*  buffer_info: Staged Voice Control buffer info
*
* Return:
*  Status of Feeding data to AE
*
*******************************************************************************/

int battery_powered_ae_feed(void* audio_input, void* aec_buffer, cy_svc_buffer_info_t buffer_info)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_svc_buffer_info_t buf_info = buffer_info;

    if (pdTRUE != xQueueSendToBack(app_buf_info_queue, &buf_info, portMAX_DELAY))
    {
        app_log_print("AFE failed to push buffer info to queue \r\n");
        return AE_APP_FAILURE;
    }
    
    result = audio_enhancement_feed_input((int16_t*)audio_input, (int16_t*)aec_buffer);
    
    if (CY_RSLT_AFE_FUNCTIONALITY_RESTRICTED == result)
    {
        app_log_print("CPU Halt: Audio Front End Restricted License Timeout - Reset the board \r\n");
        license_limitation_exit();
        return result;
    }

    if(CY_RSLT_SUCCESS != result)
    {
        app_log_print("Failed to feed data to AFE \r\n");
        return result;
    }
    return result;

    
}

/* [] END OF FILE */
