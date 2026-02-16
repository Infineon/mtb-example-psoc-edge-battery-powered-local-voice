/***************************************************************************
* File Name: ppudbg.h
*
* Description: Header file for printing power domain status.
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

#ifndef _PPU_DBG_H_
#define _PPU_DBG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "system_pse84.h"
#include "cy_syspm_pdcm.h"
#include "cy_syspm_ppu.h"
#include "cy_ppc.h"
#include "app_logger.h"

#define GET_CORE_NAME  (CY_CPU_CORTEX_M0P?"CM0P":CY_CPU_CORTEX_M33?"CM33":"CM55")


#define CY_PPU_PD1_BASE                              ((uint32_t)PWRMODE_PPU_PD1)
#define CY_PPU_SRAM0_BASE                            ((uint32_t)RAMC_PPU0_BASE)
#define CY_PPU_SRAM1_BASE                            ((uint32_t)RAMC_PPU1_BASE)
#define CY_PPU_SYSCPU_BASE                           ((uint32_t)CPUSS_PPU_BASE)
#define CY_PPU_APPCPUSS_BASE                         ((uint32_t)PPU_APPCPUSS_BASE)
#define CY_PPU_APPCPU_BASE                           ((uint32_t)PPU_CM55_0_BASE)
#define CY_PPU_SOCMEM_BASE                           ((uint32_t)SOCMEM_PPU_SOCMEM)
#define CY_PPU_U55_BASE                              ((uint32_t)U550_MXU55_PPU )


void print_current_power_domain_status(void);
void print_programmed_power_domain_status(void);
void print_power_domain_status(void);
void print_programmed_power_domain_status_wo_u55(void);
void print_current_power_domain_status_wo_u55(void);

#ifdef __cplusplus
}
#endif

#endif /* _SYSTEM_CAT1D_H_ */


/* [] END OF FILE */



