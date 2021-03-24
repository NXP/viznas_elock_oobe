/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef _SLN_TIME_H_
#define _SLN_TIME_H_

#include "fsl_common.h"

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*! @brief Time_Init.
 *
 * Init the PIT timer 1.
 */
void Time_Init(uint32_t unit_us);

/*! @brief TIME_Now.
 *
 * Get the PIT timer 1.
 */
uint32_t Time_Now();

uint32_t Time_Current();

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /*_SLN_TIME_H_*/
