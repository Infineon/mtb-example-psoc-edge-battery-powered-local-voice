/******************************************************************************
* File Name : cm33_intercore_pipe.c
*
* Description :
* Intercore IPC pipe on CM33 core for Staged Voice Control.
*
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

#include "cy_pdl.h"
#include "cycfg.h"
#include <stdio.h>
#include "cy_ipc_pipe.h"
#include "app_logger.h"
#include "intercore_ipc.h"

#include "cy_staged_voice_control.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/

const cy_stc_sysint_t ipcWakeIntConfig =
{
    .intrSrc = CY_IPC_INTR_MUX(CM33_WAKEUP_IPC_INTR_NUM),
    .intrPriority = IPC_PRIORITY,
};

const cy_stc_sysint_t ipcWakeIntConfig2 =
{
    .intrSrc = CY_IPC_INTR_MUX(CM33_WAKEUP_IPC_INTR_NUM2),
    .intrPriority = IPC_PRIORITY,
};

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void wakeup_cm33_lock_deepsleep(void);
static void wakeup_cm33_unlock_deepsleep(void);

/*******************************************************************************
* Function Name: cy_svc_lp_ipc_pre_send_hook
********************************************************************************
* Summary:
*  This the Staged Voice Control Low Power IPC Pre Send Hook.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void cy_svc_lp_ipc_pre_send_hook(void)
{
    mtb_hal_syspm_lock_deepsleep();
}
/*******************************************************************************
* Function Name: cy_svc_lp_ipc_post_send_hook
********************************************************************************
* Summary:
*  This the Staged Voice Control Low Power IPC Post Send Hook.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void cy_svc_lp_ipc_post_send_hook(void)
{
    mtb_hal_syspm_unlock_deepsleep();
}
/*******************************************************************************
* Function Name: cy_svc_lp_ipc_post_recv_hook
********************************************************************************
* Summary:
*  This the Staged Voice Control Low Power IPC Post Receive Hook.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void cy_svc_lp_ipc_post_recv_hook(void)
{
    
}


/*******************************************************************************
* Function Name: Cy_SysIpcPipeIsrCm33
********************************************************************************
* Summary:
*  This is the interrupt service routine for the system pipe.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void Cy_SysIpcPipeIsrCm33(void)
{
    Cy_IPC_Pipe_ExecuteCallback(CY_IPC_EP_CYPIPE_CM33_ADDR);
}

/*******************************************************************************
* Function Name: ipc_pipe_usecase4_cm33ns_cm55
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
    static cy_stc_ipc_pipe_ep_t systemIpcPipeEpArray[CY_IPC_MAX_ENDPOINTS];

    Cy_IPC_Pipe_Config(systemIpcPipeEpArray);

    static cy_ipc_pipe_callback_ptr_t ep1CbArray[CY_IPC_CYPIPE_CLIENT_CNT]; /* CB Array for EP1 */

    /* Pipe-2 endpoint-1 and endpoint-2. CM33 <--> CM55 */
    static const cy_stc_ipc_pipe_config_t systemIpcPipe2ConfigCm33 =
    {
    /* receiver endpoint CM33 */
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
    /* sender endpoint CM55 */
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
    /* .endpointClientsCount     */  CY_IPC_CYPIPE_CLIENT_CNT,
    /* .endpointsCallbacksArray  */  ep1CbArray,
    /* .userPipeIsrHandler         */  &Cy_SysIpcPipeIsrCm33
    };

    Cy_IPC_Pipe_Init(&systemIpcPipe2ConfigCm33); /* PIPE-2 EP1 <--> EP2 */

    Cy_SysInt_Init(&ipcWakeIntConfig, wakeup_cm33_lock_deepsleep);
    Cy_SysInt_Init(&ipcWakeIntConfig2, wakeup_cm33_unlock_deepsleep);

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

}

/*******************************************************************************
* Function Name: app_ipc_init_cm33
********************************************************************************
* Summary:
* Initializes IPC pipe on CM33 for Staged Voice Control.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_ipc_init_cm33(void)
{
    ipc_pipe_cm33ns_cm55();
}

/*******************************************************************************
* Function Name: wakeup_cm33_lock_deepsleep
********************************************************************************
* Summary:
* Initializes IPC pipe on CM33 for Staged Voice Control.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void wakeup_cm33_lock_deepsleep(void)
{
    uint32_t shadowIntr;
    IPC_INTR_STRUCT_Type *ipcIntrPtr;

    ipcIntrPtr = Cy_IPC_Drv_GetIntrBaseAddr(CM33_WAKEUP_IPC_INTR_NUM);
    shadowIntr = Cy_IPC_Drv_GetInterruptStatusMasked(ipcIntrPtr);
    
    mtb_hal_syspm_lock_deepsleep();

    /* Check to make sure the interrupt was a notify interrupt */
    if (0UL != Cy_IPC_Drv_ExtractAcquireMask(shadowIntr))
    {
        /* Clear the notify interrupt.  */
        Cy_IPC_Drv_ClearInterrupt(ipcIntrPtr, CY_IPC_NO_NOTIFICATION, Cy_IPC_Drv_ExtractAcquireMask(shadowIntr));
    }

}

/*******************************************************************************
* Function Name: wakeup_cm33_unlock_deepsleep
********************************************************************************
* Summary:
* Initializes IPC pipe on CM33 for Staged Voice Control.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void wakeup_cm33_unlock_deepsleep(void)
{
    uint32_t shadowIntr;
    IPC_INTR_STRUCT_Type *ipcIntrPtr;

    ipcIntrPtr = Cy_IPC_Drv_GetIntrBaseAddr(CM33_WAKEUP_IPC_INTR_NUM2);
    shadowIntr = Cy_IPC_Drv_GetInterruptStatusMasked(ipcIntrPtr);
    
    mtb_hal_syspm_unlock_deepsleep();

    /* Check to make sure the interrupt was a notify interrupt */
    if (0UL != Cy_IPC_Drv_ExtractAcquireMask(shadowIntr))
    {
        /* Clear the notify interrupt.  */
        Cy_IPC_Drv_ClearInterrupt(ipcIntrPtr, CY_IPC_NO_NOTIFICATION, Cy_IPC_Drv_ExtractAcquireMask(shadowIntr));
    }

}

/*******************************************************************************
* Function Name: Cy_NNLite_Lpm_Lock
********************************************************************************
* Summary:
* 
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Cy_NNLite_Lpm_Lock(void)
{
    mtb_hal_syspm_lock_deepsleep();    
}

/*******************************************************************************
* Function Name: Cy_NNLite_Lpm_Unlock
********************************************************************************
* Summary:
* 
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Cy_NNLite_Lpm_Unlock(void)
{
    mtb_hal_syspm_unlock_deepsleep();
}
/* [] END OF FILE */
