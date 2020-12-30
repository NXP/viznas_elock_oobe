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

#if RTVISION_BOARD
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "board.h"
#include "commondef.h"
#include "fsl_log.h"
#include "display.h"
#include "camera.h"
#include "sln_api.h"
#include "sln_pcal.h"
#include "sln_shell.h"
#include "sln_lpm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "switch.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void TimerCallback(TimerHandle_t xTimer);

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SWITCH_MSG_Q_COUNT 4

#define SET_PULL_UP_EN_VALUE 0xFF
#define SET_PULL_DOWN_VALUE  0x00
#define SET_INT_MASK_VALUE   0x00
#define SET_RISING_EDGE_INT  0x15 // all 3 switch

#define KW41_RESET_GPIO     GPIO2
#define KW41_RESET_GPIO_PIN 27U

#define TIMER_DEBOUNCE_SWITCH_1 1
/*******************************************************************************
 * Variables
 *******************************************************************************/
static QueueHandle_t SwitchMsgQ = NULL;
static QMsg SW1Msg;
static QMsg PCALMsg;
static uint8_t s_DisplayInterfaceMode = 1;
static int s_Running;
static TimerHandle_t s_DebouncingTimer = NULL;
static int s_TimerDebounceID           = TIMER_DEBOUNCE_SWITCH_1;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
DTC_BSS static StackType_t s_SwitchTaskStack[SWITCHTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_SwitchTaskTCB;
#endif

extern shell_handle_t usb_shellHandle[];
extern VIZN_api_client_t VIZN_API_CLIENT(Buttons);

/*******************************************************************************
 * Code
 ******************************************************************************/

static void KW41_DelayMs(uint32_t ms)
{
#if defined(FSL_RTOS_FREE_RTOS)
    TickType_t tick;

    tick = ms * configTICK_RATE_HZ / 1000;

    tick = (0 == tick) ? 1 : tick;

    vTaskDelay(tick);
#else
    uint32_t i;
    uint32_t loopPerMs = SystemCoreClock / 3000;

    while (ms--)
    {
        i = loopPerMs;

        while (i--)
        {
            __NOP();
        }
    }

#endif
}

void SW1_EnableInterrupts(void)
{
    GPIO_PortEnableInterrupts(SW1_GPIO, (1 << SW1_GPIO_PIN));
}

void SW1_DisableInterrupts(void)
{
    GPIO_PortDisableInterrupts(SW1_GPIO, (1 << SW1_GPIO_PIN));
}

uint8_t get_int_status_reg(uint8_t *status_reg)
{
    int sts = get_iox_reg(INT_STATUS_REG_PORT2, status_reg);
    if (sts == I2C_PASS)
    {
        return 1;
    }
    else
    {
        PRINTF("IOX pin get fail\r\n");
        return PCAL_PIN_ERROR;
    }
}

uint8_t set_int_clear_reg(uint8_t reg_value)
{
    int sts = set_iox_reg(INT_CLEAR_REG_PORT2, reg_value);
    if (sts == I2C_PASS)
    {
        return 1;
    }
    else
    {
        PRINTF("Failed to reset int\r\n");
        return PCAL_PIN_ERROR;
    }
}
/*!
 * @brief Reset KW41 configurations.
 */
void KW41Reset(void)
{
    /* Define the init structure for the reset pin */

    gpio_pin_config_t pin_config = {.direction = kGPIO_DigitalOutput, .outputLogic = 1};

    GPIO_PinInit(KW41_RESET_GPIO, KW41_RESET_GPIO_PIN, &pin_config);
    KW41_DelayMs(10);
    GPIO_PinWrite(KW41_RESET_GPIO, KW41_RESET_GPIO_PIN, 0U);
    KW41_DelayMs(10);
    pin_config.direction = kGPIO_DigitalInput;
    GPIO_PinInit(KW41_RESET_GPIO, KW41_RESET_GPIO_PIN, &pin_config);
}

/*!
 * @brief Switch interface pins initialization.
 */
void Switch_Init(void)
{
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput,
        0,
        kGPIO_IntRisingEdge,
    };

    /* Init input switch GPIO. */
    GPIO_PinInit(SW1_GPIO, SW1_GPIO_PIN, &sw_config);

    gpio_pin_config_t IOX_INX_config = {kGPIO_DigitalInput, 0, kGPIO_IntFallingEdge};

    GPIO_PinInit(PCAL_INT_GPIO, PCAL_INT_GPIO_PIN, &IOX_INX_config);

    /* Enable IRQ. */
    // EnableIRQ(BOARD_USER_BUTTON_IRQ);
}

