/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef _SYSTEM_STATE_H_
#define _SYSTEM_STATE_H_

/*******************************************************************************
 * Global  Declarations
 ******************************************************************************/
typedef enum _system_states
{
    sysStateNull = 0,
    sysStateIdle,
    sysStateDetectedNoUser,
    sysStateDetectedUnknownUser,
    sysStateDetectedKnownUser
} system_states_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * brief Init the SysState of the leds. Init the PIT timer for toggling led color
 *
 */
void SysState_Init();

/*!
 * brief DeInit the SysState of the leds. DeInit the PIT timer for toggling led color
 *
 */
void SysState_DeInit();

/*!
 * brief Set the state. The state will generate different led colors and effects.
 *
 * @param state One of the state from the enum  system_states_t
 */
void SysState_Set(system_states_t state);

#if defined(__cplusplus)
}
#endif

#endif /* _SYSTEM_STATE_H_ */
