/*******************************************************************************
 * File Name        : nsc_func.c
 *
 * Description      : This file contains functions related to nsc.
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
 ******************************************************************************/
#include "nsc_func.h"
#include <arm_cmse.h>
#include "cy_pdl.h"
#include "cy_ms_ctl.h"
#include "cy_mpc.h"
#include "security_config.h"
#include "cybsp.h"
#include "cy_ppc.h"
/*
* Declared structures here but they can be in the 
* reapply_pd1_secure_configs itself
*/
#define SIZEOFPPC0                        (11u)
#define SIZEOFPPC1                        (39u)

cy_en_prot_region_t reapply_pd1_ppc_0_regions[SIZEOFPPC0] =
{
    /*
    * This is the minimum set of PPC to be re-set as Secure for code in
    * reapply_pd1_secure_configs to work
    */
    PROT_PERI0_SRSS_GENERAL,
    PROT_PERI0_SRSS_MAIN,
    PROT_PERI0_PWRMODE_PWRMODE,
    PROT_PERI0_RAMC0_RAM_PWR,
    PROT_PERI0_RAMC1_RAM_PWR,
    PROT_PERI0_GR0_GROUP,
    PROT_PERI0_GR1_GROUP,
    PROT_PERI0_GR2_GROUP,
    PROT_PERI0_GR3_GROUP,
    PROT_PERI0_GR4_GROUP,
    PROT_PERI0_GR5_GROUP

};

cy_en_prot_region_t reapply_pd1_ppc_1_regions[SIZEOFPPC1] =
{
    /*
    * This is the minimum set of PPC to be re-set as Secure for code in
    * reapply_pd1_secure_configs to work
    */
    PROT_PERI1_GR0_GROUP,            //= 0x10000001u, /* Peri 1, Address 0x44004010, size 0x00000008 */
    PROT_PERI1_GR1_GROUP,           // = 0x10000002u, /* Peri 1, Address 0x44004040, size 0x00000020 */
    PROT_PERI1_GR2_GROUP,           // = 0x10000003u, /* Peri 1, Address 0x44004090, size 0x00000008 */
    PROT_PERI1_GR3_GROUP,           // = 0x10000004u, /* Peri 1, Address 0x440040c0, size 0x00000020 */
    PROT_PERI1_GR4_GROUP,          //  = 0x10000005u, /* Peri 1, Address 0x44004110, size 0x00000008 */
    PROT_PERI1_SOCMEM_SOCMEM_PWR,
    PROT_PERI1_M55APPCPUSS,
    PROT_PERI1_APPCPUSS_ALL_PC,      
    PROT_PERI1_APPCPUSS_CM33_NS,     
    PROT_PERI1_APPCPUSS_MSC_INT,    
    PROT_PERI1_APPCPUSS_AHB2AXI_INT, 
    PROT_PERI1_APPCPUSS_AP,
    PROT_PERI1_U550_U55,             /* Peri 1, Address 0x44600000, size 0x00001000 */
    PROT_PERI1_U550_ROT,             /* Peri 1, Address 0x44601000, size 0x00000004 */
    PROT_PERI1_U550_ACG,             /* Peri 1, Address 0x44601004, size 0x00000004 */
    PROT_PERI1_U550_MXU55_PPU_PPU,   /* Peri 1, Address 0x44602000, size 0x00001000 */
    PROT_PERI1_U550_PD_PD,           /* Peri 1, Address 0x44603000, size 0x00000020 */
    PROT_PERI1_SOCMEM_MAIN,          /* Peri 1, Address 0x44640000, size 0x00000080 */
    PROT_PERI1_SOCMEM_BOOT,          /* Peri 1, Address 0x44640110, size 0x00000004 */
    PROT_PERI1_SOCMEM_SOCMEM_PWR,   /* Peri 1, Address 0x44640200, size 0x00000100 */
    PROT_PERI1_SOCMEM_SRAM_MPC0_PPC_MPC_MAIN, /* Peri 1, Address 0x44642000, size 0x00000004 */
    PROT_PERI1_SOCMEM_SRAM_MPC0_PPC_MPC_PC, /* Peri 1, Address 0x44642100, size 0x00000020 */
    PROT_PERI1_SOCMEM_SRAM_MPC0_PPC_MPC_ROT, /* Peri 1, Address 0x44642200, size 0x00000020 */
    PROT_PERI1_SOCMEM_MAIN_PORT,     /* Peri 1, Address 0x44650000, size 0x00008000 */
    PROT_PERI1_SOCMEM_PPU_SOCMEM_PPU, /* Peri 1, Address 0x44660000, size 0x00001000 */
    PROT_PERI1_SOCMEM_SOCMEM_PD_PD, /* Peri 1, Address 0x44661000, size 0x00000020 */
    PROT_PERI1_SMIF0_CORE_MAIN,/* Peri 1, Address 0x44460000, size 0x00000100 */
    PROT_PERI1_SMIF0_CORE_CRYPTO,/* Peri 1, Address 0x44460200, size 0x00000200 */
    PROT_PERI1_SMIF0_CORE_MAIN2 ,/* Peri 1, Address 0x444607c0, size 0x00000040 */
    PROT_PERI1_SMIF0_CORE_DEVICE,/* Peri 1, Address 0x44460800, size 0x00000200 */
    PROT_PERI1_SMIF0_CACHE_BLOCK_MAIN, /* Peri 1, Address 0x44450000, size 0x00001000 */
    PROT_PERI1_SMIF0_CACHE_BLOCK_MMIO_MAIN, /* Peri 1, Address 0x44452000, size 0x00000080 */
    PROT_PERI1_SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0_PPC_MPC_MAIN, /* Peri 1, Address 0x44454000, size 0x00000004 */
    PROT_PERI1_SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0_PPC_MPC_PC, /* Peri 1, Address 0x44454100, size 0x00000020 */
    PROT_PERI1_SMIF0_CACHE_BLOCK_CACHEBLK_AHB_MPC0_PPC_MPC_ROT, /* Peri 1, Address 0x44454200, size 0x00000020 */
    PROT_PERI1_TDM0_TDM_STRUCT0_TDM_TX_STRUCT_TX, /* Peri 1, Address 0x44418000, size 0x00000100 */
    PROT_PERI1_TDM0_TDM_STRUCT1_TDM_TX_STRUCT_TX, /* Peri 1, Address 0x44418200, size 0x00000100 */
    PROT_PERI1_TDM0_TDM_STRUCT0_TDM_RX_STRUCT_RX, /* Peri 1, Address 0x44418100, size 0x00000100 */
    PROT_PERI1_TDM0_TDM_STRUCT1_TDM_RX_STRUCT_RX, /* Peri 1, Address 0x44418300, size 0x00000100 */
};

