/******************************************************************************
* File Name : main.c
*
* Description :
*  main function for CM33 Non Secure application for Battery Powered Local Voice.
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
#include "mtb_hal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "app_logger.h"
#include "mtb_syspm_callbacks.h"

#include "FreeRTOS.h"
#include "task.h"

#include "battery_powered_application_lp.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"

#include "ppudbg.h"
#include "cy_staged_voice_control.h"
/*******************************************************************************
* Macros
*******************************************************************************/
/* The timeout value in microsecond used to wait for core to be booted */
#define CM55_BOOT_WAIT_TIME_USEC          (10U)

/* Enabling or disabling a MCWDT requires a wait time of upto 2 CLK_LF cycles  
 * to come into effect. This wait time value will depend on the actual CLK_LF  
 * frequency set by the BSP.
 */
#define LPTIMER_0_WAIT_TIME_USEC            (62U)

/* Define the LPTimer interrupt priority number. '1' implies highest priority. 
 */
#define APP_LPTIMER_INTERRUPT_PRIORITY      (1U)

#ifdef LOW_POWER_MODE
#define CM55_EXT_FLASH_START                (void*)(0x60580400)
#define CM55_SOCMEM_START                   (void*)(CY_CM55_APP_BOOT_ADDR)
#define CM55_IMAGE_SIZE                     (1940000)
#define FIRST_LAUNCH_OF_CM55_ADDR           (0x240FEFF0)

#ifdef HP_INFERENCING_ENABLE
#define PD1_OFF_TASK_NAME                   ("CM33 PD1 Off Task")
#define PD1_OFF_TASK_STACK_SIZE             (configMINIMAL_STACK_SIZE * 4)
#define PD1_OFF_TASK_PRIORITY               (configMAX_PRIORITIES - 4)
#endif /* HP_INFERENCING_ENABLE */

#define LED_OFF_TASK_NAME                   ("LED Off Task")
#define LED_OFF_TASK_STACK_SIZE             (configMINIMAL_STACK_SIZE)
#define LED_OFF_TASK_PRIORITY               (configMAX_PRIORITIES - 4)


#define WAIT_FOR_TX_COMPLETE()              while (!(Cy_SCB_UART_IsTxComplete \
                                                       (CYBSP_DEBUG_UART_HW)))
#endif /* LOW_POWER_MODE */

/* Default syspm callback configuration elements */
#define SYSPM_SKIP_MODE                     (0U)
#define SYSPM_CALLBACK_ORDER                (1U)

#define UL_NON_BLOCKING_WAIT                (0U)

/* Disables the High Frequency Clocks */
#define CY_CFG_SYSCLK_CLKHF5                (5U)
#define CY_CFG_SYSCLK_CLKHF6                (6U)
#define CY_CFG_SYSCLK_CLKHF11               (11U)
#define CY_CFG_SYSCLK_CLKHF12               (12U)
#define CY_CFG_SYSCLK_CLKHF13               (13U)

/* Power Domain 1 (PD1) Task Delay */
#define PD1_TASK_DELAY                      (500U)

/* Wait time after license has been expired */
#define WAIT_TIME_AFTER_LICENSE_EXPIRE      (10U)

/* Wait time after CM55 is booted */
#define WAIT_TIME_AFTER_BOOT_CM55           (10U)

/* Time to check everytime if Cm55 is in deep sleep or not */
#define GET_CM55_DEEP_SLEEP_STATUS_DELAY    (2U)

/* Delay to keep LED ON in case of ASR Detected */
#define  ASRD_LED_TURN_OFF_DELAY_COUNTER    (2U)
#if APP_LOG_PRINT_EN
/* Enable CM33 Logs */
#define ENABLE_CM33_LOGS                    (1U)
#endif /* APP_LOG_PRINT_EN */
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void app_retarget_io_init(void);

static void handle_error(void);
static void lptimer_interrupt_handler(void);
static void setup_tickless_idle_timer(void);

void vApplicationDaemonTaskStartupHook( void );

#ifdef PD1_POWER_CYCLE
static void disable_cm55_n_turn_off_pd1(void);
#endif /* LOW_POWER_MODE */
/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Debug UART context */
cy_stc_scb_uart_context_t  DEBUG_UART_context;
/* Debug UART HAL object  */
mtb_hal_uart_t             DEBUG_UART_hal_obj;

/* LPTimer HAL object */
static mtb_hal_lptimer_t lptimer_obj;

#ifdef LOW_POWER_MODE
/* Define the pointer to the memory address */
volatile uint8_t* first_launch_of_cm55 = (uint8_t *)FIRST_LAUNCH_OF_CM55_ADDR; 

#ifdef PD1_POWER_CYCLE
TaskHandle_t pd1_disable_task_handle = NULL;
volatile bool disable_cm55_n_turn_off_pd1_flag = false;
#endif /* PD1_POWER_CYCLE */

