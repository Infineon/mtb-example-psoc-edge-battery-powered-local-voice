/******************************************************************************
* File Name : battery_powered_svc_hp.h
*
* Description :
* Header file for Staged Voice Control on CM55.
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
#ifndef __BATTERY_POWERED_SVC_HP_H__
#define __BATTERY_POWERED_SVC_HP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cybsp.h"
#include "cy_retarget_io.h"

#include "stdlib.h"
#include "cy_log.h"
#include "cy_staged_voice_control.h"
#ifdef COMPONENT_QBUF_POOL
#include "cy_qbuf_pool.h"
#endif /* COMPONENT_QBUF_POOL*/


/*******************************************************************************
* Macros
*******************************************************************************/
#define APP_SVC_HP_LOG_ENABLE                (0)

#if APP_SVC_HP_LOG_ENABLE
#define app_svc_log_hp(format,...)            printf(format "\r\n",##__VA_ARGS__);
#else
#define app_svc_log_hp(format,...)
#endif /* APP_SVC_HP_LOG_ENABLE */

/*******************************************************************************
* Global Variables
*******************************************************************************/

typedef struct
{
    TaskHandle_t thread;
    QueueHandle_t app_cmd_queue;
    bool init_done;
    unsigned int total_frame_received_count;
} svc_hp_app_t;

typedef struct __attribute__((packed, aligned(4)))
{
    uint8_t     cmd_id;                     /* Byte 1       - Command identifier    */
    uint8_t     dbg_message_counter;        /* Byte 2       - message counter       */
    uint8_t     bitmask;                    /* Byte 3       - bit mask  for any purpose */
    uint8_t     reserved;                   /* Byte 4                               */
    cy_svc_buffer_info_t     buffer_info;   /* Byte 5 - 8  */
    uint8_t      *data_pointer;             /* Byte 9 - 12   - Data pointer          */
    uint32_t    frame_count;                /* Byte 13 - 16  - Frame count           */
} svc_hp_app_cmd_q_data_t ;

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/

void app_svc_hp_app_init(void);
void app_svc_hp_clear_queue(void);

#ifdef __cplusplus
} /* extern C */
#endif /* __cplusplus */

#endif /* __BATTERY_POWERED_SVC_HP_H__ */

/* [] END OF FILE */

