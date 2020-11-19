/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the license terms that accompany it. By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 *
 * Created by: NXP China Solution Team.
 */

#include "FreeRTOS.h"
#include "event_groups.h"
#include "fsl_pxp.h"
#include "commondef.h"
#include "display.h"
#include "oasis.h"
#include "pxp.h"
#include "util.h"
#include "userid_ui.h"
#include "fsl_log.h"
#include "camera.h"

/*******************************************************************************
 * Definitions
 *******************************************************************************/

#define CLIP(X) ((X) > 255 ? 255 : (X) < 0 ? 0 : X)

// RGB -> YUV
#define RGB2Y(R, G, B) CLIP(((66 * (R) + 129 * (G) + 25 * (B) + 128) >> 8) + 16)
#define RGB2U(R, G, B) CLIP(((-38 * (R)-74 * (G) + 112 * (B) + 128) >> 8) + 128)
#define RGB2V(R, G, B) CLIP(((112 * (R)-94 * (G)-18 * (B) + 128) >> 8) + 128)

/*******************************************************************************
 * Variables
 *******************************************************************************/
extern EventGroupHandle_t g_SyncVideoEvents;

static QueueHandle_t s_PXPMsgQ = NULL;
/*!< Message sent to Oasis Task by Camera task to signal that a frame is available */
static QMsg s_FResMsg;
/*!< Message sent to Display Task by Camera task to signal that a frame is available */
static QMsg s_DResMsg;

static uint8_t s_DisplayInterfaceMode = DISPLAY_LAST_INTERFACE;

static pxp_output_buffer_config_t s_OutputBufferConfig;
static pxp_ps_buffer_config_t s_PsBufferConfig;
static pxp_as_buffer_config_t s_AsBufferConfig;
static pxp_as_blend_config_t s_AsBlendConfig;

static uint16_t s_asBufferPxp[APP_AS_HEIGHT * APP_AS_WIDTH];
extern uint16_t *g_pRotateBuff;

/*******************************************************************************
 * Code
 *******************************************************************************/

extern "C" {
void PXP_IRQHandler(void)
{
    if (g_SyncVideoEvents != NULL)
    {
        BaseType_t result;
        BaseType_t HigherPriorityTaskWoken = pdFALSE;
        PXP_ClearStatusFlags(APP_PXP, kPXP_CompleteFlag);
        result =
            xEventGroupSetBitsFromISR(g_SyncVideoEvents, 1 << SYNC_VIDEO_PXP_COMPLET_BIT, &HigherPriorityTaskWoken);
        if (result != pdFAIL)
        {
            portYIELD_FROM_ISR(HigherPriorityTaskWoken);
        }
    }
}
}

void RGB5652YUV(uint16_t source1, uint16_t source2, uint16_t *dest1, uint16_t *dest2)
{
    uint8_t R, G, B, Y0, Y1, U0, V0, U1, V1;
    R      = ((source1 & RGB565_RED) >> 11) << 3;
    G      = ((source1 & RGB565_GREEN) >> 5) << 2;
    B      = ((source1 & RGB565_BLUE)) << 3;
    Y0     = RGB2Y(R, G, B);
    U0     = RGB2U(R, G, B);
    V0     = RGB2V(R, G, B);
    R      = ((source2 & RGB565_RED) >> 11) << 3;
    G      = ((source2 & RGB565_GREEN) >> 5) << 2;
    B      = ((source2 & RGB565_BLUE)) << 3;
    Y1     = RGB2Y(R, G, B);
    U1     = RGB2U(R, G, B);
    V1     = RGB2V(R, G, B);
    U0     = (U0 + U1) / 2;
    V0     = (V0 + V1) / 2;
    *dest2 = (Y1 << 8) | V0;
    *dest1 = (Y0 << 8) | U0;
}

void ConvertRGB2YUV(uint16_t *sourceLcdBuffer, uint16_t *destLcdBuffer)
{
    int line, col;

    for (line = 0; line < LCD_HEIGHT; line++)
        for (col = 0; col < LCD_WIDTH - 1; col = col + 2)
        {
            RGB5652YUV(*(sourceLcdBuffer + line * LCD_WIDTH + col), *(sourceLcdBuffer + line * LCD_WIDTH + col + 1),
                       (destLcdBuffer + line * LCD_WIDTH + col), (destLcdBuffer + line * LCD_WIDTH + col + 1));
        }
}

