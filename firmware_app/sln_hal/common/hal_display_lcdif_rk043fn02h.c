/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP IoT Solutions Team.
 */

/*
 * @brief display dev hal for lcdif and rk043fn02h.
 * @Author chaosheng.xiang@nxp.com
 */

#include <FreeRTOS.h>
#include <queue.h>

#include "fsl_common_arm.h"
#include "fsl_elcdif.h"
#include "fsl_dc_fb_elcdif.h"

#include "fwk_log.h"
#include "fwk_display_manager.h"
#include "hal_display_dev.h"
#include "hal_event_descriptor_common.h"
#include "./icons/nxp_logo_240x86.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DISPLAY_NAME               "rk043fn02h"
#define DISPLAY_WIDTH              480
#define DISPLAY_HEIGHT             272
#define DISPLAY_BYTE_PER_PIXEL     2
#define DISPLAY_FRAME_BUFFER_COUNT 2

#define DISPLAY_BUFFER_WIDTH      (DISPLAY_WIDTH)
#define DISPLAY_BUFFER_HEIGHT     (DISPLAY_HEIGHT)

/* Where the frame buffer is shown in the screen. */
#define DISPLAY_BUFFER_START_X 0U
#define DISPLAY_BUFFER_START_Y 0U

#define DISPLAY_BUFFER_PIXEL_FORMAT   kVIDEO_PixelFormatRGB565
#define DISPLAY_BUFFER_BYTE_PER_PIXEL 2

#define DISPLAY_HSW 41
#define DISPLAY_HFP 4
#define DISPLAY_HBP 8
#define DISPLAY_VSW 10
#define DISPLAY_VFP 4
#define DISPLAY_VBP 2

#define DISPLAY_LCDIF_BASE       LCDIF
#define DISPLAY_LCDIF_IRQn       LCDIF_IRQn
//#define DISPLAY_LCDIF_IRQHandler LCDIF_IRQHandler
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
void board_pull_elcdif_rk043fn02h_reset_pin(bool pullUp);
void board_pull_elcdif_rk043fn02h_backlight_pin(bool pullUp);
void BOARD_InitElcdifRk043fn02hResource(void);
#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t s_FrameBuffers[DISPLAY_FRAME_BUFFER_COUNT][DISPLAY_HEIGHT][DISPLAY_WIDTH * DISPLAY_BYTE_PER_PIXEL],
    32);

static dc_fb_elcdif_handle_t s_dcFbElcdifHandle = {0}; /* The handle must be initialized to 0. */
static const dc_fb_elcdif_config_t s_dcFbElcdifConfig = {
    .elcdif        = LCDIF,
    .width         = DISPLAY_WIDTH,
    .height        = DISPLAY_HEIGHT,
    .hsw           = DISPLAY_HSW,
    .hfp           = DISPLAY_HFP,
    .hbp           = DISPLAY_HBP,
    .vsw           = DISPLAY_VSW,
    .vfp           = DISPLAY_VFP,
    .vbp           = DISPLAY_VBP,
    .polarityFlags = (kELCDIF_DataEnableActiveHigh | kELCDIF_VsyncActiveLow | kELCDIF_HsyncActiveLow |
            kELCDIF_DriveDataOnRisingClkEdge),
    .dataBus       = kELCDIF_DataBus16Bit,
};

static const dc_fb_t s_dcFb = {
    .ops     = &g_dcFbOpsElcdif,
    .prvData = &s_dcFbElcdifHandle,
    .config  = &s_dcFbElcdifConfig,
};

static dc_fb_info_t s_fbInfo;

static display_dev_t s_DisplayDev_Lcdif;

static void BOARD_InitLcdifPixelClock(void)
{
    /*
     * The desired output frame rate is 60Hz. So the pixel clock frequency is:
     * (480 + 41 + 4 + 18) * (272 + 10 + 4 + 2) * 60 = 9.2M.
     * Here set the LCDIF pixel clock to 9.3M.
     */

    /*
     * Initialize the Video PLL.
     * Video PLL output clock is OSC24M * (loopDivider + (denominator / numerator)) / postDivider = 93MHz.
     */
    clock_video_pll_config_t config = {
        .loopDivider = 31,
        .postDivider = 8,
        .numerator   = 0,
        .denominator = 0,
        .src         = 0,   /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    };

    CLOCK_InitVideoPll(&config);

    /*
     * 000 derive clock from PLL2
     * 001 derive clock from PLL3 PFD3
     * 010 derive clock from PLL5
     * 011 derive clock from PLL2 PFD0
     * 100 derive clock from PLL2 PFD1
     * 101 derive clock from PLL3 PFD1
     */
    CLOCK_SetMux(kCLOCK_LcdifPreMux, 2);

    CLOCK_SetDiv(kCLOCK_LcdifPreDiv, 4);

    CLOCK_SetDiv(kCLOCK_LcdifDiv, 1);
}