volatile bool  asr_detected_indication_flag = false;

TaskHandle_t led_task_handle = NULL;

#if ENABLE_CM33_LOGS
/* Context reference structure for Debug UART */
static mtb_syspm_uart_deepsleep_context_t retarget_io_syspm_ds_context =
{
    .uart_context       = &DEBUG_UART_context,
    .async_context      = NULL,
    .tx_pin =
    {
        .port           = CYBSP_DEBUG_UART_TX_PORT,
        .pinNum         = CYBSP_DEBUG_UART_TX_PIN,
        .hsiom          = CYBSP_DEBUG_UART_TX_HSIOM
    },
    .rts_pin = 
    {
        .port           = NULL,
        .pinNum         = 0,
        .hsiom          = HSIOM_SEL_GPIO
    }
};

/* SysPm callback parameter structure for Debug UART */
static cy_stc_syspm_callback_params_t retarget_io_syspm_cb_params =
{
    .context            = &retarget_io_syspm_ds_context,
    .base               = CYBSP_DEBUG_UART_HW
};

/* SysPm callback structure for Debug UART */
static cy_stc_syspm_callback_t retarget_io_syspm_cb =
{
    .callback           = &mtb_syspm_scb_uart_deepsleep_callback,
    .skipMode           = SYSPM_SKIP_MODE,
    .type               = CY_SYSPM_DEEPSLEEP,
    .callbackParams     = &retarget_io_syspm_cb_params,
    .prevItm            = NULL,
    .nextItm            = NULL,
    .order              = SYSPM_CALLBACK_ORDER
};
#endif

#ifndef PD1_POWER_CYCLE
/*******************************************************************************
 * Function Name: led_task 
 *******************************************************************************
 * Summary:
 * FreeRTOS Task for LED Operations when PD1 power cycling is not enabled.
 *
 * Parameters:
 *  void * arg
 *
 * Return:
 *  void
 *
 ******************************************************************************/
static void led_task(void * arg)
{
    CY_UNUSED_PARAMETER(arg);
    for (;;)
    {
        if(asr_detected_indication_flag)
        {
            static uint16_t asr_detected_indication_count = 0;
            asr_detected_indication_count++;
            if(ASRD_LED_TURN_OFF_DELAY_COUNTER == asr_detected_indication_count)
            {
                asr_detected_indication_flag = false;
                asr_detected_indication_count = 0;
                Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
                Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
                Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
                vTaskSuspend(led_task_handle);
                __DMB();
            }
        }
        vTaskDelay(PD1_TASK_DELAY);        
    }
}
#endif

#ifdef HP_INFERENCING_ENABLE
#ifdef PD1_POWER_CYCLE
/*******************************************************************************
 * Function Name: pd1_disable_task 
 *******************************************************************************
 * Summary:
 * FreeRTOS Task for Power Domain 1 (PD1) Operations.
 *
 * Parameters:
 *  void * arg
 *
 * Return:
 *  void
 *
 ******************************************************************************/
static void pd1_disable_task(void * arg)
{
    CY_UNUSED_PARAMETER(arg);
    for (;;)
    {
        if(disable_cm55_n_turn_off_pd1_flag)
        {
            extern void disable_cm55_n_turn_off_pd1_on_svc_event(void);
            if(cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_PD1_BASE))
            {
                disable_cm55_n_turn_off_pd1_on_svc_event();
            }
            else 
            {
                if(asr_detected_indication_flag)
                {
                    static uint16_t asr_detected_indication_count = 0;
                    asr_detected_indication_count++;
                    if(ASRD_LED_TURN_OFF_DELAY_COUNTER == asr_detected_indication_count)
                    {
                        asr_detected_indication_flag = false;
                        asr_detected_indication_count = 0;
                        Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
                        Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
                        Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
                        /* Whenever PD1 is turned OFF, suspend pd1_disable_task */
                        vTaskSuspend(pd1_disable_task_handle);
                        disable_cm55_n_turn_off_pd1_flag = false;
                       __DMB();
                    }
                }
                else
                {
                    /* Whenever PD1 is turned OFF, suspend pd1_disable_task */
                    vTaskSuspend(pd1_disable_task_handle);
                    disable_cm55_n_turn_off_pd1_flag = false;
                   __DMB();
                }
            }
        }
        vTaskDelay(PD1_TASK_DELAY);
    }
}
#endif /* PD1_POWER_CYCLE */
#endif /* HP_INFERENCING_ENABLE */
#endif /* LOW_POWER_MODE */

/*******************************************************************************
* Function Name: vApplicationDaemonTaskStartupHook
********************************************************************************
* Summary:
*  This function will be called once when the Daemon/Timer task starts.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void vApplicationDaemonTaskStartupHook( void )
{
    /* Secure stack allocation for the FreeRTOS timer task. */
    portALLOCATE_SECURE_CONTEXT(2 * configMINIMAL_SECURE_STACK_SIZE);
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function of the CM33 non-secure application.
*
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/

