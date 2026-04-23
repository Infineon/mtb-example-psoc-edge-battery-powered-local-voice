################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configuring the Cortex-M33 core project
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
###############################################################################

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=proj_cm33_ns
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
COMPONENTS += FREERTOS RTOS_AWARE FREERTOS_TZ

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

CORE=CM33
CORE_NAME=CM33_0

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES+=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=

# Add additional defines to the build process (without a leading -D).
DEFINES += CY_RETARGET_IO_CONVERT_LF_TO_CRLF
# DEFINES += ENABLE_SVC_CRITICAL_PRINT
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

ifeq ($(CONFIG_OP_MODE),LOW_POWER)
    DEFINES +=LOW_POWER_MODE
    DEFINES+=ENABLE_LOW_POWER_SVC
endif

ifeq ($(CONFIG_INFERENCING_MODE),HP_INFERENCING)
    DEFINES += HP_INFERENCING_ENABLE
endif

DEFINES += CY_RETARGET_IO_CONVERT_LF_TO_CRLF

#CMSIS DSP for voice processing.
COMPONENTS += CMSIS_DSP

COMPONENTS += AFE_CONFIGURATION

COMPONENTS += APP_LOGGER

COMPONENTS += AUDIO_CONV_UTILS

COMPONENTS += AUDIO_INPUT_CONFIGURATION

ifeq ($(CONFIG_MIC_TYPE),AMIC)
    COMPONENTS += AMIC
    DEFINES += ANALOG_MIC
endif

COMPONENTS += INTERCORE_PIPE

#FreeRTOS related defines.
DEFINES += CY_RTOS_AWARE
#Floating point selection.
VFP_SELECT = hardfp

#Enable Staged Voice Control Middleware
DEFINES += ENABLE_SVC_LP_MW

# Enable a specific wake word model by adding one of the following
# Set model based on preferred wake word,
  # Select for ok Infineon
  COMPONENTS+=SVC_OK_INFINEON
  # Select for ok Alexa
#  COMPONENTS+=SVC_ALEXA
  # Select for ok Hey Google
#  COMPONENTS+=SVC_HEY_GOOGLE
  # Select for Xiaodu Xiaodu
#  COMPONENTS+=SVC_XIAODU_XIAODU
# Note:- Only one of the above COMPONENTS should be enabled at a time to select the desired wake word model.

DEFINES += SVC_SHARED_BUFFER_SECTION=1

#In-case Low Power wake-word is to be overriden.
#    DEFINES += LPWWD_OVERRIDE #Override Low Power WakeWord

#Low power wake word detection configuration
DEFINES += EMBEDDED_DEV ENABLE_IFX_LPWWD
COMPONENTS += IFX_LPWWD ML_INT8x8
DEFINES += TF_LITE_STATIC_MEMORY ML_INT8x8
DEFINES += ENABLE_IFX_PRE_PROCESS_HPF

DEFINES += ENABLE_SVC_ML_MW_SUPPORT

#TensorFlowLite Micro Interpreter-Less mode.
COMPONENTS += ML_TFLM_LESS
DEFINES += TF_LITE_STRIP_ERROR_STRINGS
DEFINES += TF_LITE_MICRO_USE_OFFLINE_OP_USER_DATA
NN_INFERENCE_ENGINE=tflm_less
COMPONENTS += ML_T_LESS_NNL2
INCLUDES+=$(SEARCH_cmsis)/COMPONENT_CMSIS_NN/Include

###############################################################################
# Wake word models can only be used with LPWWD_ONLY mode
###############################################################################
ifneq ($(filter SVC_XIAODU_XIAODU SVC_ALEXA SVC_HEY_GOOGLE,$(COMPONENTS)),)
  ifneq ($(CONFIG_INFERENCING_MODE),LPWWD_ONLY)
    $(error ERROR: Wake word models (SVC_XIAODU_XIAODU, SVC_ALEXA, SVC_HEY_GOOGLE) can only be used when CONFIG_INFERENCING_MODE=LPWWD_ONLY. Currently CONFIG_INFERENCING_MODE=$(CONFIG_INFERENCING_MODE))
  endif
endif

# NN model name
ifeq ($(filter SVC_OK_INFINEON,$(COMPONENTS)),SVC_OK_INFINEON)
    NN_MODEL_NAME=OK_INFINEON_INT8
endif
ifeq ($(filter SVC_ALEXA,$(COMPONENTS)),SVC_ALEXA)
    NN_MODEL_NAME=ALEXA_INT8
endif
ifeq ($(filter SVC_HEY_GOOGLE,$(COMPONENTS)),SVC_HEY_GOOGLE)
    NN_MODEL_NAME=HEY_GOOGLE_INT8
endif
ifeq ($(filter SVC_XIAODU_XIAODU,$(COMPONENTS)),SVC_XIAODU_XIAODU)
    NN_MODEL_NAME=XIAODU_XIAODU_INT8
endif

NN_TYPE=int8x8
DEFINES +=MODEL_NAME=$(NN_MODEL_NAME)

#To enable Tflitemicro operation with NNLITE mode,
COMPONENTS += NNLITE2
#For Speech Onset Detection
DEFINES += ENABLE_IFX_SOD


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
# IDE provided compiler by default).
CY_COMPILER_PATH?=

CY_AVC_PATH=$(SEARCH_audio-voice-core)

ifeq ($(CONFIG_VOICE_CORE_MODE),FULL)
    COMPONENTS+=AVC_FULL
endif

ifeq ($(CONFIG_VOICE_CORE_MODE),LIMITED)
    COMPONENTS+=AVC_DEMO
endif

include $(CY_TOOLS_DIR)/make/start.mk
