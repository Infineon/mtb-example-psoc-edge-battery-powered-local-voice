/******************************************************************************
* File Name : battery_powered_application_lp.c
*
* Description :
* Battery powered application for CM33(LP) Core.
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
#include "battery_powered_application_lp.h"

#include "app_logger.h"
#if defined (ANALOG_MIC)
#include "lp_adc_amic_interface.h"
#endif /*ANALOG_MIC*/
#include "cy_afe_configurator_settings.h"

#include "audio_feed_to_svc.h"

/*******************************************************************************
* Function Name: battery_powered_application_lp
********************************************************************************
* Summary:
*  Initialize staged voice control and logging on CM33 core
*
* Parameters:
*  None
*
* Return:
*  CY_RSLT_SUCCESS
*
*******************************************************************************/

int battery_powered_application_lp()
{
    uint32_t status;

    /* Initialize Staged Voice Control */
    app_init_svc();


#if AFE_INPUT_SOURCE==AFE_INPUT_SOURCE_MIC
#if defined (ANALOG_MIC)
#if ENABLE_AUTONOMOUS_ANALOG
    /* Refer to analog_fifo_isr() API for the data and flow. */
    status = analog_mic_interface_init();
    if(CY_AUTANALOG_SUCCESS != status)
    {
        app_log_print("Analog mic initialization failed\r\n");
    }
    else
    {
        app_log_print("\x1b[2J\x1b[;H");
        app_log_print("******************     Battery Powered Local Voice     ******************\r\n\n");
        app_log_print("Wake word: "
#if defined(COMPONENT_SVC_OK_INFINEON)
                "Okay Infineon"
#elif defined(COMPONENT_SVC_ALEXA)
                "Alexa"
#elif defined(COMPONENT_SVC_HEY_GOOGLE)
                "Hey Google"
#elif defined(COMPONENT_SVC_XIAODU_XIAODU)
                "Xiaodu Xiaodu"
#else
                "Unknown"
#endif
                "\r\n");
#ifdef HP_INFERENCING_ENABLE
        app_log_print("Commands:\r\n"
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
#endif /* HP_INFERENCING_ENABLE  */
        app_log_print("waiting for wake word\r\n");
#ifndef HP_INFERENCING_ENABLE
        app_log_print("Important: Inferencing Code on CM55 unavailable - Enable it for CE to work with all functionality. \r\n");
#endif /* HP_INFERENCING_ENABLE */
    }
#endif /* ENABLE_AUTOMOUS_ANALOG */
#endif /*ANALOG_MIC*/
#endif /* AFE_INPUT_SOURCE_MIC */
    return CY_RSLT_SUCCESS;
}
/* [] END OF FILE */
