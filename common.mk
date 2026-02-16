################################################################################
# \file common.mk
# \version 1.0
#
# \brief
# Settings shared across all projects.
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

MTB_TYPE=PROJECT

# Target board/hardware (BSP).
# To change the target, it is recommended to use the Library manager
# ('make library-manager' from command line), which will also update Eclipse IDE launch
# configurations.
TARGET=KIT_PSE84_EVAL_EPC2

# Name of toolchain to use. Options include:
# ARM     - Arm Compiler (must be installed separately). 
TOOLCHAIN=ARM

ifeq ($(TOOLCHAIN),ARM)
#   Uncomment after importing the project or set this variable as env variable in the PC.
#    For Eg) CY_COMPILER_ARM_DIR=C:/Program Files/ArmCompilerforEmbedded6.22
endif

# Toolchains supported by this code example. See README.md file.
# This is used by automated build systems to identify the supported toolchains.
MTB_SUPPORTED_TOOLCHAINS?=ARM

# Default build configuration. Options include:
#
# Debug -- build with minimal optimizations, focus on debugging.
# Release -- build with full optimizations
# Custom -- build with custom configuration, set the optimization flag in CFLAGS
# 
# If CONFIG is manually edited, ensure to update or regenerate launch configurations 
# for your IDE.
CONFIG=Release
CFLAGS+= -o3

# SRF disabled by default as NSC based calls used for secure access.
DEFINES+=CY_SRF_DISABLE

#########################################################################################
##################### MIC type ##########################################################

# Audio input mic type. Options are
# AMIC - Analog mic.

CONFIG_MIC_TYPE=AMIC

###########################################################################################
##################### Voice Core ########################################################

# Voice Core functionality. Options are
# LIMITED - Limited time functionality. (Default)
# FULL    - Full functionality.

CONFIG_VOICE_CORE_MODE=LIMITED

##########################################################################################
##################### Inferencing Library ################################################

# Option to choose the inferencing library on High Performance core.
# VOICE_ASSISTANT - Infineon DEEPCRAFT Voice Assistant on M55 and U55.

CONFIG_INFERENCING_LIB=VOICE_ASSISTANT

#########################################################################################


##########################################################################################
##################### Code example operating mode #######################
# LOW_POWER   - Only Low power mode supported to save power in this version.
CONFIG_OP_MODE=LOW_POWER


#########################################################################################

##################### Run full functionality or just LPWWD #######################
# LPWWD_ONLY        - To detect wake word only in Low Power.
# HP_INFERENCING    - To detect wake word and command in CM55.
CONFIG_INFERENCING_MODE=HP_INFERENCING


#########################################################################################

##########################################################################################

# Search path for
# 1. Common source code shared between CM33 and CM55 cores.
# 2. Third party licensed code.

SEARCH+=../common_modules

##########################################################################################


# CM33, CM33_NS, and CM55 are placed in SRAM
SRAM_LOADING_WITH_EPB=TRUE

#Config file for postbuild sign and merge operations.
#NOTE:Check the JSON file for the command parameters

ifeq ($(SRAM_LOADING_WITH_EPB),TRUE)
#     COMBINE_SIGN_JSON=bsps/TARGET_$(TARGET)/config/GeneratedSource/boot_with_bldr.json
	DEFINES+=CY_CM33_NS_APP_BOOT_ADDR=0x24042400
	DEFINES+=CY_CM55_APP_BOOT_ADDR=0x26311400

	DEFINES+=CY_SOCMEMSRAM_NS_SIZE=0x00500000
	DEFINES+=CY_SMIF0_MAIN_NVM_NS_SIZE=0x008C0000

endif


# Building ifx-mcuboot with ARM compiler requries some
# specific symbols. However, linking when ifx-mcuboot is 
# added as a library to an existing project,
# those symbols are not required. Excluding those symbols, 
#which are not needed for this application build
ifeq ($(TOOLCHAIN),ARM)
  DEFINES+=MCUBOOT_SKIP_CLEANUP_RAM=1
endif



include ../common_app.mk
