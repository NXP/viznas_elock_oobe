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

#ifndef _UTIL_H_
#define _UTIL_H_

#include "fsl_common.h"

#define RGB565_RED   0xf800
#define RGB565_GREEN 0x07e0
#define RGB565_BLUE  0x001f
#define RGB565_BLACK 0x0001

void util_RGB565ToRGB888(uint16_t n565Color, uint8_t *pR, uint8_t *pG, uint8_t *pB);
void util_RGB888ToRGB565(uint16_t *pOut, int oW, int oH, uint8_t *pIn, int iW, int iH);
void util_RGB2GRAY(const unsigned char *rgb, int w, int h, unsigned char *gray);

void util_Crop_C3(
    unsigned char *src, int srcw, int srch, unsigned char *dst, int dstw, int dsth, int top, int left, int elemsize);
#endif /* _FACEREC_UTIL_H_ */