static void BOARD_InitLcd(void)
{
    board_pull_elcdif_rk043fn02h_backlight_pin(1);

    /*
     * Reset the LCDIF, this is only used for flash target project debug.
     *
     * Hardware reset through debugger could not reset the ELCDIF, when reset
     * through debugger, the previous ELCDIF status retains, especially the
     * interrupt pending status. So we need to reset the ELCDIF before enabling
     * interrupt in NVIC. If the application only works with POR (Power on reset),
     * then this could be removed.
     */
    CLOCK_EnableClock(kCLOCK_Lcd);
    CLOCK_EnableClock(kCLOCK_LcdPixel);
    ELCDIF_Reset(DISPLAY_LCDIF_BASE);
    NVIC_ClearPendingIRQ(DISPLAY_LCDIF_IRQn);
    CLOCK_DisableClock(kCLOCK_LcdPixel);
    CLOCK_DisableClock(kCLOCK_Lcd);

    NVIC_SetPriority(DISPLAY_LCDIF_IRQn, 3);
    EnableIRQ(DISPLAY_LCDIF_IRQn);
}

static status_t BOARD_PrepareDisplayController(void)
{
    BOARD_InitLcdifPixelClock();
    BOARD_InitLcd();

    return kStatus_Success;
}

static void BOARD_DcFbCallback(void *param, void *switchOffBuffer)
{
    if (s_DisplayDev_Lcdif.cap.callback != NULL)
    {
        uint8_t fromISR = __get_IPSR();

        if (switchOffBuffer == (uint8_t *)&s_FrameBuffers[0])
        {
            s_DisplayDev_Lcdif.cap.callback(&s_DisplayDev_Lcdif, kDisplayEvent_RequestFrame, &s_FrameBuffers[1], fromISR);
        }
        else if (switchOffBuffer == (uint8_t *)&s_FrameBuffers[1])
        {
            s_DisplayDev_Lcdif.cap.callback(&s_DisplayDev_Lcdif, kDisplayEvent_RequestFrame, &s_FrameBuffers[0], fromISR);
        }
    }
}

static void BOARD_InitElcdifRk043fn02h(void)
{
    status_t status;

    BOARD_PrepareDisplayController();

    status = s_dcFb.ops->init(&s_dcFb);
    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed\r\n");
        assert(0);
    }

    s_dcFb.ops->getLayerDefaultConfig(&s_dcFb, 0, &s_fbInfo);
    s_fbInfo.pixelFormat = DISPLAY_BUFFER_PIXEL_FORMAT;
    s_fbInfo.width       = DISPLAY_BUFFER_WIDTH;
    s_fbInfo.height      = DISPLAY_BUFFER_HEIGHT;
    s_fbInfo.startX      = DISPLAY_BUFFER_START_X;
    s_fbInfo.startY      = DISPLAY_BUFFER_START_Y;
    s_fbInfo.strideBytes = DISPLAY_BUFFER_WIDTH * DISPLAY_BUFFER_BYTE_PER_PIXEL;
    s_dcFb.ops->setLayerConfig(&s_dcFb, 0, &s_fbInfo);

    s_dcFb.ops->setCallback(&s_dcFb, 0, BOARD_DcFbCallback, NULL);
}

static hal_display_status_t HAL_DisplayDev_LcdifRk043fn02h_Init(
    display_dev_t *dev, int width, int height, display_dev_callback_t callback, void *param)
{
    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;
    LOGD("++display_dev_init");

    dev->cap.width       = width;
    dev->cap.height      = height;
    dev->cap.frameBuffer = (void *)&s_FrameBuffers[1];
    dev->cap.callback    = callback;

    BOARD_InitElcdifRk043fn02hResource();

    BOARD_InitElcdifRk043fn02h();

    s_dcFb.ops->setFrameBuffer(&s_dcFb, 0, &s_FrameBuffers[0]);

    LOGD("--display_dev_init");
    return ret;
}

