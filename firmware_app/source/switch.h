/*
 * Copyright 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#ifndef _SWITCH_H_
#define _SWITCH_H_

#if RTVISION_BOARD
	#include "switch_rtvision.h"
#elif RT106F_ELOCK_BOARD
	#include "switch_rt106f_elock.h"
#else
	#error "***ERROR***"
#endif

#endif /* _SWITCH_H_ */
