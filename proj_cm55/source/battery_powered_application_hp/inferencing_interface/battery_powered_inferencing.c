/******************************************************************************
* File Name : battery_powered_inferencing.c
*
* Description :
* Source file for Inferencing and post processing on CM55 . (battery use-case)
********************************************************************************
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
#include "cyabs_rtos_impl.h"
#include "cycfg.h"
#include "cy_log.h"
#include "cyabs_rtos.h"
#include "cybsp_types.h"
#include "battery_powered_svc_hp.h"
#include "cy_staged_voice_control.h"
#include "system_edge.h"
#include "battery_powered_inferencing.h"

#include "app_logger.h"

#include "ppudbg.h"
/*******************************************************************************
* Macros
*******************************************************************************/
#define MAX_SIMULATE_POST_WWD_REQ_FRAME_COUNT           (150)
#define MAX_AUD_SAMPLES_PER_CHANNEL_FOR_10MS_DATA       (160)
#define STREAMING_STATE_TIMEOUT_FRAME_COUNTER           (1000)
#define WWD_STAGE                                       (1)
#define ASR_STAGE                                       (2)

#define INFERENCING_LOG_CONTROL                         (0)

#if INFERENCING_LOG_CONTROL
#define hp_app_infer_log(format,...)                    printf(format "\r\n",##__VA_ARGS__);
#else
#define hp_app_infer_log(format,...)
#endif /* INFERENCING_LOG_CONTROL */

#define INFERENCING_TASK_PRIORITY                       (3)
#define INFER_QUEUE_SIZE                                (320)
#define INFER_QUEUE_ELEMENTS                            (30)
#define INFER_FRAME_SIZE                                (320)
#define INFERENCING_STACK_SIZE                          (1024*10)

#define HPWWD_POST_WWD_FRAME_CNT                        (50U)

#define CM55_NON_TICKLESS_DEEPSLEEP_FLAG_ADDRESS        (0x240FEFF4)

/****************************************************************************
* Global variables
*****************************************************************************/

bool enable_stage1_timeout = true;
bool enable_stage2_timeout = true;

cy_svc_stage_t g_cur_stage = CY_SVC_STAGE_INVALID;

unsigned int frame_counter_on_streaming_state = 0;
bool enabled_streaming_state = false;
extern QueueHandle_t map_id_q;

TaskHandle_t rtos_infer_task;

extern QueueHandle_t app_buf_info_queue;

volatile uint8_t *cm55_non_tickless_deepsleep_flag = (uint8_t *)CM55_NON_TICKLESS_DEEPSLEEP_FLAG_ADDRESS; // Define the pointer to the memory address
/****************************************************************************
* Functions Prototypes
*****************************************************************************/
extern void svc_hp_app_print_statistics();

/*******************************************************************************
* Function Name: app_request_post_wwd_data
********************************************************************************
* Summary:
* Request post WWD data.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_request_post_wwd_data( )
{
    cy_svc_hp_set_state_hpwwd_det_in_prog_info_t hpwwd_det_state_info = { 0 };

    hpwwd_det_state_info.action = CY_SVC_SEND_POST_WWD_BUFFER;
    hpwwd_det_state_info.post_wwd_frame_count = HPWWD_POST_WWD_FRAME_CNT;

    app_log_print("Requesting for more data from CM33 for Wake Word inferencing\r\n");
    mtb_hal_syspm_lock_deepsleep();
    cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_DETECT_IN_PROGRESS,
            &hpwwd_det_state_info);
    mtb_hal_syspm_unlock_deepsleep();

    return;
}

/*******************************************************************************
* Function Name: app_inferencing_callback
********************************************************************************
* Summary:
* Callback function for inferencing library.
* This function is used to print outputs from inferencing library API.
*
* Parameters:
*  function - Function from which callback is called.
*  message - Message to print.
*  parameter - Parameter passed in callback from inferencing library API.
*
* Return:
*  None
*
*******************************************************************************/

void app_inferencing_callback(const char *function, char *message, char *parameter)
{
    if (!strcmp(message,"Timeout"))
    {
        app_log_print("Wake Word not detected in High Performance Core CM55-Time out \r\n");
        mtb_hal_syspm_lock_deepsleep();
        cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED, NULL);
        mtb_hal_syspm_unlock_deepsleep();
        NVIC_DisableIRQ((IRQn_Type)m33syscpuss_interrupts_ipc_dpslp_6_IRQn);
        NVIC_DisableIRQ((IRQn_Type)srss_interrupt_mcwdt_1_IRQn);
        NVIC_DisableIRQ((IRQn_Type)mxu55_interrupt_npu_IRQn);
        *cm55_non_tickless_deepsleep_flag = 1;
        Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
    return;
}

/*******************************************************************************
* Function Name: battery_powered_post_processing
********************************************************************************
* Summary:
* Post Processing API for Inference outputs.
* Based on the inferred map id, the necessary action will be taken.
*
* Parameters:
*  map_id - Inferred Map ID.
*
* Return:
*  None
*
*******************************************************************************/

