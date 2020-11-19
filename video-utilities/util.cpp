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

#include "util.h"
#include "commondef.h"

void util_RGB565ToRGB888(uint16_t n565Color, uint8_t *pR, uint8_t *pG, uint8_t *pB)
{
    uint8_t r = (n565Color & RGB565_RED) >> 11;  //>> 8;
    uint8_t g = (n565Color & RGB565_GREEN) >> 5; //>> 3;
    uint8_t b = (n565Color & RGB565_BLUE);       //<< 3;
    r         = (r * 527 + 23) >> 6;
    g         = (g * 259 + 33) >> 6;
    b         = (b * 527 + 23) >> 6;
    *pR       = r;
    *pG       = g;
    *pB       = b;
}

void util_RGB888ToRGB565(uint16_t *pOut, int oW, int oH, uint8_t *pIn, int iW, int iH)
{
    int x_offset = (oW - iW) / 2;
    int y_offset = (oH - iH) / 2;
    memset(pOut, 0x0, (oW * oH * 2));

    for (int i = 0; i < iH; i++)
    {
        for (int j = 0; j < iW; j++)
        {
            pOut[((i + y_offset) * oH + j + x_offset)] = ((uint16_t)(pIn[(i * iH + j) * 3] & 0xF8) << 8) |
                                                         ((uint16_t)(pIn[(i * iH + j) * 3 + 1] & 0xFC) << 3) |
                                                         ((uint16_t)pIn[(i * iH + j) * 3 + 2] >> 3);
        }
    }
}

void util_Crop_C3(
    unsigned char *src, int srcw, int srch, unsigned char *dst, int dstw, int dsth, int top, int left, int elemsize)
{
    int dststride = dstw * elemsize;
    int srcstride = srcw * elemsize;

    const unsigned char *ptr = src + top * srcstride + left * elemsize;
    unsigned char *outptr    = dst;

    for (int y = 0; y < dsth; y++)
    {
        if (dststride < 12)
        {
            for (int x = 0; x < dststride; x++)
            {
                outptr[x] = ptr[x];
            }
        }
        else
        {
            memcpy(outptr, ptr, dststride);
        }

        outptr += dststride;
        ptr += srcstride;
    }
}

void util_RGB2GRAY(const unsigned char *rgb, int w, int h, unsigned char *gray)
{
    unsigned char *ptr0 = gray;

    int remain = w * h;

    int g;

    for (; remain > 0; remain--)
    {
        g     = (rgb[2] * 7472 + rgb[1] * 38469 + rgb[0] * 19595) >> 16;
        *ptr0 = g;
        rgb += 3;
        ptr0++;
    }
}
