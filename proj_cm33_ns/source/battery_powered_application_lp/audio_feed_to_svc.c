/******************************************************************************
* File Name : audio_feed_to_svc.c
*
* Description :
* Code for audio feed into Staged Voice Control middleware
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

#include "audio_feed_to_svc.h"

#include "audio_conv_utils.h"
#include "cy_scb_uart.h"
#include "user_interaction.h"
#include "cy_staged_voice_control.h"
#include "cy_staged_voice_control_common.h"
#include "cycfg_pins.h"
#include "cybsp_types.h"

#include "app_logger.h"
#include "audio_input_configuration.h"
#include "intercore_ipc.h"

#include "hs_adc_amic_interface.h"
#include "lp_adc_amic_interface.h"
#include "nsc_func.h"
#include "cy_syspm_pdcm.h"
#include "cy_smif_memslot.h"

#include "ppudbg.h"
#include "aad.h"

#include "mtb_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef USB_FEED_TO_SVC
#include "audio_usb_send_utils.h"
#include "usb_audio_interface.h"
#endif
/*******************************************************************************
* Macros
*******************************************************************************/

#define MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES     (160*2*2)
#define MONO_10MS_AUDIO_BYTES                           (160)

#define APP_INTER_CORE_BUF_SIZE                         (128000)

#define APP_CFG_MAX_SHARED_BUF_SIZE                     (APP_INTER_CORE_BUF_SIZE)

#define LPWWD_DETECT_FRAME                              (200u)
#define SOD_HIT_DELAY                                   (600u)
#define SOD_SENSITIVITY                                 (16384u)
#define STREAMING_TOGGLE_GUARD_MS                       (800u)

#ifdef LOW_POWER_MODE
#define CM55_EXT_FLASH_START                            (void*)(0x60580400)
#define CM55_SOCMEM_START                               (void*)(CY_CM55_APP_BOOT_ADDR)
#define CM55_IMAGE_SIZE                                 (1940000)

#define SMIF_INIT_TIMEOUT                               (10000UL)
/* The timeout value in microsecond used to wait for core to be booted */
#define CM55_BOOT_WAIT_TIME_USEC                        (10U)
#endif /* LOW_POWER_MODE */

#define CM55_DEEPSLEEP_FLAG_ADDRESS                     (0x240FEFF4)
#ifdef HP_INFERENCING_ENABLE
#define SVC_LP_SECURE_STACK_SIZE                        (2*configMINIMAL_SECURE_STACK_SIZE)
#endif /* HP_INFERENCING_ENABLE */
/*******************************************************************************
* Global Variables
*******************************************************************************/
int8_t noninterleave_audio[MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES] = { 0 };


int inf_count=0;
#ifdef ENABLE_LOW_POWER_SVC
CY_SECTION(".cy_sram_shared_non_cached") unsigned char app_data_shared_socmem[APP_CFG_MAX_SHARED_BUF_SIZE];
/* Low power changes */
#ifdef HP_INFERENCING_ENABLE
static cy_stc_smif_context_t SMIFContext;
typedef enum
{
    QSPI = 0,
    OSPI = 1
}en_ext_mem_t;
#endif
#else
unsigned char app_data_shared_socmem[APP_CFG_MAX_SHARED_BUF_SIZE] __attribute__((section(".cy_shared_socmem")));
#endif

volatile bool button_pressed = false;
/* Shared streaming mode flag address - accessible by both CM33 and CM55 */
#define CM55_STREAMING_MODE_FLAG_ADDRESS                (0x240FEFF5)
/* Pointer to streaming mode flag in shared memory (0/1): true when LP WWD is bypassed
 * and CM55 is handling HP WWD + ASR. PTT (BTN1) is disabled while this flag is set. */
volatile uint8_t *cm55_streaming_mode_active = (uint8_t *)CM55_STREAMING_MODE_FLAG_ADDRESS;
volatile uint8_t *cm55_deepsleep_flag = (uint8_t *)CM55_DEEPSLEEP_FLAG_ADDRESS; // Define the pointer to the memory address


extern volatile uint8_t disable_cm55_n_turn_off_pd1_flag;


extern volatile uint8_t asr_detected_indication_flag;
extern TaskHandle_t pd1_disable_task_handle;
extern TaskHandle_t led_task_handle;
/*******************************************************************************
* Functions Prototypes
*******************************************************************************/

extern const char* svc_lp_events_to_printable_string(cy_svc_event_t event);
extern void app_ipc_init_cm33(void);

#ifdef ENABLE_LOW_POWER_SVC
#ifdef HP_INFERENCING_ENABLE
static cy_en_smif_status_t external_memory_init(en_ext_mem_t interface);
#endif
#endif /* ENABLE_LOW_POWER_SVC */

#ifdef HP_INFERENCING_ENABLE
#ifndef USB_FEED_TO_SVC
/* Forward declarations for streaming mode functions */
static void app_svc_streaming_toggle_cb(void);
static void app_streaming_toggle_cm33_init(cb_user_action user_action_cb);
static TickType_t s_last_streaming_toggle_tick = 0u;
#endif
#endif /* HP_INFERENCING_ENABLE */
/*******************************************************************************
* Function Name: audio_mic_data_feed_cm33
********************************************************************************
* Summary:
* API that receives audio data from mic (PDM) and sends it to SVC.
*
* Parameters:
*  *audio_data - Audio data
*
* Return:
*  None
*
*******************************************************************************/

