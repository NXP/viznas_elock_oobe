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

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"

#include "commondef.h"
#include "database.h"
#include "display.h"
#include "font.h"
#include "oasis.h"
#include "pxp.h"
#include "sln_dev_cfg.h"
#include "sln_system_state.h"
#include "string.h"
#include "util.h"
#include "userid_ui.h"
#include "sln_shell.h"
#include "sln_dev_cfg.h"
#include "camera.h"
#include <map>

/*
Auto-calibration tool will help to generate "Face_width & PWM & liveness" Table according to the calibration settings

calibration setting:
    parameters for the detected face width:
        CALIBRATION_FACEWIDTH_MAX
        CALIBRATION_FACEWIDTH_MIN
        CALIBRATION_FACEWIDTH_INTERVAL

    parameters for PWM of IR camera
        CALIBRATION_PWM_MIN
        CALIBRATION_PWM_MAX
        CALIBRATION_PWM_INTERVAL
        CALIBRATION_PWM_FRAMES

calibration result:
    lf/vf/tf
    tf: the captured frames in current face width and PWM of IR camera ( tf = CALIBRATION_PWM_FRAMES )
    vf: valid frames in current face width and PWM of IR camera ( face had been detected and face bounding box is almost
aligned with reference box ) lf: liveness frames in current face width and PWM of IR camera ( face in valid frames is
regarded as liveness face, lf <= vf <= tf)

calibration conclusion:
    Choose face width and PWM according to lf/vf, vf/tf which are more higher more better

"Face_width & PWM & liveness Table" Format:

************************************************************************************************************************************************************************
* Calibration Completed *
* *
* face_width:CALIBRATION_FACEWIDTH_MAX *
* pwm:CALIBRATION_PWM_MIN    pwm:CALIBRATION_PWM_MIN+CALIBRATION_PWM_INTERVAL
pwm:CALIBRATION_PWM_MIN+2*CALIBRATION_PWM_INTERVAL    ....    pwm:CALIBRATION_PWM_MAX *
* lf/vf/tf                   lf/vf/tf                                            lf/vf/tf ....    lf/vf/tf *
* *
* face_width:CALIBRATION_FACEWIDTH_MAX-CALIBRATION_FACEWIDTH_INTERVAL *
* pwm:CALIBRATION_PWM_MIN    pwm:CALIBRATION_PWM_MIN+CALIBRATION_PWM_INTERVAL
pwm:CALIBRATION_PWM_MIN+2*CALIBRATION_PWM_INTERVAL    ....    pwm:CALIBRATION_PWM_MAX *
* lf/vf/tf                   lf/vf/tf                                            lf/vf/tf ....    lf/vf/tf *
* *
* face_width:CALIBRATION_FACEWIDTH_MAX-2*CALIBRATION_FACEWIDTH_INTERVAL *
* pwm:CALIBRATION_PWM_MIN    pwm:CALIBRATION_PWM_MIN+CALIBRATION_PWM_INTERVAL
pwm:CALIBRATION_PWM_MIN+2*CALIBRATION_PWM_INTERVAL    ....    pwm:CALIBRATION_PWM_MAX *
* lf/vf/tf                   lf/vf/tf                                            lf/vf/tf ....    lf/vf/tf *
* *
* ...... *
* *
* face_width:CALIBRATION_FACEWIDTH_MIN *
* pwm:CALIBRATION_PWM_MIN    pwm:CALIBRATION_PWM_MIN+CALIBRATION_PWM_INTERVAL
pwm:CALIBRATION_PWM_MIN+2*CALIBRATION_PWM_INTERVAL    ....    pwm:CALIBRATION_PWM_MAX *
* lf/vf/tf                   lf/vf/tf                                            lf/vf/tf ....    lf/vf/tf *
* *
************************************************************************************************************************************************************************
*
* calibration steps:
* 1. initialize a wide range for calibration setting parameters to get a rough estimate
* 2. according to the result of step one, modify calibration setting parameters to get more accurate result
* 3. repeatedly step two to figure out the best "Face_width & PWM & liveness" Table, and apply face width and PWM
setting to real project.
*/

