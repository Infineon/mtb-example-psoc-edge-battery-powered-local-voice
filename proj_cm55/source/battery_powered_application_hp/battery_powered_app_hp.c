/******************************************************************************
* File Name : battery_powered_app_hp.c
*
* Description :
* Battery Powered Audio application for CM55 core.
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

#include "app_logger.h"
#include "FreeRTOS.h"
#include "cy_syslib.h"
#include "task.h"
#include "timers.h"
#include "battery_powered_svc_hp.h"

#include "battery_powered_app_hp.h"
#include "battery_powered_inferencing.h"
#include "battery_powered_ae_interface.h"
#include "intercore_ipc.h"

#include "mtb_ml.h"

#include "ppudbg.h"

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
#include "audio_usb_send_utils.h"
#include "usb_audio_interface.h"
#endif /* CY_AFE_ENABLE_TUNING_FEATURE */

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/

extern void app_ipc_init_cm55(void);

#ifdef LOW_POWER_MODE
#include "ethosu_driver.h"
#if defined(COMPONENT_U55)
extern struct ethosu_driver ethosu_drv;
#endif /* COMPONENT_U55 */
#endif /* LOW_POWER_MODE */

bool usb_streaming = false;
/*******************************************************************************
* Function Name: battery_powered_app_hp
********************************************************************************
* Summary:
* Initialize CM55 audio application for battery powered use-case.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/


void battery_powered_app_hp()
{
    app_ipc_init_cm55();
    /* Inferencing Engine Initialization */
    battery_powered_inference_engine_init();
    /* Audio Enhancement Initialization */
    battery_powered_ae_init(AFE_INPUT_NUMBER_CHANNELS);

#if defined(VA_INFERENCING)
    app_svc_hp_app_init();
#endif /* VA_INFERENCING */

#if !defined(VA_INFERENCING)
    app_log_print("Important: Voice Assistant Inferencing not enabled - Enable it for CE to work with all functionality. \r\n");
#endif /* VA_INFERENCING */

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
    usb_audio_interface_init();
    usb_send_out_dbg_init_channels();
#endif /* CY_AFE_ENABLE_TUNING_FEATURE */

}
/* [] END OF FILE */