void APP_PXPInit()
{
    PXP_Init(APP_PXP);
    PXP_EnableInterrupts(APP_PXP, kPXP_CompleteInterruptEnable);
    EnableIRQ(PXP_IRQn);
    NVIC_SetPriority(PXP_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);

    /* PS configure. */
    s_PsBufferConfig.pixelFormat = kPXP_PsPixelFormatUYVY1P422;
    s_PsBufferConfig.swapByte    = false;
    // s_PsBufferConfig.bufferAddr = (uint32_t)s_psBufferPxp;
    s_PsBufferConfig.bufferAddrU = 0U;
    s_PsBufferConfig.bufferAddrV = 0U;
    s_PsBufferConfig.pitchBytes  = APP_CAMERA_WIDTH * 2; // camera data as PS input data

    PXP_SetProcessSurfaceBackGroundColor(APP_PXP, 0x1U);
    PXP_SetProcessSurfaceBufferConfig(APP_PXP, &s_PsBufferConfig);

    /* AS config. */
    s_AsBufferConfig.pixelFormat = kPXP_AsPixelFormatRGB565;
    s_AsBufferConfig.bufferAddr  = (uint32_t)s_asBufferPxp;
    s_AsBufferConfig.pitchBytes  = APP_AS_WIDTH * 2;
    s_AsBlendConfig.alpha        = 0xFU;          /* Don't care. */
    s_AsBlendConfig.invertAlpha  = false;         /* Don't care. */
    s_AsBlendConfig.alphaMode    = kPXP_AlphaRop; // kPXP_AlphaRop;
    s_AsBlendConfig.ropMode      = kPXP_RopMergeAs;
    PXP_SetAlphaSurfaceBufferConfig(APP_PXP, &s_AsBufferConfig);
    PXP_SetAlphaSurfaceBlendConfig(APP_PXP, &s_AsBlendConfig);

    s_AsBlendConfig.alpha       = 0xb0U; /* Don't care. */
    s_AsBlendConfig.invertAlpha = false; /* Don't care. */
    s_AsBlendConfig.alphaMode   = kPXP_AlphaOverride;
    s_AsBlendConfig.ropMode     = kPXP_RopMaskAs;

    PXP_SetAlphaSurfaceBufferConfig(APP_PXP, &s_AsBufferConfig);
    PXP_SetAlphaSurfaceBlendConfig(APP_PXP, &s_AsBlendConfig);

    /* Output config. */
    s_OutputBufferConfig.pixelFormat    = kPXP_OutputPixelFormatUYVY1P422;
    s_OutputBufferConfig.interlacedMode = kPXP_OutputProgressive;
    s_OutputBufferConfig.buffer0Addr    = 0U;
    s_OutputBufferConfig.buffer1Addr    = 0U;
    s_OutputBufferConfig.pitchBytes     = LCD_WIDTH * 2;
    s_OutputBufferConfig.width          = LCD_WIDTH;
    s_OutputBufferConfig.height         = LCD_HEIGHT;

    PXP_SetOutputBufferConfig(APP_PXP, &s_OutputBufferConfig);

    /* Disable CSC1, it is enabled by default. */
    PXP_SetCsc1Mode(APP_PXP, kPXP_Csc1YCbCr2RGB);
    PXP_EnableCsc1(APP_PXP, true);
}

static float getFPS()
{
    static float fps = 0.0f;
#ifdef SHOW_FPS
    static int frameCount = 0;
    static int firstFrame = 1;
    static int tStart, tEnd;
    if (firstFrame)
    {
        firstFrame = 0;
        frameCount = 0;
        tStart = tEnd = Time_Current();
    }
    else
    {
        frameCount++;
        tEnd = Time_Current();
        if (tEnd - tStart >= 1000)
        {
            fps        = (frameCount * 1000.0f) / (float)(tEnd - tStart);
            frameCount = 0;
            tStart     = tEnd;
        }
        // LOGD("[fps]:%.3f\r\n", fps);
    }
#endif
    return fps;
}

