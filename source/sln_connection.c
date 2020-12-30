/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "fsl_log.h"
#include "event_groups.h"

#include "device_utils.h"
#include "commondef.h"
#include "remote_cmds.h"
#include "sln_shell.h"
#include "sln_connection.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define WIFI_LOCK()
#define WIFI_UNLOCK()
extern EventGroupHandle_t g_SyncVideoEvents;
#if (configSUPPORT_STATIC_ALLOCATION == 1)
DTC_BSS static StackType_t s_WifiTaskStack[WIFIINITTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_WifiTaskTCB;
DTC_BSS static StackType_t s_BLETaskStack[BLEINITTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_BLETaskTCB;
DTC_BSS static StackType_t s_UDPTaskStack[DEFAULT_THREAD_STACKSIZE];
DTC_BSS static StaticTask_t s_UDPTaskTCB;
#endif

static QueueHandle_t s_WIFIMsgQ      = NULL;
static QueueHandle_t s_BLEMsgQ       = NULL;
static uint32_t s_wifi_state_machine = 0;
static uint32_t s_ble_state_machine  = 0;
static uint8_t s_stop_retrying;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void WiFi_Task(void *param);
static void APP_WifiProvisioning_ApJob(void);
static void BLE_Task(void *param);
/*******************************************************************************
 * Code
 ******************************************************************************/
static void unset_wifi_state_machine(uint32_t state)
{
    WIFI_LOCK();

    s_wifi_state_machine &= ~state;

    WIFI_UNLOCK();
}

static void set_wifi_state_machine(uint32_t state, bool clear)
{
    WIFI_LOCK();
    if (clear)
        s_wifi_state_machine = state;
    else
        s_wifi_state_machine |= state;
    WIFI_UNLOCK();
}

static void connection_update(bool status)
{
    if (status == WICED_FALSE)
    {
        unset_wifi_state_machine(1 << CONNECTION_WIFI_CONNECTED);
        set_wifi_state_machine(1 << CONNECTION_WIFI_DISCONNECTED, false);
        UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Linked lost\r\n"));
    }
    else
    {
        unset_wifi_state_machine(1 << CONNECTION_WIFI_DISCONNECTED);
        set_wifi_state_machine(1 << CONNECTION_WIFI_CONNECTED, false);
        UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Linked re-established\r\n"));
    }
}

void APP_Wifi_UX_Callback(network_wifi_conn_state_t state)
{
    switch (state)
    {
        case NETWORK_WIFI_CONN_WIFI_SETUP:
            UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Start Wi-Fi init\r\n"));
            break;
        case NETWORK_WIFI_CONN_INVALID_WIFI_CRED:
            UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Something is wrong with cred. Start Provisioning Mode\r\n"));
            break;
        case NETWORK_WIFI_CONN_NO_ACCESS_POINT:
            UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Network not reachable.\r\n"));
            break;
        case NETWORK_WIFI_CONN_ACCESS_POINT_FOUND:
            UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Network reached.\r\n"));
            break;
    }
}
/**
 * @brief Runs the AP mode job for wifi provisioning
 *
 * The job purpose is to get the wifi credentials from the companion app:
 * -The device starts the AP where the mobile will connect
 * -The udp_server_task is used for the communication between device and mobile
 * -The job waits for the ap_close_event bit which will be set after the wifi
 *  credentials were received and saved in flash.
 */

static void APP_WifiProvisioning_ApJob(void)
{
    char *serialNumber   = NULL;
    wiced_ssid_t ap_ssid = {0};

    /* Create the SSID and Password */
    /* Serial number on the box has the padding "=" character" */
    // APP_GetUniqueID(&serialNumber, false);
    APP_GetUniqueID(&serialNumber, false);
    snprintf((char *)ap_ssid.value, sizeof(ap_ssid.value), "SLN_VIZ_%s", serialNumber);
    ap_ssid.length = strlen((char *)ap_ssid.value);

    if (APP_NETWORK_Wifi_StartAP(ap_ssid, serialNumber))
    {
        UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Could not start AP\r\n"));
    }
    else
    {
        udp_server_args_t udp_server_args = {NETWORK_MODE_AP, xEventGroupCreate()};
        UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("AP started\r\n"));

#if (configSUPPORT_STATIC_ALLOCATION == 1)
        if (xTaskCreateStatic(udp_server_task, "UDP_Server_Task", DEFAULT_THREAD_STACKSIZE, &udp_server_args,
                              DEFAULT_THREAD_PRIO, s_UDPTaskStack, &s_UDPTaskTCB) == NULL)
#else
        if (xTaskCreate(udp_server_task, "UDP_Server_Task", DEFAULT_THREAD_STACKSIZE, &udp_server_args,
                        DEFAULT_THREAD_PRIO, NULL) != pdPASS)

#endif
        {
            UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Failed to crate UDP_SERVER_TASK\r\n"));
            while (1)
                ;
        }
        xEventGroupWaitBits(udp_server_args.ap_close_event, 1, pdTRUE, pdTRUE, portMAX_DELAY);
        UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("AP close and switch to Station\r\n"));
        vEventGroupDelete(udp_server_args.ap_close_event);
        APP_NETWORK_Wifi_StopAP();
        vTaskDelay(portTICK_PERIOD_MS * 1000);
    }

    vPortFree(serialNumber);
}

