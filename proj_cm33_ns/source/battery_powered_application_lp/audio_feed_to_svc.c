
/******************************************************************************
* File Name : audio_feed_to_svc.c
*
* Description :
* Code for audio feed into Staged Voice Control middleware
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

#include "audio_feed_to_svc.h"

#include "audio_conv_utils.h"
#include "cy_staged_voice_control.h"
#include "cy_staged_voice_control_common.h"
#include "cycfg_pins.h"
#include "cybsp_types.h"

#include "app_logger.h"
#include "audio_input_configuration.h"
#include "intercore_ipc.h"

#include "nsc_func.h"
#include "cy_syspm_pdcm.h"
#include "cy_smif_memslot.h"

#include "ppudbg.h"
#include "aad.h"

#include "mtb_hal.h"
/*******************************************************************************
* Macros
*******************************************************************************/

#define MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES     (160*2*2)
#define MONO_10MS_AUDIO_BYTES                           (160)

#define APP_INTER_CORE_BUF_SIZE                         (128000)

#define APP_CFG_MAX_SHARED_BUF_SIZE                     (APP_INTER_CORE_BUF_SIZE)

#define LPWWD_DETECT_FRAME                              (200u)
#define SOD_HIT_DELAY                                   (500u)
#define SOD_SENSITIVITY                                 (16384u)

#ifdef LOW_POWER_MODE
#define CM55_EXT_FLASH_START                            (void*)(0x60580400)
#define CM55_SOCMEM_START                               (void*)(CY_CM55_APP_BOOT_ADDR)
#define CM55_IMAGE_SIZE                                 (1940000)

#define SMIF_INIT_TIMEOUT                               (10000UL)
/* The timeout value in microsecond used to wait for core to be booted */
#define CM55_BOOT_WAIT_TIME_USEC                        (10U)
#endif /* LOW_POWER_MODE */

#define CM55_DEEPSLEEP_FLAG_ADDRESS                     (0x240FEFF4)
/*******************************************************************************
* Global Variables
*******************************************************************************/
int8_t noninterleave_audio[MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES] = { 0 };


int inf_count=0;
#ifdef ENABLE_LOW_POWER_SVC
unsigned char app_data_shared_socmem[APP_CFG_MAX_SHARED_BUF_SIZE];

/* Low power changes */
static cy_stc_smif_context_t SMIFContext;
typedef enum
{
    QSPI = 0,
    OSPI = 1
}en_ext_mem_t;
#else 
unsigned char app_data_shared_socmem[APP_CFG_MAX_SHARED_BUF_SIZE] __attribute__((section(".cy_shared_socmem")));
#endif

static bool config_secure_stack_once = true;
volatile uint8_t *cm55_deepsleep_flag = (uint8_t *)CM55_DEEPSLEEP_FLAG_ADDRESS; // Define the pointer to the memory address
extern volatile uint8_t disable_cm55_n_turn_off_pd1_flag;
extern volatile uint8_t asr_detected_indication_flag;
extern TaskHandle_t pd1_disable_task_handle;
/*******************************************************************************
* Functions Prototypes
*******************************************************************************/

extern const char* svc_lp_events_to_printable_string(cy_svc_event_t event);
#ifdef INTERCORE_PIPE
extern void app_ipc_init_cm33(void);
#endif /* INTERCORE_PIPE */

