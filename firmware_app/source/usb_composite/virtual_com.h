/*
 * Copyright 2017-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _USB_CDC_VCOM_H_
#define _USB_CDC_VCOM_H_

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "serial_manager.h"
#include "sln_shell.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
#define CONTROLLER_ID  kUSB_ControllerEhci0
#define DATA_BUFF_SIZE HS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
#define CONTROLLER_ID  kUSB_ControllerKhci0
#define DATA_BUFF_SIZE FS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define CONTROLLER_ID  kUSB_ControllerLpcIp3511Fs0
#define DATA_BUFF_SIZE FS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#endif

#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define CONTROLLER_ID  kUSB_ControllerLpcIp3511Hs0
#define DATA_BUFF_SIZE HS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#endif

/* Currently configured line coding */
#define LINE_CODING_SIZE       (0x07)
#define LINE_CODING_DTERATE    (115200)
#define LINE_CODING_CHARFORMAT (0x00)
#define LINE_CODING_PARITYTYPE (0x00)
#define LINE_CODING_DATABITS   (0x08)

/* Communications feature */
#define COMM_FEATURE_DATA_SIZE (0x02)
#define STATUS_ABSTRACT_STATE  (0x0000)
#define COUNTRY_SETTING        (0x0000)

/* Notification of serial state */
#define NOTIF_PACKET_SIZE  (0x08)
#define UART_BITMAP_SIZE   (0x02)
#define NOTIF_REQUEST_TYPE (0xA1)

typedef struct _serial_usb_send_state
{
    serial_manager_callback_t callback;
    void *callbackParam;
    uint8_t *buffer;
    uint32_t length;
    volatile uint8_t busy;
    volatile uint8_t waiting4Prime;
} serial_usb_send_state_t;

typedef struct _serial_usart_recv_state
{
    serial_manager_callback_t callback;
    void *callbackParam;
    volatile uint8_t busy;
} serial_usb_recv_state_t;

/* Define the information relates to abstract control model */
typedef struct _usb_cdc_acm_info
{
    uint8_t serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE]; /* Serial state buffer of the CDC device to notify the
                                                                     serial state to host. */
    bool dtePresent;          /* A flag to indicate whether DTE is present.         */
    uint16_t breakDuration;   /* Length of time in milliseconds of the break signal */
    uint8_t dteStatus;        /* Status of data terminal equipment                  */
    uint8_t currentInterface; /* Current interface index.                           */
    uint16_t uartState;       /* UART state of the CDC device.                      */
} usb_cdc_acm_info_t;

/* Define the types for application */
typedef struct _serial_usb_cdc_state
{
    usb_device_handle deviceHandle; /* USB device handle. */
    class_handle_t cdcAcmHandle; /* USB CDC ACM class handle.                                                         */
    uint8_t *lineCoding;         /* Line coding of cdc device */
    uint8_t *abstractState;      /* Abstract state of cdc device */
    uint8_t *countryCode;        /* Country code of cdc device */
    usb_cdc_acm_info_t *usbCdcAcmInfo; /* CDC ACM information */
    uint8_t *currRecvBuf;
    serial_usb_send_state_t tx;
    serial_usb_recv_state_t rx;
    uint16_t bulkOutEndpointMaxPacketSize;   /*bulk out endpoint maxpacket size */
    uint16_t bulkInEndpointMaxPacketSize;    /*bulk in endpoint maxpacket size */
    uint16_t interruptEndpointMaxPacketSize; /*interrupt  endpoint maxpacket size */
    uint8_t attach;            /* A flag to indicate whether a usb device is attached. 1: attached, 0: not attached */
    uint8_t speed;             /* Speed of USB device. USB_SPEED_FULL/USB_SPEED_LOW/USB_SPEED_HIGH.                 */
    uint8_t startTransactions; /* A flag to indicate whether a CDC device is ready to transmit and receive data.    */
    uint8_t currentConfiguration;                                           /* Current configuration value. */
    uint8_t currentInterfaceAlternateSetting[USB_CDC_VCOM_INTERFACE_COUNT]; /* Current alternate setting value for each
                                                                               interface. */
    uint8_t bulkInEndpoint;                                                 /*bulk in endpoint number*/
    uint8_t bulkOutEndpoint;                                                /*bulk out endpoint number*/
    uint8_t interruptEndpoint;                                              /*interrupt endpoint number*/
    uint8_t instance;

} serial_usb_cdc_state_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief CDC class specific callback function.
 *
 * This function handles the CDC class specific requests.
 *
 * @param handle          The CDC ACM class handle.
 * @param event           The CDC ACM class event type.
 * @param param           The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */

usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, uint32_t event, void *param);

/*!
 * @brief Virtual COM device set configuration function.
 *
 * This function sets configuration for CDC class.
 *
 * @param handle The CDC ACM class handle.
 * @param configure The CDC ACM class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcVcomSetConfigure(class_handle_t handle, uint8_t configure);

#endif /* _USB_CDC_VCOM_H_ */
