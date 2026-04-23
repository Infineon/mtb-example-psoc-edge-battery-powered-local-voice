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

To understand the Staged Voice Control (SVC) state machine in detail, refer to the [SVC README](https://github.com/Infineon/staged-voice-control/blob/master/README.md).

**SVC state machine**

![](../images/svc_state_machine.png)


In CM33 non-secure application, Acoustic Activity Detection (AAD) is the lowest power stage and the device will be in Acoustic Activity Detection (AAD) stage. Acoustic Activity Detection (AAD) is followed by Speech Onset Detection (SOD), which detects the presence of human speech in the Analog mic data and then triggers Low Power Wake Word Detection (LPWWD).

Once Low Power Wake Word (LPWW) is detected, it is sent to the CM55 application for reverification via Staged Voice Control (SVC). Once the wake word is reverified, then the spoken commands are streamed to the CM55 application for ASR commands (intent) detection. All the audio data pass through the DEEPCRAFT&trade; Audio Enhancement (AE) for audio processing.

For the detailed explanation of design flow, see **Figure 1**.

**Figure 1. Battery-powered local voice design flow**

![](../images/battery_powered_design_flow.svg)

This code example uses a single analog microphone as shown in **Figure 1**. High Pass Filter (HPF), Dereverberation (DRVB) and Noise Suppression (NS) components are enabled within DEEPCRAFT&trade; Audio Enhancement (AE).

Other AFE components such as Beam Forming (BF), Acoustic Echo Cancellation (AEC), Echo Suppression (ES) are not enabled. Since analog mic has mono output, Beam Forming (BF) is disabled. Acoustic Echo Cancellation (AEC)/Echo Suppression (ES) are disabled as there is no audio playback demonstrated in this code example.

If you need Acoustic Echo Cancellation (AEC), refer to the [PSOC&trade; Edge MCU: Mains-powered local voice Code Example](https://github.com/Infineon/mtb-example-psoc-edge-mains-powered-local-voice).

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

- Speech Onset Detection is configured for an onset gap of 500 ms <br>
- The sensitivity of SOD ranges from 0 = least sensitive to 32767 (MAX_SOD_SENSITIVITY) = most sensitive <br>
- In the code 16384 = nominal sensitive is used

    ```
    init_svc.sod_onset_detect_max_late_hit_delay_ms = SOD_HIT_DELAY;
    init_svc.sod_onset_gap_setting_ms = CY_SOD_ONSET_GAP_SETTING_500_MS;
    init_svc.sod_sensitivity = SOD_SENSITIVITY;
    ```
- Gain control can be adjusted by modifying the `gain_control.post_hpf_gain` and `gain_control.sod_processing_gain` in *proj_cm33_ns/source/battery_powered_application_lp/audio_feed_to_svc.c* file. By default, these are set to 1 for Near field detection. For Far field detection, set it to 3.

    ```
    cy_svc_lp_gain_config_t gain_control;
    gain_control.post_hpf_gain = POST_HPF_GAIN; // 1 - Near field, 3 - Far field
    gain_control.sod_processing_gain = SOD_PROCESSING_GAIN; //1 - Near field, 3 - Far field
    ```
<br>

**DEEPCRAFT&trade; Audio Enhancement (AE) configuration**

The DEEPCRAFT&trade; Audio Enhancement (AE) modules are chosen via the DEEPCRAFT&trade; Audio Enhancement (AE) Configurator tool. Use the DEEPCRAFT&trade; Audio Enhancement (AE) Configurator project file present at path *common_modules\COMPONENT_AFE_CONFIGURATION\battery_powered_ae_config.mtbafe*.

See **Figure 3** to check if number of channels is set to 1

**Figure 3. Number of channels**

![](../images/number_of_channels.png)

DEEPCRAFT&trade; Audio Enhancement (AE) Tuning and USB Receive and Transmit cannot be performed in this code example as it is intended for the low power demonstration.

> **Notes:**

- Choose the modules from High Pass Filter (HPF), Dereverberation (DRVB) and Noise Suppression (NS) to enable or disable as shown in **Figure 4** because this code example supports only High Pass Filter (HPF), Dereverberation (DRVB) and Noise Suppression (NS)<br>

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
2. Red LED will be toggled for every Low Power Wake Word Detection (LPWWD) detected

**Figure 5. LPWWD only state machine**

![](../images/lpwwd_only_state_machine.png)

To detect High Performance Wake Word and Command (enable CM55), set the **CONFIG_INFERENCING_MODE** macro to **HP_INFERENCING** in the **common.mk** file

**DEEPCRAFT&trade; Voice Assistant (VA) Customization**

For voice assistant model training and customization, Infineon has a cloud-based training framework. To access it, create an account at [DEEPCRAFT&trade; Voice Assistant cloud tool](https://deepcraft.infineon.com/solutions/voice-assistant). The tool uses a graph topology that lets you map the flow and train the model.

**Figure 6. Voice assistant overview**

![](../images/va_overview.png)

This code example includes the default voice assistant project file which user can directly import into the cloud tool, which is available at *proj_cm55\source\battery_powered_application_hp\inferencing_interface\COMPONENT_VOICE_ASSISTANT\va_proj*

For training a new cloud project,

1. After logging in, select **Design New Project** for creating a new project, then enter the project name and select the language

   or,

   to open an existing project, select **Import Project**, then enter the project name and choose the *.vaproj* file

    **Figure 7. Design or import project**

    ![](../images/va_new_import.png)

2. For adding a wakeword, like "Ok Infineon", drag the Wakeword element into the canvas as shown:

    **Figure 8. Adding wakeword**

    ![](../images/va_wwd.png)

3. To add the commands, drag the **Text** element into the canvas. To add numbers, drag the **Variables** element into the canvas. Ensure to add the required details in the **Properties** tab on the right.

    > **Note:** In the image below, there are **Optional** and **Required** words in the command flow. It shows "can you start the recording" where "can you" and "the" are optional words:

    **Figure 9. Voice assistant commands**

    ![](../images/va_command.png)

4. Each command flow should be mapped to an intent. Drag the **Intent** element into the canvas and add the intent label. Ensure to connect all the nodes

    **Figure 10. Mapping command flow to intent**

    ![](../images/va_intent.png)

5. After adding all the commands and intent flows, choose the **CM55+U55** option (only this option is supported) and click **Generate**

    **Figure 11. Choosing CM55+U55**

    ![](../images/va_U55.png)

    **Figure 12. Generating the model**

    ![](../images/va_generate.png)

6. Once the model is generated, download the files

    **Figure 13. Downloading files**

    ![](../images/va_download.png)

7. Extract the downloaded file and copy and paste all files to *proj_cm55\source\battery_powered_application_hp\inferencing_interface\COMPONENT_VOICE_ASSISTANT\va_model*

8. In *proj_cm55*/*Makefile*, set the name `DEEPCRAFT_PROJECT_NAME=[name of the VA project]`, which is the VA cloud project name prefixed to the downloaded files

9. Update the intent macros and command IDs for the new model in *proj_cm55\source\battery_powered_application_hp\inferencing_interface\inferencing_interface.h*, for example:

    **Figure 14. Adding command IDs and intent macros**

    ![](../images/va_cmd_id_intent.png)

10. Update the `va_intent_to_id()` API in *proj_cm55\source\battery_powered_application_hp\inferencing_interface\COMPONENT_VOICE_ASSISTANT\va_inferencing.c*, which sends the command ID to *battery_powered_post_processing* API defined in *proj_cm55\source\battery_powered_application_hp\inferencing_interface\battery_powered_inferencing.c* file for the post processing

11. Update the `battery_powered_post_processing()` API in *proj_cm55\source\battery_powered_application_hp\inferencing_interface\battery_powered_inferencing.c* which sets the Staged Voice Control (SVC) state and prints the detected command

    **Figure 15. Update battery_powered_post_processing API**

    ![](../images/update_battery_powered_post_processing.png)

12. To list the supported commands, update the **battery_powered_application_lp.c** file located at *proj_cm33_ns\source\battery_powered_application_lp* and the **audio_feed_to_svc.c** file located at *proj_cm33_ns\source\battery_powered_application_lp* written in **CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED, CY_SVC_EVENT_ASR_NOT_DETECTED and CY_SVC_EVENT_ASR_PROCESSING_COMPLETED events** so that the supported commands are displayed in the UART terminal

    **Figure 16. Update prints CM33 side**

    ![](../images/update_battery_powered_application_lp_prints.png)

    **Figure 17. Update audio_feed_to_svc file**

    ![](../images/update_audio_feed_to_svc_file.png)


**Power KPI**

See the [Preliminary KPI: Power profile](power_profile.md).

To profile the AFE middleware for the cycles:

1. Enable the following in the *Makefile* of *proj_cm55*:

    ```
    #For Profiler
    COMPONENTS += PROFILER
    DEFINES += PROFILER_ENABLE
    ```

2. Enable the following flag in
*proj_cm55\source\battery_powered_application_hp\audio_enhancement\battery_powered_ae_interface.h*:

    ```
    #define AE_APP_PROFILE  (1)
    ```

To profile Voice Assistant for the cycles:

1. Enable the following in the *Makefile* of *proj_cm55*:

    ```
    #For Profiler
    COMPONENTS += PROFILER
    DEFINES += PROFILER_ENABLE
    ```

2. Enable the following flag in
*proj_cm55\source\battery_powered_application_hp\audio_enhancement\battery_powered_ae_interface.h*:

    ```
    #define INFERENCING_PROFILE  (1)
    ```
**Audio Benchmarking of battery powered local voice pipeline:**

The battery powered local voice pipeline benchmarking with Analog Mic input is shown below,

Setup details:

1.	With Analog mic input to the audio pipeline, talker speaker is set at 1 feet from the analog mics on the kit.

2.	Noise speaker is set at 3 feet from the kit at an angle of 45 degrees, not facing the analog mics on the kit.

3.	SNR is controlled by adjusting the output levels on both speakers. The SNR values are as shown below

    **Figure 18. SNR details**

    ![](../images/snr_details.png)

4. The benchmarking captures the detection statistics for 50 utterances of single talker with US accent with noise continuously played via noise speaker is shown below

    **Figure 19. Full pipeline Performance benchmarking**

    ![](../images/full_pipeline_performance_benchmarking.png)

In the above table, there are 2 sets of results. 

One is the raw statistics that counts the detection% based on the number of utterances. This gives the statistics based on number of utterances.

The second is based on the previous stage in the audio pipeline. For example, HPWWD % will be number of HPWW detected/number of LPWW detected and not based on number of utterances. This gives the individual stage detection statistics.

> **Note:** The benchmarking results could vary depending on environment. The statistics will also be improved in future versions of this Code Example.
