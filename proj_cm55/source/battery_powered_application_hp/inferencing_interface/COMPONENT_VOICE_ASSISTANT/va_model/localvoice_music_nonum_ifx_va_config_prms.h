/*****************************************************************************
* \file localvoice_music_nonum_ifx_va_config_prms.h
*****************************************************************************
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
*****************************************************************************/

#ifndef LOCALVOICE_MUSIC_NONUM_IFX_VA_CONFIG_PRMS_H
#define LOCALVOICE_MUSIC_NONUM_IFX_VA_CONFIG_PRMS_H

#include "ifx_pre_post_process.h"
#include "ifx_va_prms.h"

extern int32_t localvoice_music_nonum_sod_prms[];
extern int32_t localvoice_music_nonum_pre_proc_hpf_prms[];
extern int32_t localvoice_music_nonum_denoise_prms[];
extern int32_t localvoice_music_nonum_dfww_prms[];
extern int32_t localvoice_music_nonum_dfcmd_prms[];

#endif /* LOCALVOICE_MUSIC_NONUM_IFX_VA_CONFIG_PRMS_H */