static void Switch_Task(void *param)
{
    BaseType_t ret;
    QMsg *pQMsg;

    LOGD("[Switch]:running\r\n");
    Switch_Init();
    s_DebouncingTimer =
        xTimerCreate("DebounceTimer", pdMS_TO_TICKS(400), pdFALSE, (void *)&s_TimerDebounceID, TimerCallback);

    if (s_DebouncingTimer != NULL)
    {
        /* Enable GPIO pin interrupt */

        NVIC_SetPriority(BOARD_USER_BUTTON_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);
        EnableIRQ(BOARD_USER_BUTTON_IRQ);
        SW1_EnableInterrupts();
        GPIO_PortEnableInterrupts(PCAL_INT_GPIO, 1U << PCAL_INT_GPIO_PIN);
    }
    else
    {
        LOGE("[ERROR]:Create Timer for Switch Failed\r\n");
    }

    while (1)
    {
        s_Running = 1;
        ret       = xQueueReceive(SwitchMsgQ, (void *)&pQMsg, portMAX_DELAY);

        if (ret == pdTRUE)
        {
            LPM_SendControlStatus(LPM_ButtonOperation, 1);
            switch (pQMsg->id)
            {
                case QMSG_SWITCH_1:
                {
                    s_DisplayInterfaceMode = (Cfg_AppDataGetInterfaceMode() + 1) % DISPLAY_LAST_INTERFACE;
                    if (kStatus_API_Layer_Success !=
                        VIZN_SetDispOutputInterface(&VIZN_API_CLIENT(Buttons),
                                                    (cfg_displayinterface_t)s_DisplayInterfaceMode))
                    {
                        LOGE("[WARNING]: Failed to set new interface\r\n");
                    }
                }
                break;

                case QMSG_PCAL_SWITCHES:
                {
                    uint8_t status_reg;
                    uint8_t sts = get_int_status_reg(&status_reg);
                    if (sts != PCAL_PIN_ERROR)
                    {
                        vizn_api_status_t status;
                        switch (status_reg)
                        {
                            case (1 << SW2_PIN_PCAL):
                                // schedule a remove
                                status = VIZN_DelUser(&VIZN_API_CLIENT(Buttons), (char *)"-n");
                                if (status == kStatus_API_Layer_DelUser_MultipleDelCommand)
                                {
                                    VIZN_DelUser(&VIZN_API_CLIENT(Buttons), (char *)"-s");
                                }

                                break;

                            case (1 << SW3_PIN_PCAL):
                                if (DISPLAY_MODE_RGB == Cfg_AppDataGetDisplayMode())
                                {
                                    VIZN_SetDispMode(&VIZN_API_CLIENT(Buttons), DISPLAY_MODE_IR);
                                }
                                else
                                {
                                    VIZN_SetDispMode(&VIZN_API_CLIENT(Buttons), DISPLAY_MODE_RGB);
                                }
                                break;

                            case (1 << SW4_PIN_PCAL):
                                // schedule add
                                status = VIZN_AddUser(&VIZN_API_CLIENT(Buttons));
                                if (status == kStatus_API_Layer_AddUser_MultipleAddCommand)
                                    VIZN_AddUser(&VIZN_API_CLIENT(Buttons), (char *)"-s");

                                break;

                            default:
                                break;
                        }
                        sts = set_int_clear_reg(status_reg);
                    }
                    else
                    {
                        // Reset all flags not to lose future int. Clear isr flag only for buttons.
                        set_int_clear_reg(0x07);
                    }
                }
                break;

                default:
                    break;
            }

            LPM_SendControlStatus(LPM_ButtonOperation, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

/*!
 * @brief Interrupt service fuction of switch.
 */
void GPIO2_Combined_0_15_IRQHandler(void)
{
    // LOGD("++GPIO++\r\n");
    uint32_t intPin = 0x00;
    // Get interrupt flag for the GPIO
    intPin = GPIO_PortGetInterruptFlags(SW1_GPIO);

    // Check for the interrupt pin on the GPIO SW 1
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        if ((intPin >> SW1_GPIO_PIN) & 0x01)
        {
            BaseType_t HigherPriorityTaskWoken = pdFALSE;
            QMsg *pQMsg                        = &SW1Msg;
            // LOGD("SW1 Pressed.\r\n");
            SW1_DisableInterrupts();
            /* clear the interrupt status */
            GPIO_PortClearInterruptFlags(SW1_GPIO, 1U << SW1_GPIO_PIN);
            Switch_SendQMsgFromISR((void *)&pQMsg);
            xTimerResetFromISR(s_DebouncingTimer, &HigherPriorityTaskWoken); // Debouncing
            portYIELD_FROM_ISR(HigherPriorityTaskWoken);
        }
        else if ((intPin >> PCAL_INT_GPIO_PIN) & 0x01)
        {
            BaseType_t HigherPriorityTaskWoken = pdFALSE;
            QMsg *pQMsg                        = &PCALMsg;
            GPIO_PortClearInterruptFlags(PCAL_INT_GPIO, 1U << PCAL_INT_GPIO_PIN);
            Switch_SendQMsgFromISR((void *)&pQMsg);
            portYIELD_FROM_ISR(HigherPriorityTaskWoken);
        }
    }
}

int Switch_Start(void)
{
    LOGD("[Switch]:start\r\n");
    s_Running = 0;

    PCALMsg.id = QMSG_PCAL_SWITCHES;
    SW1Msg.id  = QMSG_SWITCH_1;
    SwitchMsgQ = xQueueCreate(SWITCH_MSG_Q_COUNT, sizeof(QMsg *));

    if (SwitchMsgQ == NULL)
    {
        LOGE("[ERROR]:xQueueCreate switch queue\r\n");
        return -1;
    }

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(Switch_Task, "Switch Task", SWITCHTASK_STACKSIZE, NULL, SWITCHTASK_PRIORITY,
                                  s_SwitchTaskStack, &s_SwitchTaskTCB))
#else
    if (xTaskCreate(Switch_Task, "Switch Task", SWITCHTASK_STACKSIZE, NULL, SWITCHTASK_PRIORITY, NULL) != pdPASS)
#endif
    {
        LOGE("[ERROR]:Switch Task created failed\r\n");

        while (1)
            ;
    }
    LOGD("[Switch]:started\r\n");
    return 0;
}

int Switch_SendQMsg(void *msg)
{
    BaseType_t ret;
    LOGD("[>>Switch]:sendMsg\r\n");

    while (!s_Running)
    {
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    ret = xQueueSend(SwitchMsgQ, msg, (TickType_t)0);

    if (ret != pdPASS)
    {
        LOGE("[ERROR]:Switch_SendQMsg failed\r\n");
        return -1;
    }

    LOGD("[<<Switch]:sendMsg\r\n");
    return 0;
}

int Switch_SendQMsgFromISR(void *msg)
{
    BaseType_t ret;
    LOGD("[>>Switch]:sendMsg\r\n");

    ret = xQueueSendFromISR(SwitchMsgQ, msg, (TickType_t)0);

    if (ret != pdPASS)
    {
        LOGE("[ERROR]:Switch_SendQMsgFromISR failed\r\n");
        return -1;
    }

    LOGD("[<<Switch]:sendMsg\r\n");
    return 0;
}

static void TimerCallback(TimerHandle_t xTimer)
{
    switch (*(int *)pvTimerGetTimerID(xTimer))
    {
        case TIMER_DEBOUNCE_SWITCH_1:
            GPIO_PortClearInterruptFlags(SW1_GPIO, 1U << SW1_GPIO_PIN);
            SW1_EnableInterrupts();
            break;
    }
}
#endif
