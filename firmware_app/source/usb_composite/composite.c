/*
 * Copyright 2017-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include <stdio.h>
#include <stdlib.h>
#include "composite.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
#include "usb_phy.h"
#endif
#include "pin_mux.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NonCachedNonInit __attribute__((section("NonCacheable, \"aw\", %nobits @")))
#define NonCached        __attribute__((section("NonCacheable.init")))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
extern void USB_DeviceClockInit(void);
extern usb_status_t USB_DeviceCdcVcomInit(usb_device_composite_struct_t *deviceComposite);
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Composite device structure. */

NonCached usb_device_composite_struct_t g_composite;
extern usb_device_class_struct_t g_UsbDeviceCdcVcomConfig[USB_DEVICE_CONFIG_CDC_ACM];
extern usb_device_class_struct_t g_UsbDeviceVideoCameraConfig[USB_DEVICE_CONFIG_VIDEO];

/* Application task handle. */

/* USB device class information */
usb_device_class_config_struct_t g_CompositeClassConfig[USB_DEVICE_CONFIG_TOTAL] = {
    {
        USB_DeviceCdcVcomCallback,
        (class_handle_t)NULL,
        &g_UsbDeviceCdcVcomConfig[0],
    },
    {
        USB_DeviceVideoCallback,
        (class_handle_t)NULL,
        &g_UsbDeviceVideoCameraConfig[0],
    }};

/* USB device class configuration information */
usb_device_class_config_list_struct_t g_UsbDeviceCompositeConfigList = {
    g_CompositeClassConfig,
    USB_DeviceCallback,
    USB_DEVICE_CONFIG_TOTAL,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

void USB_OTG1_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_composite.deviceHandle);
}

void USB_OTG2_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_composite.deviceHandle);
}

void USB_DeviceIsrEnable()
{
    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    g_composite.irqNumber      = usbDeviceEhciIrq[g_composite.instance - kSerialManager_UsbControllerEhci0];

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)g_composite.irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)g_composite.irqNumber);
}

void USB_DeviceIsrDisable()
{
    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    g_composite.irqNumber      = usbDeviceEhciIrq[g_composite.instance - kSerialManager_UsbControllerEhci0];

    /* Install isr, set priority, and enable IRQ. */
    DisableIRQ((IRQn_Type)g_composite.irqNumber);
}

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle          The USB device handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;
    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            g_composite.attach               = 0;
            g_composite.currentConfiguration = 0U;
            error                            = kStatus_USB_Success;
            for (uint8_t i = 0; i < USB_DEVICE_CONFIG_CDC_ACM; i++)
            {
                g_composite.cdcVcom[i].tx.length = 0;
                g_composite.cdcVcom[i].attach    = 0;
            }
            if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
            {
                USB_DeviceVideoApplicationSetDefault();
            }

