/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USB_DEVICE_DESCRIPTOR_H_
#define _USB_DEVICE_DESCRIPTOR_H_ 1

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define USB_DEVICE_SPECIFIC_BCD_VERSION (0x0200)
#define USB_DEVICE_DEMO_BCD_VERSION     (0x0101U)
#include "commondef.h"
/* Communication Class SubClass Codes */
#define USB_CDC_DIRECT_LINE_CONTROL_MODEL         (0x01)
#define USB_CDC_ABSTRACT_CONTROL_MODEL            (0x02)
#define USB_CDC_TELEPHONE_CONTROL_MODEL           (0x03)
#define USB_CDC_MULTI_CHANNEL_CONTROL_MODEL       (0x04)
#define USB_CDC_CAPI_CONTROL_MOPDEL               (0x05)
#define USB_CDC_ETHERNET_NETWORKING_CONTROL_MODEL (0x06)
#define USB_CDC_ATM_NETWORKING_CONTROL_MODEL      (0x07)
#define USB_CDC_WIRELESS_HANDSET_CONTROL_MODEL    (0x08)
#define USB_CDC_DEVICE_MANAGEMENT                 (0x09)
#define USB_CDC_MOBILE_DIRECT_LINE_MODEL          (0x0A)
#define USB_CDC_OBEX                              (0x0B)
#define USB_CDC_ETHERNET_EMULATION_MODEL          (0x0C)

/* Communication Class Protocol Codes */
#define USB_CDC_NO_CLASS_SPECIFIC_PROTOCOL  (0x00) /*also for Data Class Protocol Code */
#define USB_CDC_AT_250_PROTOCOL             (0x01)
#define USB_CDC_AT_PCCA_101_PROTOCOL        (0x02)
#define USB_CDC_AT_PCCA_101_ANNEX_O         (0x03)
#define USB_CDC_AT_GSM_7_07                 (0x04)
#define USB_CDC_AT_3GPP_27_007              (0x05)
#define USB_CDC_AT_TIA_CDMA                 (0x06)
#define USB_CDC_ETHERNET_EMULATION_PROTOCOL (0x07)
#define USB_CDC_EXTERNAL_PROTOCOL           (0xFE)
#define USB_CDC_VENDOR_SPECIFIC             (0xFF) /*also for Data Class Protocol Code */

/* Data Class Protocol Codes */
#define USB_CDC_PYHSICAL_INTERFACE_PROTOCOL (0x30)
#define USB_CDC_HDLC_PROTOCOL               (0x31)
#define USB_CDC_TRANSPARENT_PROTOCOL        (0x32)
#define USB_CDC_MANAGEMENT_PROTOCOL         (0x50)
#define USB_CDC_DATA_LINK_Q931_PROTOCOL     (0x51)
#define USB_CDC_DATA_LINK_Q921_PROTOCOL     (0x52)
#define USB_CDC_DATA_COMPRESSION_V42BIS     (0x90)
#define USB_CDC_EURO_ISDN_PROTOCOL          (0x91)
#define USB_CDC_RATE_ADAPTION_ISDN_V24      (0x92)
#define USB_CDC_CAPI_COMMANDS               (0x93)
#define USB_CDC_HOST_BASED_DRIVER           (0xFD)
#define USB_CDC_UNIT_FUNCTIONAL             (0xFE)

