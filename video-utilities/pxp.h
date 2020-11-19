/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#ifndef _PXP_H_
#define _PXP_H_

#include "fsl_elcdif.h"
#include "fsl_pxp.h"
#include "sln_rvdisp.h"

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define PXP_MSG_Q_COUNT 10

/*******************************************************************************
 * API
 *******************************************************************************/
void APP_PXP_Start(void);
void ConvertRGB2YUV(uint16_t *sourceLcdBuffer, uint16_t *destLcdBuffer);
void RGB5652YUV(uint16_t source1, uint16_t source2, uint16_t *dest1, uint16_t *dest2);
int PXP_SendQMsg(void *msg);
void APP_PXP_Deinit(void);

#ifdef __cplusplus
}
#endif
#endif