#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(g_composite.instance, &g_composite.speed))
            {
                USB_DeviceSetSpeed(handle, g_composite.speed);
            }

            if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
            {
                if (USB_SPEED_HIGH == g_composite.speed)
                {
                    g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize =
                        HS_STREAM_IN_PACKET_SIZE * (1 + HS_STREAM_IN_PACKET_ADDITIONAL_TRANSACTION);
                }
                else
                {
                    g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize = FS_STREAM_IN_PACKET_SIZE;
                }
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize,
                    g_composite.g_UsbDeviceVideoCamera.probeStruct->dwMaxPayloadTransferSize);
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize,
                    g_composite.g_UsbDeviceVideoCamera.commitStruct->dwMaxPayloadTransferSize);
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize,
                    g_composite.g_UsbDeviceVideoCamera.stillProbeStruct->dwMaxPayloadTransferSize);
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize,
                    g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->dwMaxPayloadTransferSize);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_composite.attach               = 0U;
                g_composite.currentConfiguration = 0U;
                for (uint8_t i = 0; i < USB_DEVICE_CONFIG_CDC_ACM; i++)
                {
                    g_composite.cdcVcom[i].tx.length = 0;
                    g_composite.cdcVcom[i].attach    = 0;
                }
                if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
                    USB_DeviceVideoApplicationSetDefault();
            }
            else if (USB_COMPOSITE_CONFIGURE_INDEX == (*temp8))
            {
                g_composite.attach = 1;

                USB_DeviceCdcVcomSetConfigure(g_composite.cdcVcom[0].cdcAcmHandle, *temp8);
                g_composite.currentConfiguration = *temp8;
                error                            = kStatus_USB_Success;
                if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
                    g_composite.g_UsbDeviceVideoCamera.attach = 1;
            }
            else
            {
                error = kStatus_USB_InvalidRequest;
            }
            break;

        case kUSB_DeviceEventSetInterface:
            if (g_composite.attach)
            {
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                if (Cfg_AppDataGetOutputMode() == DISPLAY_USB &&
                    g_composite.currentInterfaceAlternateSetting[interface] != alternateSetting)
                {
                    if (!g_composite.currentInterfaceAlternateSetting[interface])
                    {
                        if (USB_VIDEO_VIRTUAL_CAMERA_STREAM_INTERFACE_INDEX == interface)
                        {
                            USB_DeviceVideoPrepareVideoData();
                            error = USB_DeviceSendRequest(g_composite.deviceHandle, USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_IN,
                                                          g_composite.g_UsbDeviceVideoCamera.imageBuffer,
                                                          g_composite.g_UsbDeviceVideoCamera.imageBufferLength);
                        }
                    }

                    g_composite.currentInterfaceAlternateSetting[interface] = alternateSetting;
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                *temp8 = g_composite.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_composite.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
                else
                {
                    error = kStatus_USB_InvalidRequest;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
        case kUSB_DeviceEventGetDeviceQualifierDescriptor:
            if (param)
            {
                /* Get device descriptor request */
                error = USB_DeviceGetDeviceQualifierDescriptor(
                    handle, (usb_device_get_device_qualifier_descriptor_struct_t *)param);
            }
            break;
#endif
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            break;
    }

    return error;
}

void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
    USB_DeviceSetConfigurationDescriptor((Cfg_AppDataGetOutputMode() == DISPLAY_USB) ? UVC_CDC_CONFIGURE_DESCRIPTOR :
                                                                                       CDC_CONFIGURE_DESCRIPTOR);
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    g_composite.instance                           = CONTROLLER_ID;
    g_composite.speed                              = USB_SPEED_FULL;
    g_composite.attach                             = 0;
    g_composite.cdcVcom[0].cdcAcmHandle            = (class_handle_t)NULL;
    g_composite.g_UsbDeviceVideoCamera.videoHandle = (class_handle_t)NULL;

    g_composite.deviceHandle = NULL;

    if (kStatus_USB_Success !=
        USB_DeviceClassInit(CONTROLLER_ID, &g_UsbDeviceCompositeConfigList, &g_composite.deviceHandle))
    {
        usb_echo("USB device composite demo init failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device composite demo\r\n");
        /*Init classhandle in cdc instance*/
        g_composite.cdcVcom[0].cdcAcmHandle = g_UsbDeviceCompositeConfigList.config[0].classHandle;
        g_composite.cdcVcom[0].deviceHandle = g_composite.deviceHandle;
        USB_DeviceCdcVcomInit(&g_composite);

        if (Cfg_AppDataGetOutputMode() == DISPLAY_USB)
        {
            g_composite.g_UsbDeviceVideoCamera.videoHandle  = g_UsbDeviceCompositeConfigList.config[1].classHandle;
            g_composite.g_UsbDeviceVideoCamera.deviceHandle = g_composite.deviceHandle;
        }
    }

    USB_DeviceIsrEnable();
    USB_DeviceRun(g_composite.deviceHandle);
}

void USB_DeviceApplicationDeInit(void)
{
    USB_DeviceStop(g_composite.deviceHandle);
    USB_DeviceIsrDisable();
    if (kStatus_USB_Success != USB_DeviceClassDeinit(CONTROLLER_ID))
    {
        usb_echo("USB device composite demo deinit failed\r\n");
        return;
    }
}
