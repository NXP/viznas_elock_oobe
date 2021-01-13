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

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"

#include "commondef.h"
#include "sln_api_internal.h"
#include "database.h"
#include "display.h"
#include "font.h"
#include "oasis.h"
#include "pxp.h"
#include "sln_dev_cfg.h"
#include "sln_connection.h"
#include "string.h"
#include "util.h"
#include "userid_ui.h"
#include "sln_audio.h"

// lock picture
#include "lock_80_80.h"
#include "unlock_80_80.h"
#include "greenlock_30x38.h"
#include "redlock_30x38.h"
#include "welcomehome_320x122.h"
#include "wifi_16x16.h"
#include "ble_16x16.h"

#if !SCREEN_PORTRAIT_MODE
#if !AUTO_CALIBRATION
/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define LINE_DIMS 5

#define LOCK_SPACING 8
#define LOCK_WIDTH   greenlock_W

#define POS_QUALITY_INFO_X 2
#define POS_QUALITY_INFO_Y 90

#define POS_NAME_X 240
#define POS_NAME_Y 20

#define POS_EMOTION_X 240
#define POS_EMOTION_Y 45

#define POS_REGISTRATION_X 100
#define POS_REGISTRATION_Y 10

#define RGB565_RED   0xf800
#define RGB565_GREEN 0x07e0
#define RGB565_BLUE  0x001f

#define RGB565_NXPGREEN 0xBEA6
#define RGB565_NXPRED   0xFD83
#define RGB565_NXPBLUE  0x6D5B

#define RECT_HEIGHT         20
#define RECT_WIDTH          (APP_AS_WIDTH - LOCK_WIDTH - LOCK_SPACING) / 3
#define RED_RECT_WIDTH      (RECT_WIDTH + LOCK_WIDTH + LOCK_SPACING)
#define GREEN_RECT_WIDTH    RED_RECT_WIDTH
#define BLUE_RECT_WIDTH     (APP_AS_WIDTH - RED_RECT_WIDTH - GREEN_RECT_WIDTH)
#define POS_RECT_Y          (APP_AS_HEIGHT - RECT_HEIGHT)
#define POS_NXPRED_RECT_X   0
#define POS_NXPBLUE_RECT_X  (POS_NXPRED_RECT_X + RED_RECT_WIDTH)
#define POS_NXPGREEN_RECT_X (POS_NXPBLUE_RECT_X + BLUE_RECT_WIDTH)

#define WIFI_ICON_RELATIVE_X    10
#define BLE_ICON_RELATIVE_X     WIFI_ICON_RELATIVE_X + WIFI_W + LOCK_SPACING / 2
#define REGISTRATION_RELATIVE_X 2
#define MODE_RELATIVE_X         (LOCK_SPACING + LOCK_WIDTH)
#define REGISTRATION_RELATIVE_Y 3
#define MODE_RELATIVE_Y         3
#define APP_RELATIVE_X          (LOCK_SPACING + LOCK_WIDTH)
#define APP_RELATIVE_Y          3
/*******************************************************************************
 * Variables
 *******************************************************************************/
uint32_t s_OasisEvents;

