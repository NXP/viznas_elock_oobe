/*
 * Copyright 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef __PJDISPLAY_H__
#define __PJDISPLAY_H__


/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
 * Prototype
 ******************************************************************************/

/* Software related display defines */
#define PJDISP_WIDTH 240
#define PJDISP_HEIGHT 320

int PJDisp_Init(void);
int PJDisp_TurnOnBacklight(void);
int PJDisp_TurnOffBacklight(void);
int PJDisp_SendFrame(uint32_t DispBuffer);

#if defined(__cplusplus)
}
#endif

#endif /* __PJDISPLAY_H__ */
