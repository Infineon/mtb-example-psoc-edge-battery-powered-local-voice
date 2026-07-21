/******************************************************************************
* File Name : battery_powered_ae_interface.h
*
* Description :
* Header file for Audio Enhancement wrapper - battery powered use-case.
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

#ifndef __BATTERY_POWERED_AE_INTERFACE_H__
#define __BATTERY_POWERED_AE_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "audio_enhancement.h"
#include "cy_staged_voice_control_common.h"

#include "FreeRTOS.h"
#include "queue.h"

#ifdef PROFILER_ENABLE
#include "cy_afe_profiler.h"
#include "cy_profiler.h"
#endif /* PROFILER_ENABLE */

/*******************************************************************************
* Macros
*******************************************************************************/
#define APP_AE_LOG_ENABLE                             (1)
#if APP_AE_LOG_ENABLE
#define app_ae_log(format,...)                        printf(format "\r\n",##__VA_ARGS__);
#else
#define app_ae_log(format,...)
#endif /* APP_AE_LOG_ENABLE */

#define NO_OF_CHANNELS_RECEIVED                       (AFE_INPUT_NUMBER_CHANNELS)
#define NO_OF_BYTES_PER_SAMPLE                        (2)
#define MONO_AUDIO_DATA_IN_BYTES                      (320)
#define STEREO_AUDIO_DATA_IN_BYTES                    (640)
#define AE_APP_FAILURE                                (-1)
#define AE_APP_PROFILE                                (0)
#define INFERENCING_PROFILE                           (0)

/*******************************************************************************
* Extern Variables
*******************************************************************************/
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
extern QueueHandle_t ae_usb_queue_handle;
#endif /* CY_AFE_ENABLE_TUNING_FEATURE */

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
int battery_powered_ae_feed(void* audio_input, void* aec_buffer, cy_svc_buffer_info_t buffer_info);
int battery_powered_ae_init(uint8_t channels);
void license_limitation_exit();
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
void bplv_ae_usb_task(void *arg);
#endif /* CY_AFE_ENABLE_TUNING_FEATURE */

#ifdef __cplusplus
} /*extern "C" */
#endif  /* __cplusplus */
#endif /* __BATTERY_POWERED_AE_INTERFACE_H__ */

/* [] END OF FILE */