/* Descriptor SubType in Communications Class Functional Descriptors */
#define USB_CDC_HEADER_FUNC_DESC              (0x00)
#define USB_CDC_CALL_MANAGEMENT_FUNC_DESC     (0x01)
#define USB_CDC_ABSTRACT_CONTROL_FUNC_DESC    (0x02)
#define USB_CDC_DIRECT_LINE_FUNC_DESC         (0x03)
#define USB_CDC_TELEPHONE_RINGER_FUNC_DESC    (0x04)
#define USB_CDC_TELEPHONE_REPORT_FUNC_DESC    (0x05)
#define USB_CDC_UNION_FUNC_DESC               (0x06)
#define USB_CDC_COUNTRY_SELECT_FUNC_DESC      (0x07)
#define USB_CDC_TELEPHONE_MODES_FUNC_DESC     (0x08)
#define USB_CDC_TERMINAL_FUNC_DESC            (0x09)
#define USB_CDC_NETWORK_CHANNEL_FUNC_DESC     (0x0A)
#define USB_CDC_PROTOCOL_UNIT_FUNC_DESC       (0x0B)
#define USB_CDC_EXTENSION_UNIT_FUNC_DESC      (0x0C)
#define USB_CDC_MULTI_CHANNEL_FUNC_DESC       (0x0D)
#define USB_CDC_CAPI_CONTROL_FUNC_DESC        (0x0E)
#define USB_CDC_ETHERNET_NETWORKING_FUNC_DESC (0x0F)
#define USB_CDC_ATM_NETWORKING_FUNC_DESC      (0x10)
#define USB_CDC_WIRELESS_CONTROL_FUNC_DESC    (0x11)
#define USB_CDC_MOBILE_DIRECT_LINE_FUNC_DESC  (0x12)
#define USB_CDC_MDLM_DETAIL_FUNC_DESC         (0x13)
#define USB_CDC_DEVICE_MANAGEMENT_FUNC_DESC   (0x14)
#define USB_CDC_OBEX_FUNC_DESC                (0x15)
#define USB_CDC_COMMAND_SET_FUNC_DESC         (0x16)
#define USB_CDC_COMMAND_SET_DETAIL_FUNC_DESC  (0x17)
#define USB_CDC_TELEPHONE_CONTROL_FUNC_DESC   (0x18)
#define USB_CDC_OBEX_SERVICE_ID_FUNC_DESC     (0x19)

/* usb descriptor length */
#define USB_CDC_VCOM_REPORT_DESCRIPTOR_LENGTH (33)
#define USB_IAD_DESC_SIZE                     (8)
#define USB_DESCRIPTOR_LENGTH_CDC_HEADER_FUNC (5)
#define USB_DESCRIPTOR_LENGTH_CDC_CALL_MANAG  (5)
#define USB_DESCRIPTOR_LENGTH_CDC_ABSTRACT    (4)
#define USB_DESCRIPTOR_LENGTH_CDC_UNION_FUNC  (5)

#define USB_DEVICE_CONFIGURATION_COUNT (1)
#define USB_DEVICE_STRING_COUNT        (7)
#define USB_DEVICE_LANGUAGE_COUNT      (1)

#define USB_INTERFACE_COUNT_USB     (USB_CDC_VCOM_INTERFACE_COUNT + USB_VIDEO_VIRTUAL_CAMERA_INTERFACE_COUNT)
#define USB_INTERFACE_COUNT_DISPLAY USB_CDC_VCOM_INTERFACE_COUNT
#define USB_INTERFACE_COUNT         (Cfg_AppDataGetOutputMode() ? USB_INTERFACE_COUNT_DISPLAY : USB_INTERFACE_COUNT_USB)

#define USB_COMPOSITE_CONFIGURE_INDEX (1)

#define USB_MSC_DISK_CLASS (0x08)
/* scsi command set */
#define USB_MSC_DISK_SUBCLASS (0x06)
/* bulk only transport protocol */
#define USB_MSC_DISK_PROTOCOL (0x50)

/* Configuration, interface and endpoint. */
#define USB_CDC_VCOM_CIC_CLASS    (0x02)
#define USB_CDC_VCOM_CIC_SUBCLASS (0x02)
#define USB_CDC_VCOM_CIC_PROTOCOL (0x00)
#define USB_CDC_VCOM_DIC_CLASS    (0x0A)
#define USB_CDC_VCOM_DIC_SUBCLASS (0x00)
#define USB_CDC_VCOM_DIC_PROTOCOL (0x00)

#define USB_CDC_VCOM_INTERFACE_COUNT           (2)
#define USB_CDC_VCOM_CIC_INTERFACE_INDEX       (0)
#define USB_CDC_VCOM_DIC_INTERFACE_INDEX       (1)
#define USB_CDC_VCOM_CIC_ENDPOINT_COUNT        (1)
#define USB_CDC_VCOM_CIC_INTERRUPT_IN_ENDPOINT (1)
#define USB_CDC_VCOM_DIC_ENDPOINT_COUNT        (2)
#define USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT      (2)
#define USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT     (2)
/* Packet size. */
#define HS_CDC_VCOM_INTERRUPT_IN_PACKET_SIZE (16)
#define FS_CDC_VCOM_INTERRUPT_IN_PACKET_SIZE (16)
#define HS_CDC_VCOM_INTERRUPT_IN_INTERVAL    (0x07) /* 2^(7-1) = 8ms */
#define FS_CDC_VCOM_INTERRUPT_IN_INTERVAL    (0x08)