static void BLE_Start()
{
    s_ble_state_machine = 1 << CONNECTION_BLE_SETUP_MODE;
    BleSetProcessDataPakcetCallback(Remote_cmd_process);
    BleStart();
    xEventGroupSetBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_BLUETOOTH_SERVER_START_INIT);
    s_ble_state_machine = 1 << CONNECTION_BLE_ON;
}

static void WIFI_Disconnect()
{
    /* start disconnecting procedure*/
    UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Start wifi disconnecting\r\n"));
    set_wifi_state_machine(1 << CONNECTION_WIFI_DISCONNECTING, false);
    APP_NETWORK_Uninit();
    /* wifi platform is disconnected */
    UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Wi-fi disconnected\r\n"));
    set_wifi_state_machine(1 << CONNECTION_WIFI_DISCONNECTED, true);
}

static void WIFI_Connect()
{
    status_t status;
    APP_Wifi_UX_Callback(NETWORK_WIFI_CONN_WIFI_SETUP);
    set_wifi_state_machine(1 << CONNECTION_WIFI_CONNECTING, false);
    s_stop_retrying = 0;
    while (s_stop_retrying == 0)
    {
        status = APP_NETWORK_Wifi_Connect(true, true);
        if (WIFI_CONNECT_SUCCESS == status)
        {
            set_wifi_state_machine(1 << CONNECTION_WIFI_INIT | 1 << CONNECTION_WIFI_CONNECTED, true);
            break;
        }
        else
        {
            UsbShell_DbgPrintf(VERBOSE_MODE_L3, "Failed to connect to wifi network, error %d\r\n", status);
            if (status == WIFI_CONNECT_WRONG_CRED)
            {
                /* Erase the wifi credentials if invalid credentials were provided */
                APP_Wifi_UX_Callback(NETWORK_WIFI_CONN_INVALID_WIFI_CRED);
                wifi_credentials_flash_reset();
            }
            else if (status == WIFI_CONNECT_NO_CRED)
            {
                /* No credentials means wrong credentials */
                APP_Wifi_UX_Callback(NETWORK_WIFI_CONN_INVALID_WIFI_CRED);
            }
        }
        set_wifi_state_machine(1 << CONNECTION_WIFI_PROVISIONING_MODE, false);
#if PROVISIONING_METHOD == PROVISIONING_USE_WIFI
        UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Start AP job\r\n"));
        APP_WifiProvisioning_ApJob();
#elif PROVISIONING_METHOD == PROVISIONING_USE_BLE
        xEventGroupWaitBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_BLUETOOTH_SERVER_START_INIT, pdFALSE, pdTRUE,
                            portMAX_DELAY);
        UsbShell_DbgPrintf(VERBOSE_MODE_L3, ("Start BLE job\r\n"));
        BleProvisioning();
#endif
        unset_wifi_state_machine(1 << CONNECTION_WIFI_PROVISIONING_MODE);
    }
    unset_wifi_state_machine(1 << CONNECTION_WIFI_CONNECTING);
}

static void BLE_Task(void *param)
{
    BaseType_t ret;
    connection_commands_t commands;
    while (1)
    {
        // pick up message
        ret = xQueueReceive(s_BLEMsgQ, (void *)&commands, portMAX_DELAY);
        if (ret)
        {
            if (commands == COMMAND_BLE_ON)
            {
                BLE_Start();
            }
        }
    }
    // Should not reach here
    vTaskDelete(NULL);
}

static void WiFi_Task(void *param)
{
    BaseType_t ret;
    connection_commands_t commands;
    APP_NETWORK_Init();
    APP_Connect_Update_Handler_Set(connection_update);
    set_wifi_state_machine(0, true);
    while (1)
    {
        // pick up message
        ret = xQueueReceive(s_WIFIMsgQ, (void *)&commands, portMAX_DELAY);
        if (ret)
        {
            switch (commands)
            {
                case COMMAND_WIFI_ON:
                    WIFI_Connect();
                    break;
                case COMMAND_WIFI_OFF:
                    WIFI_Disconnect();
                    break;
                case COMMAND_WIFI_RESET:
                    WIFI_Disconnect();
                    WIFI_Connect();
                    break;
                default:
                    LOGE("[ERROR]Wrong message received \r\n");
            }
        }
    }
    // Should not reach here
    vTaskDelete(NULL);
}