static void APP_PXPAlpha(uint32_t enable, int l, int t, int r, int b, uint8_t alpha, uint32_t enableColorKey)
{
    if (enable)
    {
        PXP_SetAlphaSurfacePosition(APP_PXP, l, t, r, b);
        s_AsBlendConfig.alpha = alpha;
        PXP_SetAlphaSurfaceBlendConfig(APP_PXP, &s_AsBlendConfig);
        if (enableColorKey)
        {
            PXP_SetAlphaSurfaceOverlayColorKey(APP_PXP, 0x0, 0x0);
            PXP_EnableAlphaSurfaceOverlayColorKey(APP_PXP, true);
        }
        else
        {
            PXP_EnableAlphaSurfaceOverlayColorKey(APP_PXP, false);
        }
    }
    else
    {
        PXP_SetAlphaSurfacePosition(APP_PXP, 0xFFFFU, 0xFFFFU, 0U, 0U);
    }
    PXP_SetAlphaSurfaceBufferConfig(APP_PXP, &s_AsBufferConfig);
}

static void APP_PXPOut(int w,
                       int h,
                       int l,
                       int t,
                       int r,
                       int b,
                       int iw,
                       int ih,
                       int ow,
                       int oh,
                       pxp_output_pixel_format_t format,
                       uint16_t pitchBytes)
{
    s_OutputBufferConfig.pitchBytes  = pitchBytes;
    s_OutputBufferConfig.pixelFormat = format;
    s_OutputBufferConfig.width       = w;
    s_OutputBufferConfig.height      = h;
    PXP_SetOutputBufferConfig(APP_PXP, &s_OutputBufferConfig);
    PXP_SetProcessSurfacePosition(APP_PXP, l, t, r, b);
    PXP_SetProcessSurfaceScaler(APP_PXP, iw, ih, ow, oh);
}

static void APP_PXPStart(uint32_t psBuffer, uint32_t outBuffer, uint32_t rotate)
{
    /* Convert the camera input picture to RGB format. */
    if (psBuffer)
    {
        s_PsBufferConfig.bufferAddr = psBuffer;
        PXP_SetProcessSurfaceBufferConfig(APP_PXP, &s_PsBufferConfig);
    }

    if (outBuffer)
    {
        s_OutputBufferConfig.buffer0Addr = outBuffer;
        if (rotate == 90)
        {
            PXP_SetRotateConfig(APP_PXP, kPXP_RotateOutputBuffer, kPXP_Rotate90, kPXP_FlipBoth);
        }
        else
        {
            PXP_SetRotateConfig(APP_PXP, kPXP_RotateOutputBuffer, kPXP_Rotate180, kPXP_FlipBoth);
        }
        PXP_SetOutputBufferConfig(APP_PXP, &s_OutputBufferConfig);
    }

    PXP_Start(APP_PXP);

    /* Wait for PXP process complete. */
    xEventGroupWaitBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_PXP_COMPLET_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
}

void APP_PXPStartCamera2Display(uint32_t cameraBuffer,
                                QUIInfoMsg infoMsg,
                                uint8_t displayInterfaceMode,
                                uint32_t outBuffer)
{
    pxp_output_pixel_format_t output_format = kPXP_OutputPixelFormatRGB565;
    int scale_factor                        = APP_CAMERA_WIDTH / APP_PS_WIDTH;

    infoMsg.fps = getFPS();
    if (displayInterfaceMode == DISPLAY_INTERFACE_LOOPBACK)
    {
        APP_PXPAlpha(0, 0, 0, 0, 0, 0, 0);
        UIInfo_Update((uint16_t*)cameraBuffer, infoMsg, displayInterfaceMode);
        if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
        {
            PXP_EnableCsc1(APP_PXP, false);
            output_format = kPXP_OutputPixelFormatUYVY1P422;
        }
        else
        {
            PXP_EnableCsc1(APP_PXP, true);
            output_format = kPXP_OutputPixelFormatRGB565;
        }
    }
    else
    {
        APP_PXPAlpha(1, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, 0xF0U, 1);
        UIInfo_Update(s_asBufferPxp, infoMsg, displayInterfaceMode);
        PXP_EnableCsc1(APP_PXP, true);
        output_format = kPXP_OutputPixelFormatRGB565;
    }

    APP_PXPOut(LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH - APP_CAMERA_WIDTH / scale_factor,
               LCD_HEIGHT - APP_CAMERA_HEIGHT / scale_factor, LCD_WIDTH - 1, LCD_HEIGHT - 1, APP_CAMERA_WIDTH,
               APP_CAMERA_HEIGHT, APP_CAMERA_WIDTH / scale_factor, APP_CAMERA_HEIGHT / scale_factor, output_format,
               DISPLAY_PITCH_BYTES);
    APP_PXPStart((uint32_t)cameraBuffer, outBuffer, DISPLAY_ROTATION);
}

