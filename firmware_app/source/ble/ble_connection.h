/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef BLE_CONNECTION_H_
#define BLE_CONNECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/* JSON Items */
#define COMMAND_STRING        "command"
#define DEVICENAME_STRING     "dev"
#define PASSCODE_STRING       "pass_code"
#define LENGTH_STRING         "len"
#define COOKIE_STRING         "cookie"
#define ACCEPT_REQUEST_STRING "accept_request"

#define MAX_BLE_MTU 244

/* Queue Parameters */
#define RECV_DATA_QUEUE_LENGTH    40
#define RECV_DATA_QUEUE_ITEM_SIZE MAX_BLE_MTU

#define BLEUARTSERVICETASK_STACKSIZE 1024

typedef int (*ble_proccess_data_packet_fun)(uint8_t *data);

void BleUartService(void *args);

int BleUartSendNotification(uint8_t *data, int len);

void BleSetProcessDataPakcetCallback(ble_proccess_data_packet_fun callback);

/**
 * @brief Runs the BLE job for wifi provisioning
 *
 * The job purpose is to get the wifi credentials from the companion app:
 * -The job waits for the ble_cred_received_event which will be set after the
 *  WIFI CHARACTERISTIC was received
 * -The characteristic is parsed to check if a valid JSON was received
 * -The wifi credentials are then saved in flash
 *
 * This functions will block the thread execution until wifi credentials are received
 */
void BleProvisioning(void);

/**
 * @brief Starts the ble server
 *
 * The job purpose is to start the ble server:
 * -The device starts the ble server
 *
 */
void BleStart(void);

#ifdef __cplusplus
}
#endif

#endif /* BLE_CONNECTION_H_ */
