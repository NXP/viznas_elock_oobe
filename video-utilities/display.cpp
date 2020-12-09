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

#include "commondef.h"

#include "semphr.h"
#include "event_groups.h"
#include "display.h"
#include "camera.h"
#include "pxp.h"
#include "fsl_log.h"

#include "sln_pcal.h"
#include "sln_rvdisp.h"
#if SCREEN_PORTRAIT_MODE
#include "nxp_vertical_logo.h"
#else
#include "nxp_facemanager.h"
#endif
/*******************************************************************************
 * Variables
 *******************************************************************************/

/*!< Semaphores used for synchronization with USB*/
SemaphoreHandle_t g_DisplayFull, g_DisplayEmpty;
/*!< Queue used by Display Task to receive messages*/
static QueueHandle_t s_DisplayMsgQ = NULL;
/*!< Message used to request camera buffer from the camera task */
static QMsg s_DisplayReqMsg;
static uint8_t s_ActiveFrameIndex = 0;
/*!< Init the interface with an invalid value and let camera task send a proper value */
static uint8_t s_DisplayInterfaceMode = DISPLAY_LAST_INTERFACE;

SDK_ALIGN(static uint16_t s_BufferLcd[2][LCD_WIDTH * LCD_HEIGHT], FRAME_BUFFER_ALIGN);

#if (configSUPPORT_STATIC_ALLOCATION == 1)
DTC_BSS static StackType_t s_DisplayTaskStack[DISPLAYTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_DisplayTaskTCB;
#endif

extern uint8_t *g_FrameBufferUSB;
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

static void Display_Task(void *param)
{
    BaseType_t ret;
    QMsg *pQMsg;

    xEventGroupWaitBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_DISPLAY_INIT_BIT | 1 << SYNC_VIDEO_CAMERA_INIT_BIT, pdFALSE,
                        pdTRUE, portMAX_DELAY);

    LOGD("[Display]:running\r\n");
    pQMsg = &s_DisplayReqMsg;
    Camera_SendQMsg((void *)&pQMsg);

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
                    pQMsg                        = &s_DisplayReqMsg;
                    s_DisplayReqMsg.msg.raw.data = (void *)s_BufferLcd[s_ActiveFrameIndex];
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

int Display_Start()
{
    LOGD("[Display]:start\r\n");

    s_DisplayReqMsg.id           = QMSG_DISPLAY_FRAME_REQ;
    s_DisplayReqMsg.msg.raw.data = (void *)s_BufferLcd[s_ActiveFrameIndex];

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

void Display_Init_Task(void *param)
{
    if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
    {
        RVDisp_PowerCycleDisplay(false);
    }
    else if (Cfg_AppDataGetOutputMode() == DISPLAY_RIVERDI)
    {
        RVDisp_Init(NULL);
    }
#if SCREEN_PORTRAIT_MODE
    Display_Update((uint32_t)nxp_vertical_logo);
#else
    Display_Update((uint32_t)nxp_facemanager);
#endif
    vTaskDelay(1200);

    xEventGroupSetBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_DISPLAY_INIT_BIT);

    vTaskDelete(NULL);
}

int Display_Update(uint32_t backBuffer)
{
    if (s_DisplayInterfaceMode == DISPLAY_LAST_INTERFACE)
        return -1;

    if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
    {
        if (s_DisplayInterfaceMode == DISPLAY_INTERFACE_INFOBAR)
            ConvertRGB2YUV((uint16_t *)backBuffer, (uint16_t *)backBuffer);

        xSemaphoreGive(g_DisplayFull);
        g_FrameBufferUSB = (uint8_t *)backBuffer;
        xSemaphoreTake(g_DisplayEmpty, portMAX_DELAY);
    }
    else if (Cfg_AppDataGetOutputMode() == DISPLAY_RIVERDI)
    {
        RVDisp_SendFrame((uint16_t *)backBuffer);
    }
    else
    {
        return -2;
    }
    return 0;
}

void Display_Deinit(void)
{
    if (Cfg_AppDataGetOutputMode() == DISPLAY_RIVERDI)
    {
        RVDisp_UnInit(NULL);
        RVDisp_PowerCycleDisplay(false);
    }
    xEventGroupClearBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_DISPLAY_INIT_BIT);
}