/* 
* The mask is copied from the ppc_default generated structure but the secure
* attribute is set to secure and the secure private attribute is set to
* secure private
*/
cy_stc_ppc_attribute_t reapply_ppc_cfg =
{
    .pcMask = 0xE7,
    .secAttribute = CY_PPC_SECURE,
    .privAttribute = CY_PPC_PRIV
};

/*******************************************************************************
* Function Name: reapply_pd1_secure_configs
********************************************************************************
* Summary:
* Function to reapply the Power Domain 1 (PD1) configs.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void __attribute__((cmse_nonsecure_entry)) reapply_pd1_secure_configs(void) 
{
    cy_rslt_t result;
    
    /* Implemented the re-configuration as a loop, it could be individual calls */
    for (uint32_t index = 0; index < SIZEOFPPC0 ; index ++)
    {
        result = Cy_Ppc_ConfigAttrib(PPC0, reapply_pd1_ppc_0_regions[index], &reapply_ppc_cfg);
        if(result != CY_RSLT_SUCCESS)
        {
            /* Handle error result as needed */
            while(1);
        }
    }

    /* Enables the PD1 Power Domain */
    Cy_System_EnablePD1();

    /* Implemented the re-configuration as a loop, it could be individual calls */
    for (uint32_t index = 0; index < SIZEOFPPC1 ; index ++)
    {
        result = Cy_Ppc_ConfigAttrib(PPC1, reapply_pd1_ppc_1_regions[index], &reapply_ppc_cfg);
        if(result != CY_RSLT_SUCCESS)
        {
            /* Handle error result as needed */
            while(1);
        }
    }

    /*
    * Initialize the clock for the CY_MMIO_CM55_TCM peripheral group.
    * This sets up the necessary clock and peripheral routing to ensure
    * the CY_MMIO_CM55_TCM can be correctly accessed and utilized.
    */
    Cy_SysClk_PeriGroupSlaveInit(
        CY_MMIO_CM55_TCM_512K_PERI_NR, 
        CY_MMIO_CM55_TCM_512K_GROUP_NR, 
        CY_MMIO_CM55_TCM_512K_SLAVE_NR, 
        CY_MMIO_CM55_TCM_512K_CLK_HF_NR
    );

    /* 
    * Initialize the clock for the CY_MMIO_TDM0 peripheral group.
    * This sets up the necessary clock and peripheral routing to ensure
    * the CY_MMIO_TDM0 can be correctly accessed and utilized.
    */
    Cy_SysClk_PeriGroupSlaveInit(
        CY_MMIO_TDM0_PERI_NR, 
        CY_MMIO_TDM0_GROUP_NR, 
        CY_MMIO_TDM0_SLAVE_NR, 
        CY_MMIO_TDM0_CLK_HF_NR
    );

    /* 
    * Initialize the clock for the SMIF0 peripheral group.
    * This sets up the necessary clock and peripheral routing to ensure
    * the SMIF0 can be correctly accessed and utilized.
    */
    Cy_SysClk_PeriGroupSlaveInit(
        CY_MMIO_SMIF0_PERI_NR,
        CY_MMIO_SMIF0_GROUP_NR,
        CY_MMIO_SMIF0_SLAVE_NR,
        CY_MMIO_SMIF0_CLK_HF_NR
    );
    
    /* Enable SOCMEM */
    Cy_SysEnableSOCMEM(true);

    /* Configure MPC for NS */
    config_socmemram_ns();
    Cy_Mpc_SetViolationResponse((MPC_Type*)SOCMEM_SRAM_MPC0, true);
    config_smif0_xip_nsc();


    Cy_GPIO_Pin_Init(CYBSP_OSPI_D0_PORT, CYBSP_OSPI_D0_PIN, &CYBSP_OSPI_D0_config);
    Cy_GPIO_Pin_Init(CYBSP_OSPI_D1_PORT, CYBSP_OSPI_D1_PIN, &CYBSP_OSPI_D1_config);
    Cy_GPIO_Pin_Init(CYBSP_OSPI_D2_PORT, CYBSP_OSPI_D2_PIN, &CYBSP_OSPI_D2_config);
    Cy_GPIO_Pin_Init(CYBSP_OSPI_D3_PORT, CYBSP_OSPI_D3_PIN, &CYBSP_OSPI_D3_config);
    /*IS IT REALLY NEEDED */ // config_smif1_xip_ns();

    /* Reapply full PPC init as it comes from device configurator to mimic status after cybsp_init in cm33_s */
    Cy_PPC0_Init();
    Cy_PPC1_Init();
}