#if AUTO_CALIBRATION

#define CALIBRATION_FACEWIDTH_MAX      160
#define CALIBRATION_FACEWIDTH_MIN      80
#define CALIBRATION_FACEWIDTH_INTERVAL 20
#define CALIBRATION_FACEWIDTH_COUNTS \
    ((CALIBRATION_FACEWIDTH_MAX - CALIBRATION_FACEWIDTH_MIN) / CALIBRATION_FACEWIDTH_INTERVAL + 1)

#define CALIBRATION_PWM_MIN      0
#define CALIBRATION_PWM_MAX      100
#define CALIBRATION_PWM_INTERVAL 20
#define CALIBRATION_PWM_FRAMES   100
#define CALIBRATION_PWM_FRAMES_IN_ONE_FACEWIDTH \
    (((CALIBRATION_PWM_MAX - CALIBRATION_PWM_MIN) / CALIBRATION_PWM_INTERVAL + 1) * CALIBRATION_PWM_FRAMES)

#define RGB565_RED   0xf800
#define RGB565_GREEN 0x07e0
#define RGB565_BLUE  0x001f

#define RGB565_NXPGREEN 0xBEA6
#define RGB565_NXPRED   0xFD83
#define RGB565_NXPBLUE  0x6D5B

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

static void draw_rectborder(int x1, int y1, int x2, int y2, int dim, int color, uint16_t *pCanvasBuffer)
{
    draw_fillrect(x1, y1, x2 - x1 + 1, dim, color, pCanvasBuffer);
    draw_fillrect(x1, y1, dim, y2 - y1 + 1, color, pCanvasBuffer);
    draw_fillrect(x2 - dim, y1, dim, y2 - y1 + 1, color, pCanvasBuffer);
    draw_fillrect(x1, y2 - dim, x2 - x1 + 1, dim, color, pCanvasBuffer);
}

