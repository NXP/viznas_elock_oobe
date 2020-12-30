/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */
#if RTVISION_BOARD
#include "platform_disp.h"
#include "App_Common.h"
#include "fsl_log.h"
#include "sln_rvdisp.h"
#include "board.h"

#define RVDISP_SW_ROTATE    (SCREEN_PORTRAIT_MODE ? 1 : 0)

/* Global used for buffer optimization */
Gpu_Hal_Context_t host, *phost;

void RVDisp_CopyFrameToDisplayBuffer(uint16_t *pFrameAddr, uint16_t *pDispBuffer)
{
#if RVDISP_SW_ROTATE
    int i, j;
    for (i = 0; i < RVDISP_WIDTH; i++)
        for (j = RVDISP_HEIGHT - 1; j >= 0; j--)
            pDispBuffer[j * RVDISP_WIDTH + i] = pFrameAddr[i * RVDISP_HEIGHT + RVDISP_HEIGHT - 1 - j];
#else
    memcpy(pDispBuffer, pFrameAddr, RVDISP_HEIGHT * RVDISP_WIDTH * 2);
#endif
}

#if 0
static const struct element {
  uint32_t src;
  uint16_t w, h, x0, y0;
} el[] = {
  { 0,      74, 145,  54,  48 },
  { 5365,  110, 218,  18,  11 },
  { 17355,  28,  29,   8,  66 },
  { 17761,  28,  29,   8, 145 },
  { 18167,  30,  28,  29,  45 },
  { 18587,  30,  29,  29, 167 },
  { 19022,  30,  28,  51,  23 },
  { 19442,  30,  28,  51, 189 },
  { 19862,  28,  28,  73,   1 },
  { 20254,  28,  28,  73, 211 },
  { 20646, 400, 149, 190,  47 },
  { 50446, 194, 120, 597,  47 },
  { 62086, 602,  29, 190, 183 }
};

static int32_t scaling;
static int16_t YOFFSET;

#define FX16(x) ((int32_t)((x)*65536.0f))

static int32_t tween(int32_t t, int32_t a, int32_t b)
{
    uint16_t tu, tu2, tu3;
    if (t < 0)
        return a;
    if (t > 65535)
        return b;
    tu = t;
    tu = ~tu;
    tu2 = ((uint32_t)tu * tu) >> 16;
    tu3 = ((uint32_t)tu2 * tu) >> 16;
    tu3 = ~tu3;

    return a + (((int32_t)tu3 * (b - a)) >> 16);
}

int32_t dc(int16_t c)
{
    return (scaling * c) >> 12;
}

static void setup_element(Gpu_Hal_Context_t *phost, const struct element *e)
{
  App_WrCoCmd_Buffer(phost, BITMAP_SOURCE(e->src));
  App_WrCoCmd_Buffer(phost, BITMAP_LAYOUT(L4, e->w / 2, e->h));
  App_WrCoCmd_Buffer(phost, BITMAP_SIZE(BILINEAR,BORDER,BORDER,(dc(e->w) >> 4) + 1,(dc(e->h) >> 4) + 1));
#ifdef FT81X_ENABLE
  App_WrCoCmd_Buffer(phost, BITMAP_LAYOUT_H((e->w / 2)>>10, 0));
  App_WrCoCmd_Buffer(phost, BITMAP_SIZE_H(((dc(e->w) >> 4) + 1)>>9, ((dc(e->h) >> 4) + 1)>>9));
#endif
}

#define FX16(x) ((int32_t)((x)*65536.0f))

static void draw(Gpu_Hal_Context_t *phost, const struct element *e, int32_t x)
{
  setup_element(phost, e);
  App_WrCoCmd_Buffer(phost, VERTEX2F(dc(e->x0) + x, YOFFSET + dc(e->y0)));
}

#endif