int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        app_log_print("Board initialization failed\r\n");
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Clear GPIO and NVIC interrupt before initializing to avoid false
     * triggering.
     */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_PIN);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN1_IRQ);

    /* Program SOCMEM for deepsleep ready state */
    Cy_SysPm_SetSOCMEMDeepSleepMode(CY_SYSPM_MODE_DEEPSLEEP);
    /* Disable the low-power for Bandgap reference circuit */
    SRSS_PWR_CTL2 |= SRSS_PWR_CTL2_BGREF_LPMODE_Msk; 

#if BATTERY_POWERED_LOCAL_VOICE_APP_ENABLE
    /* Setup the LPTimer instance for CM33 CPU. */
    setup_tickless_idle_timer();
    /* Initialize retarget-io */
    app_retarget_io_init();

    /* Initialize CM33 application with staged voice control if enabled in the audio pipeline configuration*/
    battery_powered_application_lp();

#endif

#if LOW_POWER_MODE
#ifdef PD1_POWER_CYCLE
    *first_launch_of_cm55 = 1;
#else
    *first_launch_of_cm55 = 0;
#endif /* PD1_POWER_CYCLE */
    /* Copy CM55 image from external flash to SOCMEM */
    memcpy(CM55_SOCMEM_START, CM55_EXT_FLASH_START, CM55_IMAGE_SIZE);
    /* Launch CM55 */
    Cy_SysEnableCM55(MXCM55, CY_CM55_APP_BOOT_ADDR, CM55_BOOT_WAIT_TIME_USEC);
    Cy_SysLib_Delay(WAIT_TIME_AFTER_BOOT_CM55);
#ifdef HP_INFERENCING_ENABLE
#ifdef PD1_POWER_CYCLE
    /* Create the FreeRTOS Task */
    result = xTaskCreate(pd1_disable_task, PD1_OFF_TASK_NAME,
                        PD1_OFF_TASK_STACK_SIZE, NULL,
                        PD1_OFF_TASK_PRIORITY, &pd1_disable_task_handle);
#else
    /* Create the FreeRTOS Task */
    result = xTaskCreate(led_task, LED_OFF_TASK_NAME,
                        LED_OFF_TASK_STACK_SIZE, NULL,
                        LED_OFF_TASK_PRIORITY, &led_task_handle);
    
    /* Dont start this task until LPWWD is detected */
    vTaskSuspend(led_task_handle);                        

#endif /* PD1_POWER_CYCLE */
#endif /* HP_INFERENCING_ENABLE */

#ifdef PD1_POWER_CYCLE
    /* Disable the CM55 and turn off the Power Domain 1 (PD1) */
    disable_cm55_n_turn_off_pd1();
#endif /* PD1_POWER_CYCLE */    


    Cy_SysClk_ClkHfDisable(CY_CFG_SYSCLK_CLKHF4);
    Cy_SysClk_ClkHfDisable(CY_CFG_SYSCLK_CLKHF5);
    Cy_SysClk_ClkHfDisable(CY_CFG_SYSCLK_CLKHF6);
    Cy_SysClk_ClkHfDisable(CY_CFG_SYSCLK_CLKHF11);
    Cy_SysClk_ClkHfDisable(CY_CFG_SYSCLK_CLKHF12);
    Cy_SysClk_ClkHfDisable(CY_CFG_SYSCLK_CLKHF13);
#endif /* LOW_POWER_MODE */

#ifndef PD1_POWER_CYCLE
    mtb_hal_syspm_lock_deepsleep();
#endif /* PD1_POWER_CYCLE */

#if BATTERY_POWERED_LOCAL_VOICE_APP_ENABLE
    /* Start the RTOS Scheduler */
    vTaskStartScheduler();
#else
     Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);   
#endif
    return CY_RSLT_SUCCESS; 

}