#define HS_CDC_VCOM_BULK_IN_PACKET_SIZE  (512)
#define FS_CDC_VCOM_BULK_IN_PACKET_SIZE  (64)
#define HS_CDC_VCOM_BULK_OUT_PACKET_SIZE (512)
#define FS_CDC_VCOM_BULK_OUT_PACKET_SIZE (64)

/* String descriptor length. */
#define USB_DESCRIPTOR_LENGTH_STRING0 (sizeof(g_UsbDeviceString0))
#define USB_DESCRIPTOR_LENGTH_STRING1 (sizeof(g_UsbDeviceString1))
#define USB_DESCRIPTOR_LENGTH_STRING2 (sizeof(g_UsbDeviceString2))
#define USB_DESCRIPTOR_LENGTH_STRING3 (sizeof(g_UsbDeviceString3))
#define USB_DESCRIPTOR_LENGTH_STRING4 (sizeof(g_UsbDeviceString4))
#define USB_DESCRIPTOR_LENGTH_STRING5 (sizeof(g_UsbDeviceString5))
#define USB_DESCRIPTOR_LENGTH_STRING6 (sizeof(g_UsbDeviceString6))

#define USB_DESCRIPTOR_TYPE_CDC_CS_INTERFACE (0x24)
#define USB_DESCRIPTOR_TYPE_CDC_CS_ENDPOINT  (0x25)

/* Class code. */
#define USB_DEVICE_CLASS    (0xEF)
#define USB_DEVICE_SUBCLASS (0x02)
#define USB_DEVICE_PROTOCOL (0x01)

#define USB_DEVICE_MAX_POWER (0x32)

#define USB_DEVICE_VIDEO_SPECIFIC_BCD_VERSION (0x0100U)
#define USB_DEVICE_VIDEO_CAMERA_PROTOCOL      USB_DEVICE_VIDEO_PC_PROTOCOL_UNDEFINED

#define USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOCIATION (0x08U)
//added by Raymond for MJPEG video
#define USB_VIDEO_VIRTUAL_CAMERA_CONFIGURE_INDEX (1U)

#define USB_VIDEO_VIRTUAL_CAMERA_CONTROL_INTERFACE_COUNT (1U)
#define USB_VIDEO_VIRTUAL_CAMERA_CONTROL_INTERFACE_INDEX (2U)
#define USB_VIDEO_VIRTUAL_CAMERA_CONTROL_ENDPOINT_COUNT (1U)
#define USB_VIDEO_VIRTUAL_CAMERA_CONTROL_ENDPOINT (4U)

#define HS_INTERRUPT_IN_PACKET_SIZE (8U)
#define FS_INTERRUPT_IN_PACKET_SIZE (8U)
#define HS_INTERRUPT_IN_INTERVAL (0x07U) /* 2^(7-1) = 8ms */
#define FS_INTERRUPT_IN_INTERVAL (0x08U)

#define USB_VIDEO_VIRTUAL_CAMERA_VC_INTERFACE_HEADER_LENGTH (0x0DU)
#define USB_VIDEO_VIRTUAL_CAMERA_VC_OUTPUT_TERMINAL_LENGTH (0x09U)

#if defined(USB_DEVICE_VIDEO_CLASS_VERSION_1_5) && (USB_DEVICE_VIDEO_CLASS_VERSION_1_5 > 0U)

#define USB_VIDEO_VIRTUAL_CAMERA_VC_PROCESSING_UNIT_LENGTH (0x0DU)
#define USB_VIDEO_VIRTUAL_CAMERA_VC_INPUT_TERMINAL_LENGTH (0x12U)

#elif defined(USB_DEVICE_VIDEO_CLASS_VERSION_1_1) && (USB_DEVICE_VIDEO_CLASS_VERSION_1_1 > 0U)

#define USB_VIDEO_VIRTUAL_CAMERA_VC_PROCESSING_UNIT_LENGTH (0x0CU)
#define USB_VIDEO_VIRTUAL_CAMERA_VC_INPUT_TERMINAL_LENGTH (0x12U)

#else

#define USB_VIDEO_VIRTUAL_CAMERA_VC_PROCESSING_UNIT_LENGTH (0x0BU)
#define USB_VIDEO_VIRTUAL_CAMERA_VC_INPUT_TERMINAL_LENGTH (0x12U)

#endif

