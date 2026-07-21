/******************************************************************************
* File Name : battery_powered_svc_hp.c
*
* Description :
* Staged Voice Control Application code on CM55.
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

#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_log.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#include "FreeRTOS.h"
#include "task.h"

#include "cy_staged_voice_control.h"
#include "cy_staged_voice_control_hp.h"
#include "staged_voice_control_hp_private.h"
#include "staged_voice_control_hp_ipc.h"
#include "battery_powered_svc_hp.h"

#include "inferencing_interface.h"
#include "battery_powered_inferencing.h"
#include "battery_powered_ae_interface.h"

#include "svc_app_common.h"
#include "audio_input_configuration.h"
#include "app_logger.h"

#include "intercore_ipc.h"
#include "cy_afe_configurator_settings.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define SVC_HP_APP_CMD_Q_MSG_SIZE                   (20u)
#define SVC_HP_APP_CMD_Q_SIZE                       (400u)
#define SVC_HP_APP_TASK_Q_STACK_SIZE                (10u * 1024u)
#define SVC_HP_APP_TASK_PRIORITY                    (4u)
/* Number of samples in an audio frame */
#define SVC_FRAME_SIZE                              (160u)
#define SAMPLE_INCREMENT_STEREO                     (SVC_FRAME_SIZE*4)
#define SAMPLE_INCREMENT_MONO                       (SVC_FRAME_SIZE*2)
#define FRAME_SIZE_MS                               (10u)
/* Wait time for receiving queue message */
#define WAIT_TIME_RECV_QUEUE_MSG                    (0u)
/* Wait time for sending back data to queue */
#define WAIT_TIME_SEND_BACK_QUEUE_MSG               (0u)
/*******************************************************************************
* Global Variables
*******************************************************************************/
/* SVC HP app handle */
svc_hp_app_t svc_hp_app = {0};

/*******************************************************************************
* Function Name: svc_hp_app_process_data
********************************************************************************
* Summary:
* Process data received via SVC and feed to AFE.
*
* Parameters:
*  *frame_buffer - Pointer to audio frame data.
*  frame_count - Frame count.
*  buffer_info - Buffer information.
*
* Return:
*  None
*
*******************************************************************************/
static void svc_hp_app_process_data(
        uint8_t *frame_buffer,
        uint32_t frame_count,
        cy_svc_buffer_info_t buffer_info)
{
    char *data = NULL;
    unsigned int counter = 0;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    data = (char *)frame_buffer;
    if(NULL != data)
    {
        for (counter = 0; counter < frame_count; counter++)
        {
        result = battery_powered_ae_feed(data, NULL, buffer_info);
        if(CY_RSLT_SUCCESS != result)
        {
            app_log_print("Failed to feed audio frame to AFE \r\n");
        }
        data = data + SAMPLE_INCREMENT_MONO;
        }
    }
}


/*******************************************************************************
* Function Name: svc_hp_app_data_callback
********************************************************************************
* Summary:
* Callback for audio data on CM55.
*
* Parameters:
*  *frame_buffer - Pointer to SVC data.
*  frame_count - Frame counter.
*  buf_info - Buffer information.
*  *callback_user_argument - User argument (unused)
*
* Return:
*  Result of put queue operation.
*
*******************************************************************************/
cy_rslt_t svc_hp_app_data_callback(
        CY_SVC_DATA_T *frame_buffer,
        uint32_t frame_count,
        cy_svc_buffer_info_t buf_info,
        void *callback_user_arg)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_hp_app_cmd_q_data_t data_buf_to_app = {0};
    svc_hp_app.total_frame_received_count += frame_count;

    if (NULL != frame_buffer)
    {
        data_buf_to_app.buffer_info = buf_info;

        for (int i = 0; i < frame_count; i++)
        {
            data_buf_to_app.cmd_id = (char) SVC_CMD_HEADER;
            data_buf_to_app.data_pointer = (uint8_t*) frame_buffer;
            data_buf_to_app.frame_count = 1;

            if (pdTRUE != xQueueSendToBack(svc_hp_app.app_cmd_queue, (void*)&data_buf_to_app, WAIT_TIME_SEND_BACK_QUEUE_MSG))
            {
                app_log_print("SVC queue full with frame count = one \r\n");
            }

            frame_buffer = frame_buffer + SVC_FRAME_SIZE;
            data_buf_to_app.buffer_info = 0;
        }
    }
    else
    {
        data_buf_to_app.buffer_info = buf_info;
        data_buf_to_app.cmd_id = (char) SVC_CMD_HEADER;
        data_buf_to_app.data_pointer = (uint8_t*) frame_buffer;
        data_buf_to_app.frame_count = frame_count;

        if (pdTRUE != xQueueSendToBack(svc_hp_app.app_cmd_queue, (void*)&data_buf_to_app, WAIT_TIME_SEND_BACK_QUEUE_MSG))
        {
            app_log_print("SVC buffer full with frame buffer == NULL \r\n");
        }
    }

    return ret_val;
}

