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
#define CUSTOMIZE_LIVENESS_ALGO 0

#if CUSTOMIZE_FACE_REC_ALGO

/*Light face recognition model*/
#include "face_rec_model.h"
#define FACE_REC_NANOAI_MODEL_INSTANCE xxx_int8_binary_model
#define FACE_REC_NANOAI_MODEL_DATA_INSTANCE xxx_int8_binary_model_data
#define FACE_REC_NANOAI_MODEL_OUTPUT_ID XXX_INT8_B_PRE_FC1_ID


/*Model input and output size*/
#define FACE_REC_INPUT_C 3
#define FACE_REC_INPUT_H 112
#define FACE_REC_INPUT_W 112
#define FACE_REC_OUTPUT_C 512
#define FACE_REC_THRESHOLD (0.70f)


void face_recognize_get_parameters(OASISLTModelClass_t cls, OASISLTCustFaceRec_t* para)
{
    para->model = FACE_REC_NANOAI_MODEL_INSTANCE;
    para->model_data = FACE_REC_NANOAI_MODEL_DATA_INSTANCE;
    para->outputID = FACE_REC_NANOAI_MODEL_OUTPUT_ID;
    para->th = FACE_REC_THRESHOLD;

    para->input_h = FACE_REC_INPUT_H;
    para->input_w = FACE_REC_INPUT_W;
    para->input_c = FACE_REC_INPUT_C;
    para->output_c = FACE_REC_OUTPUT_C;
}
#endif


#if CUSTOMIZE_LIVENESS_ALGO

/*Light face recognition model*/
#include "liveness_model.h"
#define LIVENESS_NANOAI_MODEL_INSTANCE xxx_int8_binary_model
#define LIVENESS_NANOAI_MODEL_DATA_INSTANCE xxx_int8_binary_model_data
#define LIVENESS_NANOAI_MODEL_OUTPUT_ID XXX_INT8_B_PRE_FC1_ID


/*Model input and output size*/
#define LIVENESS_INPUT_C 3
#define LIVENESS_INPUT_H 112
#define LIVENESS_INPUT_W 112
#define LIVENESS_THRESHOLD (0.5f)


void liveness_get_parameters(OASISLTCustLiveness_t* para)
{
    para->model = LIVENESS_NANOAI_MODEL_INSTANCE;
    para->model_data = LIVENESS_NANOAI_MODEL_DATA_INSTANCE;
    para->outputID = LIVENESS_NANOAI_MODEL_OUTPUT_ID;

    para->input_h = LIVENESS_INPUT_H;
    para->input_w = LIVENESS_INPUT_W;
    para->input_c = LIVENESS_INPUT_C;
    para->th = LIVENESS_THRESHOLD;
}
#endif