void APP_PXPStartCamera2DetBuf(uint32_t cameraBuffer, uint32_t detBuffer)
{
    PXP_EnableCsc1(APP_PXP, true);
    APP_PXPAlpha(0, 0, 0, 0, 0, 0, 0);
    APP_PXPOut(APP_CAMERA_WIDTH, APP_CAMERA_HEIGHT, 0, 0, APP_CAMERA_WIDTH - 1, APP_CAMERA_HEIGHT - 1, APP_CAMERA_WIDTH,
               APP_CAMERA_HEIGHT, APP_CAMERA_WIDTH, APP_CAMERA_HEIGHT, kPXP_OutputPixelFormatRGB888P,
               APP_CAMERA_WIDTH * 3);
    APP_PXPStart(cameraBuffer, detBuffer, 0);
}

static void PXP_Task(void *param)
{
    BaseType_t ret;
    QMsg *pQMsg;

    while (1)
    {
        // pick up message
        ret = xQueueReceive(s_PXPMsgQ, (void *)&pQMsg, portMAX_DELAY);

        if (ret)
        {
            switch (pQMsg->id)
            {
                case QMSG_PXP_FACEREC:
                {
#if CAMERA_ROTATE_FLAG
                    APP_SetPxpRotate(pQMsg->msg.pxp.in_buffer, (uint32_t)g_pRotateBuff);
                    APP_PXPStartCamera2DetBuf((uint32_t)g_pRotateBuff, pQMsg->msg.pxp.out_buffer);
#else
                    APP_PXPStartCamera2DetBuf(pQMsg->msg.pxp.in_buffer, pQMsg->msg.pxp.out_buffer);
#endif
                    pQMsg = &s_FResMsg;
                    Camera_SendQMsg((void *)&pQMsg);
                }
                break;

                case QMSG_PXP_DISPLAY:
                {
#if CAMERA_ROTATE_FLAG
                    APP_SetPxpRotate(pQMsg->msg.pxp.in_buffer, (uint32_t)g_pRotateBuff);
                    APP_PXPStartCamera2Display((uint32_t)g_pRotateBuff, *pQMsg->msg.pxp.info, 1,
                                               pQMsg->msg.pxp.out_buffer);
#else
                    APP_PXPStartCamera2Display(pQMsg->msg.pxp.in_buffer, *pQMsg->msg.pxp.info, s_DisplayInterfaceMode,
                                               pQMsg->msg.pxp.out_buffer);
#endif
                    pQMsg = &s_DResMsg;
                    Camera_SendQMsg((void *)&pQMsg);
                }
                break;

                case QMSG_DISPLAY_INTERFACE:
                {
                    s_DisplayInterfaceMode = pQMsg->msg.cmd.data.interface_mode;
                    vPortFree(pQMsg);
                }
                break;

                default:
                    break;
            }
        }
    }
}

void APP_PXP_Start(void)
{
    LOGD("[PXP]:start\r\n");
    APP_PXPInit();
    s_FResMsg.id = QMSG_PXP_FACEREC;
    s_DResMsg.id = QMSG_PXP_DISPLAY;

    s_PXPMsgQ = xQueueCreate(PXP_MSG_Q_COUNT, sizeof(QMsg *));

    s_DisplayInterfaceMode = Cfg_AppDataGetInterfaceMode();

    if (s_PXPMsgQ == NULL)
    {
        LOGE("[ERROR]:xQueueCreate pxp queue\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(PXP_Task, "PXP Task", PXPTASK_STACKSIZE, NULL, PXPTASK_PRIORITY, NULL) != pdPASS)
    {
        LOGE("[ERROR]PXP_Task created failed\r\n");

        while (1)
            ;
    }
}

int PXP_SendQMsg(void *msg)
{
    BaseType_t ret;

    ret = xQueueSend(s_PXPMsgQ, msg, (TickType_t)0);

    if (ret != pdPASS)
    {
        LOGE("[ERROR]:PXP_SendQMsg failed %d\r\n", ret);
        return -1;
    }

    return 0;
}

void APP_PXP_Deinit(void)
{
    PXP_Deinit(APP_PXP);
    PXP_DisableInterrupts(APP_PXP, kPXP_CompleteInterruptEnable);
}