/*******************************************************************************
* Function Name: svc_hp_app_init
********************************************************************************
* Summary:
* Initialization function for staged voice application on CM55.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void svc_hp_app_init(void)
{
    cy_rslt_t  ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cy_svc_hp_config_t init_svc = {0};

    init_svc.callback_user_arg = &svc_hp_app;
    init_svc.data_callback = svc_hp_app_data_callback;

    #ifndef ENABLE_LOW_POWER_SVC
    init_svc.ipc_communication_sync_sempahore = 0;
    #endif /* ENABLE_LOW_POWER_SVC */

    init_svc.ipc_pipe_config.lp_pipe_end_point_address = APP_LP_PIPE_END_POINT_ADDRESS;
    init_svc.ipc_pipe_config.lp_pipe_end_point_client_id = APP_LP_PIPE_END_POINT_CLIENT_ID_1;
    init_svc.ipc_pipe_config.lp_pipe_end_point_intr_mask =
            CY_IPC_INTR_MASK(init_svc.ipc_pipe_config.lp_pipe_end_point_client_id);

    init_svc.ipc_pipe_config.hp_pipe_end_point_address = APP_HP_PIPE_END_POINT_ADDRESS;
    init_svc.ipc_pipe_config.hp_pipe_end_point_client_id = APP_HP_PIPE_END_POINT_CLIENT_ID_2;
    init_svc.ipc_pipe_config.hp_pipe_end_point_intr_mask =
            CY_IPC_INTR_MASK(init_svc.ipc_pipe_config.hp_pipe_end_point_client_id);


    ret_val = cy_svc_hp_init(&init_svc);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        app_log_print("SVC initialization failed in CM55 \r\n");
    }
    else
    {
        svc_hp_app.init_done = true;
    }
}


/*******************************************************************************
* Function Name: svc_hp_app_deinit
********************************************************************************
* Summary:
* Deinitialize Staged Voice Control application on CM55.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void svc_hp_app_deinit(void)
{
    (void) cy_svc_hp_deinit();
}


/*******************************************************************************
* Function Name: svc_hp_app_print_statistics
********************************************************************************
* Summary:
* Print statistics of SVC application on CM55.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void svc_hp_app_print_statistics()
{
    size_t num_waiting = 0;

    if (NULL != svc_hp_app.app_cmd_queue)
    {
        num_waiting = uxQueueMessagesWaiting(svc_hp_app.app_cmd_queue);
        if(0 != num_waiting)
            app_svc_log_hp("HP APP Pending msg:%d", num_waiting);
    }
    return;
}


/*******************************************************************************
* Function Name: svc_hp_app_process_app_cmd_q
********************************************************************************
* Summary:
* Process command queue.
*
* Parameters:
*  None
*
* Return:
*  Result of get queue operation.
*
*******************************************************************************/
static cy_rslt_t svc_hp_app_process_app_cmd_q()
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    uint8_t data[SVC_HP_APP_CMD_Q_MSG_SIZE] = {0};

    if(pdTRUE != xQueueReceive(svc_hp_app.app_cmd_queue, (void*)&data[0], portMAX_DELAY))
    {
        app_svc_log_hp("Get queue failed for SVC application");
        return ret_val;
    }
    if (SVC_CMD_HEADER == data[0])
    {
        svc_hp_app_cmd_q_data_t *data_buf_to_app = (svc_hp_app_cmd_q_data_t*) data;
        (void) svc_hp_app_process_data(data_buf_to_app->data_pointer,  data_buf_to_app->frame_count,
                data_buf_to_app->buffer_info);

    }
    return CY_RSLT_SUCCESS;
}


/*******************************************************************************
* Function Name: app_svc_hp_clear_queue
********************************************************************************
* Summary:
* Clear the SVC queue.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_svc_hp_clear_queue(void)
{
    size_t num_waiting = 0;
    int index =0;
    uint8_t data[SVC_HP_APP_CMD_Q_MSG_SIZE] = {0};

    if (NULL != svc_hp_app.app_cmd_queue)
    {
        num_waiting = uxQueueMessagesWaiting(svc_hp_app.app_cmd_queue);
        app_svc_log_hp("Number of elements in the queue %d",num_waiting);
        for (index=0; index < num_waiting; index++)
        {
            if(pdTRUE != xQueueReceive(svc_hp_app.app_cmd_queue, (void*)&data[0], WAIT_TIME_RECV_QUEUE_MSG))
            {
                app_svc_log_hp("Queue cleared");
                break;
            }
        }
    }
    num_waiting = uxQueueMessagesWaiting(svc_hp_app.app_cmd_queue);
    app_svc_log_hp("Number of elements in the queue %d",num_waiting);
}

/*******************************************************************************
* Function Name: svc_hp_app_thread
********************************************************************************
* Summary:
* Task for handling commands sent via SVC.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void svc_hp_app_thread()
{

    for (;;)
    {
        if (true == svc_hp_app.init_done)
        {
            svc_hp_app_process_app_cmd_q();
        }
    }

}


/*******************************************************************************
* Function Name: app_svc_hp_app_init
********************************************************************************
* Summary:
* Initialize queues and Task for SVC application on CM55.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_svc_hp_app_init(void)
{
    BaseType_t rtos_task_status;

    svc_hp_app_init();

    svc_hp_app.app_cmd_queue = xQueueCreate(SVC_HP_APP_CMD_Q_SIZE, SVC_HP_APP_CMD_Q_MSG_SIZE);
    if (svc_hp_app.app_cmd_queue == NULL)
    {
        app_log_print("Unable to create queue for SVC application \r\n");
        CY_ASSERT(0);
    }

    /* Start SVC data process thread */

    rtos_task_status = xTaskCreate(svc_hp_app_thread, "svc_hp_app_thread",
                        SVC_HP_APP_TASK_Q_STACK_SIZE, NULL, SVC_HP_APP_TASK_PRIORITY,
                        &svc_hp_app.thread);

    if (pdPASS != rtos_task_status)
    {
        app_log_print("Unable to create SVC thread \r\n");
        CY_ASSERT(0);
    }
    return;
}

/*******************************************************************************
* Function Name: usb_mic_data_feed
********************************************************************************
* Summary:
* USB Feed on CM55 (Not supported)
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void usb_mic_data_feed(int16_t *audio_data)
{

}


/* [] END OF FILE */

