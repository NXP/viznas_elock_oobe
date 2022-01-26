/*
 * Copyright 2017-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"

#include <stdio.h>
#include <stdlib.h>
#include "fsl_debug_console.h"
#include "virtual_com.h"
#include "composite.h"
#include "usb_device_descriptor.h"
#include "serial_port_internal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (defined(SERIAL_MANAGER_NON_BLOCKING_MODE) && (SERIAL_MANAGER_NON_BLOCKING_MODE > 0U))
#define SLN_SERIAL_MANAGER_RECEIVE_BUFFER_LEN 2048U
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
#define NonCachedNonInit __attribute__((section("NonCacheable, \"aw\", %nobits @")))
#define NonCached        __attribute__((section("NonCacheable.init")))

NonCachedNonInit __attribute__((aligned(8))) uint8_t usb_serialHandleBuffer_1[SERIAL_MANAGER_HANDLE_SIZE];
NonCached serial_handle_t usb_serialHandle[USB_DEVICE_CONFIG_CDC_ACM] = {&usb_serialHandleBuffer_1};

#if (defined(SERIAL_MANAGER_NON_BLOCKING_MODE) && (SERIAL_MANAGER_NON_BLOCKING_MODE > 0U))
NonCachedNonInit uint8_t usb_readRingBuffer[USB_DEVICE_CONFIG_CDC_ACM][SLN_SERIAL_MANAGER_RECEIVE_BUFFER_LEN];
#endif

extern usb_device_class_config_list_struct_t g_UsbDeviceCompositeConfigList;
extern usb_device_endpoint_struct_t g_cdcVcomDicEndpoints[];
extern usb_device_endpoint_struct_t g_cdcVcomCicEndpoints[];
extern usb_device_class_struct_t g_UsbDeviceCdcVcomConfig[];
/* Line coding of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_lineCoding[USB_DEVICE_CONFIG_CDC_ACM][LINE_CODING_SIZE] = {
    {/* E.g. 0x00,0xC2,0x01,0x00 : 0x0001C200 is 115200 bits per second */
     (LINE_CODING_DTERATE >> 0U) & 0x000000FFU, (LINE_CODING_DTERATE >> 8U) & 0x000000FFU,
     (LINE_CODING_DTERATE >> 16U) & 0x000000FFU, (LINE_CODING_DTERATE >> 24U) & 0x000000FFU, LINE_CODING_CHARFORMAT,
     LINE_CODING_PARITYTYPE, LINE_CODING_DATABITS},
};

/* Abstract state of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_abstractState[USB_DEVICE_CONFIG_CDC_ACM][COMM_FEATURE_DATA_SIZE] = {
    {(STATUS_ABSTRACT_STATE >> 0U) & 0x00FFU, (STATUS_ABSTRACT_STATE >> 8U) & 0x00FFU},
};

/* Country code of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_countryCode[USB_DEVICE_CONFIG_CDC_ACM][COMM_FEATURE_DATA_SIZE] = {
    {(COUNTRY_SETTING >> 0U) & 0x00FFU, (COUNTRY_SETTING >> 8U) & 0x00FFU},
};

/* CDC ACM information */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static usb_cdc_acm_info_t s_usbCdcAcmInfo[USB_DEVICE_CONFIG_CDC_ACM] = {
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0, 0},
};
/* Data buffer for receiving and sending*/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_currRecvBuf[USB_DEVICE_CONFIG_CDC_ACM][DATA_BUFF_SIZE];

static usb_device_composite_struct_t *s_deviceComposite;

/*******************************************************************************
 * Code
 ******************************************************************************/

usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, uint32_t event, void *param)
{
    uint32_t len;
    uint8_t *uartBitmap;
    usb_cdc_acm_info_t *acmInfo;
    usb_device_cdc_acm_request_param_struct_t *acmReqParam;
    usb_device_endpoint_callback_message_struct_t *epCbParam;
    serial_manager_callback_message_t msg;
    volatile serial_usb_cdc_state_t *serialUsbCdc;
    usb_status_t error = kStatus_USB_Error;
    uint8_t i;
    acmReqParam = (usb_device_cdc_acm_request_param_struct_t *)param;
    epCbParam   = (usb_device_endpoint_callback_message_struct_t *)param;

    for (i = 0; i < USB_DEVICE_CONFIG_CDC_ACM; i++)
    {
        if (handle == s_deviceComposite->cdcVcom[i].cdcAcmHandle)
        {
            break;
        }
    }
    if (i >= USB_DEVICE_CONFIG_CDC_ACM)
    {
        return error;
    }
    serialUsbCdc = &s_deviceComposite->cdcVcom[i];
    acmInfo      = serialUsbCdc->usbCdcAcmInfo;
    switch (event)
    {
        case kUSB_DeviceCdcEventSendResponse:
        {
            if ((1 == serialUsbCdc->attach))
            {
                if ((epCbParam->length != 0) && (!(epCbParam->length % serialUsbCdc->bulkInEndpointMaxPacketSize)))
                {
                    /* If the last packet is the size of endpoint, then send also zero-ended packet,
                     ** meaning that we want to inform the host that we do not have any additional
                     ** data, so it can flush the output.
                     */
                    error = USB_DeviceCdcAcmSend(handle, serialUsbCdc->bulkInEndpoint, NULL, 0);
                }
                else
                {
                    serialUsbCdc->tx.busy = 0;

                    if ((NULL != serialUsbCdc->tx.callback))
                    {
                        serial_manager_status_t serialManagerStatus = kStatus_SerialManager_Success;
                        msg.buffer                                  = serialUsbCdc->tx.buffer;
                        msg.length                                  = serialUsbCdc->tx.length;
                        if (USB_UNINITIALIZED_VAL_32 == epCbParam->length)
                        {
                            serialManagerStatus = kStatus_SerialManager_Canceled;
                        }
                        serialUsbCdc->tx.callback(serialUsbCdc->tx.callbackParam, &msg, serialManagerStatus);
                    }
                }
            }
        }
        break;
        case kUSB_DeviceCdcEventRecvResponse:
        {
            serialUsbCdc->rx.busy = 0U;
            if ((1 == serialUsbCdc->attach) && (1 == serialUsbCdc->startTransactions))
            {
                serial_manager_status_t callbackStatus = kStatus_SerialManager_Success;

                if ((NULL != serialUsbCdc->rx.callback))
                {
                    msg.buffer = epCbParam->buffer;
                    msg.length = epCbParam->length;
                    if (USB_UNINITIALIZED_VAL_32 == msg.length)
                    {
                        msg.length     = 0U;
                        callbackStatus = kStatus_SerialManager_Canceled;
                    }
                    serialUsbCdc->rx.callback(serialUsbCdc->rx.callbackParam, &msg, callbackStatus);
                }
            }
        }
        break;
        case kUSB_DeviceCdcEventSerialStateNotif:
            ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 0;
            if ((serialUsbCdc->startTransactions) && (serialUsbCdc->tx.waiting4Prime))
            {
                serialUsbCdc->tx.waiting4Prime = 0;
                serialUsbCdc->tx.busy          = 1;
                if (kStatus_USB_Success != USB_DeviceCdcAcmSend(serialUsbCdc->cdcAcmHandle,
                                                                serialUsbCdc->bulkInEndpoint, serialUsbCdc->tx.buffer,
                                                                serialUsbCdc->tx.length))
                {
                    serialUsbCdc->tx.busy = 0;
                }
            }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSendEncapsulatedCommand:
            break;
        case kUSB_DeviceCdcEventGetEncapsulatedResponse:
            break;
        case kUSB_DeviceCdcEventSetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                if (1 == acmReqParam->isSetup)
                {
                    *(acmReqParam->buffer) = serialUsbCdc->abstractState;
                }
                else
                {
                    *(acmReqParam->length) = 0;
                }
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                if (1 == acmReqParam->isSetup)
                {
                    *(acmReqParam->buffer) = serialUsbCdc->countryCode;
                }
                else
                {
                    *(acmReqParam->length) = 0;
                }
            }
            else
            {
            }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventGetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = serialUsbCdc->abstractState;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = serialUsbCdc->countryCode;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
            }
            else
            {
            }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventClearCommFeature:
            break;
        case kUSB_DeviceCdcEventGetLineCoding:
            *(acmReqParam->buffer) = serialUsbCdc->lineCoding;
            *(acmReqParam->length) = LINE_CODING_SIZE;
            error                  = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSetLineCoding:
        {
            if (1 == acmReqParam->isSetup)
            {
                *(acmReqParam->buffer) = serialUsbCdc->lineCoding;
            }
            else
            {
                *(acmReqParam->length) = 0;
            }
        }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSetControlLineState:
        {
            serialUsbCdc->usbCdcAcmInfo->dteStatus = acmReqParam->setupValue;
            /* activate/deactivate Tx carrier */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
            {
                acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
            }
            else
            {
                acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
            }

            /* activate carrier and DTE. Com port of terminal tool running on PC is open now */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE)
            {
                acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
            }
            /* Com port of terminal tool running on PC is closed now */
            else
            {
                acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
            }

            /* Indicates to DCE if DTE is present or not */
            acmInfo->dtePresent = (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE) ? true : false;

            /* Initialize the serial state buffer */
            acmInfo->serialStateBuf[0] = NOTIF_REQUEST_TYPE;                /* bmRequestType */
            acmInfo->serialStateBuf[1] = USB_DEVICE_CDC_NOTIF_SERIAL_STATE; /* bNotification */
            acmInfo->serialStateBuf[2] = 0x00;                              /* wValue */
            acmInfo->serialStateBuf[3] = 0x00;
            acmInfo->serialStateBuf[4] = 0x00; /* wIndex */
            acmInfo->serialStateBuf[5] = 0x00;
            acmInfo->serialStateBuf[6] = UART_BITMAP_SIZE; /* wLength */
            acmInfo->serialStateBuf[7] = 0x00;
            /* Notify to host the line state */
            acmInfo->serialStateBuf[4] = acmReqParam->interfaceIndex;
            /* Lower byte of UART BITMAP */
            uartBitmap    = (uint8_t *)&acmInfo->serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE - 2];
            uartBitmap[0] = acmInfo->uartState & 0xFFu;
            uartBitmap[1] = (acmInfo->uartState >> 8) & 0xFFu;
            len           = (uint32_t)(NOTIF_PACKET_SIZE + UART_BITMAP_SIZE);
            if (0 == ((usb_device_cdc_acm_struct_t *)handle)->hasSentState)
            {
                error = USB_DeviceCdcAcmSend(handle, serialUsbCdc->interruptEndpoint, acmInfo->serialStateBuf, len);
                if (kStatus_USB_Success != error)
                {
                    usb_echo("kUSB_DeviceCdcEventSetControlLineState error!");
                }
                ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 1;
            }

            /* Update status */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
            {
                /*  To do: CARRIER_ACTIVATED */
            }
            else
            {
                /* To do: CARRIER_DEACTIVATED */
            }
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE)
            {
                /* DTE_ACTIVATED */
                if (1 == serialUsbCdc->attach)
                {
                    serialUsbCdc->startTransactions = 1;
                }
            }
            else
            {
                /* DTE_DEACTIVATED */
                if (1 == serialUsbCdc->attach)
                {
                    serialUsbCdc->startTransactions = 0;
                }
            }
        }
        break;
        case kUSB_DeviceCdcEventSendBreak:
            break;
        default:
            break;
    }

    return error;
}