void battery_powered_post_processing(int map_id)
 {
    BaseType_t ret = pdTRUE;
    /* Switch the model groups before continuing the inferencing */
    switch (map_id)
    {
        case OK_INFINEON_CMD_ID:
        {
            mtb_hal_syspm_lock_deepsleep();
            app_log_print("WW detected: %d\n\r",map_id);
            cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED, NULL);
            mtb_hal_syspm_unlock_deepsleep();
            break;
        }
        case PLAY_MUSIC_CMD_ID:
        case STOP_MUSIC_CMD_ID:
        case NEXT_TRACK_CMD_ID:
        case PREVIOUS_TRACK_CMD_ID:
        case INCREASE_VOL_CMD_ID:
        case DECREASE_VOL_CMD_ID:
        case PAUSE_MUSIC_CMD_ID:
        {
            mtb_hal_syspm_lock_deepsleep();
            cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_DETECTED, NULL);
            cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_PROCESSING_COMPLETED, NULL);
            if(PLAY_MUSIC_CMD_ID == map_id)
            {
                app_log_print("Intent: Play Music\n\r");
            }
            else if (STOP_MUSIC_CMD_ID == map_id) 
            {
                app_log_print("Intent: Stop Music\n\r");
            }
            else if (NEXT_TRACK_CMD_ID == map_id) 
            {
                app_log_print("Intent: Next Track\n\r");
            }
            else if (PREVIOUS_TRACK_CMD_ID == map_id) 
            {
                app_log_print("Intent: Previous Track\n\r");
            }
            else if (INCREASE_VOL_CMD_ID == map_id) 
            {
                app_log_print("Intent: Increase Volume\n\r");
            }
            else if (DECREASE_VOL_CMD_ID == map_id) 
            {
                app_log_print("Intent: Decrease Volume\n\r");
            }
            else if (PAUSE_MUSIC_CMD_ID == map_id) 
            {
                app_log_print("Intent: Pause Music\n\r");
            }
            mtb_hal_syspm_unlock_deepsleep();
            NVIC_DisableIRQ((IRQn_Type)m33syscpuss_interrupts_ipc_dpslp_6_IRQn);
            NVIC_DisableIRQ((IRQn_Type)srss_interrupt_mcwdt_1_IRQn);
            NVIC_DisableIRQ((IRQn_Type)mxu55_interrupt_npu_IRQn);
            *cm55_non_tickless_deepsleep_flag = 1;
            Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
            break;
        }
        case INFERENCING_TIMEOUT:
        default:
        {
            mtb_hal_syspm_lock_deepsleep();
            cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_NOT_DETECTED, NULL);
            mtb_hal_syspm_unlock_deepsleep();
            app_log_print("Timeout for command: %d",map_id);
            NVIC_DisableIRQ((IRQn_Type)m33syscpuss_interrupts_ipc_dpslp_6_IRQn);
            NVIC_DisableIRQ((IRQn_Type)srss_interrupt_mcwdt_1_IRQn);
            NVIC_DisableIRQ((IRQn_Type)mxu55_interrupt_npu_IRQn);
            *cm55_non_tickless_deepsleep_flag = 1;
            Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
        }
    }
}

/*******************************************************************************
* Function Name: get_cur_svc_stage_and_force_model_stages
********************************************************************************
* Summary:
* Get current stage in SVC and change.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void get_cur_svc_stage_and_force_model_stages(void)
{
    cy_svc_stage_t svc_stage = CY_SVC_STAGE_INVALID;

    cy_svc_hp_get_current_stage(&svc_stage);

    if(g_cur_stage != svc_stage)
    {
        g_cur_stage = svc_stage;
    }


    if (CY_SVC_STAGE_WAITING_FOR_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTION == svc_stage)
    {
        if (WWD_STAGE != get_current_inference_stage())
        {
            switch_to_wwd_inferencing();
        }
    }
    else if (CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT == svc_stage)
    {
        if (ASR_STAGE != get_current_inference_stage())
        {
            switch_to_asr_inferencing();
        }
    }

    return;
}
/*******************************************************************************
* Function Name: battery_powered_inference_feed
********************************************************************************
* Summary:
* Feeds audio frames from audio pipeline to inferencing engine.
*
* Parameters:
*  stereo - Audio data.
*  buffer_info - Buffer information.
* 
* Return:
*  None
*
*******************************************************************************/

void battery_powered_inference_feed(short *mono, cy_svc_buffer_info_t buffer_info)
{
    if(0 != buffer_info)
    {
        inference_reset_wwd_feed_timeout_counter();
    }
    get_cur_svc_stage_and_force_model_stages();
    inference_processing((short *)mono, INFER_FRAME_SIZE/2);
    if (true == enable_stage1_timeout && true == enable_stage2_timeout)
    {
        inference_timeout_check();
    }
    return;
}


/*******************************************************************************
* Function Name: battery_powered_inference_engine_init
********************************************************************************
* Summary:
* Initializes inferencing library.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void battery_powered_inference_engine_init()
{
    
    BaseType_t rtos_task_status;

    /* Initialize ASR  */
    if (!inferencing_engine_init(app_inferencing_callback)) {
        app_log_print("Error in Inference engine initialization \r\n");
    }

    hp_app_infer_log("Inference engine initialization completed. ");

    return;
}

/*******************************************************************************
* Function Name: battery_powered_inference_engine_deinit
********************************************************************************
* Summary:
* De-initializes inferencing library.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void battery_powered_inference_engine_deinit()
{
    inferencing_engine_deinit();
    return;
}

/* [] END OF FILE */
