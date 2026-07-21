/******************************************************************************
* File Name:   svc_model.c
*
* Description: SVC Models file. This contains MTB ML model + HMM model required
* for wakeword detection.
*
*******************************************************************************/
#include "cy_svc_model.h"

#ifndef ENABLE_SVC_ML_MW_SUPPORT
#include "MY_LPWWD_INT8_tflm_less_model_int8x8.h"
const bool SVC_MODEL_DUAL_WAKEWORD_ENABLED = false;
const char *SVC_MODEL_BINARY_BUFFER = (const char*) MY_LPWWD_MODEL_INT8_model_bin;
const char *SVC_MODEL_PARAMS_BUFFER = (const char*) NULL;
const unsigned int SVC_MODEL_ARENA_BUFFER_SIZE = MY_LPWWD_MODEL_INT8_ARENA_SIZE;
#else
const bool SVC_MODEL_DUAL_WAKEWORD_ENABLED = false;
#endif

/* HMM based post processing not supported for xiaodu xiaodu */
#ifdef ENABLE_IFX_LPWWD_HMMS
    #error "HMM based post processing not supported for xiaodu xiaodu model, please disable ENABLE_IFX_LPWWD_HMMS macro"
#else
    const int32_t lpwwd_pp_config_params[] = {
        #include "CUSTOM_LPWWD_PP_parms.h"
        };

    const int32_t* SVC_MODEL_LPWWD_PP_CONFIG_PARAMS = (const int32_t*)lpwwd_pp_config_params;
#endif
