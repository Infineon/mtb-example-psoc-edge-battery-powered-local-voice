/******************************************************************************
* File Name : battery_powered_inferencing.h
*
* Description :
* Header file for Inference processing on CM55 - battery use-cases.
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

#ifndef __BATTERY_POWERED_INFERENCING_H__
#define __BATTERY_POWERED_INFERENCING_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "cy_staged_voice_control_common.h"
#include "inferencing_interface.h"

/****************************************************************************
 * Functions Prototypes
 *****************************************************************************/
void app_request_post_wwd_data();
void app_inferencing_callback(const char *function, char *message, char *parameter);
void get_cur_svc_stage_and_force_model_stages(void);
void battery_powered_inference_feed(short *mono, cy_svc_buffer_info_t buffer_info);

void battery_powered_post_processing(int map_id);
void battery_powered_inference_engine_init();
void battery_powered_inference_engine_deinit();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BATTERY_POWERED_INFERENCING_H__ */

/* [] END OF FILE */