void audio_mic_data_feed_cm33(int16_t *audio_data)
{
#ifndef USB_FEED_TO_SVC

    cy_rslt_t ret_val = cy_svc_lp_feed((CY_SVC_DATA_T *)audio_data);

    /* Add code to print error in decoded format of the error if ret_val is not CY_RSLT_SUCCESS */
    if (ret_val != CY_RSLT_SUCCESS)
    {
        app_log_print("SVC feed failed: 0x%X\r\n", ret_val);
    }

    if (ret_val == CY_RSLT_SVC_LICENSE_EXPIRED)
    {
        app_log_print("The Evaluation period has ended, please reset the board to start again.\r\n");
        CY_ASSERT(0);
        CY_HALT();
    }
#endif
}

#ifdef USB_FEED_TO_SVC
void usb_mic_data_feed(int16_t *audio_data)
{
    cy_rslt_t ret_val = cy_svc_lp_feed((CY_SVC_DATA_T *)audio_data);

    if (ret_val == CY_RSLT_SVC_LICENSE_EXPIRED)
    {
        app_log_print("The Evaluation period has ended, please reset the board to start again.\r\n");
        CY_ASSERT(0);
        CY_HALT();
    }
    usb_send_out_dbg_put(USB_CHANNEL_1, audio_data);
}
#else
void usb_mic_data_feed(int16_t *audio_data)
{
}
#endif

/*******************************************************************************
* Function Name: svc_lp_event_handler
********************************************************************************
* Summary:
* Event handler for SVC events.
*
* Parameters:
*  event - Events from SVC (staged voice control) middleware.
*  *callback_user_arg - NULL
*
* Return:
*  CY_RSLT_SUCCESS
*
*******************************************************************************/

