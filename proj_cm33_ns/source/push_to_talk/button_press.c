/******************************************************************************
* File Name : cy_button_press.c
*
* Description :
* Code for controlling user button.
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

#include "button_press.h"
#include "cybsp.h"
#include "cy_gpio.h"
#include "FreeRTOS.h"
#include "timers.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define USER_BUTTON_DELAY                (500)
#define USER_BUTTON_POLL_DELAY           (100)

#define USER_BTN_1_INIT_VAL               (1u)
#define USER_BTN_1_IRQ_MASK               (1u)

/* Non-secure button interrupt priority */
#define USER_BTN_1_ISR_PRIORITY           (5u)
#define PORT_INTR_MASK                    (0x00000001UL << 8U)
#define INTERRUPT_MASKED                  (1U)
#define BTN_DEBOUNCE_INTERVAL_MS          (300u)

#define CYBSP_USER_BTN2_PORT    GPIO_PRT8
#define CYBSP_USER_BTN2_PIN     (7U)
#define CYBSP_USER_BTN2_IRQ     CYBSP_USER_BTN1_IRQ

/* GPIO configuration for BTN2  mirrors the BSP SW2 (BTN1) config */
static const cy_stc_gpio_pin_config_t s_btn2_gpio_config =
{
    .outVal      = 1U,
    .driveMode   = CY_GPIO_DM_PULLUP,
    .hsiom       = HSIOM_SEL_GPIO,
    .intEdge     = CY_GPIO_INTR_FALLING,
    .intMask     = 1UL,
    .vtrip       = CY_GPIO_VTRIP_CMOS,
    .slewRate    = CY_GPIO_SLEW_FAST,
    .driveSel    = CY_GPIO_DRIVE_1_2,
    .vregEn      = 0UL,
    .ibufMode    = 0UL,
    .vtripSel    = 0UL,
    .vrefSel     = 0UL,
    .vohSel      = 0UL,
    .pullUpRes   = CY_GPIO_PULLUP_RES_DISABLE,
    .nonSec      = 1U,
};

/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool gpio_intr_flag = false;

/* Per-button pressed flags set inside the ISR */
static volatile bool user_btn1_pressed = false;
static volatile bool user_btn2_pressed = false;

static TimerHandle_t btn_debounce_timer;

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/
static void button_interrupt_handler(void);
void (*button_callback)(void);
void (*button2_callback)(void);
static void btn_debounce_timer_callback(TimerHandle_t xTimer);
/*******************************************************************************
* Function Name: user_button_init
********************************************************************************
* Summary:
* Initialize user button (BTN1) and User Button 2 (BTN2).
*
* Parameters:
*  arg - callback for BTN1 press events
*
* Return:
*  None
*
*******************************************************************************/

void user_button_init(cb_user_action arg)
{
    button_callback = (cb_user_action)arg;

    /* Interrupt config structure - BTN1 and BTN2 are both on port 8 and share
     * the same NVIC interrupt line. */
    cy_stc_sysint_t intrCfg =
    {
        .intrSrc = CYBSP_USER_BTN1_IRQ,
        .intrPriority = USER_BTN_1_ISR_PRIORITY
    };

    /* Clear GPIO and NVIC interrupt before initializing to avoid false
     * triggering. */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_PIN);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN1_IRQ);

    /* Initialize BTN2 GPIO (SW4, port 8 pin 7)  not configured by the BSP */
    Cy_GPIO_Pin_Init(CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_PIN, &s_btn2_gpio_config);
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_PIN);

    /* Initialize the interrupt and register interrupt callback */
    Cy_SysInt_Init(&intrCfg, &button_interrupt_handler);

    /* Create the FreeRTOS timers for debouncing and long press detection */
    btn_debounce_timer = xTimerCreate("Debounce Timer",
                                      pdMS_TO_TICKS(BTN_DEBOUNCE_INTERVAL_MS),
                                      pdFALSE,
                                      (void *) 0,
                                      btn_debounce_timer_callback);
    CY_ASSERT(btn_debounce_timer != NULL);

    /* Enable the interrupt in the NVIC */
    NVIC_EnableIRQ(intrCfg.intrSrc);
}

/*******************************************************************************
* Function Name: user_button2_register
********************************************************************************
* Summary:
* Register a callback for User Button 2 (BTN2) press events.
*
* Parameters:
*  arg - callback to invoke on BTN2 press
*
* Return:
*  None
*
*******************************************************************************/
void user_button2_register(cb_user_action arg)
{
    button2_callback = (cb_user_action)arg;
}

/*******************************************************************************
* Function Name: button_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler for User Buttons (BTN1 and BTN2 share port 8).
*
* Parameters:
*  void
*
*******************************************************************************/
static void button_interrupt_handler(void)
{
    /* Get interrupt cause */
    uint32_t interrupt_cause = Cy_GPIO_GetInterruptCause0();

    /* Check if the interrupt was from port 8 (shared by BTN1 and BTN2) */
    if (PORT_INTR_MASK == (interrupt_cause & PORT_INTR_MASK))
    {
        /* BTN1 (port 8, pin 3) */
        if (INTERRUPT_MASKED == Cy_GPIO_GetInterruptStatusMasked(CYBSP_USER_BTN1_PORT,
                CYBSP_USER_BTN1_PIN))
        {
            Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_PIN);
            NVIC_ClearPendingIRQ(CYBSP_USER_BTN1_IRQ);
            user_btn1_pressed = true;
        }

        /* BTN2 (port 8, pin 7) */
        if (INTERRUPT_MASKED == Cy_GPIO_GetInterruptStatusMasked(CYBSP_USER_BTN2_PORT,
                CYBSP_USER_BTN2_PIN))
        {
            Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_PIN);
            user_btn2_pressed = true;
        }
    }

    /* Disable the port 8 interrupt until debounce timer fires */
    NVIC_DisableIRQ(CYBSP_USER_BTN1_IRQ);

    /* Reset the debouncing timer */
    xTimerStopFromISR(btn_debounce_timer, NULL);
    xTimerResetFromISR(btn_debounce_timer, NULL);
    xTimerStartFromISR(btn_debounce_timer, NULL);
}

/******************************************************************************
* Function Name: btn_debounce_timer_callback
*******************************************************************************
* Summary:
*   Timer callback for User Button debouncing.
*
* Parameters:
*   xTimer : Timer handle
*
* Return:
*   None
*
*******************************************************************************/
static void btn_debounce_timer_callback(TimerHandle_t xTimer)
{
    (void) xTimer;

    if (user_btn2_pressed && (button2_callback != NULL))
    {
        /* BTN2 press  always invoke the callback (toggle logic is in the
         * callback itself, e.g. streaming mode enter/exit). */
        user_btn2_pressed = false;
        button2_callback();
    }
    else if (gpio_intr_flag == false)
    {
        /* BTN1 first edge (press) */
        gpio_intr_flag = true;
        user_btn1_pressed = false;
        button_callback();
    }
    else
    {
        /* BTN1 second edge (release / debounce settle) */
        gpio_intr_flag = false;
        user_btn1_pressed = false;
    }

    /* Clear any pending BTN1 interrupt and re-enable the shared NVIC line */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_PIN);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN1_IRQ);
    NVIC_EnableIRQ(CYBSP_USER_BTN1_IRQ);
}

/* [] END OF FILE */