#define USB_VIDEO_VIRTUAL_CAMERA_VC_INTERFACE_TOTAL_LENGTH                                                     \
    (USB_VIDEO_VIRTUAL_CAMERA_VC_INTERFACE_HEADER_LENGTH + USB_VIDEO_VIRTUAL_CAMERA_VC_INPUT_TERMINAL_LENGTH + \
     USB_VIDEO_VIRTUAL_CAMERA_VC_OUTPUT_TERMINAL_LENGTH + USB_VIDEO_VIRTUAL_CAMERA_VC_PROCESSING_UNIT_LENGTH)

#define USB_VIDEO_VIRTUAL_CAMERA_CLOCK_FREQUENCY (6000000U) /* 6MHz */

#define USB_VIDEO_VIRTUAL_CAMERA_STREAM_INTERFACE_COUNT (1U)
#define USB_VIDEO_VIRTUAL_CAMERA_STREAM_INTERFACE_INDEX (3U)
#define USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_COUNT (1U)
#define USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_IN (3U)
#define HS_STREAM_IN_PACKET_ADDITIONAL_TRANSACTION (0U) /* MAX Value is 2U*/
#define HS_STREAM_IN_PACKET_SIZE (1024U)
#define FS_STREAM_IN_PACKET_SIZE (512U)
#define HS_STREAM_IN_INTERVAL (0x01U) /* 2^(4-1) = 1ms */
#define FS_STREAM_IN_INTERVAL (0x01U)

#define USB_VIDEO_VIRTUAL_CAMERA_STREAM_STILL_CAPTURE_METHOD (0x02U)
#define USB_VIDEO_VIRTUAL_CAMERA_STREAM_STILL_CAPTURE_TRIGGER_SUPPOTED (0x00U)

#define USB_VIDEO_VIRTUAL_CAMERA_VS_INTERFACE_HEADER_LENGTH (0x0EU)
#define USB_VIDEO_MJPEG_FORMAT_DESCRIPTOR_LENGTH (0x0BU)
#define USB_VIDEO_MJPEG_FRAME_DESCRIPTOR_LENGTH (34U)
#define USB_VIDEO_MJPEG_FRAME_STILL_DESCRIPTOR_LENGTH (0x0AU)
#define USB_VIDEO_VIRTUAL_CAMERA_VS_INTERFACE_TOTAL_LENGTH                                            \
    (USB_VIDEO_VIRTUAL_CAMERA_VS_INTERFACE_HEADER_LENGTH + USB_VIDEO_MJPEG_FORMAT_DESCRIPTOR_LENGTH + \
     USB_VIDEO_MJPEG_FRAME_DESCRIPTOR_LENGTH + USB_VIDEO_MJPEG_FRAME_STILL_DESCRIPTOR_LENGTH)

#define USB_VIDEO_VIRTUAL_CAMERA_INTERFACE_COUNT \
    (USB_VIDEO_VIRTUAL_CAMERA_CONTROL_INTERFACE_COUNT + USB_VIDEO_VIRTUAL_CAMERA_STREAM_INTERFACE_COUNT)

//added by Raymond
#if defined(USB_DEVICE_VIDEO_CLASS_VERSION_1_5) && (USB_DEVICE_VIDEO_CLASS_VERSION_1_5 > 0U)

#define USB_DEVICE_VIDEO_SPECIFIC_BCD_VERSION (0x0150U)
#define USB_DEVICE_VIDEO_VIRTUAL_CAMERA_PROTOCOL USB_DEVICE_VIDEO_PC_PROTOCOL_15
#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL (sizeof(g_UsbDeviceConfigurationDescriptor))

#elif defined(USB_DEVICE_VIDEO_CLASS_VERSION_1_1) && (USB_DEVICE_VIDEO_CLASS_VERSION_1_1 > 0U)

#define USB_DEVICE_VIDEO_SPECIFIC_BCD_VERSION (0x0110U)
#define USB_DEVICE_VIDEO_VIRTUAL_CAMERA_PROTOCOL USB_DEVICE_VIDEO_PC_PROTOCOL_UNDEFINED
#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL (sizeof(g_UsbDeviceConfigurationDescriptor))

#else

#define USB_DEVICE_VIDEO_SPECIFIC_BCD_VERSION (0x0100U)
#define USB_DEVICE_VIDEO_VIRTUAL_CAMERA_PROTOCOL USB_DEVICE_VIDEO_PC_PROTOCOL_UNDEFINED
#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL (sizeof(g_UsbDeviceConfigurationDescriptor))

#endif

/* Stream format */
#define USB_VIDEO_VIRTUAL_CAMERA_FORMAT_COUNT (1U)
#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FORMAT_INDEX (1U)

