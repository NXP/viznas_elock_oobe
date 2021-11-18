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

#include "board.h"
#include "commondef.h"

#include "semphr.h"
#include "event_groups.h"
#include "display.h"
#include "camera.h"
#include "pxp.h"
#include "fsl_log.h"
#include "sln_shell.h"

#if LCD_TYPE == LCD_SPI_RIVERDI
#include "sln_rvdisp.h"
#elif LCD_TYPE == LCD_SPI_WZ032HN89V022
#include "fsl_stdisp.h"
#elif LCD_TYPE == LCD_RGB_PJ2805H02
#include "fsl_pjdisp.h"
#endif

#if SCREEN_PORTRAIT_MODE
#include "nxp_vertical_logo.h"
#else
#include "nxp_facemanager.h"
#endif
#if BOARD_SUPPORT_PARALLEL_LCD
#include "fsl_elcdif.h"
#define APP_POL_FLAGS (kELCDIF_DataEnableActiveHigh | kELCDIF_VsyncActiveLow | kELCDIF_HsyncActiveLow | kELCDIF_DriveDataOnRisingClkEdge)
#define APP_LCDIF_DATA_BUS kELCDIF_DataBus16Bit
#endif

SDK_ALIGN(static uint8_t s_tmpBuffer4Jpeg[LCD_WIDTH*LCD_HEIGHT*3], FRAME_BUFFER_ALIGN);
static uint32_t s_dataSizeInJpeg = 0;
#include "toojpeg.h"


/*******************************************************************************
 * Variables
 *******************************************************************************/

/*!< Semaphores used for synchronization with USB*/
SemaphoreHandle_t g_DisplayFull, g_DisplayEmpty;
/*!< Queue used by Display Task to receive messages*/
static QueueHandle_t s_DisplayMsgQ = NULL;
static uint8_t s_ActiveFrameIndex = 0;
/*!< Init the interface with an invalid value and let camera task send a proper value */
static uint8_t s_DisplayInterfaceMode = DISPLAY_LAST_INTERFACE;

SDK_ALIGN(static uint8_t s_BufferLcd[2][LCD_WIDTH * LCD_HEIGHT*3], FRAME_BUFFER_ALIGN);

#if (configSUPPORT_STATIC_ALLOCATION == 1)
DTC_BSS static StackType_t s_DisplayTaskStack[DISPLAYTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_DisplayTaskTCB;
#endif

extern uint8_t *g_FrameBufferUSB;
extern uint32_t g_FrameBufferUSBLength;
extern EventGroupHandle_t g_SyncVideoEvents;

/*******************************************************************************
 * Code
 *******************************************************************************/

int Display_SendQMsg(void *msg)
{
    BaseType_t ret;
    ret = xQueueSend(s_DisplayMsgQ, msg, (TickType_t)0);

    if (ret != pdPASS)
    {
        LOGE("[ERROR]:Display_SendQMsg failed\r\n");
        return -1;
    }

    return 0;
}

//send msg to camera task by display task to request camera buffer 
static int Display_sendReqMsg(void *data)
{
    QMsg *pDisplayReqMsg = (QMsg*)pvPortMalloc(sizeof(QMsg));
    pDisplayReqMsg->id = QMSG_DISPLAY_FRAME_REQ;
    pDisplayReqMsg->msg.raw.IR_frame_data = data;
    return Camera_SendQMsg((void *)&pDisplayReqMsg);
}

static void Display_Task(void *param)
{
    BaseType_t ret;
    QMsg *pQMsg;

    xEventGroupWaitBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_DISPLAY_INIT_BIT | 1 << SYNC_VIDEO_CAMERA_INIT_BIT, pdFALSE,
                        pdTRUE, portMAX_DELAY);

    LOGD("[Display]:running\r\n");
    Display_sendReqMsg((void *)s_BufferLcd[s_ActiveFrameIndex]);
    while (1)
    {
        // pick up message
        ret = xQueueReceive(s_DisplayMsgQ, (void *)&pQMsg, portMAX_DELAY);

        if (ret == pdTRUE)
        {
            switch (pQMsg->id)
            {
                case QMSG_DISPLAY_FRAME_RES:
                {
                    Display_Update((uint32_t)s_BufferLcd[s_ActiveFrameIndex]);
                    s_ActiveFrameIndex ^= 1U;
                    Display_sendReqMsg((void *)s_BufferLcd[s_ActiveFrameIndex]);
                }
                break;

                case QMSG_DISPLAY_INTERFACE:
                {
                    s_DisplayInterfaceMode = pQMsg->msg.cmd.data.interface_mode;
                }
                break;

                default:
                    break;
            }
        }
        vPortFree(pQMsg);
    }
}