void UIInfo_Update(uint16_t *pAsBufferAddr, QUIInfoMsg infoMsg, bool erase, uint8_t p_isLiveView)
{
    static int frame_id = 0;
    int start_frames    = 10;
    int scale_factor    = APP_CAMERA_WIDTH / APP_PS_WIDTH;
    int shift_x         = (APP_CAMERA_WIDTH - REC_RECT_WIDTH) / 2;
    int shift_y         = (APP_CAMERA_HEIGHT - REC_RECT_HEIGHT) / 2;

    int x1                           = CAMERA_SURFACE_SHIFT + (infoMsg.rect[0] + shift_x) / scale_factor;
    int y1                           = (infoMsg.rect[1] + shift_y) / scale_factor;
    int x2                           = CAMERA_SURFACE_SHIFT + (infoMsg.rect[2] + shift_x) / scale_factor;
    int y2                           = (infoMsg.rect[3] + shift_y) / scale_factor;
    std::string name                 = infoMsg.name;
    int similar                      = (int)(infoMsg.similar * 10000 / 100);
    static system_states_t sys_state = sysStateDetectedNoUser;
    static int valid_frames          = 0;
    static int liveness_frames       = 0;
    static int pwm                   = CALIBRATION_PWM_MIN;
    static int last_rect[4]          = {0, 0, 0, 0};
    static int valid_flag            = 0;
    static int first                 = 0;

    static std::map<int, std::map<int, int> > valid_table;
    static std::map<int, std::map<int, int> > liveness_table;

    UsbShell_Printf("x1=%d, y1=%d, x2=%d, y2=%d\r\n", x1, y1, x2, y2);

    if (first == 0)
    {
        // Camera_SetDispMode(DISPALY_MODE_IR);
        Camera_SetDispMode(1);
        Camera_QMsgSetPWM(1, pwm);
        first = 1;
    }

    // clear screen
    memset(pAsBufferAddr, 0, 2 * (APP_AS_HEIGHT * APP_AS_WIDTH - 1));

    char tstring[64];
    memset(tstring, 0x0, 64);

    if ((frame_id - start_frames) > CALIBRATION_PWM_FRAMES_IN_ONE_FACEWIDTH * CALIBRATION_FACEWIDTH_COUNTS)
    {
        // completed page
        Camera_QMsgSetPWM(1, 0);
        sprintf(tstring, "Calibration Completed");
        draw_text(tstring, 0, 0, RGB565_BLUE, -1, OPENSANS16, pAsBufferAddr);
        int p = 0;
        for (int n = CALIBRATION_FACEWIDTH_MAX; n >= CALIBRATION_FACEWIDTH_MIN; n -= CALIBRATION_FACEWIDTH_INTERVAL)
        {
            int q = 0;
            if ((50 + p * 40) < APP_AS_HEIGHT)
            {
                sprintf(tstring, "face width:%d", n);
                draw_text(tstring, 0, 40 + p * 40, RGB565_BLUE, -1, OPENSANS8, pAsBufferAddr);
            }
            for (int m = CALIBRATION_PWM_MIN; m <= CALIBRATION_PWM_MAX; m += CALIBRATION_PWM_INTERVAL)
            {
                UsbShell_Printf("face_width=%d, pwm=%d, liveness_frames=%d, valid_frames=%d\r\n", n, m,
                                liveness_table[n][m], valid_table[n][m]);

                if (((q + 1) * 53 >= APP_AS_WIDTH) || ((70 + p * 40) >= APP_AS_HEIGHT))
                    continue;
                sprintf(tstring, "pwm:%d", m);
                draw_text(tstring, q * 53, 50 + p * 40, RGB565_BLUE, -1, OPENSANS8, pAsBufferAddr);
                memset(tstring, 0x0, 64);
                sprintf(tstring, "%d/%d/%d", liveness_table[n][m], valid_table[n][m], CALIBRATION_PWM_FRAMES);
                int color = RGB565_RED;
                if ((valid_table[n][m] != 0) && (liveness_table[n][m] != 0) &&
                    (((float)liveness_table[n][m] / (float)valid_table[n][m]) < 0.5))
                    color = RGB565_NXPRED;
                else if ((valid_table[n][m] != 0) && (((float)liveness_table[n][m] / (float)valid_table[n][m]) >= 0.5))
                    color = RGB565_GREEN;
                draw_text(tstring, q * 53, 60 + p * 40, color, -1, OPENSANS8, pAsBufferAddr);
                q++;
            }
            p++;
        }
        return;
    }

    sprintf(tstring, "Calibration Starting");
    draw_text(tstring, 0, 10, RGB565_BLUE, -1, OPENSANS16, pAsBufferAddr);

    sprintf(tstring, "Fill you face bounding box");
    draw_text(tstring, 0, 30, RGB565_BLUE, -1, OPENSANS16, pAsBufferAddr);

    sprintf(tstring, "to the blue Rectangle");
    draw_text(tstring, 0, 50, RGB565_BLUE, -1, OPENSANS16, pAsBufferAddr);

    int i      = (frame_id - start_frames) / CALIBRATION_PWM_FRAMES_IN_ONE_FACEWIDTH;
    int length = CALIBRATION_FACEWIDTH_MAX;
    draw_rectborder(CAMERA_SURFACE_SHIFT + (LCD_WIDTH - length) / 2 + i * CALIBRATION_FACEWIDTH_INTERVAL / 2,
                    (LCD_HEIGHT - length) / 2 + i * CALIBRATION_FACEWIDTH_INTERVAL / 2,
                    CAMERA_SURFACE_SHIFT + (LCD_WIDTH + length) / 2 - i * CALIBRATION_FACEWIDTH_INTERVAL / 2,
                    (LCD_HEIGHT + length) / 2 - i * CALIBRATION_FACEWIDTH_INTERVAL / 2, 4, RGB565_BLUE, pAsBufferAddr);

    if ((frame_id > 0) && (frame_id < start_frames))
    {
        // preparing page
        sprintf(tstring, "preparing:%d", (start_frames - frame_id) / 2 + 1);
        draw_text(tstring, 0, 150, RGB565_BLUE, -1, OPENSANS16, pAsBufferAddr);
        frame_id++;
        if (frame_id == start_frames)
            Camera_SetDispMode(1);
        return;
    }

    sprintf(tstring, "face width:%d", length - i * CALIBRATION_FACEWIDTH_INTERVAL);
    draw_text(tstring, 0, 90, RGB565_BLUE, -1, OPENSANS16, pAsBufferAddr);

    sprintf(tstring, "PWM:%d", pwm);
    draw_text(tstring, 0, 110, RGB565_BLUE, -1, OPENSANS16, pAsBufferAddr);

    sprintf(tstring, "valid frames:%d", valid_frames);
    draw_text(tstring, 0, 130, RGB565_BLUE, -1, OPENSANS16, pAsBufferAddr);

    sprintf(tstring, "liveness frames:%d", liveness_frames);
    draw_text(tstring, 0, 150, RGB565_BLUE, -1, OPENSANS16, pAsBufferAddr);

    if ((x1 >= CAMERA_SURFACE_SHIFT) && (y1 >= 0) && (x2 >= CAMERA_SURFACE_SHIFT) && (y2 > 0) && (x1 != x2) &&
        (y1 != y2) && (x2 < APP_AS_WIDTH) && (y2 < APP_AS_HEIGHT))
    {
        UsbShell_Printf("x1=%d, y1=%d, x2=%d, y2=%d, liveness=%d\r\n", x1, y1, x2, y2, infoMsg.liveness);
        SysState_Set(sys_state);
        draw_rectborder(x1, y1, x2, y2, 2, RGB565_RED, pAsBufferAddr);
        sprintf(tstring, "liveness:%d", infoMsg.liveness);
        draw_text(tstring, x1, y1, -1, RGB565_RED, OPENSANS8, pAsBufferAddr);

        if ((abs(CAMERA_SURFACE_SHIFT + (LCD_WIDTH - length) / 2 + i * CALIBRATION_FACEWIDTH_INTERVAL / 2 - x1) <
             (CALIBRATION_FACEWIDTH_INTERVAL / 2)) &&
            (abs(CAMERA_SURFACE_SHIFT + (LCD_WIDTH + length) / 2 - i * CALIBRATION_FACEWIDTH_INTERVAL / 2 - x2) <
             (CALIBRATION_FACEWIDTH_INTERVAL / 2)))
        {
            if (memcmp(infoMsg.rect, last_rect, sizeof(last_rect)))
            {
                valid_frames++;
                UsbShell_Printf("valid frame %d!!!!\r\n", valid_frames);
                valid_flag = 1;
            }
            if ((!memcmp(infoMsg.rect, last_rect, sizeof(last_rect))) && infoMsg.liveness && valid_flag)
            {
                liveness_frames++;
                valid_flag = 0;
                UsbShell_Printf("liveness frame %d!!!!\r\n", liveness_frames);
            }
        }
    }
    else
    {
        name = "";
    }
    if ((((frame_id - start_frames) + 1) % CALIBRATION_PWM_FRAMES) == 0)
    {
        UsbShell_Printf("*face_width=%d, pwm=%d, liveness_frames=%d, valid_frames=%d\r\n",
                        length - i * CALIBRATION_FACEWIDTH_INTERVAL, pwm, liveness_frames, valid_frames);

        liveness_table[length - i * CALIBRATION_FACEWIDTH_INTERVAL][pwm] = liveness_frames;
        valid_table[length - i * CALIBRATION_FACEWIDTH_INTERVAL][pwm]    = valid_frames;

        valid_frames    = 0;
        liveness_frames = 0;
        valid_flag      = 0;

        pwm += CALIBRATION_PWM_INTERVAL;
        if (pwm > CALIBRATION_PWM_MAX)
            pwm = CALIBRATION_PWM_MIN;
        Camera_QMsgSetPWM(1, pwm);
    }
    memcpy(last_rect, infoMsg.rect, sizeof(last_rect));
    frame_id++;
}
#endif
