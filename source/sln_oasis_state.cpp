/*
 * Copyright 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "FreeRTOS.h"
#include "semphr.h"

#include "commondef.h"
#include "database.h"
#include "sln_oasis_state.h"
#include "sln_dev_cfg.h"
#include "sln_timers.h"
#include "sln_lpm.h"
#if VOICE_PROMPT
#include "sln_audio.h"
#endif
#include "sln_system_state.h"

#include <string>

#include "oasis.h"
#include "oasislite_runtime.h"

static QMsg sFaceAddNew;
static QMsg sFaceDel;
static QMsg sFaceRec;
static QMsg sFaceLock;
static uint8_t s_appType;
static bool s_face_detect   = false;
static bool s_firstDetected = false;

extern volatile uint8_t g_AddNewFace;
extern volatile uint8_t g_RemoveExistingFace;
extern volatile uint8_t g_RecFace;
extern volatile uint8_t g_FaceSystemLocked;
volatile static uint32_t s_API_Events = 0;
extern std::string g_AddNewFaceName;

void StartDeregistrationProcess(void)
{
    QMsg *pQMsg;

    StartDeregistrationTimers();
    g_RemoveExistingFace           = 1;
    s_face_detect                  = false;
    s_firstDetected                = false;
    sFaceDel.msg.cmd.data.del_face = g_RemoveExistingFace;
    pQMsg                          = &sFaceDel;
    Oasis_SendQMsg((void *)&pQMsg);
    StopLockProcess();
}

void StopDeregistrationProcess(uint8_t event)
{
    QMsg *pQMsg;

    StopDeregistrationTimers();
    s_API_Events                   = 1 << event;
    g_RemoveExistingFace           = 0;
    sFaceDel.msg.cmd.data.del_face = g_RemoveExistingFace;
    pQMsg                          = &sFaceDel;
    Oasis_SendQMsg((void *)&pQMsg);

    if (event != kEvents_API_Layer_DeregCanceled)
    {
        StartLockProcess(true);
    }
}

void StartRegistrationProcess(void)
{
    QMsg *pQMsg;

    g_AddNewFace                         = 1;
    s_face_detect                        = false;
    s_firstDetected                      = false;
    sFaceAddNew.msg.cmd.data.add_newface = g_AddNewFace;
    pQMsg                                = &sFaceAddNew;
    Oasis_SendQMsg((void *)&pQMsg);

    StartRegistrationTimers();
    StopLockProcess();
}

void StopRegistrationProcess(uint8_t event)
{
    QMsg *pQMsg;
    StopRegistrationTimers();

    if (s_appType != APP_TYPE_USERID)
    {
        if (event == kEvents_API_Layer_RegSuccess)
        {
#if VOICE_PROMPT
            Audio_Play_Samples(AUDIO_REGISTRATION_SUCCESSFUL);
#endif
        }
        else if (event == kEvents_API_Layer_RegFailed)
        {
#if VOICE_PROMPT
            Audio_Play_Samples(AUDIO_REGISTRATION_FAILED);
#endif
        }
    }
    s_API_Events = 1 << event;
    g_AddNewFaceName.assign("");
    g_AddNewFace                         = 0;
    sFaceAddNew.msg.cmd.data.add_newface = g_AddNewFace;
    pQMsg                                = &sFaceAddNew;
    Oasis_SendQMsg((void *)&pQMsg);

    if (event != kEvents_API_Layer_RegCanceled)
    {
        StartLockProcess(true);
    }
}

void StartRecognitionProcess(void)
{
    QMsg *pQMsg;
    g_RecFace                      = 1;
    s_face_detect                  = false;
    s_firstDetected                = false;
    sFaceRec.msg.cmd.data.rec_face = g_RecFace;
    pQMsg                          = &sFaceRec;
    Oasis_SendQMsg((void *)&pQMsg);
    if (Cfg_AppDataGetLowPowerMode() == LOW_POWER_MODE_ON)
    {
        StartRecNoFaceTimers();
        StartDetNoFaceTimers();
    }
    else
    {
        StopRecNoFaceTimers();
        StopDetNoFaceTimers();
    }
    StopLockProcess();
}

void StopRecognitionProcess(uint8_t event)
{
    QMsg *pQMsg;

    StopRecNoFaceTimers();
    StopDetNoFaceTimers();
    s_API_Events                   = 1 << event;
    g_RecFace                      = 0;
    sFaceRec.msg.cmd.data.rec_face = g_RecFace;
    pQMsg                          = &sFaceRec;
    Oasis_SendQMsg((void *)&pQMsg);

    if (s_appType == APP_TYPE_USERID && event != kEvents_API_Layer_RecCanceled)
    {
        StartRecognitionProcess();
        return;
    }

    if (event == kEvents_API_Layer_RecSuccess)
    {
#if VOICE_PROMPT
        Audio_Play_Samples(AUDIO_HOME_WELCOME);
#endif
        StartLockProcess(true);
    }
    else if (event == kEvents_API_Layer_RecFailed)
    {
        StartLockProcess(true);
    }
}

void StartLockProcess(bool timerEnable)
{
    QMsg *pQMsg;

    if (APP_TYPE_USERID == s_appType)
    {
        /* If is is userid app_type just restart the recognition proceess */
        StartRecognitionProcess();
        return;
    }
    g_FaceSystemLocked = 1;
    sFaceLock.id       = QMSG_FACEREC_STOP;
    pQMsg              = &sFaceLock;
    Oasis_SendQMsg((void *)&pQMsg);

    if (timerEnable)
    {
        StartLockTimers();
    }
}

