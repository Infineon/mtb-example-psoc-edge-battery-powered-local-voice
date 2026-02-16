/******************************************************************************
* File Name : battery_powered_app_hp.c
*
* Description :
* Battery Powered Audio application for CM55 core.
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

#include "app_logger.h"
#include "FreeRTOS.h"
#include "cy_syslib.h"
#include "task.h"
#include "timers.h"
#include "battery_powered_svc_hp.h"

#include "battery_powered_app_hp.h"
#include "battery_powered_inferencing.h"
#include "battery_powered_ae_interface.h"
#include "intercore_ipc.h"

#include "mtb_ml.h"

#include "ppudbg.h"

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/

#ifdef INTERCORE_PIPE
extern void app_ipc_init_cm55(void);
#endif /* INTERCORE_PIPE */

#ifdef LOW_POWER_MODE
#include "ethosu_driver.h"
#if defined(COMPONENT_U55)
extern struct ethosu_driver ethosu_drv;
#endif /* COMPONENT_U55 */
#endif /* LOW_POWER_MODE */
/*******************************************************************************
* Function Name: battery_powered_app_hp
********************************************************************************
* Summary:
* Initialize CM55 audio application for battery powered use-case.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/


void battery_powered_app_hp()
{
    app_core2_boot_log();

#ifdef INTERCORE_PIPE
    app_ipc_init_cm55();
#endif /* INTERCORE_PIPE */
    /* Inferencing Engine Initialization */
    battery_powered_inference_engine_init();
    /* Audio Enhancement Initialization */
    battery_powered_ae_init(AFE_INPUT_NUMBER_CHANNELS);
    
#if defined(VA_INFERENCING)
    app_svc_hp_app_init();
#endif /* VA_INFERENCING */  

    app_log_print("\x1b[2J\x1b[;H");

    app_log_print("****************** \
    Battery Powered Local Voice \
    ****************** \r\n\n");

    app_log_print("Wake word: Okay Infineon\r\n"
           "Commands:\r\n"
           "    Start music\r\n"
           "    Next track\r\n"
           "    Previous track\r\n"
           "    End music\r\n"
           "    Pause music \r\n"
           "    Raise volume\r\n"
           "    Lower volume\r\n"
           
           "Additional intents for the commands: [Optional word] {one of required words} \t --> Matching commands\r\n"
           "[can you] {play/start} [the] {music/track}  \t--> Play music \r\n"
           "[can you] {end} [the] {music/track}  \t--> Stop music \r\n"
           "[can you] {pause} [the] {music/track}  \t--> Pause music \r\n"
           "[can you] [goto/switch to] {next} {track} \t--> Next track\r\n"
           "[can you] [goto/switch to] {previous} {track} \t--> Previous track\r\n"
           "[can you] {raise} {volume} \t--> Raise volume\r\n"
           "[can you] {lower} {volume} \t--> Lower volume\r\n" 
           );


#if !defined(VA_INFERENCING)
    app_log_print("Important: Voice Assistant Inferencing not enabled - Enable it for CE to work with all functionality. \r\n");
#else
    app_log_print("Waiting for Wake Word \r\n");
#endif /* VA_INFERENCING */

}
/* [] END OF FILE */