cy_rslt_t svc_lp_event_handler(
        cy_svc_event_t event,
        void *callback_user_arg)
{
    switch(event)
    {
        case CY_SVC_EVENT_SPEECH_ONSET_DETECTED:
        {
            if (*cm55_streaming_mode_active != 0u)
            {
                break;
            }
            app_log_print("\r\nSpeech Onset Detection detected speech on Low Power Core CM33\r\n");
            break;
        }
        case CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_DETECTED:
        {
            if (*cm55_streaming_mode_active != 0u)
            {
                /* LPWWD is bypassed in streaming mode; ignore LP detection events. */
                Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
                break;
            }
            app_log_print("Wake Word detected on Low Power Core CM33\r\n");
        #ifdef HP_INFERENCING_ENABLE
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
#ifdef PD1_POWER_CYCLE            
            app_log_print("Enabling Power Domain 1 and High Performance Core CM55\r\n");
            /* Always relaunch CM55 - even if PD1 is ON, CM55 might be in deep sleep */
            Enable_pd1_n_relaunch_cm55();
            vTaskResume(pd1_disable_task_handle);
#else
            vTaskResume(led_task_handle);
#endif /* PD1_POWER_CYCLE */
        #else
            Cy_GPIO_Inv(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN);
        #endif /* HP_INFERENCING_ENABLE */
            break;
        }
        case CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_NOT_DETECTED:
        {
            if (*cm55_streaming_mode_active != 0u)
            {
                /* LPWWD is bypassed in streaming mode; keep LP indicator ON. */
                Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
                break;
            }
            app_log_print("Wake Word not detected on Low Power Core CM33\r\n");
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
            break;
        }
        case CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED:
        {
            if (!button_pressed)
            {
                app_log_print("Wake Word detected on High Performance Core CM55\r\n");
            }
            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_ON);
            break;
        }
        case CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED:
        {
            if (*cm55_streaming_mode_active != 0u)
            {
                /* LPWWD is bypassed in streaming mode: keep LP indicator ON */
                Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
                Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
                /* Streaming: CM55 manages HPWWD internally. Do NOT call
                 * cy_svc_lp_set_stage() here — it is synchronous and would
                 * deadlock the SVC task (this callback runs inside it).
                 * SVC stays in its current stage; audio pipeline is still active. */
                app_log_print("Streaming mode: HP WWD not detected, CM55 restarting internally\r\n");
            }
            else
            {
                Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
                Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
#ifdef PD1_POWER_CYCLE
            disable_cm55_n_turn_off_pd1_flag = true;
#endif /* PD1_POWER_CYCLE */
                button_pressed = false;
                app_log_print("Wake Word not detected on High Performance Core CM55 - Timeout\r\n");
                app_log_print("Disabling Power Domain 1 and High Performance Core CM55 for low power, switching back to Low Power Core CM33.\r\n");
                app_log_print("******************     Battery Powered Local Voice     ******************\r\n\n");
                app_log_print("Wake word: Okay Infineon\r\n");
                app_log_print("Commands:\r\n"
                    "    Start music\r\n"
                    "    Next track\r\n"
                    "    Previous track\r\n"
                    "    End music\r\n"
                    "    Pause music \r\n"
                    "    Raise volume\r\n"
                    "    Lower volume\r\n"

                    "Additional intents for the commands: [Optional word] {one of required words} \t --> Matching commands\r\n"
                    "[can you] {play/start} [the] {music/track}  \t--> Play music \r\n"
                    "[can you] {end} [the] {music/track}  \t--> Stop music \r\n"
                    "[can you] {pause} [the] {music/track}  \t--> Pause music \r\n"
                    "[can you] [goto/switch to] {next} {track} \t--> Next track\r\n"
                    "[can you] [goto/switch to] {previous} {track} \t--> Previous track\r\n"
                    "[can you] {raise} {volume} \t--> Raise volume\r\n"
                    "[can you] {lower} {volume} \t--> Lower volume\r\n"
                    );
                app_log_print("waiting for wake word\r\n");
            }
            break;
        }
        case CY_SVC_EVENT_ASR_DETECTED:
        {
            Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_ON);
            if (*cm55_streaming_mode_active != 0u)
            {
                /* Streaming: CM55 skips ASR_PROCESSING_COMPLETED and
                 * restarts HPWWD inference internally. Just set the VA
                 * indication flag here. Do NOT call cy_svc_lp_set_stage()
                 * — it would deadlock (this callback runs inside SVC task). */
                asr_detected_indication_flag = true;
                app_log_print("Streaming mode: ASR detected on CM55\r\n");
                Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
            }
            break;
        }
        case CY_SVC_EVENT_ASR_NOT_DETECTED:
        {
            if (*cm55_streaming_mode_active != 0u)
            {
                /* LPWWD is bypassed in streaming mode: keep LP indicator ON */
                Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
                Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
                Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
                /* Streaming: CM55 manages ASR internally. Do NOT call
                 * cy_svc_lp_set_stage() — deadlock risk (this is SVC task context). */
                app_log_print("Streaming mode: ASR not detected, CM55 restarting internally\r\n");
            }
            else
            {
                Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
                Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
                Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
#ifdef PD1_POWER_CYCLE
            disable_cm55_n_turn_off_pd1_flag = true;
#endif /* PD1_POWER_CYCLE */
                if (button_pressed == true)
                {
                    /* Switch to Low Power (LP) ADC Autonomous analog configuration */
                    switch_to_lp_adc_AA_config();
                }
                button_pressed = false;
                app_log_print("ASR command not detected on High Performance Core CM55\r\n");
                app_log_print("Disabling Power Domain 1 and High Performance Core CM55 for low power, switching back to Low Power Core CM33.\r\n");
                app_log_print("******************     Battery Powered Local Voice     ******************\r\n\n");
                app_log_print("Wake word: Okay Infineon\r\n");
                app_log_print("Commands:\r\n"
                    "    Start music\r\n"
                    "    Next track\r\n"
                    "    Previous track\r\n"
                    "    End music\r\n"
                    "    Pause music \r\n"
                    "    Raise volume\r\n"
                    "    Lower volume\r\n"

                    "Additional intents for the commands: [Optional word] {one of required words} \t --> Matching commands\r\n"
                    "[can you] {play/start} [the] {music/track}  \t--> Play music \r\n"
                    "[can you] {end} [the] {music/track}  \t--> Stop music \r\n"
                    "[can you] {pause} [the] {music/track}  \t--> Pause music \r\n"
                    "[can you] [goto/switch to] {next} {track} \t--> Next track\r\n"
                    "[can you] [goto/switch to] {previous} {track} \t--> Previous track\r\n"
                    "[can you] {raise} {volume} \t--> Raise volume\r\n"
                    "[can you] {lower} {volume} \t--> Lower volume\r\n"
                    );
                app_log_print("waiting for wake word\r\n");
            }
            break;
        }
        case CY_SVC_EVENT_ASR_PROCESSING_COMPLETED:
        {
#ifdef PD1_POWER_CYCLE
            disable_cm55_n_turn_off_pd1_flag = true;
#endif        

            if (*cm55_streaming_mode_active != 0u)
            {
                Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
                /* Streaming: CM55 has already restarted HPWWD internally.
                 * Do NOT call cy_svc_lp_set_stage() — deadlock risk. */
                app_log_print("Streaming mode: ASR processing completed, CM55 cycling\r\n");
            }
            else
            {
                asr_detected_indication_flag = true;
                if (button_pressed == true)
                {
                    /* Switch to Low Power (LP) ADC Autonomous analog configuration */
                    switch_to_lp_adc_AA_config();
                }
                button_pressed = false;
                app_log_print("Disabling Power Domain 1 and High Performance Core CM55 for low power, switching back to Low Power Core CM33.\r\n");
                app_log_print("******************     Battery Powered Local Voice     ******************\r\n\n");
                app_log_print("Wake word: Okay Infineon\r\n");
                app_log_print("Commands:\r\n"
                    "    Start music\r\n"
                    "    Next track\r\n"
                    "    Previous track\r\n"
                    "    End music\r\n"
                    "    Pause music \r\n"
                    "    Raise volume\r\n"
                    "    Lower volume\r\n"

                    "Additional intents for the commands: [Optional word] {one of required words} \t --> Matching commands\r\n"
                    "[can you] {play/start} [the] {music/track}  \t--> Play music \r\n"
                    "[can you] {end} [the] {music/track}  \t--> Stop music \r\n"
                    "[can you] {pause} [the] {music/track}  \t--> Pause music \r\n"
                    "[can you] [goto/switch to] {next} {track} \t--> Next track\r\n"
                    "[can you] [goto/switch to] {previous} {track} \t--> Previous track\r\n"
                    "[can you] {raise} {volume} \t--> Raise volume\r\n"
                    "[can you] {lower} {volume} \t--> Lower volume\r\n"
                    );
                app_log_print("waiting for wake word\r\n");
            }
            break;
        }
        case CY_SVC_EVENT_LOW_NOISE_DETECTED:
        {
            app_log_print("Low Noise Detected AAD Restarted \r\n");
            aad_restart();
            break;
        }
        default:
        {
            break;
        }
    }
    return CY_RSLT_SUCCESS;
}

