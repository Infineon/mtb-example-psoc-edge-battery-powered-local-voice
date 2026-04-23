/******************************************************************************
* File Name:   security_config.c
*
* Description: This is the security configuration applied by the CM33 secure project
*
*
*******************************************************************************
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
#include "cy_pdl.h"
#include "cybsp.h"
#include "partition_ARMCM33.h"
#include "cy_ppc.h"
#include "cy_mpc.h"
#include "cy_ms_ctl.h"
#include "ifx_se_platform.h"
#include "ifx_se_crc32.h"
#include "cy_syspm_pdcm.h"
#include "security_config.h"

/*****************************************************************************
* Global variables
*****************************************************************************/

/*  configuration structures for PPC (Peripheral Protection Controller) */
cy_stc_ppc_attribute_t ppcAttribute;

const cy_en_ppc_resp_cfg_t cy_response_ppc = CY_PPC_BUS_ERR;
/*****************************************************************************
* Function Name: config_socmemram_ns
******************************************************************************
* Summary:
* Configures the MPC for SOCMEM non secure regions
*
* Parameters:
* none
*
* Return:
* void
*****************************************************************************/
void config_socmemram_ns(void)
{
    cy_stc_mpc_rot_cfg_t rotConfig;

    rotConfig.secure = CY_MPC_NON_SECURE ;
    rotConfig.access = CY_MPC_ACCESS_RW ;

    rotConfig.pc = CY_MPC_PC_2 ;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SOCMEM_SRAM_MPC0, CYMEM_CM55_0_m55_code_secondary_OFFSET, CY_SOCMEMSRAM_NS_SIZE, &rotConfig);
    rotConfig.pc = CY_MPC_PC_5 ;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SOCMEM_SRAM_MPC0, CYMEM_CM55_0_m55_code_secondary_OFFSET, CY_SOCMEMSRAM_NS_SIZE, &rotConfig);
    rotConfig.pc = CY_MPC_PC_6 ;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SOCMEM_SRAM_MPC0, CYMEM_CM55_0_m55_code_secondary_OFFSET, CY_SOCMEMSRAM_NS_SIZE, &rotConfig);
}


/*****************************************************************************
* Function Name: config_smif0_xip_ns
******************************************************************************
* Summary:    Configures the MPC for non secure SMIF0 regions
*
* Parameters:
* none
*
* Return:
* void
*****************************************************************************/
 void config_smif0_xip_ns(void)
{
    cy_stc_mpc_rot_cfg_t rotConfig;
    
    rotConfig.pc = CY_MPC_PC_2;
    rotConfig.secure = CY_MPC_NON_SECURE;
    rotConfig.access = CY_MPC_ACCESS_RW;

    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0, CYMEM_CM55_0_m55_nvm_1_OFFSET, CYMEM_CM55_0_m55_nvm_1_SIZE + CYMEM_CM55_0_m55_trailer_SIZE, &rotConfig);

    rotConfig.pc = CY_MPC_PC_5;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0, CYMEM_CM55_0_m55_nvm_1_OFFSET,CYMEM_CM55_0_m55_nvm_1_SIZE + CYMEM_CM55_0_m55_trailer_SIZE, &rotConfig);

    rotConfig.pc = CY_MPC_PC_6;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0, CYMEM_CM55_0_m55_nvm_1_OFFSET,CYMEM_CM55_0_m55_nvm_1_SIZE + CYMEM_CM55_0_m55_trailer_SIZE, &rotConfig);
    

    rotConfig.pc = CY_MPC_PC_2;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0, CYMEM_CM55_0_m55_upgrade_slot_OFFSET,CYMEM_CM55_0_m55_upgrade_slot_SIZE, &rotConfig);

    rotConfig.pc = CY_MPC_PC_5;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0, CYMEM_CM55_0_m55_upgrade_slot_OFFSET,CYMEM_CM55_0_m55_upgrade_slot_SIZE, &rotConfig);

    rotConfig.pc = CY_MPC_PC_6;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0, CYMEM_CM55_0_m55_upgrade_slot_OFFSET,CYMEM_CM55_0_m55_upgrade_slot_SIZE, &rotConfig);

    rotConfig.pc = CY_MPC_PC_6;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SMIF0_CORE_AXI_MPC0, CYMEM_CM55_0_m55_nvm_1_OFFSET,CY_SMIF0_MAIN_NVM_NS_SIZE, &rotConfig);
}

 void config_smif0_xip_nsc(void)
{
    cy_stc_mpc_rot_cfg_t rotConfig;

    cy_stc_smif_context_t SMIF_CORE1_Context;

     const cy_stc_smif_config_t SMIF_0_CORE_1_config =
    {
        .mode = (uint32_t)CY_SMIF_NORMAL,
        .deselectDelay = SMIF_0_CORE_0_DESELECT_DELAY,
        .blockEvent = (uint32_t)CY_SMIF_BUS_ERROR,
        .enable_internal_dll = false
    };

    /* Enable IP with default configuration */

    (void) Cy_SMIF_Init(SMIF0_CORE, &SMIF_0_CORE_1_config, TIMEOUT_1_MS, &SMIF_CORE1_Context);
    Cy_SMIF_Enable(SMIF0_CORE, &SMIF_CORE1_Context);

    rotConfig.pc = CY_MPC_PC_2 ;
    rotConfig.secure = CY_MPC_NON_SECURE ;
    rotConfig.access = CY_MPC_ACCESS_RW ;
    
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0, CYMEM_CM55_0_m55_nvm_1_OFFSET, CY_SMIF0_MAIN_NVM_NS_SIZE, &rotConfig);

    rotConfig.pc = CY_MPC_PC_5 ;
    Cy_Mpc_ConfigRotMpcStruct((MPC_Type*)SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0, CYMEM_CM55_0_m55_nvm_1_OFFSET, CY_SMIF0_MAIN_NVM_NS_SIZE, &rotConfig);

    /* Disable IP as MPC configuration is complete*/
     Cy_SMIF_Disable(SMIF0_CORE);
}
/* [] END OF FILE */
