/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include "sln_api_internal.h"
#include "sln_timers.h"

static TimerHandle_t s_Timer_Dereg_No_Face      = NULL;
static TimerHandle_t s_Timer_Dereg_Has_Face     = NULL;
static TimerHandle_t s_Timer_Reg_No_Face        = NULL;
static TimerHandle_t s_Timer_Reg_Has_Face       = NULL;
static TimerHandle_t s_Timer_Lock               = NULL;
static TimerHandle_t s_Timer_Det_No_Face        = NULL;
static TimerHandle_t s_Timer_Rec_No_Face        = NULL;
static uint8_t timer_dereg_no_face_ID           = TIMER_DEREG_NO_FACE;
static uint8_t timer_dereg_has_face_ID          = TIMER_DEREG_HAS_FACE;
static uint8_t timer_reg_no_face_ID             = TIMER_REG_NO_FACE;
static uint8_t timer_reg_has_face_ID            = TIMER_REG_HAS_FACE;
static uint8_t timer_lock_ID                    = TIMER_SYSTEM_LOCKED;
static uint8_t timer_det_no_face_ID             = TIMER_DET_NO_FACE;
static uint8_t timer_rec_no_face_ID             = TIMER_REC_NO_FACE;

void StartDeregistrationTimers(void)
{
    if (s_Timer_Dereg_Has_Face != NULL && s_Timer_Dereg_No_Face != NULL)
    {
        xTimerStart(s_Timer_Dereg_No_Face, 0);
        xTimerStart(s_Timer_Dereg_Has_Face, 0);
    }
}

void ResetDeregNoFaceTimers(void)
{
    if (s_Timer_Dereg_No_Face != NULL && xTimerIsTimerActive(s_Timer_Dereg_No_Face))
    {
        xTimerReset(s_Timer_Dereg_No_Face, 0);
    }
}

void ResetDeregHasFaceTimers(void)
{
    if (s_Timer_Dereg_Has_Face != NULL && xTimerIsTimerActive(s_Timer_Dereg_Has_Face))
    {
        xTimerReset(s_Timer_Dereg_Has_Face, 0);
    }
}

void StopDeregistrationTimers(void)
{
    if (s_Timer_Dereg_Has_Face != NULL && s_Timer_Dereg_No_Face != NULL)
    {
        if (xTimerIsTimerActive(s_Timer_Dereg_No_Face))
            xTimerStop(s_Timer_Dereg_No_Face, 0);
        if (xTimerIsTimerActive(s_Timer_Dereg_Has_Face))
            xTimerStop(s_Timer_Dereg_Has_Face, 0);
    }
}

void StartRegistrationTimers(void)
{
    if (s_Timer_Reg_Has_Face != NULL && s_Timer_Reg_No_Face != NULL)
    {
        xTimerStart(s_Timer_Reg_No_Face, 0);
        xTimerStart(s_Timer_Reg_Has_Face, 0);
    }
}

void ResetRegNoFaceTimers(void)
{
    if (s_Timer_Reg_No_Face != NULL && xTimerIsTimerActive(s_Timer_Reg_No_Face))
    {
        xTimerReset(s_Timer_Reg_No_Face, 0);
    }
}

void ResetRegHasFaceTimers(void)
{
    if (s_Timer_Reg_Has_Face != NULL && xTimerIsTimerActive(s_Timer_Reg_Has_Face))
    {
        xTimerReset(s_Timer_Reg_Has_Face, 0);
    }
}

void StopRegistrationTimers(void)
{
    if (s_Timer_Reg_Has_Face != NULL && s_Timer_Reg_No_Face != NULL)
    {
        if (xTimerIsTimerActive(s_Timer_Reg_No_Face))
            xTimerStop(s_Timer_Reg_No_Face, 0);
        if (xTimerIsTimerActive(s_Timer_Reg_Has_Face))
            xTimerStop(s_Timer_Reg_Has_Face, 0);
    }
}

void StartLockTimers(void)
{
    if (s_Timer_Lock != NULL)
    {
        xTimerStart(s_Timer_Lock, 0);
    }
}