void StopLockProcess(void)
{
    QMsg *pQMsg;

    if (APP_TYPE_USERID == s_appType)
    {
        /* If it is userid do nothing  */
        return;
    }
    StopLockTimers();
    SysState_Set(sysStateDetectedNoUser);
    g_FaceSystemLocked = 0;
    s_API_Events       = kEvents_API_Layer_NoEvent;
    sFaceLock.id       = QMSG_FACEREC_START;
    pQMsg              = &sFaceLock;
    Oasis_SendQMsg((void *)&pQMsg);
}

void Oasis_API_Detect(int dt)
{
    if (dt != -1)
    {
        s_face_detect = true;
        if (g_AddNewFace)
        {
            ResetRegNoFaceTimers();
            if (!s_firstDetected)
            {
                s_firstDetected = true;
                ResetRegHasFaceTimers();
            }
        }
        else if (g_RemoveExistingFace)
        {
            ResetDeregNoFaceTimers();
            if (!s_firstDetected)
            {
                s_firstDetected = true;
                ResetDeregHasFaceTimers();
            }
        }
        else if (g_RecFace)
        {
            ResetDetNoFaceTimers();
            if (!s_firstDetected)
            {
                s_firstDetected = true;
                ResetRecNoFaceTimers();
            }
        }
    }
    else
    {
        s_face_detect = false;
    }
}

void Oasis_API_Recognize(face_info_t face_info)
{
    if (face_info.recognize)
    {
        if (g_AddNewFace)
        {
            // fix recognized prompt in registration, no need to do anything here.
        }
        else if (g_RemoveExistingFace)
        {
            DB_Del(face_info.name);
            StopDeregistrationProcess(kEvents_API_Layer_DeregSuccess);
        }
        else if (g_RecFace)
        {
            StopRecognitionProcess(kEvents_API_Layer_RecSuccess);
        }
    }
    else
    {
        // recognised failed
    }
}

void Oasis_API_Enrolment(face_info_t face_info)
{
    if (face_info.enrolment == false)
    {
        if (face_info.enrolment_result == OASIS_REG_RESULT_CANCELED)
        {
            // Do nothing.
        }
        else
        {
            StopRegistrationProcess(kEvents_API_Layer_RegFailed);
        }
    }
    else
    {
        if (face_info.enrolment_result == OASIS_REG_RESULT_OK)
        {
            // Do nothing.
        }
        else if (face_info.enrolment_result == OASIS_REG_RESULT_DUP)
        {
            StopRegistrationProcess(kEvents_API_Layer_FaceExist);
        }
    }
}

void Oasis_TimerCallback(uint8_t id_timer)
{
    switch (id_timer)
    {
        case TIMER_REG_NO_FACE:
        {
            if (g_AddNewFace)
            {
                StopRegistrationProcess(kEvents_API_Layer_RegFailed);
            }
        }
        break;
        case TIMER_REG_HAS_FACE:
        {
            if (g_AddNewFace)
            {
                StopRegistrationProcess(kEvents_API_Layer_RegFailed);
            }
        }
        break;
        case TIMER_DEREG_NO_FACE:
        {
            StopDeregistrationProcess(kEvents_API_Layer_DeregFailed);
        }
        break;
        case TIMER_DEREG_HAS_FACE:
        {
            StopDeregistrationProcess(kEvents_API_Layer_DeregFailed);
        }
        break;
        case TIMER_SYSTEM_LOCKED:
        {
            StartRecognitionProcess();
        }
        break;
        case TIMER_DET_NO_FACE:
        {
            StopRecognitionProcess(kEvents_API_Layer_RecFailed);
            LPM_SendControlStatus(LPM_DetNoFaceTimeout, 0);
        }
        break;
        case TIMER_REC_NO_FACE:
        {
            StopRecognitionProcess(kEvents_API_Layer_RecFailed);
            LPM_SendControlStatus(LPM_RecNoFaceTimeout, 0);
        }
        break;
    }
}
uint32_t Oasis_Get_Events()
{
    return s_API_Events;
}

void Oasis_State_Init(uint8_t appType)
{
    sFaceAddNew.id = QMSG_FACEREC_ADDNEWFACE;
    sFaceDel.id    = QMSG_FACEREC_DELFACE;
    sFaceRec.id    = QMSG_FACEREC_RECFACE;
}