static hal_display_status_t HAL_DisplayDev_LcdifRk043fn02h_Uninit(const display_dev_t *dev)
{
    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;
    return ret;
}

static hal_display_status_t HAL_DisplayDev_LcdifRk043fn02h_Start(const display_dev_t *dev)
{
    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;
    LOGD("++display_dev_start");

    s_dcFb.ops->enableLayer(&s_dcFb, 0);

    LOGD("--display_dev_start");
    return ret;
}

static hal_display_status_t HAL_DisplayDev_LcdifRk043fn02h_Blit(const display_dev_t *dev, void *frame, int width, int height)
{
    hal_display_status_t ret = kStatus_HAL_DisplayNonBlocking;
    LOGI("++display_dev_blit");

    if (frame != NULL)
    {
        s_dcFb.ops->setFrameBuffer(&s_dcFb, 0, frame);
    }

    LOGI("--display_dev_blit");
    return ret;
}

static hal_display_status_t HAL_DisplayDev_LcdifRk043fn02h_InputNotify(const display_dev_t *receiver, void *data)
{
    hal_display_status_t error           = kStatus_HAL_DisplaySuccess;
    event_base_t event_base              = *(event_base_t *)data;
    event_status_t event_response_status = kEventStatus_Ok;

    if (event_base.eventId == kEventID_SetDisplayOutputSource)
    {
        event_common_t event                = *(event_common_t *)data;
        s_DisplayDev_Lcdif.cap.srcFormat     = event.displayOutput.displayOutputSource;

        if (event_base.respond != NULL)
        {
            event_base.respond(event_base.eventId, &event.displayOutput, event_response_status, true);
        }
        LOGI("[display_dev_inputNotify]: SET_DISPLAY_OUTPUT_SOURCE devID %d, src_format %d",
                receiver->id, event.displayOutput.displayOutputSource);
    }
    else if (event_base.eventId == kEventID_GetDisplayOutputSource)
    {
        display_output_event_t display;
        display.displayOutputSource = s_DisplayDev_Lcdif.cap.srcFormat;
        if (event_base.respond != NULL)
        {
            event_base.respond(event_base.eventId, &display, event_response_status, true);
        }
        LOGI("[display_dev_inputNotify]: GET_DISPLAY_OUTPUT_SOURCE devID %d, src_format %d",
                receiver->id, display.displayOutputSource);
    }

    return error;
}

void DISPLAY_LCDIF_IRQHandler(void)
{
    DC_FB_ELCDIF_IRQHandler(&s_dcFb);
}

const static display_dev_operator_t s_DisplayDev_LcdifOps = {
    .init         = HAL_DisplayDev_LcdifRk043fn02h_Init,
    .deinit       = HAL_DisplayDev_LcdifRk043fn02h_Uninit,
    .start        = HAL_DisplayDev_LcdifRk043fn02h_Start,
    .blit         = HAL_DisplayDev_LcdifRk043fn02h_Blit,
    .inputNotify  = HAL_DisplayDev_LcdifRk043fn02h_InputNotify,
};

static display_dev_t s_DisplayDev_Lcdif = {
    .id   = 0,
    .name = DISPLAY_NAME,
    .ops  = &s_DisplayDev_LcdifOps,
    .cap  = {
            .width       = DISPLAY_WIDTH,
            .height      = DISPLAY_HEIGHT,
            .pitch       = DISPLAY_WIDTH * DISPLAY_BYTE_PER_PIXEL,
            .left        = 0,
            .top         = 0,
            .right       = DISPLAY_WIDTH - 1,
            .bottom      = DISPLAY_HEIGHT - 1,
            .rotate      = kCWRotateDegree_0,
            .format      = kPixelFormat_RGB565,
            .srcFormat   = kPixelFormat_UYVY1P422_RGB,
            .frameBuffer = NULL,
            .callback    = NULL,
            .param       = NULL,
    }
};

int HAL_DisplayDev_LcdifRk043fn02h_Register()
{
    int ret = 0;
    LOGD("HAL_DisplayDev_LcdifRk043fn02h_Register");
    ret = FWK_DisplayManager_DeviceRegister(&s_DisplayDev_Lcdif);
    return ret;
}