#ifdef HP_INFERENCING_ENABLE
/*******************************************************************************
* Function Name: app_svc_user_action_cb
********************************************************************************
* Summary:
* Callback for user button - Push to Talk - Goes to ASR detection state.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_svc_user_action_cb(void)
{

#if defined(CY_AFE_ENABLE_TUNING_FEATURE) || defined(USB_FEED_TO_SVC)
    app_log_print("PTT feature is disabled when USB streaming is active\r\n");
    return;
#else    
    cy_svc_stage_t stage = CY_SVC_STAGE_UNKNOWN;
    cy_svc_lp_get_current_stage(&stage);

    /* PTT in streaming mode: notify CM55 to switch to ASR (PTT flag will be set).
     * ADC is already in HS mode from streaming entry, so no need to switch. */
    if (*cm55_streaming_mode_active != 0u)
    {
        app_log_print("Push-To-Talk: User button pressed in streaming mode - triggering ASR on CM55\r\n");
        svc_lp_event_handler(CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED, NULL);
        cy_svc_lp_set_stage(CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT);
        button_pressed = true;
        return;
    }

    button_pressed = true;

    app_log_print("Push-To-Talk : User button pressed - Waiting for ASR command\r\n");
    /* Switch to High Speed (HS) ADC Autonomous analog configuration */
    switch_to_hs_adc_AA_config();

    svc_lp_event_handler(CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_DETECTED, NULL);
    svc_lp_event_handler(CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED, NULL);
    cy_svc_lp_set_stage(CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT);
#endif
}

#ifndef USB_FEED_TO_SVC
/*******************************************************************************
* Function Name: app_svc_streaming_toggle_cb
********************************************************************************
* Summary:
* Callback for User Button 2. Toggles the CM55 streaming state:
*   - First press: bypass LP WWD, hand control to CM55 for HP WWD + ASR
*                  (follows standard HP timeout). PTT is disabled.
*   - Second press: abort streaming, restore LP WWD state.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void app_svc_streaming_toggle_cb(void)
{

    TickType_t current_tick = xTaskGetTickCount(); 
    /* Ignore duplicate BTN2 callbacks generated by switch bounce/retrigger
     * from a single physical press. */
    if ((current_tick - s_last_streaming_toggle_tick) < pdMS_TO_TICKS(STREAMING_TOGGLE_GUARD_MS))
    {
        return;
    }
    s_last_streaming_toggle_tick = current_tick;

    if (*cm55_streaming_mode_active == 0u)
    {
        /* ---- Enter streaming mode ---- */
        button_pressed = false;

        app_log_print("Streaming mode ON: LP WWD bypassed, CM55 listening for HP WWD\r\n");

        /* Switch ADC to high-speed mode needed by CM55 */
        switch_to_hs_adc_AA_config();
#ifdef PD1_POWER_CYCLE
        /* If PD1 is still powered from a previous CM55 cycle, wait for the
         * shutdown to complete.  Calling Enable_pd1_n_relaunch_cm55() while
         * PD1/CM55 is active would memcpy over running CM55 code. */
        if (cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_PD1_BASE))
        {
            app_log_print("Waiting for previous CM55/PD1 cycle to shut down...\r\n");
            while (cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_PD1_BASE))
            {
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }

        /* Power up CM55/PD1 explicitly for streaming mode */
        Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
        /* Clear any stale shutdown request before resuming CM55/PD1 */
        disable_cm55_n_turn_off_pd1_flag = false;
        vTaskResume(pd1_disable_task_handle);
        Enable_pd1_n_relaunch_cm55();