#ifdef ENABLE_LOW_POWER_SVC
static cy_en_smif_status_t external_memory_init(en_ext_mem_t interface);
#endif /* ENABLE_LOW_POWER_SVC */
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
    cy_rslt_t ret_val = cy_svc_lp_feed((CY_SVC_DATA_T *)audio_data);

    if (ret_val == CY_RSLT_SVC_LICENSE_EXPIRED)
    {
        app_log_print("The Evaluation period has ended, please reset the board to start again.\r\n");
    }
}

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
            app_log_print("\r\nSpeech Onset Detection detected speech \r\n");
            break;
        }
        case CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_DETECTED:
        {
            app_log_print("Wake Word detected in Low Power Core CM33 \r\n");
        #ifdef HP_INFERENCING_ENABLE
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
            app_log_print("***********Enabling Power Domain 1*************\n\r")
            vTaskResume(pd1_disable_task_handle);
            /* Used for debug of CM55 deepsleep across PD1 power cycles */
            Enable_pd1_n_relaunch_cm55();
        #else
            Cy_GPIO_Inv(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN);
        #endif /* HP_INFERENCING_ENABLE */
            break;
        }
        case CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_NOT_DETECTED:
        {
            app_log_print("Wake Word not detected in Low Power Core CM33 \r\n");
            app_log_print("Waiting for Wake Word \r\n");
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
            break;
        }
        case CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED:
        {
            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_ON);
            break;
        }
        case CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED:
        {
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
            app_log_print("Waiting for Wake Word \r\n");
            disable_cm55_n_turn_off_pd1_flag = true;
            break;
        }
        case CY_SVC_EVENT_ASR_DETECTED:
        {
            Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_ON);
            break;
        }
        case CY_SVC_EVENT_ASR_NOT_DETECTED:
        {
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
            Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
            disable_cm55_n_turn_off_pd1_flag = true;
            app_log_print("Waiting for Wake Word \r\n");
            break;
        }
        case CY_SVC_EVENT_ASR_PROCESSING_COMPLETED:
        {
            asr_detected_indication_flag = true;
            disable_cm55_n_turn_off_pd1_flag = true;
            app_log_print("Waiting for Wake Word \r\n");
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
    cy_svc_lp_set_hp_core_info(CY_SVC_BEFORE_HP_CORE_ENABLE);
    Cy_System_EnablePD1_NS();
    if(config_secure_stack_once)
    {
        portALLOCATE_SECURE_CONTEXT(2*configMINIMAL_SECURE_STACK_SIZE);
        config_secure_stack_once = false;
    }
    /* Reapplying security protections for PD1 memories and peripherals */
    reapply_pd1_secure_configs();
    
    Cy_System_EnablePD1();
    /* Initialize External Memory */
    cy_en_smif_status_t status = external_memory_init(QSPI);
    Cy_System_EnablePD1();
    /* Enable SOCMEM */
    Cy_SysEnableSOCMEM(true);
    /* Reapply socmem deepsleep mode */
    Cy_SysPm_SetSOCMEMDeepSleepMode(CY_SYSPM_MODE_DEEPSLEEP);

    /* Clear SYSCPU and APPCPU power domain dependency set by boot code */
    cy_pd_pdcm_clear_dependency(CY_PD_PDCM_APPCPU, CY_PD_PDCM_SYSCPU);
    /* To allow APPCPU to go in Deepsleep, clear U55 dependency */
    cy_pd_pdcm_clear_dependency(CY_PD_PDCM_APPCPU, CY_PD_PDCM_U55);

    /* Set IPC1 interrupt 13 and 14 mask */
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
    app_log_print("\r*************Re-launching CM55*************\r\n");
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
    app_log_print("\n\r************* CM55 entered CPU Deep Sleep*************\r\n"); 
    /* Disable CM55 */
    Cy_SysDisableCM55();
    /* Disable SOCMEM */
    Cy_SysEnableSOCMEM(false);
    /* Disable PD1. Comment below line to keep PD1 ON, uncomment to turn it OFF */ 
    Cy_System_DisablePD1();
    app_log_print("\n\r************* Power Domain 1 turned OFF *************\r\n");
    disable_cm55_n_turn_off_pd1_flag = true;
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

      app_log_print("Alloc: MEMID:%d, Addr:%p, Size:%ld", (int)mem_id, *buffer,(long)size);

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
      app_log_print("Free: MEMID:%d, Addr:%p",mem_id, buffer);

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

#ifdef INTERCORE_PIPE
    app_ipc_init_cm33();
#endif /* INTERCORE_PIPE */

    init_svc.is_lpwwd_external = false;

    init_svc.lpwwd_external_data_callback = NULL;
    init_svc.pre_roll_frame_count_from_lpwwd_detect_frame = LPWWD_DETECT_FRAME;
    init_svc.single_frame_time_ms = CY_SVC_SUPPORTED_FRAME_TIME_MS;

    init_svc.audio_input_type = CY_SVC_AUDIO_INPUT_TYPE_MONO;

    init_svc.event_callback = svc_lp_event_handler;
    init_svc.callback_user_arg = NULL;
    init_svc.sample_rate = CY_SVC_SAMPLE_RATE_16KHZ;

    init_svc.sod_onset_detect_max_late_hit_delay_ms = SOD_HIT_DELAY;
    init_svc.sod_onset_gap_setting_ms = CY_SOD_ONSET_GAP_SETTING_400_MS;
    init_svc.sod_sensitivity = SOD_SENSITIVITY;
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
    else
    {
        app_log_print("Staged Voice Initialization success");
    }
    cy_svc_lp_low_noise_config_t aad_config;
    aad_config.timeout_ms = SOD_TO_AAD_TIMEOUT_MS; //7 sec
    aad_config.low_noise_threshold = SOD_TO_AAD_THRESHOLD;
    aad_config.enable_feature = true;    
    cy_svc_lp_low_noise_config(&aad_config);
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
#endif /* ENABLE_LOW_POWER_SVC */

/* [] END OF FILE */
