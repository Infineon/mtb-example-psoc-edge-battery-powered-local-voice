[Click here](../README.md) to view the README.

## Design and implementation

The design of this application is minimalistic to get started with code examples on PSOC&trade; Edge MCU devices. All PSOC&trade; Edge E84 MCU applications have a dual-CPU three-project structure to develop code for the CM33 and CM55 cores. The CM33 core has two separate projects for the secure processing environment (SPE) and non-secure processing environment (NSPE). A project folder consists of various subfolders, each denoting a specific aspect of the project. The three project folders are as follows:

**Table 1. Application projects**

Project | Description
--------|------------------------
*proj_cm33_s* | Project for CM33 secure processing environment (SPE)
*proj_cm33_ns* | Project for CM33 non-secure processing environment (NSPE)
*proj_cm55* | CM55 project

In this code example, the Edge Protect bootloader is used to load the CM33 secure and non-secure projects to SRAM. On power on, Extended Boot launches the Edge Protect Bootloader. Both CM33 projects, namely proj_cm33_s, proj_cm33_ns are configured to be stored in external flash memory and loaded to SRAM by the Edge Protect Bootloader. CM55 project is also stored in external flash but loaded into System RAM (SoCMEM) by CM33 non-secure whenever Low Power Wake Word (LPWW) is detected.

**Battery-powered local voice with analog microphone:** 

In the CM33 non-secure application running on Arm&reg; Cortex&reg;-M33 core, the clocks and system resources are initialized by the BSP initialization function. The retarget-io middleware is configured to use the debug UART and the user LEDs is initialized. Battery-powered application on Low Power (LP) core initializes the Staged Voice Control (SVC) Low Power (LP) component of Staged Voice Control (SVC) middleware and analog microphone.

In the CM55 application running on Arm&reg; Cortex&reg;-M55 core, the clocks and system resources are initialized by the BSP initialization function. The retarget-io middleware is configured to use the debug UART. Battery-powered application on CM55 initializes Staged Voice Control (SVC) High Performance (HP) component of Staged Voice Control (SVC) middleware, DEEPCRAFT&trade; Audio Enhancement (AE) and DEEPCRAFT&trade; Voice Assistant (VA) libraries.

In CM33 non-secure application, Acoustic Activity Detection (AAD) is the lowest power stage and the device will be in Acoustic Activity Detection (AAD) stage. Acoustic Activity Detection (AAD) is followed by Speech Onset Detection (SOD), which detects the presence of human speech in the Analog mic data and then triggers Low Power Wake Word Detection (LPWWD).

Once Low Power Wake Word (LPWW) is detected, it is sent to the CM55 application for reverification via Staged Voice Control (SVC). Once the wake word is reverified, then the spoken commands are streamed to the CM55 application for ASR commands (intent) detection. All the audio data pass through the DEEPCRAFT&trade; Audio Enhancement (AE) for audio processing.

For the detailed explanation of design flow, see **Figure 1**.
   
**Figure 1. Battery-powered local voice design flow**
   
![](../images/battery_powered_design_flow.svg)

This code example uses a single analog microphone as shown in **Figure 1**. High Pass Filter (HPF), Dereverberation (DRVB) and Noise Suppression (NS) components are enabled within DEEPCRAFT&trade; Audio Enhancement (AE).

Other AFE components such as Beam Forming (BF), Acoustic Echo Cancellation (AEC), Echo Suppression (ES) are not enabled. Since analog mic has mono output, Beam Forming (BF) is disabled. Acoustic Echo Cancellation (AEC)/Echo Suppression (ES) are disabled as there is no audio playback demonstrated in this code example.

See this file for all the commands depending on the voice assistant project loaded to the code example:
*proj_cm55\source\battery_powered_application_hp\inferencing_interface\COMPONENT_VOICE_ASSISTANT\va_model\command_list_[project_name].txt*.


## Logging Configuration For Low Power

- **Enabling the logs**
   
   To enable the logs, set the **APP_LOG_PRINT_EN** to **1U** in the **common_modules/COMPONENT_APP_LOGGER/app_logger.h** file. For debug purpose, logs of CM33 can be enabled by setting the **ENABLE_CM33_LOGS** to **1U** in the **proj_cm33_ns/main.c** file

- **Disabling the logs**
   
   To disable the logs, set the **APP_LOG_PRINT_EN** to **0U** in the **common_modules/COMPONENT_APP_LOGGER/app_logger.h** file.


## Code structure

The code is structured into dedicated folders for every configuration. **Figure 2** shows the high-level folder organization of the code example.

**Figure 2. Project folder structure**

![](../images/code_structure.png)
 

## Code flow
   
**Figure 3** shows the code flow for battery-powered configuration for Ok Infineon wake word and play music command.

**Figure 3. Code flow**

![](../images/battery_powered_code_flow.svg)