int Display_Start()
{
    LOGD("[Display]:start\r\n");

    s_DisplayInterfaceMode = Cfg_AppDataGetInterfaceMode();

    s_DisplayMsgQ  = xQueueCreate(DISPLAY_MSG_Q_COUNT, sizeof(QMsg *));
    g_DisplayFull  = xSemaphoreCreateCounting(1, 0);
    g_DisplayEmpty = xSemaphoreCreateCounting(1, 1);
    if (g_DisplayFull == NULL || g_DisplayEmpty == NULL)
    {
        LOGE("[ERROR]:xSemaphoreCreateCounting failed to create \r\n");
        return -1;
    }

    if (s_DisplayMsgQ == NULL)
    {
        LOGE("[ERROR]:xQueueCreate display queue\r\n");
        return -1;
    }

    if (xTaskCreate(Display_Init_Task, "Display_Init", DISPLAYINITTASK_STACKSIZE, NULL, DISPLAYINITTASK_PRIORITY,
                    NULL) != pdPASS)
    {
        LOGE("[ERROR]Display Init created failed\r\n");

        while (1)
            ;
    }

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(Display_Task, "Display Task", DISPLAYTASK_STACKSIZE, NULL, DISPLAYTASK_PRIORITY,
                                  s_DisplayTaskStack, &s_DisplayTaskTCB))
#else
    if (xTaskCreate(Display_Task, "Display Task", DISPLAYTASK_STACKSIZE, NULL, DISPLAYTASK_PRIORITY, NULL) != pdPASS)
#endif
    {
        LOGE("[ERROR]DisplayTask created failed\r\n");

        while (1)
            ;
    }

    LOGD("[Display]:started\r\n");
    return 0;
}

int Display_Sync()
{
    return 0;
}

#if BOARD_SUPPORT_PARALLEL_LCD
static void ELCDIF_Init(void)
{
    const elcdif_rgb_mode_config_t config = {
        .panelWidth    = 240,
        .panelHeight   = 320,
        .hsw           = 10,
        .hfp           = 10,
        .hbp           = 20,
        .vsw           = 2,
        .vfp           = 4,
        .vbp           = 2,
        .polarityFlags = APP_POL_FLAGS,
        .bufferAddr    = (uint32_t)s_BufferLcd,
        .pixelFormat   = kELCDIF_PixelFormatRGB565,
        .dataBus       = APP_LCDIF_DATA_BUS,
    };
    ELCDIF_RgbModeInit(LCDIF, &config);
}

