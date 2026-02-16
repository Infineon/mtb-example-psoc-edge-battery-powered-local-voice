/******************************************************************************
* File Name : inferencing_interface.c
*
* Description :
* Source file for Inference processing on CM55.
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
#include "inferencing_interface.h"
#include "app_logger.h"
#include "mtb_hal.h"

#ifdef VA_INFERENCING
#include "va_inferencing.h"
#endif /* VA_INFERENCING */


/*******************************************************************************
* Macros
*******************************************************************************/
#if defined (VA_INFERENCING)
#define VA_DIGITAL_GAIN                   (2)

#define VA_AUDIO_SAMPLES                  (160)
#endif /* VA_INFERENCING */

#define WAKE_WORD_INFER_STAGE             (1)
#define ASR_INFER_STAGE                   (2)
/*******************************************************************************
* Global Variables
*******************************************************************************/
#if defined (VA_INFERENCING)
short boosted_data[VA_AUDIO_SAMPLES] __attribute__((section(".dtcm_data"), aligned(4))) = {0};
#endif /*VA_INFERENCING*/

#if defined (VA_INFERENCING)
short inferencing_stage = 0;
int inferencing_frame_count_ww = 0;
int inferencing_frame_count_asr = 0;
inferencing_callback g_callback_function;
extern void app_inferencing_callback(const char *function, char *message, char *parameter);
#endif /* VA_INFERENCING */


/*******************************************************************************
* Function Name: inferencing_engine_init
********************************************************************************
* Summary:
*  Initializes the inferencing engine.
*
* Parameters:
*  callback_function: Inferencing callback function
*
* Return:
*  status of inference engine initialization
*
*******************************************************************************/
int inferencing_engine_init(inferencing_callback callback_function)
{

#ifdef VA_INFERENCING
    va_rslt_t status = 0;
    status = voice_assistant_init(VA_MODE_WW_SINGLE_CMD);
    if (status != VA_RSLT_SUCCESS)
    {
        app_log_print("Voice Assistant Init failed \r\n");
    }
    g_callback_function=callback_function;
#endif /* VA_INFERENCING */
    return 1;

}

/*******************************************************************************
* Function Name: inferencing_engine_deinit
********************************************************************************
* Summary:
*  De-initializes the inferencing engine.
*
* Parameters:
*  None
*
* Return:
*  None  
*
*******************************************************************************/
void inferencing_engine_deinit()
{

    return;

}


/*******************************************************************************
* Function Name: inference_processing
********************************************************************************
* Summary:
*    Perform inferencing on the data.
*
* Parameters:
*  Data : Audio Data for processing
*  num_samples: Number of damples
*
* Return:
*  None
*
*******************************************************************************/
void inference_processing(short *data, int num_samples)
{

#ifdef VA_INFERENCING
    int index = 0;
    for (index = 0; index < VA_AUDIO_SAMPLES; index++)
    {
        boosted_data[index] = data[index]*VA_DIGITAL_GAIN;
    }
    inferencing_frame_count_ww++;
    inferencing_frame_count_asr++;
    run_voice_assistant_process(boosted_data);
#endif /* VA_INFERENCING*/
    return;

}

/*******************************************************************************
* Function Name: is_inference_license_valid
********************************************************************************
* Summary:
*  Check license of inference library.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

int is_inference_license_valid()
{
    return 1;
}


/*******************************************************************************
* Function Name: get_current_inference_stage
********************************************************************************
* Summary:
*  Get the current inferencing stage (WWD or ASR).
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int get_current_inference_stage()
{

#if defined (VA_INFERENCING)
    return inferencing_stage;
#endif /*VA_INFERENCING */


}

/*******************************************************************************
* Function Name: set_current_inference_stage
********************************************************************************
* Summary:
*  Set the current inferencing stage (WWD or ASR).
*
* Parameters:
*  stage: Inferencing stage
*
* Return:
*  Status of setting inference stage
*
*******************************************************************************/
int set_current_inference_stage(int stage)
{

#if defined (VA_INFERENCING)
    inferencing_stage = stage;
    return 0;
#endif /*VA_INFERENCING */

}


