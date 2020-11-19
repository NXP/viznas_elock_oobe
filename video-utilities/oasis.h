/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#ifndef _OASIS_H_
#define _OASIS_H_

#include "commondef.h"
#include "oasislite_runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define FACEREC_MSG_Q_COUNT 8

enum{
	E_ANGER = OASIS_EMO_ID_ANGER,
	E_DISGUST = OASIS_EMO_ID_DISGUST,
	E_FEAR =  OASIS_EMO_ID_FEAR,
	E_HAPPY =    OASIS_EMO_ID_HAPPY,
	E_SAD = OASIS_EMO_ID_SAD,
	E_SURPRISED = OASIS_EMO_ID_SURPRISED,
	E_NEUTRAL =    OASIS_EMO_ID_NEUTRAL,
	E_INVALID =    OASIS_EMO_ID_INVALID

};

enum{
	EMOTION_2_TYPES = OASIS_EMOTION_MODE_2,
	EMOTION_4_TYPES = OASIS_EMOTION_MODE_4,
	EMOTION_7_TYPES = OASIS_EMOTION_MODE_7,

};

typedef enum{
    OASIS_STATE_FACE_REC_START,
    OASIS_STATE_FACE_REC_STOP,
    OASIS_STATE_FACE_DEREG_START,
    OASIS_STATE_FACE_DEREG_STOP,
    OASIS_STATE_FACE_REG_START,
    OASIS_STATE_FACE_REG_STOP,
    OASIS_STATE_INVALID
} OasisState;

typedef struct {

    uint8_t*  dataIR;   //buffer size REC_RECT_WIDTH*REC_RECT_HEIGHT*3
    uint8_t*  dataRGB;  //secondary camera raw data

} FaceRecBuffer;

/*******************************************************************************
 * API
 *******************************************************************************/

int Oasis_Start();
int Oasis_SendQMsg(void* msg);

#ifdef __cplusplus
}
#endif

#endif /* _OASIS_H_ */
