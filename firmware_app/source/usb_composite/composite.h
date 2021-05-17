/*
 * Copyright 2017-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USB_DEVICE_COMPOSITE_H_
#define _USB_DEVICE_COMPOSITE_H_

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "virtual_com.h"
#include "video_camera.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif

typedef struct _usb_device_composite_struct
{
    usb_device_handle deviceHandle;                            /* USB device handle. */
    serial_usb_cdc_state_t cdcVcom[USB_DEVICE_CONFIG_CDC_ACM]; /* CDC virtual com device structure. */
    usb_video_camera_struct_t g_UsbDeviceVideoCamera;

    TaskHandle_t applicationTaskHandle; /* Application task handle. */
    TaskHandle_t deviceTaskHandle;      /* USB device task handle. */
    uint8_t speed;  /* Speed of USB device. USB_SPEED_FULL/USB_SPEED_LOW/USB_SPEED_HIGH.                 */
    uint8_t attach; /* A flag to indicate whether a usb device is attached. 1: attached, 0: not attached */
    uint8_t currentConfiguration;                                      /* Current configuration value. */
    uint8_t currentInterfaceAlternateSetting[USB_INTERFACE_COUNT_USB]; /* Current alternate setting value for each
                                                                          interface. */
    uint8_t instance;                                                  /* The instance of the interface */
    uint8_t irqNumber;                                                 /* The IRQ number of the interface */
} usb_device_composite_struct_t;

extern usb_device_composite_struct_t g_composite;
/*******************************************************************************
 * API
 ******************************************************************************/

void USB_DeviceApplicationInit(void);

void USB_DeviceApplicationDeInit(void);

#endif /* _USB_DEVICE_COMPOSITE_H_ */
