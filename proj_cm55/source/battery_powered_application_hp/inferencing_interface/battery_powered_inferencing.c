/******************************************************************************
* File Name : battery_powered_inferencing.c
*
* Description :
* Source file for Inferencing and post processing on CM55 . (battery use-case)
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
#include "cycfg.h"
#include "cy_log.h"
#include "cybsp_types.h"
#include "battery_powered_svc_hp.h"
#include "cy_staged_voice_control.h"
#include "system_edge.h"
#include "battery_powered_inferencing.h"
#include "va_inferencing.h"

#include "app_logger.h"

#include "ppudbg.h"

#ifdef PROFILER_ENABLE
#include "cy_afe_profiler.h"
#include "cy_profiler.h"
#endif /*PROFILER_ENABLE */
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

#define INFER_FRAME_SIZE                                (320)

#define HPWWD_POST_WWD_FRAME_CNT                        (50U)

#define CM55_NON_TICKLESS_DEEPSLEEP_FLAG_ADDRESS        (0x240FEFF4)

/****************************************************************************
* Global variables
*****************************************************************************/
volatile bool ptt_flag = false;
volatile bool stream_flag = false;

bool enable_stage1_timeout = true;
bool enable_stage2_timeout = true;

cy_svc_stage_t g_cur_stage = CY_SVC_STAGE_INVALID;

unsigned int frame_counter_on_streaming_state = 0;
bool enabled_streaming_state = false;

#if defined(PD1_POWER_CYCLE)
volatile uint8_t *cm55_non_tickless_deepsleep_flag = (uint8_t *)CM55_NON_TICKLESS_DEEPSLEEP_FLAG_ADDRESS; // Define the pointer to the memory address
#endif /* PD1_POWER_CYCLE */
/* Shared streaming mode flag address - accessible by both CM33 and CM55 */
#define CM55_STREAMING_MODE_FLAG_ADDRESS                (0x240FEFF5)
/* Pointer to streaming mode flag in shared memory (0/1): when true, CM55 stays active for
 * continuous HP WWD + ASR cycles. When false, normal behavior with deepsleep on timeouts. */
volatile uint8_t *cm55_streaming_mode_active = (uint8_t *)CM55_STREAMING_MODE_FLAG_ADDRESS;
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
        if (*cm55_streaming_mode_active != 0u)
        {
            /* Streaming: HPWWD timed out - restart WWD inference for next cycle.
             * Don't notify SVC-LP (it is kept in ASR stage to maintain audio pipeline). */
            app_log_print("Streaming: HPWWD timeout, restarting WWD\r\n");
            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
            Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
            switch_to_wwd_inferencing();
            return;
        }

        mtb_hal_syspm_lock_deepsleep();
        cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED, NULL);
        mtb_hal_syspm_unlock_deepsleep();
#ifdef PD1_POWER_CYCLE
            NVIC_DisableIRQ((IRQn_Type)m33syscpuss_interrupts_ipc_dpslp_6_IRQn);
            NVIC_DisableIRQ((IRQn_Type)srss_interrupt_mcwdt_1_IRQn);
            NVIC_DisableIRQ((IRQn_Type)mxu55_interrupt_npu_IRQn);
            *cm55_non_tickless_deepsleep_flag = 1;
#endif /* PD1_POWER_CYCLE */   
#if AE_APP_PROFILE
            cy_profiler_deinit();
#endif  
#ifdef PD1_POWER_CYCLE
            Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
#endif /* PD1_POWER_CYCLE */    
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
    /* Switch the model groups before continuing the inferencing */
    switch (map_id)
    {
        case OK_INFINEON_CMD_ID:
        {
            if (*cm55_streaming_mode_active != 0u)
            {
                /* Streaming: HPWWD detected - switch to ASR inference internally.
                 * Don't notify SVC (it stays in ASR stage for audio pipeline). */
                Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
                Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_ON);
                app_log_print("Streaming: Wake word detected, switching to ASR\r\n");
                switch_to_asr_inferencing();
            }
            else
            {
                mtb_hal_syspm_lock_deepsleep();
                cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED, NULL);
                mtb_hal_syspm_unlock_deepsleep();
            }
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
            if (ptt_flag)
            {
                ptt_flag = false;
            }

            if (*cm55_streaming_mode_active != 0u)
            {
                /* Streaming: Do NOT send ASR_DETECTED or ASR_PROCESSING_COMPLETED
                 * to SVC. Sending ASR_DETECTED would transition SVC to
                 * ASR_PROCESSING_QUERY_DETECTED stage which stops the audio
                 * pipeline - CM55 would never receive frames again.
                 * Instead, just log the command and restart HPWWD internally.
                 * SVC stays in WAITING_FOR_ASR_REQUEST_DETECT & audio keeps flowing. */
                app_log_print("ASR command detected\r\n");
                app_log_print("Command: %s\r\n", command_text);
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
                app_log_print("Streaming: ASR complete, restarting HPWWD\r\n");
                /* ASR detected: show GREEN+BLUE. They will be turned OFF
                 * when the next HPWWD cycle starts (detection or timeout). */
                Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_ON);
                switch_to_wwd_inferencing();
            }
            else
            {
                /* Normal flow: send ASR_DETECTED, print intents, then send
                 * ASR_PROCESSING_COMPLETED. The prints between the two IPC
                 * calls give SVC-LP time to process the first message. */
                mtb_hal_syspm_lock_deepsleep();
                cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_DETECTED, NULL);
                app_log_print("ASR command detected\r\n");
                app_log_print("Command: %s\r\n", command_text);
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
                cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_PROCESSING_COMPLETED, NULL);
                
                mtb_hal_syspm_unlock_deepsleep();
