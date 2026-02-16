/*******************************************************************************
 * File Name: cy_fifo.h
 *
 * Description: Provides APIs for handling a FIFO in firmware.
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
#ifndef CY_AMIC_INPUT_H
#define CY_AMIC_INPUT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "cybsp.h"
#include "cy_pdl.h"
#include "mtb_hal.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define ANALOG_SAR_IDX                      (0U)
#define ANALOG_SAR_CH0_IDX                  (0U)
#define ANALOG_FIFO_IDX                     (0U)
#define ANALOG_FIFO_BUF0_IDX                (0U)
#define ANALOG_FIFO_BUF1_IDX                (1U)
#define FRAME_SIZE                          (160u)

/*******************************************************************************
* Global Variables
*******************************************************************************/
extern int32_t databuffer0[512U];
extern volatile bool intr_fifo_flag;
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
cy_rslt_t analog_init(void);
void analog_interrupt_init(void);
void analog_fifo_isr(void);
cy_rslt_t analog_mic_interface_init(void);

#if defined(__cplusplus)
}
#endif

#endif /* CY_AMIC_INPUT_H */





