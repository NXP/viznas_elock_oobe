/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef _SLN_AUDIO_H_
#define _SLN_AUDIO_H_

/*! @brief Audio samples */
typedef enum
{
    AUDIO_HOME_WELCOME            = 0,
    AUDIO_REGISTRATION_FAILED     = 1,
    AUDIO_REGISTRATION_SUCCESSFUL = 2,

} AudioSamplesID;

#if defined(__cplusplus)
extern "C" {
#endif

void Audio_Start(void);

int Audio_Play_Samples(AudioSamplesID audioSampleId);

uint8_t Get_Audio_Player_Status(void);

#if defined(__cplusplus)
}
#endif

#endif
