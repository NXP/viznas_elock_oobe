
/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _SLN_WIFI_H_
#define _SLN_WIFI_H_

#include "sln_shell.h"
#include "sln_dev_cfg.h"
#include "network_connection.h"
#include "wifi_credentials.h"
#include "udp_server.h"

#include "ble_connection.h"
#include "ble_server.h"

#define WIFI_MSG_Q_COUNT 3
#define BLE_MSG_Q_COUNT  2

typedef enum connection_commands
{
    COMMAND_WIFI_ON,
    COMMAND_WIFI_OFF,
    COMMAND_WIFI_RESET,
    COMMAND_BLE_ON,
    COMMAND_BLE_OFF,
} connection_commands_t;

typedef enum connection_wifi_status
{
    CONNECTION_WIFI_SUCCESS,
    /* Possible errors */
    CONNECTION_WIFI_INIT,
    CONNECTION_WIFI_RESET,
    CONNECTION_WIFI_CONNECTING,
    CONNECTION_WIFI_CONNECTED,
    CONNECTION_WIFI_DISCONNECTING,
    CONNECTION_WIFI_DISCONNECTED,
    CONNECTION_WIFI_PROVISIONING_MODE,
    CONNECTION_WIFI_STOPPED_FAIELD,
    CONNECTION_WIFI_APSERVER_STOPPED

} connection_wifi_status_t;

typedef enum connection_ble_status
{
    CONNECTION_BLE_NOT_INIT,
    CONNECTION_BLE_ON,
    CONNECTION_BLE_OFF,
    CONNECTION_BLE_SETUP_MODE,
} connection_ble_status_t;

#if defined(__cplusplus)
extern "C" {
#endif
void SLN_Connections_Init();
uint32_t SLN_Connection_Commands(connection_commands_t command);
bool SLN_Connection_WIFI_isConnected();
uint32_t SLN_Connection_WIFI_getIPAddress();
bool SLN_Connection_BLE_isOn();

#if defined(__cplusplus)
}
#endif

#endif
