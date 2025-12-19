/***************************************************************************
* File Name: ppudbg.c
*
* Description: To print the status of different power domain.
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

#include "ppudbg.h"

/*******************************************************************************
* Function Name: print_current_power_domain_status
********************************************************************************
* Summary:
* Prints the current power domain (PD) status for all available domains,
* including U55. The function retrieves and displays the real-time power mode
* of each domain by reading from its corresponding PPU registers.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void print_current_power_domain_status()
{
    /* OFF(0), MEM_RET(2), FULL_RET(5), ON(8) */
    app_log_print("[%s]:****CURRENT PD STATUS****\n", GET_CORE_NAME);
    app_log_print("[%s]:PD0      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_MAIN_BASE));
    app_log_print("[%s]:SRAM0    - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_SRAM0_BASE));
    app_log_print("[%s]:SRAM1    - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_SRAM1_BASE));
    app_log_print("[%s]:SYSCPU   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_SYSCPU_BASE));

    app_log_print("[%s]:PD1      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_PD1_BASE));
    app_log_print("[%s]:APPCPUSS - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_APPCPUSS_BASE));
    app_log_print("[%s]:APPCPU   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_APPCPU_BASE));
    app_log_print("[%s]:SOCMEM   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_SOCMEM_BASE));
    app_log_print("[%s]:U55      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_U55_BASE));
}

/*******************************************************************************
* Function Name: print_programmed_power_domain_status
********************************************************************************
* Summary:
* Prints the programmed (target) power domain (PD) status for all available
* domains, including U55. The function displays the power modes that have been
* configured but may not yet be active.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void print_programmed_power_domain_status()
{
    /* OFF(0), MEM_RET(2), FULL_RET(5), ON(8) */
    app_log_print("[%s]:****PROGRAMMED PD STATUS****\n", GET_CORE_NAME);
    app_log_print("[%s]:PD0      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_MAIN_BASE));
    app_log_print("[%s]:SRAM0    - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_SRAM0_BASE));
    app_log_print("[%s]:SRAM1    - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_SRAM1_BASE));
    app_log_print("[%s]:SYSCPU   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_SYSCPU_BASE));

    app_log_print("[%s]:PD1      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_PD1_BASE));
    app_log_print("[%s]:APPCPUSS - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_APPCPUSS_BASE));
    app_log_print("[%s]:APPCPU   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_APPCPU_BASE));
    app_log_print("[%s]:SOCMEM   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_SOCMEM_BASE));
    app_log_print("[%s]:U55      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_U55_BASE));
}

/*******************************************************************************
* Function Name: print_current_power_domain_status_wo_u55
********************************************************************************
* Summary:
* Prints the current power domain (PD) status for all domains except U55.
* This function is useful for systems that do not include or monitor the U55
* subsystem.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void print_current_power_domain_status_wo_u55()
{
    /* OFF(0), MEM_RET(2), FULL_RET(5), ON(8) */
    app_log_print("[%s]:****CURRENT PD STATUS****\n", GET_CORE_NAME);
    app_log_print("[%s]:PD0      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_MAIN_BASE));
    app_log_print("[%s]:SRAM0    - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_SRAM0_BASE));
    app_log_print("[%s]:SRAM1    - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_SRAM1_BASE));
    app_log_print("[%s]:SYSCPU   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_SYSCPU_BASE));

    app_log_print("[%s]:PD1      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_PD1_BASE));
    app_log_print("[%s]:APPCPUSS - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_APPCPUSS_BASE));
    app_log_print("[%s]:APPCPU   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_APPCPU_BASE));
    app_log_print("[%s]:SOCMEM   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_power_mode((struct ppu_v1_reg *)CY_PPU_SOCMEM_BASE));
}

/*******************************************************************************
* Function Name: print_programmed_power_domain_status_wo_u55
********************************************************************************
* Summary:
* Prints the programmed (target) power domain (PD) status for all domains
* except U55. This function is typically used when the U55 subsystem is not
* part of the configuration or not under monitoring.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void print_programmed_power_domain_status_wo_u55()
{
    /* OFF(0), MEM_RET(2), FULL_RET(5), ON(8) */
    app_log_print("[%s]:****PROGRAMMED PD STATUS****\n", GET_CORE_NAME);
    app_log_print("[%s]:PD0      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_MAIN_BASE));
    app_log_print("[%s]:SRAM0    - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_SRAM0_BASE));
    app_log_print("[%s]:SRAM1    - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_SRAM1_BASE));
    app_log_print("[%s]:SYSCPU   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_SYSCPU_BASE));

    app_log_print("[%s]:PD1      - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_PD1_BASE));
    app_log_print("[%s]:APPCPUSS - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_APPCPUSS_BASE));
    app_log_print("[%s]:APPCPU   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_APPCPU_BASE));
    app_log_print("[%s]:SOCMEM   - Status:0x%x\r\n", GET_CORE_NAME, cy_pd_ppu_get_programmed_power_mode((struct ppu_v1_reg *)CY_PPU_SOCMEM_BASE));
}

/*******************************************************************************
* Function Name: print_power_domain_status
********************************************************************************
* Summary:
* Prints both the programmed and current power domain statuses (excluding U55).
* This provides a quick comparison between the target and actual power modes
* for debugging and validation purposes.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void print_power_domain_status()
{
    print_programmed_power_domain_status_wo_u55();
    print_current_power_domain_status_wo_u55();
}