static void LcdifPixelClock_Init(void)
{
    /*
     * (240 + 10 + 20 + 10) * (320 + 2 + 2 +4) *60 = 5.51M
     * The desired output frame rate is 60Hz. So the pixel clock frequency is:
     * (480 + 41 + 4 + 18) * (272 + 10 + 4 + 2) * 60 = 9.2M.
     * Here set the LCDIF pixel clock to 9.3M.
     */

    /*
     * Initialize the Video PLL.
     * Video PLL output clock is OSC24M * (loopDivider + (denominator / numerator)) / postDivider = 93MHz.
     */
    clock_video_pll_config_t config = {
        .loopDivider = 29,
        .postDivider = 16,
        .numerator   = 3,
        .denominator = 8,
        .src = 0
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

    CLOCK_SetDiv(kCLOCK_LcdifPreDiv, 1);

    CLOCK_SetDiv(kCLOCK_LcdifDiv, 3);
}

void BOARD_LCD_Init(void)
{
	CLOCK_EnableClock(kCLOCK_Gpio1);
	CLOCK_EnableClock(kCLOCK_Gpio2);
	CLOCK_EnableClock(kCLOCK_Gpio3);
	LcdifPixelClock_Init();
	ELCDIF_Init();
}
#endif

void Display_Init_Task(void *param)
{
    if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
    {
#if LCD_TYPE == LCD_SPI_RIVERDI
        RVDisp_PowerCycleDisplay(false);
#endif
    }
    else if (Cfg_AppDataGetOutputMode() == DISPLAY_LCD)
    {
#if BOARD_SUPPORT_PARALLEL_LCD
        BOARD_LCD_Init();
        ELCDIF_RgbModeStart(LCDIF);
#endif
#if LCD_TYPE == LCD_SPI_RIVERDI
        RVDisp_Init(NULL);
#elif LCD_TYPE == LCD_SPI_WZ032HN89V022
        STDisp_Init(NULL);
#elif LCD_TYPE == LCD_RGB_PJ2805H02
        PJDisp_Init();
		PJDisp_TurnOnBacklight();
#endif
    }
#if SCREEN_PORTRAIT_MODE
    //Display_Update((uint32_t)nxp_vertical_logo);
#else
    //Display_Update((uint32_t)nxp_facemanager);
#endif
    vTaskDelay(1200);

    xEventGroupSetBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_DISPLAY_INIT_BIT);

    vTaskDelete(NULL);
}


static void Oasis_WriteJpegBuffer(uint8_t byte)
{
	s_tmpBuffer4Jpeg[s_dataSizeInJpeg++] = byte;
}


int Display_Update(uint32_t backBuffer)
{
    if (s_DisplayInterfaceMode == DISPLAY_LAST_INTERFACE)
        return -1;

    if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
    {

        //compress the buffer into JPEG, support RGB565 format
    	xSemaphoreTake(g_DisplayEmpty, portMAX_DELAY);
		s_dataSizeInJpeg = 0;

		uint32_t jpeg_us_start = Time_Now();
		//Be careful, in this function, at least 20KB+ stack should be left!!!!!!
		TooJpeg::writeJpeg(Oasis_WriteJpegBuffer,
				(void*)backBuffer,
				LCD_WIDTH,
				LCD_HEIGHT,
				1,90,1);

        g_FrameBufferUSB = s_tmpBuffer4Jpeg;
        g_FrameBufferUSBLength = s_dataSizeInJpeg;
        //UsbShell_Printf("TooJpeg size:%d, cost:%d\r\n",s_dataSizeInJpeg,jpeg_us_start-Time_Now());
		xSemaphoreGive(g_DisplayFull);

    }
    else if (Cfg_AppDataGetOutputMode() == DISPLAY_LCD)
    {
#if LCD_TYPE == LCD_SPI_RIVERDI
        RVDisp_SendFrame((uint16_t *)backBuffer);
#elif LCD_TYPE == LCD_SPI_WZ032HN89V022
        STDisp_SendFrame((uint16_t *)backBuffer);
#elif LCD_TYPE == LCD_RGB_PJ2805H02
        PJDisp_SendFrame(backBuffer);
#endif
    }
    else
    {
        return -2;
    }
    return 0;
}

void Display_Deinit(void)
{
    if (Cfg_AppDataGetOutputMode() == DISPLAY_LCD)
    {
#if LCD_TYPE == LCD_SPI_RIVERDI
        RVDisp_UnInit(NULL);
        RVDisp_PowerCycleDisplay(false);
#endif
    }
    xEventGroupClearBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_DISPLAY_INIT_BIT);
}