void SLN_Connections_Init()
{
    connection_commands_t msg;
    cJSON_Hooks hooks;
    s_wifi_state_machine = 0;
    s_ble_state_machine  = 0;

    /* Initialize cJSON library to use FreeRTOS heap memory management. */
    hooks.malloc_fn = pvPortMalloc;
    hooks.free_fn   = vPortFree;
    cJSON_InitHooks(&hooks);

    // Always start with WIFI and BLE on.
    s_WIFIMsgQ = xQueueCreate(WIFI_MSG_Q_COUNT, sizeof(connection_commands_t));
    s_BLEMsgQ  = xQueueCreate(BLE_MSG_Q_COUNT, sizeof(connection_commands_t));

    if (s_WIFIMsgQ == NULL || s_BLEMsgQ == NULL)
    {
        LOGE("[ERROR]MSGQ  created failed\r\n");
        while (1)
            ;
    }
    msg = COMMAND_WIFI_ON;
    xQueueSend(s_WIFIMsgQ, (void *)&msg, (TickType_t)0);
    msg = COMMAND_BLE_ON;
    xQueueSend(s_BLEMsgQ, (void *)&msg, (TickType_t)0);

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (xTaskCreateStatic(BLE_Task, "BLE_Init", BLEINITTASK_STACKSIZE, NULL, BLEINITTASK_PRIORITY, s_BLETaskStack,
                          &s_BLETaskTCB) == NULL)
#else
    if (xTaskCreate(BLE_Task, "BLE_Init", BLEINITTASK_STACKSIZE, NULL, BLEINITTASK_PRIORITY, NULL) != pdPASS)
#endif
    {
        LOGE("[ERROR]BLE Init created failed\r\n");

        while (1)
            ;
    }

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (xTaskCreateStatic(WiFi_Task, "WiFi_Init", WIFIINITTASK_STACKSIZE, NULL, WIFIINITTASK_PRIORITY, s_WifiTaskStack,
                          &s_WifiTaskTCB) == NULL)
#else
    if (xTaskCreate(WiFi_Task, "WiFi_Init", WIFIINITTASK_STACKSIZE, NULL, WIFIINITTASK_PRIORITY, NULL) != pdPASS)
#endif
    {
        LOGE("[ERROR]Wifi Init created failed\r\n");
        while (1)
            ;
    }
}

bool SLN_Connection_WIFI_isConnected()
{
    return get_connect_state();
}

uint32_t SLN_Connection_WIFI_getIPAddress()
{
    return get_ipaddress().addr;
}

bool SLN_Connection_BLE_isOn()
{
    return ((s_ble_state_machine >> CONNECTION_BLE_ON) & 1) ? true : false;
}

uint32_t SLN_Connection_Commands(connection_commands_t command)
{
    if ((COMMAND_WIFI_OFF == command) || (COMMAND_WIFI_ON == command) || (COMMAND_WIFI_RESET == command))
    {
        uint32_t wifi_state_machine = s_wifi_state_machine;
        uint8_t wifi_new_state, wifi_old_state;

        if (((wifi_state_machine >> CONNECTION_WIFI_PROVISIONING_MODE) & 1) &&
            ((COMMAND_WIFI_OFF == command) || (COMMAND_WIFI_RESET == command)))
        {
            /* Stop provisioning if the command is wifi off or wifi reset*/
#if PROVISIONING_METHOD == PROVISIONING_USE_WIFI
            /* Stop UDP server */
            if (udp_server_close_event_set() != 0)
            {
                /* Stoping the UDP - failed try again later */
                return CONNECTION_WIFI_STOPPED_FAIELD;
            }
#elif PROVISIONING_METHOD == PROVISIONING_USE_BLE
            ble_provisioning_close_event_set();
#endif
            s_stop_retrying = 1;
            xQueueSend(s_WIFIMsgQ, (void *)&command, (TickType_t)0);
            return CONNECTION_WIFI_SUCCESS;
        }
        else if ((wifi_state_machine >> CONNECTION_WIFI_PROVISIONING_MODE) & 1)
        {
            /* command wifi on, wifi already on in provisioning mode*/
            return CONNECTION_WIFI_PROVISIONING_MODE;
        }
        else if ((wifi_state_machine >> CONNECTION_WIFI_CONNECTING) & 1)
        {
            /* wifi is in set-up mode and not yet provisioning */
            return CONNECTION_WIFI_CONNECTING;
        }
        else if ((wifi_state_machine >> CONNECTION_WIFI_DISCONNECTING) & 1)
        {
            /* wifi is disconecting */
            return CONNECTION_WIFI_DISCONNECTING;
        }

        if ((wifi_state_machine >> CONNECTION_WIFI_INIT) & 1)
        {
            /* wifi is on */
            if ((COMMAND_WIFI_OFF == command) || (COMMAND_WIFI_RESET == command))
            { /* The wifi is on and it will either reset or stop */
                xQueueSend(s_WIFIMsgQ, (void *)&command, (TickType_t)0);
                return CONNECTION_WIFI_SUCCESS;
            }
            else
            {
                /* wifi is already on, do nothing */
                return CONNECTION_WIFI_INIT;
            }
        }
        else if ((wifi_state_machine >> CONNECTION_WIFI_DISCONNECTED) & 1)
        {
            /* the wifi is off */
            if (COMMAND_WIFI_ON == command)
            { /* The wifi is off start it */
                xQueueSend(s_WIFIMsgQ, (void *)&command, (TickType_t)0);
                return CONNECTION_WIFI_SUCCESS;
            }
            else
            {
                /*  wifi is off do nothing*/
                return CONNECTION_WIFI_DISCONNECTED;
            }
        }
    }

    return -1;
}