usb_status_t USB_DeviceCdcVcomSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        /*endpoint information for cdc 1*/
        s_deviceComposite->cdcVcom[0].attach                         = 1;
        s_deviceComposite->cdcVcom[0].currRecvBuf                    = s_currRecvBuf[0];
        s_deviceComposite->cdcVcom[0].interruptEndpoint              = USB_CDC_VCOM_CIC_INTERRUPT_IN_ENDPOINT;
        s_deviceComposite->cdcVcom[0].interruptEndpointMaxPacketSize = g_cdcVcomCicEndpoints[0].maxPacketSize;

        s_deviceComposite->cdcVcom[0].bulkInEndpoint              = USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT;
        s_deviceComposite->cdcVcom[0].bulkInEndpointMaxPacketSize = g_cdcVcomDicEndpoints[0].maxPacketSize;

        s_deviceComposite->cdcVcom[0].bulkOutEndpoint              = USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT;
        s_deviceComposite->cdcVcom[0].bulkOutEndpointMaxPacketSize = g_cdcVcomDicEndpoints[1].maxPacketSize;

        /* Schedule buffer for receive */
        USB_DeviceCdcAcmRecv(s_deviceComposite->cdcVcom[0].cdcAcmHandle, s_deviceComposite->cdcVcom[0].bulkOutEndpoint,
                             s_deviceComposite->cdcVcom[0].currRecvBuf,
                             s_deviceComposite->cdcVcom[0].bulkOutEndpointMaxPacketSize);
    }
    return kStatus_USB_Success;
}