## Audio pipeline customizations (how to's/recommendations)

The audio pipeline can be customized. The following details about the customizations.

**Acoustic Activity Detection (AAD) configuration**

The settings can be adjusted in the *proj_cm33_ns/source/battery_powered_application_lp/audio_feed_to_svc.h* file.

- Acoustic Activity Detection timeout is configured to detect silence for 7000 ms <br>
- Acoustic Activity Detection threshold is set to 7000 counts to detect silence <br>
- Acoustic Activity Detection feature is set to true to enable the feature <br>

    ```
    aad_config.timeout_ms = SOD_TO_AAD_TIMEOUT_MS;
    aad_config.low_noise_threshold = SOD_TO_AAD_THRESHOLD;
    aad_config.enable_feature = true;
    ```
<br>

**Speech Onset Detection (SOD) configuration**

The settings can be adjusted in the *proj_cm33_ns/source/battery_powered_application_lp/audio_feed_to_svc.c* file.

- Speech Onset Detection is configured for an onset gap of 400 ms <br>
- The sensitivity of SOD ranges from 0 = least sensitive to 32767 (MAX_SOD_SENSITIVITY) = most sensitive <br>
- In the code 16384 = nominal sensitive is used

    ```
    init_svc.sod_onset_detect_max_late_hit_delay_ms = SOD_HIT_DELAY;
    init_svc.sod_onset_gap_setting_ms = CY_SOD_ONSET_GAP_SETTING_400_MS;
    init_svc.sod_sensitivity = SOD_SENSITIVITY;
    ```
<br>

**DEEPCRAFT&trade; Audio Enhancement (AE) configuration**

The DEEPCRAFT&trade; Audio Enhancement (AE) modules are chosen via the DEEPCRAFT&trade; Audio Enhancement (AE) Configurator tool. Use the DEEPCRAFT&trade; Audio Enhancement (AE) Configurator project file present at path *commom_modules\COMPONENT_AFE_CONFIGURATION\battery_powered_ae_config.mtbafe*.

See **Figure 3** to check if number of channels is set to 1

**Figure 3. Number of channels**

![](../images/number_of_channels.png)

DEEPCRAFT&trade; Audio Enhancement (AE) Tuning and USB Receive and Transmit cannot be performed in this code example as it is intended for the low power demonstration.

> **Notes:**
  
- Choose the modules from High Pass Filter (HPF), Dereverberation (DRVB) and Noise Suppression (NS) to enable or disable as shown in **Figure 4** beacuse this code example supports only High Pass Filter (HPF), Dereverberation (DRVB) and Noise Suppression (NS)<br>

- High Pass Filter (HPF), Dereverberation (DRVB) and Noise Suppression (NS) components are enabled in DEEPCRAFT&trade; Audio Enhancement (AE) by default. <br>

**Figure 4. AFE configuration**
   
![](../images/afe_configuration.png)

- Save the DEEPCRAFT&trade; Audio Enhancement (AE) configuration

- Build and flash the code <br>

> **Notes:**

- Refer to [Audio Enhancement (AN240916)](https://www.infineon.com/AN240916) – DEEPCRAFT&trade; Audio Enhancement (AE) for understanding about the Audio Enhancement (AE) and tuning.
  
- For DEEPCRAFT&trade; Audio Enhancement (AE) and DEEPCRAFT&trade; Voice Assistant (VA) performance KPI, please refer to the [mains-powered-local-voice design guide](https://github.com/Infineon/mtb-example-psoc-edge-mains-powered-local-voice/blob/master/docs/local_voice_design_guide.md)

<br>

**DEEPCRAFT&trade; Voice Assistant (VA)**

DEEPCRAFT&trade; Voice Assistant (VA) middleware is used for speech recognition on edge devices. It supports applications such as wake-word detection for smart assistants, command recognition, and spoken language understanding. The middleware uses models generated through the DEEPCRAFT&trade; Voice Assistant (VA) Cloud Tool to detect wake words and commands.

Please refer to [voice-assistant deploy](https://github.com/Infineon/mtb-example-psoc-edge-voice-assistant-deploy) code example to know more about the voice assistant.

**Staged Voice Control (SVC) Customization**

To detect Low Power Wake Word only, set the **CONFIG_INFERENCING_MODE** macro to **LPWWD_ONLY** in the **common.mk** file.
1. Enable CM33 prints by setting **ENABLE_CM33_LOGS** macro to **1U** in **proj_cm33_ns/main.c** file
2. Red LED will be toggled for evry Low Power Wake Word Detection (LPWWD) detected

To detect High Performance Wakw Word and Command (enable CM55), set the **CONFIG_INFERENCING_MODE** macro to **HP_INFERENCING** in the **common.mk** file

**Power KPI**

See the [Preliminary KPI: Power profile](power_profile.md).

<br>