#endif
        app_log_print("Enabling Power Domain 1 and High Performance Core CM55 (streaming)\r\n");

        /* IMPORTANT: Set the streaming flag AFTER Enable_pd1_n_relaunch_cm55()
         * because that function does a memcpy into SOCMEM which would overwrite
         * the flag if it were set earlier. CM55 takes time to boot and init
         * before it first reads this flag, so this ordering is safe. */
        *cm55_streaming_mode_active = 1u;

        /* Force SVC to ASR stage — the only stage accepted by set_stage().
         * This starts the audio pipeline to CM55. CM55 internally starts in
         * HPWWD inference and manages the HPWWD→ASR→HPWWD cycle on its own. */
        cy_svc_lp_set_stage(CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT);
        /* In streaming mode LPWWD is bypassed, keep LP indicator ON */
        Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
        
    }
    else
    {
        /* ---- Exit streaming mode ---- */
        *cm55_streaming_mode_active = 0u;
        button_pressed = false;

        app_log_print("Streaming mode OFF: returning to LP WWD\r\n");

        /* Restore LP ADC configuration immediately.  Streaming ON switched
         * to HS ADC which initialised a TCPWM timer.  If we don't de-init
         * it here, the next streaming ON will try to Cy_TCPWM_Counter_Init
         * on an already-running counter, hit CY_ASSERT(0) and hang.
         * Switching back to LP ADC also properly unlocks deepsleep. */
        switch_to_lp_adc_AA_config();

        /* Turn off LEDs to indicate streaming ended */
        Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
        Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
        Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);

        /* CM55 will see streaming flag = 0 on its next timeout or ASR
         * completion and will go to deepsleep normally.  The existing
         * event handlers (HPWWD_NOT_DETECTED / ASR_NOT_DETECTED /
         * ASR_PROCESSING_COMPLETED) with flag == 0 will then:
         *   - signal pd1_disable_task
         *   - set stage back to LPWWD */
    }

}

/*******************************************************************************
* Function Name: app_streaming_toggle_cm33_init
********************************************************************************
* Summary:
* Register the streaming toggle callback on User Button 2.
*
* Parameters:
*  user_action_cb - callback (typically app_svc_streaming_toggle_cb)
*
* Return:
*  None
*
*******************************************************************************/
static void app_streaming_toggle_cm33_init(cb_user_action user_action_cb)
{
    user_interaction_button2_register(user_action_cb);
}

#endif
/*******************************************************************************
* Function Name: app_push_to_talk_cm33_init
********************************************************************************
* Summary:
* Initialize user button and pass the callback.
*
* Parameters:
*  user_action_cb - User Buttton callback
*
* Return:
*  None
*
*******************************************************************************/

void app_push_to_talk_cm33_init(cb_user_action user_action_cb)
{
    user_interaction_init(USER_INTERACTION_BUTTON,user_action_cb);
}

/*******************************************************************************
* Function Name: Enable_pd1_n_relaunch_cm55
********************************************************************************
* Summary:
*  Enabled Power Domain 1 (PD1) and relaunches CM55
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Enable_pd1_n_relaunch_cm55(void)
{
    /* Ensure the calling task has a valid secure context before making the
     * NSC call to reapply_pd1_secure_configs().  The FreeRTOS global
     * xSecureContext tracks the currently-loaded secure context handle;
     * a value of 0 means the initial portALLOCATE_SECURE_CONTEXT() in the
     * calling task silently failed (configASSERT is a NOP in Release builds).
     * Re-issuing the allocation here is safe: if a context already exists
     * for this task, SecureContext_AllocateContext() returns 0 (duplicate
     * detection) but the existing context and xSecureContext remain valid
     * because PendSV restores the per-task handle on every context switch.
     * If no context exists yet, a fresh one is allocated and loaded. */
    {
        extern volatile uint32_t xSecureContext;  /* FreeRTOS port global */
        if (xSecureContext == 0U)
        {
            app_log_print("WARN: no secure context – allocating now\r\n");
            portALLOCATE_SECURE_CONTEXT(SVC_LP_SECURE_STACK_SIZE);
        }
    }

    cy_svc_lp_set_hp_core_info(CY_SVC_BEFORE_HP_CORE_ENABLE);
    Cy_System_EnablePD1_NS();

    /* Reapplying security protections for PD1 memories and peripherals */
    reapply_pd1_secure_configs();
    Cy_System_EnablePD1();
    /* Initialize External Memory */
    external_memory_init(QSPI);
    Cy_System_EnablePD1();

    /* Enable SOCMEM */
    Cy_SysEnableSOCMEM(true);
    /* Reapply socmem deepsleep mode */
    Cy_SysPm_SetSOCMEMDeepSleepMode(CY_SYSPM_MODE_DEEPSLEEP);

    /* Clear SYSCPU and APPCPU power domain dependency set by boot code */
    cy_pd_pdcm_clear_dependency(CY_PD_PDCM_APPCPU, CY_PD_PDCM_SYSCPU);
    /* To allow APPCPU to go in Deepsleep, clear U55 dependency */
    cy_pd_pdcm_clear_dependency(CY_PD_PDCM_APPCPU, CY_PD_PDCM_U55);

    /* Set IPC1 interrupt 13 and 14 mask (for wakeup) */
    Cy_IPC_Drv_SetInterruptMask(
            Cy_IPC_Drv_GetIntrBaseAddr(CM33_WAKEUP_IPC_INTR_NUM),
            CM33_WAKEUP_IPC_CH_MASK,
            CM33_WAKEUP_IPC_CH_MASK);
    NVIC_EnableIRQ(CY_IPC_INTR_MUX(CM33_WAKEUP_IPC_INTR_NUM));

    Cy_IPC_Drv_SetInterruptMask(
        Cy_IPC_Drv_GetIntrBaseAddr(CM33_WAKEUP_IPC_INTR_NUM2),
        CM33_WAKEUP_IPC_CH_MASK2,
        CM33_WAKEUP_IPC_CH_MASK2);
    NVIC_EnableIRQ(CY_IPC_INTR_MUX(CM33_WAKEUP_IPC_INTR_NUM2));

    /* Copy CM55 image from ext. flash to SOCMEM */
    memcpy(CM55_SOCMEM_START, CM55_EXT_FLASH_START, CM55_IMAGE_SIZE);
    /* Relaunching CM55 on LP Wake Word Detection */
    Cy_SysEnableCM55(MXCM55, CY_CM55_APP_BOOT_ADDR, CM55_BOOT_WAIT_TIME_USEC);
}