extern volatile uint8_t g_AddNewFace;
extern volatile uint8_t g_RemoveExistingFace;
extern volatile uint8_t g_FaceSystemLocked;
static int s_Color        = 0x0;
const char *emotion_str[] = {
    "Anger", "Disgust", "Fear", "Happy", "Sad", "Surprised", "Normal",
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void draw_text(char *pText, int x, int y, int text_color, int bg_color, font_vizn type, uint16_t *pCanvasBuffer);
static void draw_icon(uint16_t *pIcon, int x, int y, int w, int h, int alpha, uint16_t *pCanvasBuffer);
static void draw_fillrect(int x, int y, int w, int h, int rect_color, uint16_t *pCanvasBuffer);
static void UIInfo_UpdateBottomInfoBar(uint16_t *pBufferAddr, QUIInfoMsg infoMsg, uint8_t appType);
static void UIInfo_FaceGuideLines(uint16_t *pBufferAddr);
static void UIInfo_UpdateFaceInfo(uint16_t *pBufferAddr, QUIInfoMsg infoMsg);
static void UIInfo_UpdateFaceBoxUYVY(uint16_t *pBufferAddr, int x1, int x2, int y1, int y2, int dim);
static void UIInfo_UpdateFaceBoxRGB565(uint16_t *pBufferAddr, int x1, int x2, int y1, int y2, int dim);
static void UIInfo_UpdateEmotion(uint16_t *pBufferAddr, uint8_t emotion, int x1, int x2, int y1, int y2, int dim);
static void UIInfo_UpdateNameConf(uint16_t *pBufferAddr, std::string name, int x1, int x2, int y1, int y2, int dim);
static void UIInfo_Userid(uint16_t *pBufferAddr, QUIInfoMsg infoMsg, uint8_t p_DisplayInterfaceMode);
static void UIInfo_Elock(uint16_t *pBufferAddr, QUIInfoMsg infoMsg, uint8_t p_DisplayInterfaceMode);

/*******************************************************************************
 * Code
 *******************************************************************************/

static void draw_text(char *pText, int x, int y, int text_color, int bg_color, font_vizn type, uint16_t *pCanvasBuffer)
{
    put_string(x, y, pText, text_color, bg_color, type, pCanvasBuffer, APP_AS_WIDTH);
}

static void draw_icon(uint16_t *pIcon, int x, int y, int w, int h, int alpha, uint16_t *pCanvasBuffer)
{
    for (int i = y; i < (y + h); i++)
    {
        for (int j = x; j < (x + w); j++)
        {
            if (*pIcon <= alpha)
                *(pCanvasBuffer + i * APP_AS_WIDTH + j) = *pIcon++;
            else
                pIcon++;
        }
    }
}

static void draw_fillrect(int x, int y, int w, int h, int rect_color, uint16_t *pCanvasBuffer)
{
    for (int i = x; i < (x + w); i++)
        for (int j = y; j < (y + h); j++)
            *(pCanvasBuffer + j * APP_AS_WIDTH + i) = rect_color;
}

static void UIInfo_UpdateQualityInfo(uint16_t *pAsBufferAddr, QUIInfoMsg infoMsg)
{
    char tstring[64];
    int color;
    font_vizn type = OPENSANS16;

    if (Cfg_AppDataGetVerbosity() != VERBOSE_MODE_L3)
        return;

    memset(tstring, 0x0, 64);
    sprintf(tstring, "sim:%.2f",infoMsg.similar/100.0f);

    if ((int)infoMsg.similar > 0)
    {
        color = RGB565_BLUE;
    }
    else
    {
        color = RGB565_RED;
    }
    draw_text(tstring, POS_QUALITY_INFO_X + CAMERA_SURFACE_SHIFT, POS_QUALITY_INFO_Y, color, -1, type, pAsBufferAddr);

    memset(tstring, 0x0, 64);
    sprintf(tstring, "blur:%d", infoMsg.blur);
    if (infoMsg.blur == 1)
    {
        color = RGB565_RED;
    }
    else if (infoMsg.blur == 0)
    {
        color = RGB565_GREEN;
    }
    else
    {
        color = RGB565_BLUE;
    }
    draw_text(tstring, POS_QUALITY_INFO_X + CAMERA_SURFACE_SHIFT, POS_QUALITY_INFO_Y + 20, color, -1, type,
              pAsBufferAddr);

    memset(tstring, 0x0, 64);
    sprintf(tstring, "rgbLive:%d", infoMsg.rgbLive);
    if (infoMsg.rgbLive == 0)
    {
        color = RGB565_RED;
    }
    else if (infoMsg.rgbLive == 1)
    {
        color = RGB565_GREEN;
    }
    else
    {
        color = RGB565_BLUE;
    }
    draw_text(tstring, POS_QUALITY_INFO_X + CAMERA_SURFACE_SHIFT, POS_QUALITY_INFO_Y + 40, color, -1, type,
              pAsBufferAddr);

    memset(tstring, 0x0, 64);
    sprintf(tstring, "irLive:%d", infoMsg.irLive);
    if (infoMsg.irLive == 0)
    {
        color = RGB565_RED;
    }
    else if (infoMsg.irLive == 1)
    {
        color = RGB565_GREEN;
    }
    else
    {
        color = RGB565_BLUE;
    }

    draw_text(tstring, POS_QUALITY_INFO_X + CAMERA_SURFACE_SHIFT, POS_QUALITY_INFO_Y + 60, color, -1, type,
              pAsBufferAddr);

    memset(tstring, 0x0, 64);
    sprintf(tstring, "front:%d", infoMsg.front);
    if (infoMsg.front == 0)
    {
        color = RGB565_RED;
    }
    else if (infoMsg.front == 1)
    {
        color = RGB565_GREEN;
    }
    else
    {
        color = RGB565_BLUE;
    }

    draw_text(tstring, POS_QUALITY_INFO_X + CAMERA_SURFACE_SHIFT, POS_QUALITY_INFO_Y + 80, color, -1, type,
              pAsBufferAddr);
}

static void UIInfo_UpdateBottomInfoBar(uint16_t *pBufferAddr, QUIInfoMsg infoMsg, uint8_t appType)
{
    char tstring[64];
    int db_count     = 0;
    std::string name = infoMsg.name;
    uint16_t *pIcon  = NULL;

    draw_fillrect(POS_NXPGREEN_RECT_X, POS_RECT_Y, GREEN_RECT_WIDTH, RECT_HEIGHT, RGB565_NXPGREEN, pBufferAddr);
    draw_fillrect(POS_NXPBLUE_RECT_X, POS_RECT_Y, BLUE_RECT_WIDTH, RECT_HEIGHT, RGB565_NXPBLUE, pBufferAddr);
    draw_fillrect(POS_NXPRED_RECT_X, POS_RECT_Y, RED_RECT_WIDTH, RECT_HEIGHT, RGB565_NXPRED, pBufferAddr);

    DB_Count(&db_count);
    memset(tstring, 0x0, 64);
    sprintf(tstring, "Registered Users:%d", db_count);
    draw_text(tstring, POS_NXPGREEN_RECT_X + REGISTRATION_RELATIVE_X, POS_RECT_Y + REGISTRATION_RELATIVE_Y, RGB565_BLUE,
              RGB565_NXPGREEN, OPENSANS8, pBufferAddr);

    memset(tstring, 0x0, 64);
    if (APP_TYPE_USERID == appType)
    {
        sprintf(tstring, "APP: %s", "userid");
    }
    else if (APP_TYPE_ELOCK_LIGHT == appType || APP_TYPE_ELOCK_HEAVY == appType)
    {
        sprintf(tstring, "APP: %s", "elock");
    }
    else
    {
        sprintf(tstring, "APP: %s", "access");
    }
    draw_text(tstring, POS_NXPRED_RECT_X + APP_RELATIVE_X, POS_RECT_Y + APP_RELATIVE_Y, RGB565_BLUE, RGB565_NXPRED,
              OPENSANS8, pBufferAddr);

    // memset(tstring, 0x0, 64);
    // sprintf(tstring, "Mode:%s", ENROLMENT_MODE_MANUAL == Cfg_AppDataGetEnrolmentMode() ? "Manual" : "Auto");
    // draw_text(tstring, POS_NXPRED_RECT_X + MODE_RELATIVE_X, POS_RECT_Y + REGISTRATION_RELATIVE_Y, RGB565_BLUE,
    //           RGB565_NXPRED, OPENSANS8, pBufferAddr);

    if (SLN_Connection_WIFI_isConnected())
    {
        pIcon = (uint16_t *)wifi16x16_data;
    }
    else
    {
        pIcon = (uint16_t *)no_wifi16x16_data;
    }
    draw_icon(pIcon, POS_NXPBLUE_RECT_X + WIFI_ICON_RELATIVE_X, 223, WIFI_W, WIFI_H, 0xE000, pBufferAddr);
    if (SLN_Connection_BLE_isOn())
    {
        pIcon = (uint16_t *)bluetooth16x16_data;
    }
    else
    {
        pIcon = (uint16_t *)no_bluetooth16x16_data;
    }
    draw_icon(pIcon, POS_NXPBLUE_RECT_X + BLE_ICON_RELATIVE_X, 223, BLE_W, BLE_H, 0xE000, pBufferAddr);
    // draw lock icon

    if (!name.empty())
    {
        pIcon = (uint16_t *)greenlock_30x38;
    }
    else
    {
        pIcon = (uint16_t *)redlock_30x38;
    }
    draw_icon(pIcon, LOCK_SPACING / 2 + POS_NXPRED_RECT_X, 202, 30, 38, 0xfc00, pBufferAddr);
}

static void UIInfo_UpdateOasisState(uint16_t *pBufferAddr)
{
    char tstring[64];
    if (g_AddNewFace)
    {
        memset(tstring, 0x0, 64);
        sprintf(tstring, "Registering");
        draw_text(tstring, CAMERA_SURFACE_SHIFT + POS_REGISTRATION_X, POS_REGISTRATION_Y + 6, 0, RGB565_GREEN,
                  OPENSANS16, pBufferAddr);
    }
    else if (g_RemoveExistingFace)
    {
        memset(tstring, 0x0, 64);
        sprintf(tstring, "Deregistering");
        draw_text(tstring, CAMERA_SURFACE_SHIFT + POS_REGISTRATION_X, POS_REGISTRATION_Y + 6, 0, RGB565_RED, OPENSANS16,
                  pBufferAddr);
    }
}

static void UIInfo_FaceGuideLines(uint16_t *pBufferAddr)
{
    char tstring[64];
    int reg_rect_color;
    memset(tstring, 0x0, 64);

    // registration and unregistration page
    if (!g_FaceSystemLocked)
    {
        //  need mask both side during face adding and removing
        if ((g_AddNewFace || g_RemoveExistingFace))
        {
            draw_fillrect(CAMERA_SURFACE_SHIFT, 0, 60, 240, RGB565_NXPBLUE, pBufferAddr);
            draw_fillrect(CAMERA_SURFACE_SHIFT + 260, 0, 60, 240, RGB565_NXPBLUE, pBufferAddr);
        }

        if (g_AddNewFace)
            reg_rect_color = RGB565_GREEN;
        else if (g_RemoveExistingFace)
            reg_rect_color = RGB565_RED;
        else
            reg_rect_color = RGB565_NXPBLUE;

        draw_fillrect(70 + CAMERA_SURFACE_SHIFT, 10, 2, 60, reg_rect_color, pBufferAddr);
        draw_fillrect(70 + CAMERA_SURFACE_SHIFT, 150, 2, 60, reg_rect_color, pBufferAddr);
        draw_fillrect(250 + CAMERA_SURFACE_SHIFT, 10, 2, 60, reg_rect_color, pBufferAddr);
        draw_fillrect(250 + CAMERA_SURFACE_SHIFT, 150, 2, 60, reg_rect_color, pBufferAddr);

        draw_fillrect(70 + CAMERA_SURFACE_SHIFT, 10, 60, 2, reg_rect_color, pBufferAddr);
        draw_fillrect(70 + CAMERA_SURFACE_SHIFT, 208, 60, 2, reg_rect_color, pBufferAddr);
        draw_fillrect(190 + CAMERA_SURFACE_SHIFT, 10, 60, 2, reg_rect_color, pBufferAddr);
        draw_fillrect(190 + CAMERA_SURFACE_SHIFT, 208, 60, 2, reg_rect_color, pBufferAddr);
    }
}

static void UIInfo_UpdateFaceInfo(uint16_t *pBufferAddr, QUIInfoMsg infoMsg)
{
    char tstring[64];
    std::string name = infoMsg.name;

    switch (s_OasisEvents)
    {
        case kEvents_API_Layer_NoEvent:
            break;
        case 1 << kEvents_API_Layer_FaceExist:
        {
            sprintf(tstring, "Already Registered");
            draw_text(tstring, CAMERA_SURFACE_SHIFT + 70, 10, 0x0, RGB565_RED, OPENSANS16, pBufferAddr);
        }
        break;
        case 1 << kEvents_API_Layer_DeregFailed:
        {
            sprintf(tstring, "Remove Failed");
            draw_text(tstring, CAMERA_SURFACE_SHIFT + 70, 10, 0x0, RGB565_RED, OPENSANS16, pBufferAddr);
        }
        break;
        case 1 << kEvents_API_Layer_DeregSuccess:
        {
            sprintf(tstring, "%s removed", name.c_str());
            draw_text(tstring, CAMERA_SURFACE_SHIFT + 70, 10, 0x0, RGB565_GREEN, OPENSANS16, pBufferAddr);
        }
        break;
        case 1 << kEvents_API_Layer_RegSuccess:
        {
            sprintf(tstring, "%s Added", name.c_str());
            draw_text(tstring, CAMERA_SURFACE_SHIFT + 100, 100, 0x0, RGB565_GREEN, OPENSANS16, pBufferAddr);
        }
        break;
        case 1 << kEvents_API_Layer_RegFailed:
        {
            sprintf(tstring, "Registration Failed");
            draw_text(tstring, CAMERA_SURFACE_SHIFT + 70, 10, 0x0, RGB565_RED, OPENSANS16, pBufferAddr);
        }
        break;
        case 1 << kEvents_API_Layer_RecSuccess:
        {
            uint16_t *pIcon;
            sprintf(tstring, "Unlocked");
            draw_text(tstring, CAMERA_SURFACE_SHIFT + 120, 10, 0, RGB565_GREEN, OPENSANS16, pBufferAddr);

            pIcon = (uint16_t *)welcomehome_320x122;
            draw_icon(pIcon, CAMERA_SURFACE_SHIFT, 60, welcomehome_W, welcomehome_H, 0xffff, pBufferAddr);
        }
        break;

        case 1 << kEvents_API_Layer_RecFailed:
        {
            sprintf(tstring, "Recognition Timeout");
            draw_text(tstring, CAMERA_SURFACE_SHIFT + 70, 10, 0x0, RGB565_RED, OPENSANS16, pBufferAddr);
        }
        break;
        default:
            break;
    }
}

static void UIInfo_UpdateFaceBoxUYVY(uint16_t *pBufferAddr, int x1, int x2, int y1, int y2, int dim)
{
    uint16_t UY, VY, camera_width;
    if (pBufferAddr == NULL)
        return;
    // the conversion si 2 pixel in 2 bytes

    camera_width = APP_CAMERA_WIDTH;
    RGB5652YUV((uint16_t)s_Color, (uint16_t)s_Color, &UY, &VY);

    x1  = x1 - x1 % 2;
    x2  = x2 - x2 % 2;
    dim = dim - dim % 2;

    // x1 and x2 and dim are even are even

    for (int i = x1; i < x2; i = i + 2)
    {
        for (int j = y1; j < (y1 + dim); j++)
        {
            *(pBufferAddr + j * camera_width + i)     = UY;
            *(pBufferAddr + j * camera_width + i + 1) = VY;
        }
    }
    for (int i = y1; i < y2; i++)
    {
        for (int j = x1; j < (x1 + dim - 1); j = j + 2)
        {
            *(pBufferAddr + i * camera_width + j)     = UY;
            *(pBufferAddr + i * camera_width + j + 1) = VY;
        }
    }

    for (int i = x1 + dim; i < x2 - 1; i = i + 2)
    {
        for (int j = 0; j < dim; j++)
        {
            *(pBufferAddr + (y2 - 1 - j) * camera_width + i)     = UY;
            *(pBufferAddr + (y2 - 1 - j) * camera_width + i + 1) = VY;
        }
    }

    for (int i = y1 + dim; i < y2; i++)
    {
        for (int j = 0; j < dim - 1; j = j + 2)
        {
            *(pBufferAddr + i * camera_width + (x2 - j - 2)) = UY;
            *(pBufferAddr + i * camera_width + (x2 - j - 1)) = VY;
        }
    }
}

static void UIInfo_UpdateFaceBoxRGB565(uint16_t *pBufferAddr, int x1, int x2, int y1, int y2, int dim)
{
    if (pBufferAddr == NULL)
        return;
    // face rect
    draw_fillrect(x1, y1, x2 - x1 + 1, dim, s_Color, pBufferAddr);
    draw_fillrect(x1, y1, dim, y2 - y1 + 1, s_Color, pBufferAddr);
    draw_fillrect(x2 - dim, y1, dim, y2 - y1 + 1, s_Color, pBufferAddr);
    draw_fillrect(x1, y2 - dim, x2 - x1 + 1, dim, s_Color, pBufferAddr);
}

static void UIInfo_UpdateNameConf(uint16_t *pBufferAddr, std::string name, int x1, int x2, int y1, int y2, int dim)
{
    // face name and confidence
    if (name.empty())
    {
        return;
    }

    if (pBufferAddr == NULL)
    {
        return;
    }
    put_string(x1 + dim, y1 + dim, (char *)name.c_str(), RGB565_BLACK, s_Color, OPENSANS8, pBufferAddr, APP_AS_WIDTH);
}

static void UIInfo_UpdateEmotion(uint16_t *pBufferAddr, uint8_t emotion, int x1, int x2, int y1, int y2, int dim)
{
//    char tstring[64];
//    if (pBufferAddr == NULL)
//        return;
//    // show the emotion info
//    if (emotion >= E_ANGER && emotion <= E_SURPRISED)
//    {
//        sprintf(tstring, emotion_str[emotion]);
//        draw_text(tstring, x2 - dim - get_stringwidth(tstring, OPENSANS8), y1 + dim, RGB565_BLACK, s_Color, OPENSANS8,
//                  pBufferAddr);
//    }
}

static void UIInfo_Userid(uint16_t *pBufferAddr, QUIInfoMsg infoMsg, uint8_t p_DisplayInterfaceMode)
{
    uint16_t screenWidth, screenHeight;
    int scale_factor = APP_CAMERA_WIDTH / APP_PS_WIDTH;
    int shift_x      = (APP_CAMERA_WIDTH - REC_RECT_WIDTH) / 2;
    int shift_y      = (APP_CAMERA_HEIGHT - REC_RECT_HEIGHT) / 2;
    if (p_DisplayInterfaceMode == DISPLAY_INTERFACE_LOOPBACK)
    {
        // The box is added before the scaling
        scale_factor = 1;
    }

    int x1 = CAMERA_SURFACE_SHIFT + (infoMsg.rect[0] + shift_x) / scale_factor;
    int y1 = (infoMsg.rect[1] + shift_y) / scale_factor;
    int x2 = CAMERA_SURFACE_SHIFT + (infoMsg.rect[2] + shift_x) / scale_factor;
    int y2 = (infoMsg.rect[3] + shift_y) / scale_factor;

    screenWidth  = APP_CAMERA_WIDTH / scale_factor;
    screenHeight = APP_CAMERA_HEIGHT / scale_factor;

    if ((x1 >= CAMERA_SURFACE_SHIFT) && (y1 >= 0) && (x2 >= CAMERA_SURFACE_SHIFT) && (y2 > 0) && (x1 != x2) &&
        (y1 != y2) && x2 < screenWidth && y2 < screenHeight)
    {
        std::string name = infoMsg.name;

        if (name.empty())
        {
            s_Color = RGB565_RED;
        }
        else
        {
            s_Color = RGB565_GREEN;
        }

        if (p_DisplayInterfaceMode == DISPLAY_INTERFACE_INFOBAR)
        {
            UIInfo_UpdateFaceBoxRGB565(pBufferAddr, x1, x2, y1, y2, LINE_DIMS);
            UIInfo_UpdateEmotion(pBufferAddr, infoMsg.emotion, x1, x2, y1, y2, LINE_DIMS);
            UIInfo_UpdateNameConf(pBufferAddr, name, x1, x2, y1, y2, LINE_DIMS);
        }
        else
        {
            UIInfo_UpdateFaceBoxUYVY(pBufferAddr, x1, x2, y1, y2, LINE_DIMS);
        }
    }
}

static void UIInfo_Elock(uint16_t *pBufferAddr, QUIInfoMsg infoMsg, uint8_t p_DisplayInterfaceMode)
{
    if (p_DisplayInterfaceMode == DISPLAY_INTERFACE_INFOBAR)
    {
        memset(pBufferAddr, 0, 2 * (APP_AS_HEIGHT * APP_AS_WIDTH - 1));
        UIInfo_FaceGuideLines(pBufferAddr);
        VIZN_GetEvents(NULL, &s_OasisEvents);
        UIInfo_UpdateFaceInfo(pBufferAddr, infoMsg);
        UIInfo_UpdateQualityInfo(pBufferAddr, infoMsg);
    }
}

void UIInfo_Update(uint16_t *pBufferAddr, QUIInfoMsg infoMsg, uint8_t p_DisplayInterfaceMode)
{
    uint8_t appType = Cfg_AppDataGetApplicationType();
    if (p_DisplayInterfaceMode == DISPLAY_INTERFACE_INFOBAR)
    {
        memset(pBufferAddr, 0, 2 * (APP_AS_HEIGHT * APP_AS_WIDTH - 1));
    }
    switch (appType)
    {
        case APP_TYPE_USERID:
            UIInfo_Userid(pBufferAddr, infoMsg, p_DisplayInterfaceMode);
            break;
        case APP_TYPE_ELOCK_LIGHT:
        case APP_TYPE_ELOCK_HEAVY:
        case APP_TYPE_DOOR_ACCESS_LIGHT:
        case APP_TYPE_DOOR_ACCESS_HEAVY:
            UIInfo_Elock(pBufferAddr, infoMsg, p_DisplayInterfaceMode);
            break;

        default:
            while (1)
            {
            }
    }
    if (p_DisplayInterfaceMode == DISPLAY_INTERFACE_INFOBAR)
    {
        UIInfo_UpdateOasisState(pBufferAddr);
        UIInfo_UpdateBottomInfoBar(pBufferAddr, infoMsg, appType);
    }
}

#endif //#if !AUTO_CALIBRATION
#endif //#if !SCREEN_PORTRAIT_MODE
