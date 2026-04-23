/*******************************************************************************
 * File Name:   aad.h
 *
 * Description:  This file is the public interface of the 
 *               Acoustic Activity Detection (AAD)
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

#ifndef _AAD_H_
#define _AAD_H_

/******************************************************************************
 * Typedefs
 *****************************************************************************/
typedef enum
{
    AAD_RSLT_SUCCESS            = 0,
    AAD_RSLT_FAIL               = -1,    
} aad_rslt_t;

typedef enum
{
    AAD_WINDOW_SIZE_200_US      = 0,
    AAD_WINDOW_SIZE_400_US      = 1,
    AAD_WINDOW_SIZE_800_US      = 2,
    AAD_WINDOW_SIZE_1600_US     = 3,
    AAD_WINDOW_SIZE_3200_US     = 4,
    AAD_WINDOW_SIZE_6400_US     = 5,
    AAD_WINDOW_SIZE_12800_US    = 6,
} aad_window_size_t;

typedef enum
{
    AAD_TRIGGER_COUNT_10_PERCENT = 1,
    AAD_TRIGGER_COUNT_20_PERCENT = 2,
    AAD_TRIGGER_COUNT_30_PERCENT = 3,
    AAD_TRIGGER_COUNT_40_PERCENT = 4,
    AAD_TRIGGER_COUNT_50_PERCENT = 5,
    AAD_TRIGGER_COUNT_60_PERCENT = 6,
    AAD_TRIGGER_COUNT_70_PERCENT = 7,
    AAD_TRIGGER_COUNT_80_PERCENT = 8,
    AAD_TRIGGER_COUNT_90_PERCENT = 9,
} aad_trigger_count_t;

typedef enum
{
    AAD_TRIGGER_THRESHOLD_112_5_MV = 0,
    AAD_TRIGGER_THRESHOLD_225_0_MV = 1,
    AAD_TRIGGER_THRESHOLD_337_5_MV = 2,
} aad_trigger_threshold_t;

/*******************************************************************************
* Macros
*******************************************************************************/
#ifndef AAD_DEFAULT_WINDOW_SIZE
#define AAD_DEFAULT_WINDOW_SIZE         AAD_WINDOW_SIZE_3200_US
#endif
#ifndef AAD_DEFAULT_TRIGGER_COUNT
#define AAD_DEFAULT_TRIGGER_COUNT       AAD_TRIGGER_COUNT_20_PERCENT
#endif
#ifndef AAD_DEFAULT_TRIGGER_THRESHOLD
#define AAD_DEFAULT_TRIGGER_THRESHOLD   AAD_TRIGGER_THRESHOLD_112_5_MV
#endif

/*******************************************************************************
* Function prototypes
*******************************************************************************/

aad_rslt_t aad_init(void);
aad_rslt_t aad_restart(void);
aad_rslt_t aad_set_window_size(aad_window_size_t window_size);
aad_rslt_t aad_set_trigger_count(aad_trigger_count_t trigger_count);
aad_rslt_t aad_set_trigger_threshold(aad_trigger_threshold_t trigger_threshold);

#endif /* _AAD_H_ */

/* [] END OF FILE */