/*******************************************************************************
* Function Name: disable_cm55_n_turn_off_pd1_on_svc_event
********************************************************************************
* Summary:
*  Disables CM55 and turns off Power Domain 1 (PD1) when SVC event is Occurred
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void disable_cm55_n_turn_off_pd1_on_svc_event(void)
{
    cy_svc_lp_set_hp_core_info(CY_SVC_BEFORE_HP_CORE_DISABLE);
    /* Check if CM55 is in CPU Deep sleep, then Turn OFF PD1 */
    while((!Cy_SysPm_Cm55IsDeepSleep()) || (!(*cm55_deepsleep_flag)))
    {
        vTaskDelay(2);
    };
    /* Disable CM55 */
    Cy_SysDisableCM55();
    /* Disable SOCMEM */
    Cy_SysEnableSOCMEM(false);
    /* Disable PD1. Comment below line to keep PD1 ON, uncomment to turn it OFF */
    Cy_System_DisablePD1();
    //disable_cm55_n_turn_off_pd1_flag = true;
}
#endif /* HP_INFERENCING_ENABLE */

/*******************************************************************************
* Function Name: app_svc_alloc_memory
********************************************************************************
* Summary:
*  Callback function to allocate memory for the SVC middleware based on
*  the specified memory identifier. Depending on the memory type, this
*  function either assigns a pre-allocated shared memory region or
*  dynamically allocates memory from the heap.
*
* Parameters:
*  cy_svc_lp_mem_id_t mem_id : Memory identifier that specifies the type
*                              of memory to be allocated.
*  uint32_t size             : Size of the memory block to allocate, in bytes.
*  void **buffer             : Pointer to a pointer that will receive the
*                              allocated memory address.
*
* Return:
*  Result of the memory allocation operation.
*
*******************************************************************************/


cy_rslt_t app_svc_alloc_memory(cy_svc_lp_mem_id_t mem_id,
        uint32_t size, void **buffer)
{
      cy_rslt_t  ret_val = CY_RSLT_SVC_GENERIC_ERROR;

      if(NULL == buffer)
      {
          return ret_val;
      }
      *buffer = NULL;

      switch(mem_id)
      {
          case CY_SVC_MEM_ID_INTERCORE_SHARED_BUFFER:
          {
              if(APP_CFG_MAX_SHARED_BUF_SIZE != size)
              {
                  app_log_print("Inter Core Shared Buffer mismatch %d: %ld",
                          (int)APP_CFG_MAX_SHARED_BUF_SIZE, (long)size);
                  break;
              }
              *buffer = &app_data_shared_socmem[0];
              memset(*buffer, 0 , size);
              ret_val = CY_RSLT_SUCCESS;
              break;
          }
          default:
          {
              *buffer = (void *)calloc(size,1);
               ret_val = CY_RSLT_SUCCESS;
              break;
          }
      }


      return ret_val;
}


/*******************************************************************************
* Function Name: app_svc_free_memory
********************************************************************************
* Summary:
*  Callback function to free memory previously allocated by the SVC middleware.
*  Depending on the memory type, this function either releases dynamically
*  allocated memory or performs no operation for shared memory regions.
*
* Parameters:
*  cy_svc_lp_mem_id_t mem_id : Memory identifier that specifies the type
*                              of memory to be freed.
*  void *buffer              : Pointer to the memory block to be freed.
*
* Return:
*  Result of the memory free operation.
*
*******************************************************************************/

cy_rslt_t app_svc_free_memory(cy_svc_lp_mem_id_t mem_id,
        void *buffer)
{
      cy_rslt_t  ret_val = CY_RSLT_SVC_GENERIC_ERROR;

      if(NULL == buffer)
      {
          return ret_val;
      }

      switch(mem_id)
      {
          case CY_SVC_MEM_ID_INTERCORE_SHARED_BUFFER:
          {
              break;
          }
          default:
          {
              free(buffer);
              ret_val = CY_RSLT_SUCCESS;
              break;
          }
      }
      buffer = NULL;

      return ret_val;
}