/*!
 * @brief Virtual COM device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param deviceComposite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcVcomInit(usb_device_composite_struct_t *deviceComposite)
{
    status_t status = 0;
    serial_manager_config_t serialConfig;
    s_deviceComposite = deviceComposite;
#if (defined(SERIAL_PORT_TYPE_USBCDC) && (SERIAL_PORT_TYPE_USBCDC > 0U))
    serial_port_usb_cdc_config_t usbCdcConfig;
#endif
    for (uint8_t i = 0; i < USB_DEVICE_CONFIG_CDC_ACM; i++)
    {
        s_deviceComposite->cdcVcom[i].lineCoding    = (uint8_t *)&s_lineCoding[i];
        s_deviceComposite->cdcVcom[i].abstractState = (uint8_t *)&s_abstractState[i];
        s_deviceComposite->cdcVcom[i].countryCode   = (uint8_t *)&s_countryCode[i];
        s_deviceComposite->cdcVcom[i].usbCdcAcmInfo = &s_usbCdcAcmInfo[i];
        s_deviceComposite->cdcVcom[i].speed         = USB_SPEED_FULL;
        s_deviceComposite->cdcVcom[i].instance      = CONTROLLER_ID;
#if (defined(SERIAL_PORT_TYPE_USBCDC) && (SERIAL_PORT_TYPE_USBCDC > 0U))
        /* Init Serial Manager for USB CDC */
        serialConfig.type = kSerialPort_UsbCdc;
#if (defined(SERIAL_MANAGER_NON_BLOCKING_MODE) && (SERIAL_MANAGER_NON_BLOCKING_MODE > 0U))
        serialConfig.ringBuffer     = &usb_readRingBuffer[i][0];
        serialConfig.ringBufferSize = SLN_SERIAL_MANAGER_RECEIVE_BUFFER_LEN;
#endif
        serialConfig.portConfig = &usbCdcConfig;

        status = SerialManager_Init(usb_serialHandle[i], &serialConfig);
        if (status != kStatus_SerialManager_Success)
        {
            return (int32_t)status;
        }
#endif
    }
    return kStatus_USB_Success;
}