#ifdef PD1_POWER_CYCLE
            NVIC_DisableIRQ((IRQn_Type)m33syscpuss_interrupts_ipc_dpslp_6_IRQn);
            NVIC_DisableIRQ((IRQn_Type)srss_interrupt_mcwdt_1_IRQn);
            NVIC_DisableIRQ((IRQn_Type)mxu55_interrupt_npu_IRQn);
            *cm55_non_tickless_deepsleep_flag = 1;
#endif /* PD1_POWER_CYCLE */   
#if AE_APP_PROFILE
            cy_profiler_deinit();
#endif  
#ifdef PD1_POWER_CYCLE
            Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
#endif /* PD1_POWER_CYCLE */        
            }
            break;
        }
        case INFERENCING_TIMEOUT:
        default:
        {
            if (ptt_flag)
            {
                ptt_flag = false;
            }
            if (*cm55_streaming_mode_active != 0u)
            {
                /* Streaming: ASR timed out, rollback to HPWWD.
                 * Don't notify SVC (it stays in ASR stage for audio pipeline). */
                app_log_print("Streaming: ASR timeout, restarting HPWWD\r\n");
                Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
                Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
                switch_to_wwd_inferencing();
            }
            else
            {
            mtb_hal_syspm_lock_deepsleep();
            if (stream_flag==true) {
                app_log_print("Exiting Streaming Mode from CM55 \r\n");
                stream_flag=false;
                cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_NOT_DETECTED, NULL);
                cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_PROCESSING_COMPLETED, NULL);
            } else {
                app_log_print("ASR not detected \r\n");
                cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_NOT_DETECTED, NULL);
                cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_PROCESSING_COMPLETED, NULL);
            }

            mtb_hal_syspm_unlock_deepsleep();

#ifdef PD1_POWER_CYCLE
            NVIC_DisableIRQ((IRQn_Type)m33syscpuss_interrupts_ipc_dpslp_6_IRQn);
            NVIC_DisableIRQ((IRQn_Type)srss_interrupt_mcwdt_1_IRQn);
            NVIC_DisableIRQ((IRQn_Type)mxu55_interrupt_npu_IRQn);
            *cm55_non_tickless_deepsleep_flag = 1;
#endif /* PD1_POWER_CYCLE  */            
#if AE_APP_PROFILE
            cy_profiler_deinit();
#endif
#ifdef PD1_POWER_CYCLE
            Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
#endif /* PD1_POWER_CYCLE */            
            }
        }
        break;
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

    if (*cm55_streaming_mode_active != 0u)
    {
        if (get_current_inference_stage() != WWD_STAGE &&
            get_current_inference_stage() != ASR_STAGE)
        {
            switch_to_wwd_inferencing();
        }
        return;
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
        if (ASR_STAGE != get_current_inference_stage() && ptt_flag ==false)
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
        voice_assistant_reset();
        inference_reset_wwd_feed_timeout_counter();
    }
    if(CY_SVC_BUF_INFO_ASR_FORCE_START & buffer_info)
    {
        /* PTT (ASR force-start) should be processed in all modes - streaming or not.
         * In streaming mode, the timeout behavior differs (restarts HPWWD instead of
         * deep sleep), but PTT should still trigger ASR mode. */
         
         /* Both PTT and Streaming use the same SVC flag so setting the flow accordingly.*/
        switch_to_asr_inferencing();
        if (*cm55_streaming_mode_active == 0u)
        {
            ptt_flag = true;
        }else if (*cm55_streaming_mode_active == 1u)
        {
            stream_flag = true;
        }
        
    }
    get_cur_svc_stage_and_force_model_stages();
    inference_processing((short *)mono, INFER_FRAME_SIZE/2);
 /* Check for timeouts in LPWWD reverification and Streaming states*/   
    if (*cm55_streaming_mode_active == 0u)
    {
        inference_timeout_check();
        if (stream_flag==true)
        {
            battery_powered_post_processing(INFERENCING_TIMEOUT);
        }
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
