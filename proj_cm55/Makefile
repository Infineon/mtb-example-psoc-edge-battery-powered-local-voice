################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configuring the Cortex-M55 core project
#
################################################################################
# \copyright
# (c) 2025-2026, Infineon Technologies AG, or an affiliate of Infineon
# Technologies AG.  SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

include ../common.mk

################################################################################
# Basic Configuration
################################################################################


# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=proj_cm55
APPTYPE=flash

# If set to "true" or "1", display full command-lines when building.
VERBOSE=


################################################################################
# Advanced Configuration
################################################################################

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS += FREERTOS RTOS_AWARE

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS+=

CORE=CM55
CORE_NAME=CM55_0
NPU=U55

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES+=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=

# Add additional defines to the build process (without a leading -D).
DEFINES += CY_RTOS_AWARE

DEFINES += FLASH_BOOT

# This code example supports only hardfp.
VFP_SELECT = hardfp

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS+=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS+=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS+=

# Additional / custom linker flags.
LDFLAGS+=

# Additional / custom libraries to link in to the application.
LDLIBS+=

# Path to the linker script to use (if empty, use the default linker script).
 LINKER_SCRIPT=
# Custom pre-build commands to run.
PREBUILD+=

# Custom post-build commands to run.
POSTBUILD+=

################################################################################
# Application specific Configurations to use COMPONENTS, DEFINES and flags.
################################################################################
ifeq ($(CONFIG_USB_STREAMING),CM55)
DEFINES += CY_AFE_ENABLE_TUNING_FEATURE
endif

DEFINES += CY_RETARGET_IO_CONVERT_LF_TO_CRLF

COMPONENTS += USB_AUDIO
COMPONENTS += EMUSB
COMPONENTS += USBD_BASE


COMPONENTS +=AFE_CONFIGURATION
COMPONENTS +=APP_LOGGER
COMPONENTS +=AUDIO_CONV_UTILS
COMPONENTS +=INTERCORE_PIPE
COMPONENTS +=AUDIO_INPUT_CONFIGURATION

ifeq ($(CONFIG_USB_STREAMING),CM55)
DEFINES += CY_AFE_ENABLE_TUNING_FEATURE
endif

ifeq ($(CONFIG_OP_MODE),LOW_POWER)
    DEFINES +=LOW_POWER_MODE
    DEFINES+=ENABLE_LOW_POWER_SVC
endif

#For Profiler
COMPONENTS += PROFILER
DEFINES+=PROFILER_ENABLE


#AFE configuration and related macros

COMPONENTS += CMSIS_DSP

DEFINES += RUN_FLOAT_FE
DEFINES += ENABLE_AFE_MW_SUPPORT
#DEFINES +=AFE_AUDIO_PROCESSING_TASK_PRIORITY=2

#Set the memory area for ML Based Noise Suppression weights and parameters
DEFINES+=DSNS_ENABLE_MTB_ML
COMPONENTS+=MW_MDL_VA MW_DSNS_MODEL MW_MDL_DSNS MW_MDL_DSES
DEFINES+=USE_MTB_ML


ifeq ($(CONFIG_INFERENCING_LIB),VOICE_ASSISTANT)
    VA_LIB=Y
    DEFINES += VA_INFERENCING
    COMPONENTS += VOICE_ASSISTANT
endif

# Neural Network Configuration. Currently only int16x8 is supported.
NN_TYPE=int16x8

# Choose the inference engine
# tflm      -- TensorFlow Lite for Microcontrollers inference engine with interpreter
# Note tflm_less option is not supported on CM55+U55
NN_INFERENCE_ENGINE=tflm

NPU=U55

DEFINES+=ETHOSU55

# Add additional components related to MTB ML
COMPONENTS+=CMSIS_DSP U55

#Default Cache management is all layers.
#DEFINES+=MTB_ML_ETHOSU_CACHE_MGMT_TYPE=MTB_ML_ETHOSU_CACHE_MGMT_ALL_LAYERS


# Add additional define/components to select the inference engine
ifeq (tflm, $(NN_INFERENCE_ENGINE))
    COMPONENTS+=ML_TFLM
    DEFINES+=TF_LITE_STATIC_MEMORY
endif

DEFINES+= CY_ML_MODEL_MEM=.cy_socmem_data
ifeq (int16x8, $(NN_TYPE))
    COMPONENTS+=ML_INT16x8
endif
ifeq (int8x8, $(NN_TYPE))
    COMPONENTS+=ML_INT8x8
endif


ifeq ($(VA_LIB),Y)
    DEFINES += EMBEDDED_DEV    #EMBEDDED_DEV is must to build #IFX algorithms for SOD, LPWWD MW
    DEFINES += ENABLE_IFX_SOD  #ENABLE_IFX_SOD is must to build #IFX algorithms for SOD
    DEFINES += ENABLE_IFX_PRE_PROCESS_HPF #To enable HPF

    DEFINES += ENABLE_IFX_VA_WWD
    DEFINES += ENABLE_IFX_VA_CMD
    DEEPCRAFT_PROJECT_NAME=localvoice_music_nonum
    DEFINES+=PROJECT_PREFIX=$(DEEPCRAFT_PROJECT_NAME)
    DEFINES +=VA_VARIABLE_ENABLE
endif

#Staged Voice Control Middleware on M55
COMPONENTS += HP_APP_SVC
DEFINES += ENABLE_SVC_HP_MW

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> field
# begins with $$ASSET_REPO$$, then the repo is deposited in the path specified by
# the CY_GETLIBS_SHARED_PATH variable. The default location is one directory level
# above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the directory name.
CY_GETLIBS_SHARED_PATH=../..

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

# Absolute path to the compiler's "bin" directory.
#
# The default depends on the selected TOOLCHAIN (GCC_ARM uses the ModusToolbox
# software provided compiler by default).
CY_COMPILER_PATH?=


CY_AVC_PATH=$(SEARCH_audio-voice-core)
CY_AFE_PATH=$(SEARCH_audio-front-end)
CY_SOD_PATH=$(SEARCH_speech-onset-detection)

ifeq ($(CONFIG_VOICE_CORE_MODE),FULL)
    COMPONENTS+=AVC_FULL
endif

ifeq ($(CONFIG_VOICE_CORE_MODE),LIMITED)
    COMPONENTS+=AVC_DEMO
endif

include $(CY_TOOLS_DIR)/make/start.mk