#if (defined(SERIAL_PORT_TYPE_USBCDC) && (SERIAL_PORT_TYPE_USBCDC > 0U))
serial_manager_status_t Serial_UsbCdcInit(serial_handle_t serialHandle, void *serialConfig)
{
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
    *(uint32_t *)serialHandle = (uint32_t) & (s_deviceComposite->cdcVcom[0]);
    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_UsbCdcDeinit(serial_handle_t serialHandle)
{
    serial_usb_cdc_state_t *serialUsbCdc;

    assert(serialHandle);

    serialUsbCdc = (serial_usb_cdc_state_t *)(*(uint32_t *)serialHandle);

    USB_DeviceClassDeinit(serialUsbCdc->instance);

    //    USB_DeviceRemoveItem(&s_UsbCdcHead, serialUsbCdc);

    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_UsbCdcWrite(serial_handle_t serialHandle, uint8_t *buffer, uint32_t length)
{
    serial_usb_cdc_state_t *serialUsbCdc;
    uint32_t needToPrime = 0;

    assert(serialHandle);

    serialUsbCdc = (serial_usb_cdc_state_t *)(*(uint32_t *)serialHandle);

    if (serialUsbCdc->tx.busy)
    {
        return kStatus_SerialManager_Busy;
    }

    /* Prevents SerialManager_Write from blocking when no USB Host is attached */
    if ((serialUsbCdc->attach == 0))
    {
        return kStatus_SerialManager_NotConnected;
    }

    /* Prevents SerialManager_Write from blocking when USB Host is attached but CDC terminal is closed */
    if ((serialUsbCdc->attach == 1) && (serialUsbCdc->startTransactions == 0))
    {
        return kStatus_SerialManager_NotConnected;
    }

    serialUsbCdc->tx.busy          = 1;
    serialUsbCdc->tx.waiting4Prime = 0;

    serialUsbCdc->tx.buffer = buffer;
    serialUsbCdc->tx.length = length;

    if ((serialUsbCdc->attach))
    {
        needToPrime = 1;
    }

    if (needToPrime)
    {
        if (kStatus_USB_Success !=
            USB_DeviceCdcAcmSend(serialUsbCdc->cdcAcmHandle, serialUsbCdc->bulkInEndpoint, buffer, length))
        {
            serialUsbCdc->tx.busy = 0;
            return kStatus_SerialManager_Error;
        }
    }
    else
    {
        serialUsbCdc->tx.waiting4Prime = 1;
    }

    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_UsbCdcRead(serial_handle_t serialHandle, uint8_t *buffer, uint32_t length)
{
    serial_usb_cdc_state_t *serialUsbCdc;
    uint8_t *primeBuffer;
    uint32_t primeLength;
    uint32_t regPrimask;

    assert(serialHandle);

    serialUsbCdc = (serial_usb_cdc_state_t *)(*(uint32_t *)serialHandle);

    if (!(serialUsbCdc->attach))
    {
        return kStatus_SerialManager_Error;
    }

    regPrimask = DisableGlobalIRQ();
    if (serialUsbCdc->rx.busy)
    {
        EnableGlobalIRQ(regPrimask);
        return kStatus_SerialManager_Busy;
    }
    serialUsbCdc->rx.busy = 1U;
    EnableGlobalIRQ(regPrimask);

    if (length < serialUsbCdc->bulkOutEndpointMaxPacketSize)
    {
        serialUsbCdc->rx.busy = 0U;
        return kStatus_SerialManager_Error;
    }

    if (NULL == buffer)
    {
        primeBuffer = serialUsbCdc->currRecvBuf;
        primeLength = serialUsbCdc->bulkOutEndpointMaxPacketSize;
    }
    else
    {
        primeBuffer = buffer;
        primeLength = length;
    }
    /* Schedule buffer for next receive event */

    if (kStatus_USB_Success !=
        USB_DeviceCdcAcmRecv(serialUsbCdc->cdcAcmHandle, serialUsbCdc->bulkOutEndpoint, primeBuffer, primeLength))
    {
        serialUsbCdc->rx.busy = 0U;
        return kStatus_SerialManager_Error;
    }

    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_UsbCdcCancelWrite(serial_handle_t serialHandle)
{
    serial_usb_cdc_state_t *serialUsbCdc;

    assert(serialHandle);

    serialUsbCdc = (serial_usb_cdc_state_t *)(*(uint32_t *)serialHandle);
    USB_DeviceCancel(serialUsbCdc->deviceHandle,
                     (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT) | serialUsbCdc->bulkInEndpoint);
    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_UsbCdcInstallTxCallback(serial_handle_t serialHandle,
                                                       serial_manager_callback_t callback,
                                                       void *callbackParam)
{
    serial_usb_cdc_state_t *serialUsbCdc;

    assert(serialHandle);

    serialUsbCdc = (serial_usb_cdc_state_t *)(*(uint32_t *)serialHandle);

    serialUsbCdc->tx.callback      = callback;
    serialUsbCdc->tx.callbackParam = callbackParam;

    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_UsbCdcInstallRxCallback(serial_handle_t serialHandle,
                                                       serial_manager_callback_t callback,
                                                       void *callbackParam)
{
    serial_usb_cdc_state_t *serialUsbCdc;

    assert(serialHandle);

    serialUsbCdc = (serial_usb_cdc_state_t *)(*(uint32_t *)serialHandle);

    serialUsbCdc->rx.callback      = callback;
    serialUsbCdc->rx.callbackParam = callbackParam;

    return kStatus_SerialManager_Success;
}

void Serial_UsbCdcIsrFunction(serial_handle_t serialHandle)
{
    serial_usb_cdc_state_t *serialUsbCdc;

    assert(serialHandle);

    serialUsbCdc = (serial_usb_cdc_state_t *)(*(uint32_t *)serialHandle);
#if 0
    DisableIRQ((IRQn_Type)serialUsbCdc->irqNumber);
#endif
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
    if ((kSerialManager_UsbControllerEhci0 == serialUsbCdc->instance) ||
        (kSerialManager_UsbControllerEhci1 == serialUsbCdc->instance))
    {
        USB_DeviceEhciIsrFunction(serialUsbCdc->deviceHandle);
    }
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
    if ((kSerialManager_UsbControllerKhci0 == serialUsbCdc->instance) ||
        (kSerialManager_UsbControllerKhci1 == serialUsbCdc->instance))
    {
        USB_DeviceKhciIsrFunction(serialUsbCdc->deviceHandle);
    }
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    if ((kSerialManager_UsbControllerLpcIp3511Fs0 == serialUsbCdc->instance) ||
        (kSerialManager_UsbControllerLpcIp3511Fs1 == serialUsbCdc->instance))
    {
        USB_DeviceLpcIp3511IsrFunction(serialUsbCdc->deviceHandle);
    }
#endif

#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    if ((kSerialManager_UsbControllerLpcIp3511Hs0 == serialUsbCdc->instance) ||
        (kSerialManager_UsbControllerLpcIp3511Hs1 == serialUsbCdc->instance))
    {
        USB_DeviceLpcIp3511IsrFunction(serialUsbCdc->deviceHandle);
    }
#endif
#if 0
    EnableIRQ((IRQn_Type)serialUsbCdc->irqNumber);
#endif
}
#endif