void StopLockTimers(void)
{
    if (s_Timer_Lock != NULL && xTimerIsTimerActive(s_Timer_Lock))
    {
        xTimerStop(s_Timer_Lock, 0);
    }
}

void StartDetNoFaceTimers(void)
{
    if (s_Timer_Det_No_Face != NULL)
    {
        xTimerStart(s_Timer_Det_No_Face, 0);
    }
}

void ResetDetNoFaceTimers(void)
{
    if (s_Timer_Det_No_Face != NULL && xTimerIsTimerActive(s_Timer_Det_No_Face))
    {
        xTimerReset(s_Timer_Det_No_Face, 0);
    }
}

void StopDetNoFaceTimers(void)
{
    if (s_Timer_Det_No_Face != NULL && xTimerIsTimerActive(s_Timer_Det_No_Face))
    {
        xTimerStop(s_Timer_Det_No_Face, 0);
    }
}

void StartRecNoFaceTimers(void)
{
    if (s_Timer_Rec_No_Face != NULL)
    {
        xTimerStart(s_Timer_Rec_No_Face, 0);
    }
}

void ResetRecNoFaceTimers(void)
{
    if (s_Timer_Rec_No_Face != NULL && xTimerIsTimerActive(s_Timer_Rec_No_Face))
    {
        xTimerReset(s_Timer_Rec_No_Face, 0);
    }
}

void StopRecNoFaceTimers(void)
{
    if (s_Timer_Rec_No_Face != NULL && xTimerIsTimerActive(s_Timer_Rec_No_Face))
    {
        xTimerStop(s_Timer_Rec_No_Face, 0);
    }
}

int SLN_Init_Timers(TimerCallbackFunction_t pxCallbackFunction)
{
    s_Timer_Reg_No_Face = xTimerCreate("RegTimerNoFace", pdMS_TO_TICKS(MS_MANUAL_REG_NO_FACE), pdFALSE,
                                       (void *)&timer_reg_no_face_ID, pxCallbackFunction);
    if (s_Timer_Reg_No_Face == NULL)
    {
        return -1;
    }
    s_Timer_Reg_Has_Face = xTimerCreate("RegTimerHasFace", pdMS_TO_TICKS(MS_MANUAL_REG_HAS_FACE), pdFALSE,
                                        (void *)&timer_reg_has_face_ID, pxCallbackFunction);
    if (s_Timer_Reg_Has_Face == NULL)
    {
        return -1;
    }
    s_Timer_Dereg_No_Face = xTimerCreate("DeregTimerNoFace", pdMS_TO_TICKS(MS_MANUAL_REG_NO_FACE), pdFALSE,
                                         (void *)&timer_dereg_no_face_ID, pxCallbackFunction);
    if (s_Timer_Dereg_No_Face == NULL)
    {
        return -1;
    }
    s_Timer_Dereg_Has_Face = xTimerCreate("DeRegTimerHasFace", pdMS_TO_TICKS(MS_MANUAL_REG_HAS_FACE), pdFALSE,
                                          (void *)&timer_dereg_has_face_ID, pxCallbackFunction);
    if (s_Timer_Dereg_Has_Face == NULL)
    {
        return -1;
    }
    s_Timer_Lock = xTimerCreate("TimeLockOasis", pdMS_TO_TICKS(MS_SYSTEM_LOCKED), pdFALSE, (void *)&timer_lock_ID,
                                pxCallbackFunction);
    if (s_Timer_Lock == NULL)
    {
        return -1;
    }
    s_Timer_Det_No_Face  =
        xTimerCreate("DetectNoFace", pdMS_TO_TICKS(MS_DET_NO_FACE), pdFALSE, (void *)&timer_det_no_face_ID, pxCallbackFunction);
    if (s_Timer_Det_No_Face == NULL )
    {
        return -1;
    }
    s_Timer_Rec_No_Face  =
        xTimerCreate("RecognizeNoFace", pdMS_TO_TICKS(MS_REC_NO_FACE), pdFALSE, (void *)&timer_rec_no_face_ID, pxCallbackFunction);
    if (s_Timer_Rec_No_Face == NULL )
    {
        return -1;
    }
    return 0;
}
