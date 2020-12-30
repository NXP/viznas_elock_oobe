/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#if RTVISION_BOARD
	#include "board_rtvision.h"
#elif RT106F_ELOCK_BOARD
	#include "board_rt106f_elock.h"
#else
	#error "***ERROR***"
#endif

#endif /* _BOARD_H_ */
