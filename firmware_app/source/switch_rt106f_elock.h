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

#ifndef _SWITCH_RT106F_ELOCK_H_
#define _SWITCH_RT106F_ELOCK_H_

#if RT106F_ELOCK_BOARD
#include "board.h"

#if defined(__cplusplus)
extern "C" {
#endif

int Switch_Start(void);
#if BOARD_SUPPORT_PARALLEL_LCD
void GPIO2_Combined_16_31_IRQHandler(void);
#else
void GPIO3_Combined_0_15_IRQHandler(void);
#endif
int Switch_SendQMsg(void *msg);
int Switch_SendQMsgFromISR(void *msg);

#if defined(__cplusplus)
}
#endif

#endif
#endif /* _SWITCH_RT106F_ELOCK_H_ */
