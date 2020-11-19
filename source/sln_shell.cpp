/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include <stdarg.h>

#include "sln_shell.h"
#include "commondef.h"

#include "serial_manager.h"
#include "serial_port_usb.h"
#include "usb.h"
#include "usb_phy.h"
#include "usb_device_board_config.h"
#include "usb_device_config.h"

#include "sln_cli.h"
#include "fsl_debug_console.h"
#include "fsl_log.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define APPLICATION_RESET_ISR_ADDRESS (SCB->VTOR + 0x4)
#define APP_VECTOR_TABLE_APP_A (0x60300000U)
#define APP_VECTOR_TABLE_APP_B (0x60D00000U)
#define JSON_FILEPATH_APPA "AppA"
#define JSON_FILEPATH_APPB "AppB"
#define JSON_FILEPATH_APPUNK "UnknownApp"

#define CONTROLLER_ID kSerialManager_UsbControllerEhci0
#if (defined(SERIAL_MANAGER_NON_BLOCKING_MODE) && (SERIAL_MANAGER_NON_BLOCKING_MODE > 0U))
#define SLN_SERIAL_MANAGER_RECEIVE_BUFFER_LEN 2048U
#endif

/* Sets the length of the buffers into which logging messages are written - so
 * also defines the maximum length of each log message. */
#define configLOGGING_MAX_MESSAGE_LENGTH 250

/*******************************************************************************
 * GLobal  Declarations
 ******************************************************************************/
SDK_ALIGN(static uint8_t uart_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
shell_handle_t ble_shellHandle;
extern serial_handle_t g_serialHandle;

NonCachedNonInit static uint8_t usb_shellHandleBuffer_1[SHELL_HANDLE_SIZE];
NonCached /*static*/ shell_handle_t usb_shellHandle[USB_DEVICE_CONFIG_CDC_ACM] = {&usb_shellHandleBuffer_1};

extern uint8_t usb_serialHandleBuffer[USB_DEVICE_CONFIG_CDC_ACM][SERIAL_MANAGER_HANDLE_SIZE];
extern serial_handle_t usb_serialHandle[USB_DEVICE_CONFIG_CDC_ACM];

QueueHandle_t g_UsbShellQueue;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
DTC_BSS static StackType_t s_UartTaskStack[UARTTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_UartTaskTCB;
DTC_BSS static StackType_t s_USBTaskStack[USBTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_USBTaskTCB;
#endif

/*******************************************************************************
 * Start Implementation
 ******************************************************************************/
static void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
#endif
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    if (CONTROLLER_ID == kSerialManager_UsbControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
#endif
}

#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
static void uart_shell_task(void *args)
{
    SHELL_Printf(ble_shellHandle, "UART Shell started\r\n");
    SHELL_Printf(ble_shellHandle, SHELL_PROMPT);

    SHELL_Task(ble_shellHandle);

    SHELL_Printf(ble_shellHandle, "UART Shell stopped\r\n");
    vTaskDelete(NULL);
}
#endif

static void uart_shell_init_task(void *arg)
{
    ble_shellHandle = &uart_shellHandleBuffer[0];

    DbgConsole_Flush();
    SHELL_Init(ble_shellHandle, g_serialHandle, (char *)SHELL_PROMPT);
    RegisterFFICmds(ble_shellHandle);

#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    xTaskCreate(uart_shell_task, "UART_Shell_Task", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif
    vTaskDelete(NULL);
}

void uart_shell_init(void)
{
    BOARD_InitDebugConsole();
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(uart_shell_init_task, "uart_shell_init_task", UARTTASK_STACKSIZE, NULL,
                                  tskIDLE_PRIORITY + 1, s_UartTaskStack, &s_UartTaskTCB))
#else
    if (xTaskCreate(uart_shell_init_task, "uart_shell_init_task", UARTTASK_STACKSIZE, NULL, tskIDLE_PRIORITY + 1, NULL))
#endif
    {
        LOGE("[ERROR]:Uart Task created failed\r\n");

        while (1)
            ;
    }
}

void usb_shell_init(void)
{
    /* Init SHELL */
    SHELL_Init(usb_shellHandle[0], usb_serialHandle[0], (char *)USB_SHELL_PROMPT);
    RegisterFFICmds(usb_shellHandle[0]);
    /* Init SHELL command processor */
    g_UsbShellQueue = xQueueCreate(USB_SHELL_CMDQUEUE_SIZE, sizeof(UsbShellCmdQueue_t));

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(UsbShell_CmdProcess_Task, "UsbShell_CmdProcess_Task", USBTASK_STACKSIZE, NULL,
                                  tskIDLE_PRIORITY + 1, s_USBTaskStack, &s_USBTaskTCB))
#else
    if (xTaskCreate(UsbShell_CmdProcess_Task, "UsbShell_CmdProcess_Task", USBTASK_STACKSIZE, NULL, tskIDLE_PRIORITY + 1,
                    NULL))
#endif
    {
        LOGE("[ERROR]:USB_CmdProcess Task created failed\r\n");

        while (1)
            ;
    }
}

char *getAppBank(void)
{
    /* Find the current running bank by checking the ResetISR Address in the vector table (which is loaded into
     * DTC) */
    uint32_t runningFromBankA =
        (((*(uint32_t *)(APPLICATION_RESET_ISR_ADDRESS)) & APP_VECTOR_TABLE_APP_A) == APP_VECTOR_TABLE_APP_A);
    uint32_t runningFromBankb =
        (((*(uint32_t *)(APPLICATION_RESET_ISR_ADDRESS)) & APP_VECTOR_TABLE_APP_B) == APP_VECTOR_TABLE_APP_B);

    if (runningFromBankA)
        return JSON_FILEPATH_APPA;
    if (runningFromBankb)
        return JSON_FILEPATH_APPB;
    return JSON_FILEPATH_APPUNK;
}