/*******************************************************************************
* Function Name: app_init_svc
********************************************************************************
* Summary:
*  Initialize the staged voice control middleware.
*
* Parameters:
*  None
*
* Return:
*  Status of SVC initialization.
*
*******************************************************************************/

cy_rslt_t app_init_svc(void)
{
    cy_rslt_t  ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cy_svc_lp_config_t init_svc = {0};

    /* Default behavior after boot is regular LPWWD flow */
    *cm55_streaming_mode_active = 0u;

    app_ipc_init_cm33();

    init_svc.is_lpwwd_external = false;

    init_svc.lpwwd_external_data_callback = NULL;
    init_svc.pre_roll_frame_count_from_lpwwd_detect_frame = LPWWD_DETECT_FRAME;
    init_svc.single_frame_time_ms = CY_SVC_SUPPORTED_FRAME_TIME_MS;

    init_svc.audio_input_type = CY_SVC_AUDIO_INPUT_TYPE_MONO;

    init_svc.event_callback = svc_lp_event_handler;
    init_svc.callback_user_arg = NULL;
    init_svc.sample_rate = CY_SVC_SAMPLE_RATE_16KHZ;

    init_svc.sod_onset_detect_max_late_hit_delay_ms = SOD_HIT_DELAY;
    init_svc.sod_onset_gap_setting_ms = CY_SOD_ONSET_GAP_SETTING_500_MS;
    init_svc.sod_sensitivity = SOD_SENSITIVITY;
    init_svc.mel_low_freq = 20;
    init_svc.mel_high_freq = 7200;
#ifndef ENABLE_LOW_POWER_SVC
    init_svc.ipc_communication_sync_sempahore = 0;
#endif

    init_svc.stage_config_list = CY_SVC_ENABLE_SOD ;

#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    app_log_print("HPF enabled at LP side \r\n");
    init_svc.stage_config_list |= CY_SVC_ENABLE_HPF;
#endif /* ENABLE_IFX_PRE_PROCESS_HPF */



#ifndef LPWWD_OVERRIDE
    init_svc.stage_config_list |= CY_SVC_ENABLE_LPWWD;
#endif /* LPWWD_OVERRIDE */

#ifdef HP_INFERENCING_ENABLE
    init_svc.stage_config_list |= CY_SVC_ENABLE_HPWWD_STATE_TRANSITIONS;
    init_svc.stage_config_list |= CY_SVC_ENABLE_ASR_STATE_TRANSITIONS;
#endif /*HP_INFERENCING_ENABLE */

    init_svc.ipc_pipe_config.lp_pipe_end_point_address = APP_LP_PIPE_END_POINT_ADDRESS;
    init_svc.ipc_pipe_config.lp_pipe_end_point_client_id = APP_LP_PIPE_END_POINT_CLIENT_ID_1;
    init_svc.ipc_pipe_config.lp_pipe_end_point_intr_mask =
            CY_IPC_INTR_MASK(init_svc.ipc_pipe_config.lp_pipe_end_point_client_id);

    init_svc.ipc_pipe_config.hp_pipe_end_point_address = APP_HP_PIPE_END_POINT_ADDRESS;
    init_svc.ipc_pipe_config.hp_pipe_end_point_client_id = APP_HP_PIPE_END_POINT_CLIENT_ID_2;
    init_svc.ipc_pipe_config.hp_pipe_end_point_intr_mask =
            CY_IPC_INTR_MASK(init_svc.ipc_pipe_config.hp_pipe_end_point_client_id);

    init_svc.alloc_memory = app_svc_alloc_memory;
    init_svc.free_memory = app_svc_free_memory;
    init_svc.inter_core_shared_buffer_size = APP_INTER_CORE_BUF_SIZE;
    ret_val = cy_svc_lp_init(&init_svc);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        app_log_print("Staged Voice Initialization failed");
        CY_ASSERT(0);
    }
    cy_svc_lp_gain_config_t gain_control;
    gain_control.post_hpf_gain = POST_HPF_GAIN; // 1 - Near field, 3 - Far field
    gain_control.sod_processing_gain = SOD_PROCESSING_GAIN; //1 - Near field, 3 - Far field
    cy_svc_lp_gain_config(&gain_control);

    cy_svc_lp_low_noise_config_t aad_config;
    aad_config.timeout_ms = SOD_TO_AAD_TIMEOUT_MS; //7 sec
    aad_config.low_noise_threshold = SOD_TO_AAD_THRESHOLD;
    aad_config.enable_feature = true;
    cy_svc_lp_low_noise_config(&aad_config);
#if AFE_INPUT_SOURCE==AFE_INPUT_SOURCE_MIC
#ifdef HP_INFERENCING_ENABLE
    app_push_to_talk_cm33_init(app_svc_user_action_cb);
#ifndef USB_FEED_TO_SVC
    /* Register BTN2 as streaming mode toggle */
    app_streaming_toggle_cm33_init(app_svc_streaming_toggle_cb);
#endif
#endif /* HP_INFERENCING_ENABLE */
#endif /* AFE_INPUT_SOURCE_MIC */
    return ret_val;
}

