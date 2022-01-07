/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SLN_TIMERS_H_
#define SLN_TIMERS_H_

#include <timers.h>
#define MS_SYSTEM_LOCKED       2000  // MS in which the board is in a locked state after a reg/rec.
#define MS_MANUAL_REG_NO_FACE  15000  // THE TIMEOUT OF MANUAL REGISTRATION WHILE NO FACE HAS BEEN DETECTED
#define MS_MANUAL_REG_HAS_FACE 30000 // THE TIMEOUT OF MANUAL REGISTRATION WHILE A FACE HAS BEEN DETECTED
#define MS_DET_NO_FACE         10000 // MS in which device will enter sleep if no face is detected while running normally
#define MS_REC_NO_FACE         15000 // MS in which device will enter sleep if a face is detected while running normally

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

typedef enum
{
    TIMER_REG_NO_FACE,
    TIMER_REG_HAS_FACE,
    TIMER_DEREG_NO_FACE,
    TIMER_DEREG_HAS_FACE,
    TIMER_SYSTEM_LOCKED,
    TIMER_DET_NO_FACE,
    TIMER_REC_NO_FACE,
} TimerID;

void StartDeregistrationTimers(void);
void ResetDeregNoFaceTimers(void);
void ResetDeregHasFaceTimers(void);
void StopDeregistrationTimers(void);
void StartRegistrationTimers(void);
void ResetRegNoFaceTimers(void);
void ResetRegHasFaceTimers(void);
void StopRegistrationTimers(void);
void StartLockTimers(void);
void StopLockTimers(void);
void StartDetNoFaceTimers(void);
void ResetDetNoFaceTimers(void);
void StopDetNoFaceTimers(void);
void StartRecNoFaceTimers(void);
void ResetRecNoFaceTimers(void);
void StopRecNoFaceTimers(void);

int SLN_Init_Timers(TimerCallbackFunction_t pxCallbackFunction);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* SLN_TIMERS_H_ */
