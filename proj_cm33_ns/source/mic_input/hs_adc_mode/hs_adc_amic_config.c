/*******************************************************************************
 * File Name:   hs_adc_amic_config.c
 *
 * Description: This file contains the high speed analog mic configuration
 *
 * Related Document : See README.md
 *
 *******************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
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
 
#include "hs_adc_amic_config.h"
 
cy_en_autanalog_ac_out_trigger_mask_t amic_hq_mono_states_out_trig_mask[] =
{
    CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY, 
    CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY, 
    CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY, 
    CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY, 
    CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY, 
    CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY, 
    CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY, 
    CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY, 
};
cy_stc_autanalog_ac_t amic_hq_mono_states_cfg =
{
    .gpioOutEn =  
 CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    .mask =
    {
        &amic_hq_mono_states_out_trig_mask[0U], 
        &amic_hq_mono_states_out_trig_mask[1U], 
        &amic_hq_mono_states_out_trig_mask[2U], 
        &amic_hq_mono_states_out_trig_mask[3U], 
        &amic_hq_mono_states_out_trig_mask[4U], 
        &amic_hq_mono_states_out_trig_mask[5U], 
        &amic_hq_mono_states_out_trig_mask[6U], 
        &amic_hq_mono_states_out_trig_mask[7U], 
    },
    .timer =
    {
        .enable = false,
        .clkSrc = CY_AUTANALOG_TIMER_CLK_LP,
        .period = 255U,
    },
};
cy_stc_autanalog_stt_ac_t amic_hq_mono_states_stt[] =
{
    {
        .unlock = true,
        .lpMode = false,
        .condition = CY_AUTANALOG_STT_AC_CONDITION_BLOCK_READY,
        .action = CY_AUTANALOG_STT_AC_ACTION_WAIT_FOR,
        .branchState = 1U,
        .trigInt = false,
        .count = 0U,
        .unlockGpioOut = false,
        .gpioOut = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    },
    {
        .unlock = false,
        .lpMode = false,
        .condition = CY_AUTANALOG_STT_AC_CONDITION_TR_AUTANALOG_IN3,
        .action = CY_AUTANALOG_STT_AC_ACTION_WAIT_FOR,
        .branchState = 0U,
        .trigInt = false,
        .count = 0U,
        .unlockGpioOut = false,
        .gpioOut = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    },
    {
        .unlock = false,
        .lpMode = false,
        .condition = CY_AUTANALOG_STT_AC_CONDITION_BLOCK_READY,
        .action = CY_AUTANALOG_STT_AC_ACTION_WAIT_FOR,
        .branchState = 0U,
        .trigInt = false,
        .count = 0U,
        .unlockGpioOut = false,
        .gpioOut = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    },
    {
        .unlock = false,
        .lpMode = false,
        .condition = CY_AUTANALOG_STT_AC_CONDITION_SAR_FIFO_DONE,
        .action = CY_AUTANALOG_STT_AC_ACTION_WAIT_FOR,
        .branchState = 0U,
        .trigInt = false,
        .count = 0U,
        .unlockGpioOut = false,
        .gpioOut = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    },
    {
        .unlock = false,
        .lpMode = false,
        .condition = CY_AUTANALOG_STT_AC_CONDITION_TRUE,
        .action = CY_AUTANALOG_STT_AC_ACTION_BRANCH_IF_TRUE,
        .branchState = 1U,
        .trigInt = false,
        .count = 0U,
        .unlockGpioOut = false,
        .gpioOut = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    },
};
cy_stc_autanalog_ctb_sta_t mic_preamps_and_filters_hq_mono_sta_cfg =
{
    .pwrOpamp0 = AMIC_HQ_PREAMP_POWER,
    .pwrOpamp1 = CY_AUTANALOG_CTB_OA_PWR_OFF,
    .topologyOpamp0 = CY_AUTANALOG_CTB_OA_TOPO_OPEN_LOOP_OPAMP,
    .topologyOpamp1 = CY_AUTANALOG_CTB_OA_TOPO_COMPARATOR,
    .intComp0 = CY_AUTANALOG_CTB_COMP_INT_EDGE_DISABLED,
    .intComp1 = CY_AUTANALOG_CTB_COMP_INT_EDGE_DISABLED,
    .capCcOpamp0 = CY_AUTANALOG_CTB_OA_CC_CAP_5_1_pF,
    .capCcOpamp1 = CY_AUTANALOG_CTB_OA_CC_CAP_DISABLED,
    .capFbOpamp0 = CY_AUTANALOG_CTB_OA_FB_CAP_0_pF,
    .capFbOpamp1 = CY_AUTANALOG_CTB_OA_FB_CAP_0_pF,
};
cy_stc_autanalog_ctb_dyn_t mic_preamps_and_filters_hq_mono_dyn_cfg[] =
{
    {
        .ninvInpPin = CY_AUTANALOG_CTB_OA_NINV_PIN_OA0_P0_OA1_P5,
        .ninvInpRef = CY_AUTANALOG_CTB_OA_NINV_REF_DISCONNECT,
        .invInpPin = CY_AUTANALOG_CTB_OA_INV_PIN_OA0_P1_OA1_P4,
        .resInpPin = CY_AUTANALOG_CTB_OA_RES_PIN_DISCONNECT,
        .resInpRef = CY_AUTANALOG_CTB_OA_RES_REF_DISCONNECT,
        .sharedMuxIn = CY_AUTANALOG_CTB_OA_MUX_IN_DISCONNECT,
        .sharedMuxOut = CY_AUTANALOG_CTB_OA_MUX_OUT_DISCONNECT,
        .outToPin = true,
    },
};
cy_stc_autanalog_ctb_t mic_preamps_and_filters_hq_mono_cfg =
{
    .ctbStaCfg = &mic_preamps_and_filters_hq_mono_sta_cfg,
    .ctbDynCfgNum = 1U,
    .ctbDynCfgArr = mic_preamps_and_filters_hq_mono_dyn_cfg,
};
cy_stc_autanalog_stt_ctb_t mic_preamps_and_filters_hq_mono_stt[] =
{
    {
        .unlock = true,
        .enableOpamp0 = true,
        .cfgOpamp0 = 0U,
        .gainOpamp0 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1 = 0U,
        .gainOpamp1 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
    {
        .unlock = false,
        .enableOpamp0 = false,
        .cfgOpamp0 = 0U,
        .gainOpamp0 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1 = 0U,
        .gainOpamp1 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
    {
        .unlock = false,
        .enableOpamp0 = false,
        .cfgOpamp0 = 0U,
        .gainOpamp0 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1 = 0U,
        .gainOpamp1 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
    {
        .unlock = false,
        .enableOpamp0 = false,
        .cfgOpamp0 = 0U,
        .gainOpamp0 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1 = 0U,
        .gainOpamp1 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
    {
        .unlock = false,
        .enableOpamp0 = false,
        .cfgOpamp0 = 0U,
        .gainOpamp0 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1 = 0U,
        .gainOpamp1 = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
};
cy_stc_autanalog_sar_mux_chan_t adc_hs_mono_mux_ch_cfg[] =
{
    {
        .posPin = CY_AUTANALOG_SAR_PIN_MUX_CTB0_OA0_OUT,
        // .posPin = CY_AUTANALOG_SAR_PIN_MUX_GPIO1,
        .sign = true,
        .posCoeff = CY_AUTANALOG_SAR_CH_COEFF_DISABLED,
        .negPin = CY_AUTANALOG_SAR_PIN_MUX_VSSA,
        .buffBypass = true,
        .accShift = true,
        .negCoeff = CY_AUTANALOG_SAR_CH_COEFF_DISABLED,
        .muxLimit = CY_AUTANALOG_SAR_LIMIT_STATUS_DISABLED,
        .fifoSel = CY_AUTANALOG_FIFO_SELECT0,
    },
};
cy_stc_autanalog_sar_sta_hs_t adc_hs_mono_sta_hs_cfg =
{
    .hsVref = CY_AUTANALOG_SAR_VREF_VDDA,
    .hsSampleTime =
    {
        31U, 
        31U, 
        31U, 
        31U, 
    },
    .hsGpioChan =
    {
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
    },
    .hsGpioResultMask =  
 CY_AUTANALOG_SAR_CHAN_MASK_GPIO_DISABLED,
};
cy_stc_autanalog_sar_sta_t adc_hs_mono_sta_cfg =
{
    .lpStaCfg = NULL,
    .hsStaCfg = &adc_hs_mono_sta_hs_cfg,
    .posBufPwr = CY_AUTANALOG_SAR_BUF_PWR_OFF,
    .negBufPwr = CY_AUTANALOG_SAR_BUF_PWR_OFF,
    .accMode = CY_AUTANALOG_SAR_ACC_ACCUNDUMP,
    .startupCal = CY_AUTANALOG_SAR_CAL_DISABLED,
    .chanID = false,
    .shiftMode = true,
    .intMuxChan =
    {
        &adc_hs_mono_mux_ch_cfg[0U], 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
    },
    .limitCond =
    {
        NULL, 
        NULL, 
        NULL, 
        NULL, 
    },
    .muxResultMask =  
 CY_AUTANALOG_SAR_CHAN_MASK_MUX0,
    .firResultMask =  
 CY_AUTANALOG_SAR_MASK_FIR_DISABLED,
};
cy_stc_autanalog_sar_seq_tab_hs_t adc_hs_mono_seq_hs_cfg[] =
{
    {
        .chanEn = CY_AUTANALOG_SAR_CHAN_MASK_GPIO_DISABLED,
        .muxMode = CY_AUTANALOG_SAR_CHAN_CFG_MUX0_SINGLE_ENDED,
        .mux0Sel = CY_AUTANALOG_SAR_CHAN_CFG_MUX0,
        .mux1Sel = CY_AUTANALOG_SAR_CHAN_CFG_MUX0,
        .sampleTimeEn = true,
        .sampleTime = CY_AUTANALOG_SAR_SAMPLE_TIME0,
        .accEn = true,
        .accCount = AMIC_HQ_ACCUMULATION,
        .calReq = CY_AUTANALOG_SAR_CAL_DISABLED,
        .nextAction = CY_AUTANALOG_SAR_NEXT_ACTION_STATE_STOP,
    },
};
cy_stc_autanalog_fifo_cfg_t adc_hs_mono_fifo_cfg =
{
    .split = CY_AUTANALOG_FIFO_SPLIT1,
    .level =
    {
        159U, 
        0U, 
        0U, 
        0U, 
        0U, 
        0U, 
        0U, 
        0U, 
    },
};
cy_stc_autanalog_sar_t adc_hs_mono_cfg =
{
    .sarStaCfg = &adc_hs_mono_sta_cfg,
    .hsSeqTabNum = sizeof(adc_hs_mono_seq_hs_cfg)/sizeof(adc_hs_mono_seq_hs_cfg[0U]),
    .hsSeqTabArr = &adc_hs_mono_seq_hs_cfg[0U],
    .lpSeqTabNum = 0U,
    .lpSeqTabArr = NULL,
    .firNum = 0U,
    .firCfg = NULL,
    .fifoCfg = &adc_hs_mono_fifo_cfg,
};
cy_stc_autanalog_stt_sar_t adc_hs_mono_stt[] =
{
    {
        .unlock = false,
        .enable = false,
        .trigger = false,
        .entryState = 0U,
    },
    {
        .unlock = false,
        .enable = false,
        .trigger = false,
        .entryState = 0U,
    },
    {
        .unlock = true,
        .enable = true,
        .trigger = false,
        .entryState = 0U,
    },
    {
        .unlock = true,
        .enable = true,
        .trigger = true,
        .entryState = 0U,
    },
    {
        .unlock = true,
        .enable = false,
        .trigger = false,
        .entryState = 0U,
    },
};
cy_stc_autanalog_cfg_t amic_hq_mono_cfg =
{
    .prb = NULL,
    .ac = &amic_hq_mono_states_cfg,
    .ctb =
    {
        &mic_preamps_and_filters_hq_mono_cfg, 
        NULL, 
    },
    .ptcomp =
    {
        NULL, 
    },
    .dac =
    {
        NULL, 
        NULL, 
    },
    .sar =
    {
        &adc_hs_mono_cfg, 
    },
};
cy_stc_autanalog_stt_t amic_hq_mono_stt[] =
{
    {
        .ac = &amic_hq_mono_states_stt[0U],
        .prb = NULL,
        .ctb =
        {
            &mic_preamps_and_filters_hq_mono_stt[0U], 
            NULL, 
        },
        .ptcomp =
        {
            NULL, 
        },
        .dac =
        {
            NULL, 
            NULL, 
        },
        .sar =
        {
            &adc_hs_mono_stt[0U], 
        },
    },
    {
        .ac = &amic_hq_mono_states_stt[1U],
        .prb = NULL,
        .ctb =
        {
            &mic_preamps_and_filters_hq_mono_stt[1U], 
            NULL, 
        },
        .ptcomp =
        {
            NULL, 
        },
        .dac =
        {
            NULL, 
            NULL, 
        },
        .sar =
        {
            &adc_hs_mono_stt[1U], 
        },
    },
    {
        .ac = &amic_hq_mono_states_stt[2U],
        .prb = NULL,
        .ctb =
        {
            &mic_preamps_and_filters_hq_mono_stt[2U], 
            NULL, 
        },
        .ptcomp =
        {
            NULL, 
        },
        .dac =
        {
            NULL, 
            NULL, 
        },
        .sar =
        {
            &adc_hs_mono_stt[2U], 
        },
    },
    {
        .ac = &amic_hq_mono_states_stt[3U],
        .prb = NULL,
        .ctb =
        {
            &mic_preamps_and_filters_hq_mono_stt[3U], 
            NULL, 
        },
        .ptcomp =
        {
            NULL, 
        },
        .dac =
        {
            NULL, 
            NULL, 
        },
        .sar =
        {
            &adc_hs_mono_stt[3U], 
        },
    },
    {
        .ac = &amic_hq_mono_states_stt[4U],
        .prb = NULL,
        .ctb =
        {
            &mic_preamps_and_filters_hq_mono_stt[4U], 
            NULL, 
        },
        .ptcomp =
        {
            NULL, 
        },
        .dac =
        {
            NULL, 
            NULL, 
        },
        .sar =
        {
            &adc_hs_mono_stt[4U], 
        },
    },
};
cy_stc_autanalog_t amic_hq_mono_init =
{
    .configuration = &amic_hq_mono_cfg,
    .numSttEntries = sizeof(amic_hq_mono_stt)/sizeof(amic_hq_mono_stt[0U]),
    .stateTransitionTable = &amic_hq_mono_stt[0U],
};