#ifdef PD1_POWER_CYCLE
/*******************************************************************************
* Function Name: disable_cm55_n_turn_off_pd1
********************************************************************************
* Summary:
* This function waits for CM55 to enter CPU Deep Sleep mode, then disables CM55
*
* Parameters:
*  none
*
* Return :
*  none
*
*******************************************************************************/
static void disable_cm55_n_turn_off_pd1(void)
{
    *first_launch_of_cm55 = 0;
    cy_svc_lp_set_hp_core_info(CY_SVC_BEFORE_HP_CORE_DISABLE);
    /* Check if CM55 is in CPU Deep Sleep, then Turn OFF PD1 */
    while(!Cy_SysPm_Cm55IsDeepSleep()) 
    { 
        Cy_SysLib_Delay(GET_CM55_DEEP_SLEEP_STATUS_DELAY);
    };
    /* Disable CM55 */
    Cy_SysDisableCM55();
    /* Disable SOCMEM */
    Cy_SysEnableSOCMEM(false);
    /* Disable PD1 */ 
    Cy_System_DisablePD1();
    /* On First launch of CM55 suspend pd1_disable_task*/
    disable_cm55_n_turn_off_pd1_flag = true;
}
#endif /* LOW_POWER_MODE */
/*******************************************************************************
* Function Name: app_retarget_io_init
********************************************************************************
* Summary:
* This function initializes Debug UART
*
* Parameters:
*  none
*
* Return :
*  none
*
*******************************************************************************/
void app_retarget_io_init()
{
    cy_rslt_t result;

    result = (cy_rslt_t)Cy_SCB_UART_Init(CYBSP_DEBUG_UART_HW, &CYBSP_DEBUG_UART_config, &DEBUG_UART_context);
    if (result != CY_RSLT_SUCCESS)
    {
        app_log_print("UART initialization failed\r\n");
        CY_ASSERT(0);
    }

    Cy_SCB_UART_Enable(CYBSP_DEBUG_UART_HW);
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &CYBSP_DEBUG_UART_hal_config, &DEBUG_UART_context, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        app_log_print("mtb hal UART setup failed\r\n");
        CY_ASSERT(0);
    }

    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);
    #if ENABLE_CM33_LOGS
    /* UART SysPm callback registration for retarget-io */
    Cy_SysPm_RegisterCallback(&retarget_io_syspm_cb);
    #endif
}


/*******************************************************************************
* Function Name: lptimer_interrupt_handler
********************************************************************************
* Summary:
* Interrupt handler function for LPTimer instance. 
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void lptimer_interrupt_handler(void)
{
    mtb_hal_lptimer_process_interrupt(&lptimer_obj);
}

/*******************************************************************************
* Function Name: setup_tickless_idle_timer
********************************************************************************
* Summary:
* 1. This function first configures and initializes an interrupt for LPTimer.
* 2. Then it initializes the LPTimer HAL object to be used in the RTOS 
*    tickless idle mode implementation to allow the device enter deep sleep 
*    when idle task runs. LPTIMER_0 instance is configured for CM33 CPU.
* 3. It then passes the LPTimer object to abstraction RTOS library that 
*    implements tickless idle mode
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void setup_tickless_idle_timer(void)
{
    /* Interrupt configuration structure for LPTimer */
    cy_stc_sysint_t lptimer_intr_cfg =
    {
        .intrSrc = CYBSP_CM33_LPTIMER_0_IRQ,
        .intrPriority = APP_LPTIMER_INTERRUPT_PRIORITY
    };

    /* Initialize the LPTimer interrupt and specify the interrupt handler. */
    cy_en_sysint_status_t interrupt_init_status = 
                                    Cy_SysInt_Init(&lptimer_intr_cfg, 
                                                    lptimer_interrupt_handler);
    
    /* LPTimer interrupt initialization failed. Stop program execution. */
    if(CY_SYSINT_SUCCESS != interrupt_init_status)
    {
        handle_error();
    }

    /* Enable NVIC interrupt. */
    NVIC_EnableIRQ(lptimer_intr_cfg.intrSrc);

    /* Initialize the MCWDT block */
    cy_en_mcwdt_status_t mcwdt_init_status = 
                                    Cy_MCWDT_Init(CYBSP_CM33_LPTIMER_0_HW, 
                                                &CYBSP_CM33_LPTIMER_0_config);

    /* MCWDT initialization failed. Stop program execution. */
    if(CY_MCWDT_SUCCESS != mcwdt_init_status)
    {
        handle_error();
    }
  
    /* Enable MCWDT instance */
    Cy_MCWDT_Enable(CYBSP_CM33_LPTIMER_0_HW,
                    CY_MCWDT_CTR_Msk, 
                    LPTIMER_0_WAIT_TIME_USEC);

    /* Setup LPTimer using the HAL object and desired configuration as defined
     * in the device configurator. */
    cy_rslt_t result = mtb_hal_lptimer_setup(&lptimer_obj, 
                                            &CYBSP_CM33_LPTIMER_0_hal_config);
    
    /* LPTimer setup failed. Stop program execution. */
    if(CY_RSLT_SUCCESS != result)
    {
        handle_error();
    }

    /* Pass the LPTimer object to abstraction RTOS library that implements 
     * tickless idle mode 
     */
    cyabs_rtos_set_lptimer(&lptimer_obj);
}

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void handle_error(void)
{
    /* Disable all interrupts. */
    __disable_irq();
    
    app_log_print("Disable all the interrupts\r\n");
    CY_ASSERT(0);
}


/* [] END OF FILE */