/*******************************************************************************
* Function Name: switch_to_wwd_inferencing
********************************************************************************
* Summary:
*    Switch the counters for WWD inferencing.
*
* Parameters:
*  None
*
* Return:
*  status of switching to wake word detection inferencing
*
*******************************************************************************/
int switch_to_wwd_inferencing()
{

#if defined (VA_INFERENCING)
    inferencing_frame_count_ww=0;
    set_current_inference_stage(WAKE_WORD_INFER_STAGE);
#endif /*VA_INFERENCING */
    return 0;

}

/*******************************************************************************
* Function Name: switch_to_asr_inferencing
********************************************************************************
* Summary:
*    Switch the counters for ASR inferencing.
*
* Parameters:
*  None
*
* Return:
*  status of switching to ASR inferencing
*
*******************************************************************************/
int switch_to_asr_inferencing()
{

#if defined (VA_INFERENCING)
    inferencing_frame_count_asr=0;
    set_current_inference_stage(ASR_INFER_STAGE);
#endif /*VA_INFERENCING */
    return 0;

}

/*******************************************************************************
* Function Name: switch_to_wwd_inferencing_streaming
********************************************************************************
* Summary:
*    Switch the counters for WWD in streaming state.
*
* Parameters:
*  None
*
* Return:
*  status of switching to wake word detection inferencing streaming
*
*******************************************************************************/
int switch_to_wwd_inferencing_streaming()
{

#if defined (VA_INFERENCING)
    inferencing_frame_count_ww=0;
    set_current_inference_stage(WAKE_WORD_INFER_STAGE);
#endif /*VA_INFERENCING */
    return 0;

}

/*******************************************************************************
* Function Name: switch_to_asr_inferencing_streaming
********************************************************************************
* Summary:
*    Switch the counters for ASR in streaming state.
*
* Parameters:
*  None
*
* Return:
*  status of switching to ASR inferencing streaming
*
*******************************************************************************/
int switch_to_asr_inferencing_streaming()
{

#if defined (VA_INFERENCING)
    inferencing_frame_count_asr=0;
    set_current_inference_stage(ASR_INFER_STAGE);
#endif /*VA_INFERENCING */
    return 0;
}

/*******************************************************************************
* Function Name: get_inferencing_frame_counters
********************************************************************************
* Summary:
*    Get the frame count that is being sent for inferencing.
*
* Parameters:
*  *current_feed_counter_stage1: Pointer to feed counter stage 1
*  *current_feed_counter_stage2: Pointer to feed counter stage 2
*
* Return:
*  None
*
*******************************************************************************/

void get_inferencing_frame_counters(unsigned int *current_feed_counter_stage1,unsigned int *current_feed_counter_stage2 )
{

#if defined (VA_INFERENCING)
    *current_feed_counter_stage1=inferencing_frame_count_ww;
    *current_feed_counter_stage2=inferencing_frame_count_asr;
    return;
#endif /*VA_INFERENCING */

}

/*******************************************************************************
* Function Name: inference_reset_wwd_feed_timeout_counter
********************************************************************************
* Summary:
*  Reset frame counter.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void inference_reset_wwd_feed_timeout_counter()
{

#if defined (VA_INFERENCING)
    inferencing_frame_count_ww=0;
    inferencing_frame_count_asr=0;
    return;
#endif /*VA_INFERENCING */

}

/*******************************************************************************
* Function Name: inference_timeout_check
********************************************************************************
* Summary:
*  Check for timeout.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void inference_timeout_check()
{

#if defined (VA_INFERENCING)
    if (get_current_inference_stage() == WAKE_WORD_INFER_STAGE)
    {
        if (inferencing_frame_count_ww==WAKEWORD_FRAME_CNT)
        {
            app_request_post_wwd_data();
        }
        else if (inferencing_frame_count_ww == WAKEWORD_INFERENCE_TIMEOUT_FRAMES_CNT)
        {
            app_inferencing_callback(__func__,"Timeout",NULL);
        }
    }
    else if(get_current_inference_stage() == ASR_INFER_STAGE)
    {
        if (inferencing_frame_count_asr == ASR_APP_TIMEOUT) 
        {
            battery_powered_post_processing(INFERENCING_TIMEOUT);
        }
    }
    return;
#endif /*VA_INFERENCING */
}
/* [] END OF FILE */
