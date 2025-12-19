/******************************************************************************
* File Name : cm55_intercore_pipe.c
*
* Description :
* Intercore IPC pipe on CM55 core for Staged Voice Control.
*
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

#include "cy_pdl.h"
#include "cycfg.h"
#include <stdio.h>
#include "cy_ipc_pipe.h"
#include "app_logger.h"
#include "intercore_ipc.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/

/*******************************************************************************
* Function Name: wake_cm33_from_cm55_before_send_ipc
********************************************************************************
* Summary:
*  Wake CM33 from CM55 before sending IPC
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void wake_cm33_from_cm55_before_send_ipc(void)
{
    Cy_IPC_Drv_SetInterrupt(
            Cy_IPC_Drv_GetIntrBaseAddr(
                    CM33_WAKEUP_IPC_INTR_NUM),
            0, CM33_WAKEUP_IPC_CH_MASK );
}
/*******************************************************************************
* Function Name: wake_cm33_from_cm55_after_send_ipc
********************************************************************************
* Summary:
*  Wake CM33 from CM55 after sending IPC
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void wake_cm33_from_cm55_after_send_ipc(void)
{
    Cy_IPC_Drv_SetInterrupt(
            Cy_IPC_Drv_GetIntrBaseAddr(
                    CM33_WAKEUP_IPC_INTR_NUM2),
            0, CM33_WAKEUP_IPC_CH_MASK2 );
}

/*******************************************************************************
* Function Name: cy_svc_hp_ipc_pre_send_hook
********************************************************************************
* Summary:
*  This the Staged Voice Control High Perfomance IPC Pre Send Hook.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void cy_svc_hp_ipc_pre_send_hook(void)
{
    wake_cm33_from_cm55_before_send_ipc();
}
/*******************************************************************************
* Function Name: cy_svc_hp_ipc_pre_send_hook
********************************************************************************
* Summary:
*  This the Staged Voice Control High Perfomance IPC Post Send Hook.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void cy_svc_hp_ipc_post_send_hook(void)
{
    wake_cm33_from_cm55_after_send_ipc();
}
/*******************************************************************************
* Function Name: cy_svc_hp_ipc_pre_send_hook
********************************************************************************
* Summary:
*  This the Staged Voice Control High Perfomance IPC Post Receive Hook.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void cy_svc_hp_ipc_post_recv_hook(void)
{
    
}

/*******************************************************************************
* Function Name: Cy_SysIpc1PipeIsrCm55
********************************************************************************
* Summary:
* This is the interrupt service routine for the system pipe.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void Cy_SysIpc1PipeIsrCm55(void)
{

    Cy_SysLib_Delay(10);
    app_log_print("IPC1: [CM55]EP[3] is interrupted \r\n");

    Cy_SysLib_Delay(10);

    Cy_IPC_Pipe_ExecuteCallback(CY_IPC_EP_CYPIPE_CM55_ADDR);
}

/*******************************************************************************
* Function Name: Cy_SysIpcPipeIsrCm55
********************************************************************************
* Summary:
* This is the interrupt service routine for the system pipe.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void Cy_SysIpcPipeIsrCm55(void)
{

    Cy_IPC_Pipe_ExecuteCallback(CY_IPC_EP_CYPIPE_CM55_ADDR);
}

/*******************************************************************************
* Function Name: ipc_pipe_cm33ns_cm55
********************************************************************************
* Summary:
*  IPC pipe code
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ipc_pipe_cm33ns_cm55(void)
{
/*    * 1. CM33 boots CM55
      * 2. CM55 waits for message from CM33 in Pipe-0
      * 3. CM33 sends message to CM55
      * 4. Once CM55 receives message from CM33 in Pipe-0 then it sends message to
      *     CM33 in Pipe-0.
*/

    /* Create an array of endpoint structures */
    static cy_stc_ipc_pipe_ep_t systemIpcPipeEpArray[CY_IPC_MAX_ENDPOINTS] = {0};

    Cy_IPC_Pipe_Config(systemIpcPipeEpArray);

    app_log_print("[CM55] IPC Application Main...\r\n");

    static cy_ipc_pipe_callback_ptr_t ep2CbArray[CY_IPC_CYPIPE_CLIENT_CNT]; /* CB Array for EP2 */

    /* Pipe-2 endpoint-1 and endpoint-2. CM55 <--> CM33 */
    static const cy_stc_ipc_pipe_config_t systemIpcPipe2ConfigCm55 =
    {
    /* receiver endpoint CM55 */
        {
            /* .ipcNotifierNumber     */  CY_IPC_INTR_CYPIPE_EP2,
            /* .ipcNotifierPriority  */  CY_IPC_INTR_CYPIPE_PRIOR_EP2,
            /* .ipcNotifierMuxNumber */  CY_IPC_INTR_CYPIPE_MUX_EP2,
            /* .epAddress             */  CY_IPC_EP_CYPIPE_CM55_ADDR,
            {
                /* .epChannel */CY_IPC_CHAN_CYPIPE_EP2,
                /* .epIntr */ CY_IPC_INTR_CYPIPE_EP2,
                /* .epIntrmask */ CY_IPC_CYPIPE_INTR_MASK
            }
        },
    /* sender endpoint CM33 */
        {
            /* .ipcNotifierNumber     */  CY_IPC_INTR_CYPIPE_EP1,
            /* .ipcNotifierPriority  */  CY_IPC_INTR_CYPIPE_PRIOR_EP1,
            /* .ipcNotifierMuxNumber */  CY_IPC_INTR_CYPIPE_MUX_EP1,
            /* .epAddress             */  CY_IPC_EP_CYPIPE_CM33_ADDR,
            {
                /* .epChannel */CY_IPC_CHAN_CYPIPE_EP1,
                /* .epIntr */ CY_IPC_INTR_CYPIPE_EP1,
                /* .epIntrmask */ CY_IPC_CYPIPE_INTR_MASK
            }
        },
    /* .endpointClientsCount     */  CY_IPC_CYPIPE_CLIENT_CNT,
    /* .endpointsCallbacksArray  */  ep2CbArray,
    /* .userPipeIsrHandler         */  &Cy_SysIpcPipeIsrCm55
    };

