/*
 * Copyright 2018,2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/* Platform includes */
#include "platform_imxrt.h"

#include "wiced_bt_dev.h"

/* Bluetooth includes */
#include "ble_connection.h"
#include "ble_server.h"

/* FreeRTOS kernel includes */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"

/* Json includes */
#include "cJSON.h"

/* Network connection includes */
#include "network_connection.h"

/* Device specific includes */
#include "device_utils.h"

/* Wifi includes */
#include "wifi_credentials.h"

#include "sln_lpm.h"

#if USE_WIFI_CONNECTION

extern EventGroupHandle_t xEventGapSemaphore;
extern EventGroupHandle_t ble_cred_received_event;
extern QueueHandle_t ble_uart_recv_data_queue;

char *ble_serialNumber = NULL;
static uint8_t s_ble_provisioning_stop_event;
static ble_proccess_data_packet_fun s_ble_process_data_packet;

void BleSetProcessDataPakcetCallback(ble_proccess_data_packet_fun callback)
{
    s_ble_process_data_packet = callback;
}

int BleUartSendNotification(uint8_t *data, int len)
{
    ble_send_notification((uint8_t *)data, len);

    return 0;
}

int BleReceieveDataPacket(cJSON *head_json)
{
    uint32_t len;
    uint32_t packet_count;
    uint8_t *buffer_str, *p_str;

    cJSON *length_json = cJSON_GetObjectItemCaseSensitive(head_json, LENGTH_STRING);
    if (length_json == NULL)
    {
        configPRINTF("frame header length parse error.\r\n");
        return -1;
    }

    len = length_json->valuedouble;
    packet_count =
        (len % RECV_DATA_QUEUE_ITEM_SIZE) ? ((len / RECV_DATA_QUEUE_ITEM_SIZE) + 1) : (len / RECV_DATA_QUEUE_ITEM_SIZE);

    buffer_str = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * (packet_count * RECV_DATA_QUEUE_ITEM_SIZE));
    if (buffer_str == NULL)
    {
        configPRINTF("malloc failed\r\n");
        return -1;
    }

    p_str = buffer_str;

    while (packet_count--)
    {
        vTaskDelay(pdMS_TO_TICKS(5));

        if ((xQueueReceive(ble_uart_recv_data_queue, p_str, portMAX_DELAY)) != pdPASS)
        {
            configPRINTF("Cannot receive data from queue\r\n");
            continue;
        }

        p_str += RECV_DATA_QUEUE_ITEM_SIZE;
    }

    s_ble_process_data_packet(buffer_str);

    vPortFree(buffer_str);

    return 0;
}

void BleStart(void)
{
    /* Serial number on the box has the padding "=" character" */
    xEventGapSemaphore = xEventGroupCreate();
    APP_GetUniqueID(&ble_serialNumber, false);
    ble_pwr_on();
    ble_server_start(ble_serialNumber);

    ble_uart_recv_data_queue = xQueueCreate(RECV_DATA_QUEUE_LENGTH, RECV_DATA_QUEUE_ITEM_SIZE);

    if (xTaskCreate(BleUartService, "wiced_ble_receive_task", BLEUARTSERVICETASK_STACKSIZE, NULL,
                    (configMAX_PRIORITIES)-5UL, NULL) != pdPASS)
    {
        configPRINTF("wiced_ble_host_task created failed\r\n");
        while (1)
            ;
    }

    xEventGroupWaitBits(xEventGapSemaphore, 1 << BTM_ENABLED_EVT, pdFALSE, pdTRUE, portMAX_DELAY);
    configPRINTF(("BLE: Server started\r\n"));
}

void BleUartService(void *args)
{
    char received_str[256];

    while (1)
    {
        if ((xQueueReceive(ble_uart_recv_data_queue, received_str, portMAX_DELAY)) != pdPASS)
        {
            configPRINTF("[BLE]:Cannot receive data from queue\r\n");
            continue;
        }

        LPM_SendControlStatus(LPM_BleRemoteTransmit, 1);

        cJSON *frame_hand_head_json = cJSON_Parse(received_str);
        if (frame_hand_head_json == NULL)
        {
            configPRINTF("frame header parse error.\r\n");
            continue;
        }

        BleReceieveDataPacket(frame_hand_head_json);

        cJSON_Delete(frame_hand_head_json);

        vTaskDelay(pdMS_TO_TICKS(100));

        LPM_SendControlStatus(LPM_BleRemoteTransmit, 0);
    }
}

