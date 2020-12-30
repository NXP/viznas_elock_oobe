/*
 * Copyright 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include <sln_tcp_connection.h>
#include "commondef.h"
#include "FreeRTOS.h"
#include "task.h"

#include "fsl_common.h"
#include "fsl_log.h"

#include "sln_app_fwupdate.h"
#include "sln_connection.h"
#include "sln_api.h"

#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/api.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static err_t netconn_write_blocking(struct netconn *pConnection, void *message, int size);
static void ota_server_callback(struct netconn *pConnection, enum netconn_evt event, uint16_t len);
static void parse_buffer(uint8_t *buff);
static int send_error_code(int statusCode);
static tcp_connection_status_t read_connection(void);
static void TCP_Connection_Server(void *param);
/*******************************************************************************
 * Variables
 ******************************************************************************/

#if (configSUPPORT_STATIC_ALLOCATION == 1)
DTC_BSS static StackType_t s_TCPTaskStack[TCPTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_TCPTaskTCB;
#endif

static struct netconn *conn                = NULL;
static struct netconn *newconn             = NULL;
static EventGroupHandle_t s_netconn_events = NULL;

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief Netconn write is an async operation. Wrapper to wait for the message to be delivered
 *
 * @param *pConnection  connection on which to send the message
 * @param *message      pointer to the message
 * @param size          size of the message
 */
static err_t netconn_write_blocking(struct netconn *pConnection, void *message, int size)
{
    EventBits_t uxBits = 0;
    err_t err          = ERR_OK;

    xEventGroupClearBits(s_netconn_events, 1 << NETCONN_EVT_SENDPLUS);

    err = netconn_write(pConnection, message, size, NETCONN_COPY);
    if (err != ERR_OK)
    {
        return err;
    }

    uxBits = xEventGroupWaitBits(s_netconn_events, 1 << NETCONN_EVT_SENDPLUS, pdTRUE, pdTRUE, TCP_COMMS_TIMEOUT);
    if ((uxBits & (1 << NETCONN_EVT_SENDPLUS)) == 0)
    {
        LOGE("[ERROR]TCP failed to send caused by timeout \r\n");
        err = ERR_TIMEOUT;
    }

    return err;
}

/**
 * @brief Callback function called from tcp thread after a message was send
 *
 * @param *pConnection  connection on which the operation was ended
 * @param event         type of the event
 * @param len           lenght of bytes send/recv
 */
static void ota_server_callback(struct netconn *pConnection, enum netconn_evt event, uint16_t len)
{
    switch (event)
    {
        case NETCONN_EVT_SENDPLUS:
            if ((pConnection == newconn) && (len != 0))
            {
                xEventGroupSetBits(s_netconn_events, 1 << NETCONN_EVT_SENDPLUS);
            }
            break;
        default:
        {
            LOGI("Do nothing \r\n");
        }
    }
}

/**
 * @brief Sends error code back.
 *
 * @param statusCode    The status of the operation
 */
static int send_error_code(int statusCode)
{
    /* Used if an OTA message received */
    int status            = 0;
    uint32_t size         = 0;
    cJSON *jsonMessage    = NULL;
    cJSON *jsonErrorState = NULL;
    char *jsonStr         = NULL;
    err_t err             = ERR_OK;

    /* Create JSON objects */
    jsonMessage    = cJSON_CreateObject();
    jsonErrorState = cJSON_CreateNumber(statusCode);

    if (jsonMessage == NULL || jsonErrorState == NULL)
    {
        LOGE("[ERROR]send_error_code failed to create json objects \r\n");
        return 1;
    }

    /* Add jsonErrorState to jsonMessage */
    cJSON_AddItemToObject(jsonMessage, "error", jsonErrorState);

    jsonStr = cJSON_PrintUnformatted(jsonMessage);
    size    = (uint32_t)strlen(jsonStr);

    err = netconn_write_blocking(newconn, (void *)&size, 4);
    if (err == ERR_OK)
    {
        err = netconn_write_blocking(newconn, (void *)jsonStr, size);
        /* Failed to send status */
        if (err != ERR_OK)
        {
            LOGE("[ERROR]TCP failed to send status back  \r\n");
            status = 1;
        }
    }
    else
    {
        /* Failed to send size */
        LOGE("[ERROR]TCP failed to send size \r\n");
        status = 1;
    }

    /* Send was done, delete message */
    cJSON_Delete(jsonMessage);
    vPortFree(jsonStr);
    return status;
}

/**
 * @brief Parse the buffer to look for fwupdate command
 *
 * @param *buff message received
 */
static void parse_buffer(uint8_t *buff)
{
    /* Check if this can be a message used for fwupdate */
    fwupdate_check_status_t status;

    status = FWUpdate_check_start_command(buff);
    if (FWUPDATE_OK == status)
    {
        /* Right message received, signal status ok */
        if (send_error_code(status) != 0)
        {
            /* Failed to send back status. Do not set FWUpdate bit */
            return;
        }

        /* Set the flag for update */
        uint32_t status = VIZN_FWUpdate_Set(NULL, FWUPDATE_OTA);
        if (status == kStatus_API_Layer_Success)
        {
            LOGI("The firmware update with method OTA will be started\r\n");
        }
        else
        {
            LOGI("Cannot start the OTA firmware update\r\n");
        }
    }
    else if (FWUPDATE_WRONG_MESSAGETYPE == status)
    {
        /*It has the format of the OTA, wrong messagetype */
        send_error_code(status);
    }
}

/**
 * @brief Read the network connection which was established
 */
static tcp_connection_status_t read_connection(void)
{
    tcp_connection_status_t status = kCommon_Success;
    err_t err                      = ERR_OK;
    struct netbuf *buf             = NULL;
    uint8_t *rcvBuffer             = NULL;
    uint8_t *packet_buffer         = NULL;
    uint16_t pSize                 = 0;
    uint16_t bytes_read            = 0;
    uint32_t packet_size           = 0;

    err = netconn_recv(newconn, &buf);
    if (err != ERR_OK)
    {
        return kCommon_ConnectionLost;
    }
    /* First send how much information does the packet has */
    err = netbuf_data(buf, (void **)(&rcvBuffer), &pSize);
    if (err != ERR_OK)
    {
        return kCommon_NoDataRead;
    }
    memcpy(&packet_size, rcvBuffer, pSize);
    netbuf_delete(buf);

    /* If the packet is too big, abort */
    if (packet_size > TCP_MAX_BUFFER_SIZE)
    {
        /* The packet is too big, something is wrong abort */
        return kCommon_ToManyBytes;
    }

    packet_buffer = (uint8_t *)pvPortMalloc(packet_size);
    if (packet_buffer == NULL)
    {
        /* Failed to allocate memory for the future packet */
        return kCommon_Failed;
    }

    /* Wait to received everything */
    do
    {
        pSize     = 0;
        rcvBuffer = NULL;
        err       = netconn_recv(newconn, &buf);
        if (err != ERR_OK)
        {
            /* If it fails to received all, drop the packet and free the buffer */
            vPortFree(packet_buffer);
            return kCommon_ConnectionLost;
        }
        err = netbuf_data(buf, (void **)(&rcvBuffer), &pSize);
        if (err != ERR_OK)
        {
            vPortFree(packet_buffer);
            return kCommon_NoDataRead;
        }

        /* Copy data into a local buffer */
        memcpy((void *)(packet_buffer + bytes_read), rcvBuffer, pSize);
        bytes_read += pSize;
        netbuf_delete(buf);
    } while (bytes_read != packet_size);

    /* Parse the received data */
    parse_buffer(packet_buffer);

    return status;
}

/* TCP server */
static void TCP_Connection_Server(void *param)
{
    err_t err = ERR_OK;

    /* Wait for wifi to connect */
    while (0 == SLN_Connection_WIFI_getIPAddress())
    {
        /* Give time to the network task too connect to Wi-Fi */
        vTaskDelay(1000);
    }

    LOGI("TCP server start\n");

    conn = netconn_new_with_callback(NETCONN_TCP, ota_server_callback);
    if (conn == NULL)
    {
        LOGE(("tcp: invalid conn\n"));
    }
    err = netconn_bind(conn, IP_ADDR_ANY, TCP_COMMS_PORT);
    if (err != ERR_OK)
    {
        LOGE(("tcp: error bind\n"));
    }

    err = netconn_listen(conn);
    if (err != ERR_OK)
    {
        return;
    }
    conn->recv_timeout = TCP_COMMS_TIMEOUT;

    while (1)
    {
        if (newconn != NULL)
        {
            netconn_delete(newconn);
            newconn = NULL;
        }

        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK)
        {
            tcp_connection_status_t status;
            newconn->recv_timeout = TCP_COMMS_TIMEOUT;
            status                = read_connection();
            if (kCommon_Success != status)
            {
                LOGE("[ERROR]TCP failed to recv \r\n");
            }
        }
    }

    vTaskDelete(NULL);
}

void TCP_Connection_Start()
{
    s_netconn_events = xEventGroupCreate();
    if (s_netconn_events == NULL)
    {
        LOGE("[ERROR]Event Group failed\r\n");
        while (1)
            ;
    }
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(TCP_Connection_Server, "TCP_Server", TCPTASK_STACKSIZE, NULL, TCPTASK_PRIORITY,
                                  s_TCPTaskStack, &s_TCPTaskTCB))
#else
    if (xTaskCreate(TCP_Connection_Server, "TCP_Server", TCPTASK_STACKSIZE, NULL, TCPTASK_PRIORITY, NULL) != pdPASS)
#endif
    {
        LOGE("[ERROR]TCP Task created failed\r\n");

        while (1)
            ;
    }
}