#if defined (__ARMCC_VERSION)
    app_log_print ("[CM55][EP2] IPC Channel-%d address[0x%x] Intr-%ld address[0x%x] \r\n", CY_IPC_CHAN_CYPIPE_EP2,
                                                                                    (uint32_t)Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_CYPIPE_EP2),
                                                                                    (long)CY_IPC_INTR_CYPIPE_EP2,
                                                                                    (uint32_t)Cy_IPC_Drv_GetIntrBaseAddr(CY_IPC_INTR_CYPIPE_EP2));
#else
    app_log_print ("[CM55][EP2] IPC Channel-%d address[0x%lx] Intr-%d address[0x%lx] \r\n", CY_IPC_CHAN_CYPIPE_EP2,
                                                                                    (uint32_t)Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_CYPIPE_EP2),
                                                                                    CY_IPC_INTR_CYPIPE_EP2,
                                                                                    (uint32_t)Cy_IPC_Drv_GetIntrBaseAddr(CY_IPC_INTR_CYPIPE_EP2));
#endif // __ARMCC_VERSION

    Cy_IPC_Pipe_Init(&systemIpcPipe2ConfigCm55); /* PIPE-2 EP2 <--> EP1 */
    app_log_print("\r ============================END============================== \r\n");

}



/*******************************************************************************
* Function Name: app_ipc_init_cm55
********************************************************************************
* Summary:
* Initializes IPC pipe on CM55 for Staged Voice Control.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_ipc_init_cm55(void)
{
    ipc_pipe_cm33ns_cm55();
}

/* [] END OF FILE */