#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_COUNT (1U)
#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INDEX (1U)

#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_WIDTH (240U)//(176U)
#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_HEIGHT (320U)//(144U)

#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_TYPE (2U)
//#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_30FPS (10000000U / 30U)
//#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_25FPS (10000000U / 25U)
//#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_20FPS (10000000U / 20U)
#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_15FPS (10000000U / 15U)
#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_10FPS (10000000U / 10U)
//#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_5FPS (10000000U / 5U)

#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_DEFAULT_INTERVAL USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_15FPS
#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MIN_INTERVAL USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_15FPS
#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_INTERVAL USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INTERVAL_10FPS

#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MIN_BIT_RATE                                             \
    (16U * USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_WIDTH * USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_HEIGHT * \
     (10000000U / USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_INTERVAL))
#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_BIT_RATE                                             \
    (16U * USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_WIDTH * USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_HEIGHT * \
     (10000000U / USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MIN_INTERVAL))

#define USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_FRAME_SIZE \
    (2U * USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_WIDTH * USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_HEIGHT)

/* terminal unit ID */
#define USB_VIDEO_VIRTUAL_CAMERA_VC_INPUT_TERMINAL_ID (1U)
#define USB_VIDEO_VIRTUAL_CAMERA_VC_INPUT_TERMINAL_TYPE (USB_DEVICE_VIDEO_ITT_CAMERA)
#define USB_VIDEO_VIRTUAL_CAMERA_VC_OUTPUT_TERMINAL_ID (2U)
#define USB_VIDEO_VIRTUAL_CAMERA_VC_PROCESSING_UNIT_ID (3U)

typedef enum _usb_device_configuration_descriptor_struct
{
    UVC_CDC_CONFIGURE_DESCRIPTOR = 0,
    CDC_CONFIGURE_DESCRIPTOR
} usb_device_configuration_descriptor_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief USB device set speed function.
 *
 * This function sets the speed of the USB device.
 *
 * Due to the difference of HS and FS descriptors, the device descriptors and configurations need to be updated to match
 * current speed.
 * As the default, the device descriptors and configurations are configured by using FS parameters for both EHCI and
 * KHCI.
 * When the EHCI is enabled, the application needs to call this function to update device by using current speed.
 * The updated information includes endpoint max packet size, endpoint interval, etc.
 *
 * @param handle The USB device handle.
 * @param speed Speed type. USB_SPEED_HIGH/USB_SPEED_FULL/USB_SPEED_LOW.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceSetSpeed(usb_device_handle handle, uint8_t speed);
#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
/* Get device qualifier descriptor request */
usb_status_t USB_DeviceGetDeviceQualifierDescriptor(
    usb_device_handle handle, usb_device_get_device_qualifier_descriptor_struct_t *deviceQualifierDescriptor);
#endif
/*!
 * @brief USB device get device descriptor function.
 *
 * This function gets the device descriptor of the USB device.
 *
 * @param handle The USB device handle.
 * @param deviceDescriptor The pointer to the device descriptor structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetDeviceDescriptor(usb_device_handle handle,
                                           usb_device_get_device_descriptor_struct_t *deviceDescriptor);

/*!
 * @brief USB device get configuration descriptor function.
 *
 * This function gets the configuration descriptor of the USB device.
 *
 * @param handle The USB device handle.
 * @param configurationDescriptor The pointer to the configuration descriptor structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetConfigurationDescriptor(
    usb_device_handle handle, usb_device_get_configuration_descriptor_struct_t *configurationDescriptor);

/*!
 * @brief USB device get string descriptor function.
 *
 * This function gets the string descriptor of the USB device.
 *
 * @param handle The USB device handle.
 * @param stringDescriptor Pointer to the string descriptor structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetStringDescriptor(usb_device_handle handle,
                                           usb_device_get_string_descriptor_struct_t *stringDescriptor);

/*!
 * @brief USB device set string descriptor function.
 *
 * This function set the  configuration descriptor to be used.
 *
 * @param p_device_configuration Value that specify what descriptors to be used.
 * UVC_CDC_CONFIGURE_DESCRIPTOR allows to have video over usb and a  vcom console.
 * CDC_CONFIGURE_DESCRIPTOR allows only vcom console.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceSetConfigurationDescriptor(usb_device_configuration_descriptor_struct_t p_device_configuration);

#endif /* _USB_DEVICE_DESCRIPTOR_H_ */
