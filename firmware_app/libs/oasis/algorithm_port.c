/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 */

#include "stddef.h"
#include "oasislite2D_runtime.h"

/*Define whether enable customized face recognition algorithm*/
#define CUSTOMIZE_FACE_REC_ALGO 0

#if CUSTOMIZE_FACE_REC_ALGO

/*Light face recognition model*/
#include "xxxxxx.h"
#define FACE_REC_NANOAI_MODEL_INSTANCE xxx_int8_binary_model
#define FACE_REC_NANOAI_MODEL_DATA_INSTANCE xxx_int8_binary_model_data
#define FACE_REC_NANOAI_MODEL_OUTPUT_ID XXX_INT8_B_PRE_FC1_ID

#if 0
/*Heavy face recognition model, it is optional, if no heavy model, use same value with Light model*/
#include "yyyyyy.h"
#define FACE_REC_NANOAI_MODEL_INSTANCE_HEAVY yyy_int8_binary_model
#define FACE_REC_NANOAI_MODEL_DATA_INSTANCE_HEAVY yyy_int8_binary_model_data
#define FACE_REC_NANOAI_MODEL_OUTPUT_ID_HEAVY YYY_INT8_B_PRE_FC1_ID
#else
#define FACE_REC_NANOAI_MODEL_INSTANCE_HEAVY NANOAI_MODEL_INSTANCE
#define FACE_REC_NANOAI_MODEL_DATA_INSTANCE_HEAVY NANOAI_MODEL_DATA_INSTANCE
#define FACE_REC_NANOAI_MODEL_OUTPUT_ID_HEAVY NANOAI_MODEL_OUTPUT_ID

#endif

/*Model input and output size*/
#define FACE_REC_FACE_RECOGNIZE_INPUT_C 3
#define FACE_REC_FACE_RECOGNIZE_INPUT_H 112
#define FACE_REC_FACE_RECOGNIZE_INPUT_W 112
#define FACE_REC_FACE_RECOGNIZE_OUTPUT_C 512


void face_recognize_get_parameters(OASISLTModelClass_t cls, OASISLTCustFaceRec_t* para)
{
    if (cls == OASISLT_MODEL_CLASS_LIGHT) {
        para->model = FACE_REC_NANOAI_MODEL_INSTANCE;
        para->model_data = FACE_REC_NANOAI_MODEL_DATA_INSTANCE;
        para->outputID = FACE_REC_NANOAI_MODEL_OUTPUT_ID;

    } else {
        para->model = FACE_REC_NANOAI_MODEL_INSTANCE_HEAVY;
        para->model_data = FACE_REC_NANOAI_MODEL_DATA_INSTANCE_HEAVY;
        para->outputID = FACE_REC_NANOAI_MODEL_OUTPUT_ID_HEAVY;

    }

    para->input_h = FACE_REC_FACE_RECOGNIZE_INPUT_H;
    para->input_w = FACE_REC_FACE_RECOGNIZE_INPUT_W;
    para->input_c = FACE_REC_FACE_RECOGNIZE_INPUT_C;
    para->output_c = FACE_REC_FACE_RECOGNIZE_OUTPUT_C;
}
#endif
