/******************************************************************************
* File Name : audio_feed_to_svc.h
*
* Description :
* Header file for audio feed into Staged Voice Control Middleware
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
#ifndef __AUDIO_FEED_TO_SVC_H__
#define __AUDIO_FEED_TO_SVC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cybsp.h"
#include "cy_retarget_io.h"

#include "stdlib.h"
#include "cy_log.h"
#include "cy_buffer_pool.h"
#include "cy_staged_voice_control.h"

/*******************************************************************************
* Macros
*******************************************************************************/

#define ENABLE_APP_SVC_LOG                  (0)

/* SOD to AAD constants */
#define SOD_TO_AAD_TIMEOUT_MS               (7000U)
#define SOD_TO_AAD_THRESHOLD                (7000U)

#if ENABLE_APP_SVC_LOG
#define app_svc_log(format, ...)            printf(format "\r\n",##__VA_ARGS__);
#else
#define app_svc_log(format, ...)
#endif

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/
cy_rslt_t app_init_svc(void);
cy_rslt_t app_deinit_svc(void);

#ifdef HP_INFERENCING_ENABLE
void disable_cm55_n_turn_off_pd1_on_svc_event(void);
void Enable_pd1_n_relaunch_cm55(void);
#endif /* HP_INFERENCING_ENABLE */

#ifdef __cplusplus
} /* extern C */
#endif /* __cplusplus */

#endif /* __AUDIO_FEED_TO_SVC_H__ */

/* [] END OF FILE */