void BleStop(void)
{
    ble_server_stop();
    ble_pwr_off();
    xEventGroupWaitBits(xEventGapSemaphore, 1 << BTM_DISABLED_EVT, pdFALSE, pdTRUE, portMAX_DELAY);
    vEventGroupDelete(xEventGapSemaphore);
    vPortFree(ble_serialNumber);
}

void ble_provisioning_close_event_set()
{
    s_ble_provisioning_stop_event = 1;
}

void BleProvisioning(void)
{
    cJSON *parsed_message   = NULL;
    const cJSON *json_item1 = NULL;
    const cJSON *json_item2 = NULL;

    char wifi_characteristic_str[COMMISSIONING_CHAR_WIFI_VAL_SIZE_B + 1];
    status_t credsave_status = 1;
    wifi_cred_t wifi_cred    = {0};
    ble_cred_received_event  = xEventGroupCreate();
    configPRINTF(("BLE: Provisioning job started)\r\n"));
    s_ble_provisioning_stop_event = 0;
    while (!s_ble_provisioning_stop_event)
    {
        EventBits_t event;
        configPRINTF(("BLE: Wait for wifi characteristic\r\n"));
        event = xEventGroupWaitBits(ble_cred_received_event, 1, pdTRUE, pdFALSE, portTICK_PERIOD_MS * 500);
        if (event == 0)
        {
            /* Timeout expired, try again */
            continue;
        }
        ble_server_get_char_wifi_str(wifi_characteristic_str);
        configPRINTF(("BLE: Wifi characteristic value received\r\n", wifi_characteristic_str));

        if (parsed_message != NULL)
        {
            cJSON_Delete(parsed_message);
        }
        parsed_message = cJSON_Parse(wifi_characteristic_str);
        if (parsed_message == NULL)
        {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL)
            {
                configPRINTF(("BLE: Error at cJSON_Parse: %s\n", error_ptr));
            }
            continue;
        }

        /* Check if wifi characteristic was received */
        if (cJSON_GetArraySize(parsed_message) == 2)
        {
            /* Expected Json {"SSID":"<ssid>","Password":"<password>"} */
            json_item1 = cJSON_GetObjectItemCaseSensitive(parsed_message, "SSID");
            json_item2 = cJSON_GetObjectItemCaseSensitive(parsed_message, "Password");
            if (cJSON_IsString(json_item1) && (json_item1->valuestring != NULL) && cJSON_IsString(json_item2) &&
                (json_item2->valuestring != NULL))
            {
                configPRINTF(("BLE: Wifi credentials received: SSID '%s', Password '%s'\r\n", json_item1->valuestring,
                              json_item2->valuestring));

                wifi_cred.ssid.length     = strlen(json_item1->valuestring);
                wifi_cred.password.length = strlen(json_item2->valuestring);

                if (wifi_cred.ssid.length <= sizeof(wifi_cred.ssid.value))
                {
                    memcpy(wifi_cred.ssid.value, json_item1->valuestring, wifi_cred.ssid.length);
                }
                else
                {
                    configPRINTF(("BLE: Cannot get wifi SSID\r\n"));
                    continue;
                }

                if (wifi_cred.password.length <= sizeof(wifi_cred.password.value))
                {
                    memcpy(wifi_cred.password.value, json_item2->valuestring, wifi_cred.password.length);
                }
                else
                {
                    configPRINTF(("BLE: Cannot get wifi Password\r\n"));
                    continue;
                }

                credsave_status = wifi_credentials_flash_set(&wifi_cred);
                if (!credsave_status)
                {
                    configPRINTF(("BLE: Wifi credentials saved in flash\r\n"));
                    break;
                }
                else
                {
                    configPRINTF(
                        ("BLE: Failed to write wifi credentials in flash, error code %d\r\n", credsave_status));
                    continue;
                }
            }
            else
            {
                configPRINTF(("BLE: Wrong wifi characteristic Json size"));
            }
        }
        else
        {
            configPRINTF(("BLE: Wrong wifi characteristic Json parameters"));
        }
    }

    if (parsed_message != NULL)
    {
        cJSON_Delete(parsed_message);
    }

    vEventGroupDelete(ble_cred_received_event);
    configPRINTF(("BLE: Provisioning job stopped\r\n"));
}

#endif
