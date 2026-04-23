/******************************************************************************
* File Name : intercore_ipc.h
*
* Description :
* Header for intercore IPC - Staged Voice Control
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

#ifndef __INTERCORE_IPC_H__
#define __INTERCORE_IPC_H__
/* For PSOC Edge, there are 32 channels and 16 interrupts are available */

/*******************************************************************************
* Macros
*******************************************************************************/

/* change this macro for valid usecase */
#define CM55_PRINT_ON                           (1)

#define CY_IPC_MAX_ENDPOINTS                    (5UL) /* <<< 3 endpoints */
#define CY_IPC_CYPIPE_CLIENT_CNT                (8UL)

/* Channel 2 goes to IPC0 */
#define CY_IPC_CHAN_CYPIPE_EP1                  (CM33_NS_IPC0_CH_NUM) /* IPC data channel for CYPIPE EP1 */
#define CY_IPC_CYPIPE_CHAN_MASK_EP1             (CM33_NS_IPC0_CH_MASK)
#define CY_IPC_INTR_CYPIPE_EP1                  (CM33_NS_IPC0_INTR_NUM)   /* IPC Intr for EP1 */
#define CY_IPC_CYPIPE_INTR_MASK_EP1             (CM33_NS_IPC0_INTR_MASK)   /* IPC Intr Mask for EP0 */
#define CY_IPC_INTR_CYPIPE_PRIOR_EP1            (5UL)   /* Notifier Priority */
#define CY_IPC_INTR_CYPIPE_MUX_EP1              (CM33_NS_IPC0_INTR_MUX)   /* Intr Mux for CM33 */
#define CY_IPC_EP_CYPIPE_CM33_ADDR              (1UL)   /* EP1 Index of Endpoint Array */
#define CY_CLIENT_CYPIPE2_CM33_ID1              (3UL)   /* EP1 Pipe2 (CM33 <--> CM55) Index of client cb Array */

/* Channel 2 goes to IPC0 */
#define CY_IPC_CHAN_CYPIPE_EP2                  (CY_IPC_CHAN_CYPIPE_EP1+1) /* IPC data channel for CYPIPE EP2 */
#define CY_IPC_CYPIPE_CHAN_MASK_EP2             (CY_IPC_CH_MASK(CY_IPC_CHAN_CYPIPE_EP2))
#define CY_IPC_INTR_CYPIPE_EP2                  (CY_IPC_INTR_CYPIPE_EP1+1)
#define CY_IPC_CYPIPE_INTR_MASK_EP2             (CY_IPC_INTR_MASK(CY_IPC_INTR_CYPIPE_EP2))   /* IPC Intr Mask for EP2 */
#define CY_IPC_INTR_CYPIPE_PRIOR_EP2            (5UL)   /* Notifier Priority */
#define CY_IPC_INTR_CYPIPE_MUX_EP2              (CY_IPC0_INTR_MUX(CY_IPC_INTR_CYPIPE_EP2))   /* Intr Mux for CM33 */
#define CY_IPC_EP_CYPIPE_CM55_ADDR              (2UL)   /* EP2 Index of Endpoint Array */
#define CY_CLIENT_CYPIPE2_CM55_ID1              (5UL)   /* EP2 Pipe2 (CM55 <--> CM33) Index of client cb Array */


#define APP_LP_PIPE_END_POINT_ADDRESS           (CY_IPC_EP_CYPIPE_CM33_ADDR)
#define APP_LP_PIPE_END_POINT_CLIENT_ID_1       (CY_CLIENT_CYPIPE2_CM33_ID1)
#define APP_HP_PIPE_END_POINT_ADDRESS           (CY_IPC_EP_CYPIPE_CM55_ADDR)
#define APP_HP_PIPE_END_POINT_CLIENT_ID_2       (CY_CLIENT_CYPIPE2_CM55_ID1)


#define IPC_PRIORITY                            (1u)
#define CM33_WAKEUP_IPC_CH_NUM                  (18) 
#define CM33_WAKEUP_IPC_INTR_NUM                (13) 

#define CM33_WAKEUP_IPC_CH_MASK                 CY_IPC_CH_MASK(CM33_WAKEUP_IPC_CH_NUM)
#define CM33_WAKEUP_IPC_INTR_MASK               CY_IPC_INTR_MASK(CM33_WAKEUP_IPC_INTR_NUM)
#define CM33_WAKEUP_IPC_INTR_MUX                CY_IPC_INTR_MUX(CM33_WAKEUP_IPC_INTR_NUM)

#define CM33_WAKEUP_IPC_CH_NUM2                 (19)
#define CM33_WAKEUP_IPC_INTR_NUM2               (14)
#define CM33_WAKEUP_IPC_CH_MASK2                CY_IPC_CH_MASK(CM33_WAKEUP_IPC_CH_NUM2)
#define CM33_WAKEUP_IPC_INTR_MASK2              CY_IPC_INTR_MASK(CM33_WAKEUP_IPC_INTR_NUM2)
#define CM33_WAKEUP_IPC_INTR_MUX2               CY_IPC_INTR_MUX(CM33_WAKEUP_IPC_INTR_NUM2)

/*
For Explorer device:
      Channels   Interrupts
IPC0:   0 - 15     0 -  7
IPC1:  16 - 31     8 - 15
*/
#define CY_IPC_CYPIPE_INTR_MASK   ( CY_IPC_CYPIPE_CHAN_MASK_EP1 | CY_IPC_CYPIPE_CHAN_MASK_EP2)



#endif /* __INTERCORE_IPC_H__ */
