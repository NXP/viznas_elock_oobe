/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DISPLAY_BUFFERS     2
#define DISPLAY_MSG_Q_COUNT 8

/*******************************************************************************
 * API
 *******************************************************************************/
int Display_SendQMsg(void *msg);
int Display_Start();
void Display_Init_Task(void *param);
int Display_Sync();
int Display_Update(uint32_t backBuffer);
void Display_Deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* _DISPLAY_H_ */