void RVDisp_SendFrame(uint16_t *pFrame)
{
    static uint8_t execCoInit = 1;

    // Upload the framebuffer as a bitmap/texture
#if RVDISP_SW_ROTATE
    Gpu_Hal_WrMem(phost, RAM_G, (uint8_t *)pFrame, RVDISP_WIDTH * RVDISP_HEIGHT * 2);
#else
    Gpu_Hal_WrMem(phost, RAM_G, (uint8_t *)pFrame, RVDISP_HEIGHT * RVDISP_WIDTH * 2);
#endif

    if (execCoInit)
    {
        execCoInit = 0;
#if 0
    // Generate a small display list to use the bitmap
    App_WrDl_Buffer(phost, CLEAR_COLOR_RGB(128, 128, 0) );
    App_WrDl_Buffer(phost, CLEAR(1, 1, 1));

    App_WrDl_Buffer(phost, BITMAP_SOURCE(0));
    App_WrDl_Buffer(phost, BITMAP_LAYOUT(RGB565, RVDISP_WIDTH*2, RVDISP_HEIGHT));
    App_WrDl_Buffer(phost, BITMAP_SIZE(NEAREST, BORDER, BORDER, RVDISP_WIDTH, RVDISP_HEIGHT));

#if 0
    App_WrDl_Buffer(phost, BITMAP_TRANSFORM_A(0));
    App_WrDl_Buffer(phost, BITMAP_TRANSFORM_B(255*256));
    App_WrDl_Buffer(phost, BITMAP_TRANSFORM_D(256));
    App_WrDl_Buffer(phost, BITMAP_TRANSFORM_E(0));
#else
//     App_WrDl_Buffer(phost, BITMAP_TRANSFORM_A(256));
//     App_WrDl_Buffer(phost, BITMAP_TRANSFORM_B(0));
//     App_WrDl_Buffer(phost, BITMAP_TRANSFORM_D(0));
//     App_WrDl_Buffer(phost, BITMAP_TRANSFORM_E(256));
#endif
    App_WrDl_Buffer(phost, BEGIN(BITMAPS));
    App_WrDl_Buffer(phost, VERTEX2II(0, 0, 0, 0) );
    App_WrDl_Buffer(phost, END());

    App_WrDl_Buffer(phost,DISPLAY());

    /* Download the DL into DL RAM */
    App_Flush_DL_Buffer(phost);

     /* Do a swap */
    Gpu_Hal_DLSwap(phost,DLSWAP_FRAME);

#else
    Gpu_CoCmd_Dlstart(phost);
    App_WrCoCmd_Buffer(phost, CLEAR_COLOR_RGB(255, 255, 255));
    App_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));
    App_WrCoCmd_Buffer(phost, BITMAP_HANDLE(0));
    App_WrCoCmd_Buffer(phost, BITMAP_SOURCE(0));
#if RVDISP_SW_ROTATE
    App_WrCoCmd_Buffer(phost, BITMAP_LAYOUT(RGB565, RVDISP_WIDTH * 2, RVDISP_HEIGHT));
    App_WrCoCmd_Buffer(phost, BITMAP_SIZE(NEAREST, BORDER, BORDER, RVDISP_WIDTH, RVDISP_HEIGHT));
    App_WrCoCmd_Buffer(phost, BEGIN(BITMAPS));
    App_WrCoCmd_Buffer(phost, VERTEX2II(0, 0, 0, 0));
    App_WrCoCmd_Buffer(phost, END());
#else
    App_WrCoCmd_Buffer(phost, BITMAP_LAYOUT(RGB565, RVDISP_HEIGHT * 2, RVDISP_WIDTH));
    App_WrCoCmd_Buffer(phost, BITMAP_SIZE(NEAREST, BORDER, BORDER, 0, 0));
    App_WrCoCmd_Buffer(phost, BEGIN(BITMAPS));
    Gpu_CoCmd_LoadIdentity(phost);
    Gpu_CoCmd_Translate(phost, 65536 * (RVDISP_WIDTH/2), 65536 * (RVDISP_HEIGHT/2));
    Gpu_CoCmd_Rotate(phost, 270 * 65536 / 360);
    Gpu_CoCmd_Translate(phost, 65536 * - (RVDISP_HEIGHT/2), 65536 * -(RVDISP_WIDTH/2));
    Gpu_CoCmd_SetMatrix(phost);
    App_WrCoCmd_Buffer(phost, VERTEX2II(0, 0, 0, 0));
    App_WrCoCmd_Buffer(phost, END());
#endif

    App_WrCoCmd_Buffer(phost, DISPLAY());

    Gpu_CoCmd_Swap(phost);
    App_Flush_Co_Buffer(phost);
    Gpu_Hal_WaitCmdfifo_empty(phost);

#endif
    }
}

int RVDisp_PowerCycleDisplay(bool up)
{
    Gpu_Hal_Powercycle(phost, up);
    return 1;
}

int RVDisp_UnInit(uint16_t *pDispBuffer)
{
    App_Common_Close(&host);
    return 1;
}

int RVDisp_Init(uint16_t *pDispBuffer)
{
    phost = &host;

    /* Init HW Hal */
    App_Common_Init(&host);

    return 1;
}
#endif