/*******************************************************************************
* Function Name: app_deinit_svc
********************************************************************************
* Summary:
* Deinitialize the staged voice control middleware.
*
* Parameters:
*  None
*
* Return:
*  Status of SVC deinitialization.
*
*******************************************************************************/

cy_rslt_t app_deinit_svc(void)
{
    cy_rslt_t ret_val = CY_RSLT_SUCCESS;

    ret_val = cy_svc_lp_deinit();
    if (ret_val != CY_RSLT_SUCCESS)
    {
        app_svc_log("Staged Voice Control deinit failed");
    }
    else
    {
        app_svc_log("Staged Voice control deinit success");
    }

    return ret_val;
}

#ifdef ENABLE_LOW_POWER_SVC
#ifdef HP_INFERENCING_ENABLE
/*******************************************************************************
* Function Name: external_memory_init
********************************************************************************
* Summary:
* Function to init the external memory
*
* Parameters:
*  en_ext_mem_t mode: External Memory Mode
*
* Return:
*  Status of Memory initialization.
*
*******************************************************************************/
static cy_en_smif_status_t external_memory_init(en_ext_mem_t mode)
{
    /* Initialize SMIF status variable with an error value */
    cy_en_smif_status_t smif_status = CY_SMIF_GENERAL_ERROR;

    do
    {
        /* De-initialize the SMIF core */
        Cy_SMIF_DeInit(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base);

        /* Reset the external memory connected to the SMIF core */
        Cy_SMIF_Reset_Memory(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base,smifBlockConfig.memConfig[0]->slaveSelect);

        /* Initialize the SMIF core with the provided configuration */
        smif_status = Cy_SMIF_Init(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.config, SMIF_INIT_TIMEOUT, &SMIFContext);

        if (CY_SMIF_SUCCESS != smif_status)
        {
            /* Break the loop if initialization fails */
            break;
        }

        /* Set the data select signal for the memory device */
        Cy_SMIF_SetDataSelect(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, smifBlockConfig.memConfig[0]->slaveSelect, smifBlockConfig.memConfig[0]->dataSelect);

        /* Enable the SMIF core */
        Cy_SMIF_Enable(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, &SMIFContext);

        /* Initialize the external memory device */
        smif_status = Cy_SMIF_MemInit(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, &smifBlockConfig, &SMIFContext);

        if (CY_SMIF_SUCCESS != smif_status)
        {
            /* Break the loop if memory initialization fails */
            break;
        }

        if (mode == QSPI)
        {
            bool qe_status = false;

            /* Check if QUAD Mode is already enabled */
            smif_status = Cy_SMIF_MemIsQuadEnabled(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, smifBlockConfig.memConfig[0], &qe_status, &SMIFContext);

            if (smif_status != CY_SMIF_SUCCESS)
            {
                /* Break the loop if checking QE status fails */
                break;
            }

            /* If QUAD Mode is not enabled, enable it */
            if (!qe_status)
            {
                smif_status = Cy_SMIF_MemQuadEnable(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, smifBlockConfig.memConfig[0], &SMIFContext);

                if (smif_status != CY_SMIF_SUCCESS)
                {
                    /* Break the loop if enabling QE fails */
                    break;
                }
            }
        }
        else
        {
            /* Get the desired data rate for the memory device */
            cy_en_smif_data_rate_t data_rate = smifBlockConfig.memConfig[0]->deviceCfg->readCmd->dataRate;

            /* Enable OCTAL mode for the memory device */
            smif_status = Cy_SMIF_MemOctalEnable(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, smifBlockConfig.memConfig[0], data_rate, &SMIFContext);

            if (CY_SMIF_SUCCESS != smif_status)
            {
                /* Break the loop if enabling Octal mode fails */
                break;
            }

            /* If DDR mode is enabled, configure for Hyperbus with DQS */
            if (CY_SMIF_DDR == data_rate)
            {
                Cy_SMIF_Disable(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base);

                smif_status = Cy_SMIF_SetRxCaptureMode(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, CY_SMIF_SEL_XSPI_HYPERBUS_WITH_DQS, smifBlockConfig.memConfig[0]->slaveSelect);

                if (CY_SMIF_SUCCESS != smif_status)
                {
                    /* Break the loop if setting RX capture mode fails */
                    break;
                }
            }
        }
        Cy_SMIF_Enable(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, &SMIFContext);
        /* Set the SMIF core to memory mode */
        Cy_SMIF_SetMode(CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base, CY_SMIF_MEMORY);

    } while (false); // The loop will only iterate once

    /* Assert if any of the initialization steps failed */
    if (CY_SMIF_SUCCESS != smif_status)
    {
        app_log_print("serial memory initialization failed\r\n");
        CY_ASSERT(0);
    }

    return smif_status;
}
#endif
#endif /* ENABLE_LOW_POWER_SVC */

#ifdef HP_INFERENCING_ENABLE
void cy_svc_lp_thread_start_hook(void)
{
    portALLOCATE_SECURE_CONTEXT(SVC_LP_SECURE_STACK_SIZE);
}
#endif /* HP_INFERENCING_ENABLE */

/* [] END OF FILE */
