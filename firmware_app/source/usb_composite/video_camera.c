/*
 * Copyright (c) 2015 - 2020, Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "video_camera.h"
#include "fsl_camera_receiver.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#include "usb_phy.h"
#include "composite.h"
#include "commondef.h"
#endif
#if ((defined(USB_DEVICE_CONFIG_VIDEO)) && (USB_DEVICE_CONFIG_VIDEO > 0U))
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static usb_status_t USB_DeviceVideoRequest(class_handle_t handle, uint32_t event, void *param);

/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static usb_device_video_probe_and_commit_controls_struct_t s_ProbeStruct;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static usb_device_video_probe_and_commit_controls_struct_t s_CommitStruct;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static usb_device_video_still_probe_and_commit_controls_struct_t s_StillProbeStruct;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static usb_device_video_still_probe_and_commit_controls_struct_t s_StillCommitStruct;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint32_t s_ClassRequestBuffer[(sizeof(usb_device_video_probe_and_commit_controls_struct_t) >> 2U) + 1U];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_ImageBuffer[HS_STREAM_IN_PACKET_SIZE * (1 + HS_STREAM_IN_PACKET_ADDITIONAL_TRANSACTION)];

uint8_t *g_FrameBufferUSB = NULL;
uint32_t g_FrameBufferUSBLength = 0;

USB_LINK_NONCACHE_NONINIT_DATA static uint8_t imageBuffer[4096];

/*******************************************************************************
 * Code
 ******************************************************************************/

static usb_status_t USB_DeviceVideoRequest(class_handle_t handle, uint32_t event, void *param)
{
    usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;
    usb_device_video_probe_and_commit_controls_struct_t *probe =
        (usb_device_video_probe_and_commit_controls_struct_t *)(request->buffer);
    usb_device_video_probe_and_commit_controls_struct_t *commit =
        (usb_device_video_probe_and_commit_controls_struct_t *)(request->buffer);
    usb_device_video_still_probe_and_commit_controls_struct_t *still_probe =
        (usb_device_video_still_probe_and_commit_controls_struct_t *)(request->buffer);
    usb_device_video_still_probe_and_commit_controls_struct_t *still_commit =
        (usb_device_video_still_probe_and_commit_controls_struct_t *)(request->buffer);
    uint32_t temp32;
    usb_status_t error = kStatus_USB_Success;

    switch (event)
    {
        /* probe request */
        case USB_DEVICE_VIDEO_SET_CUR_VS_PROBE_CONTROL:
            if ((request->buffer == NULL) || (request->length == 0U))
            {
                return kStatus_USB_InvalidRequest;
            }
            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(probe->dwFrameInterval);
            if ((temp32 >= USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MIN_INTERVAL) &&
                (temp32 <= USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_INTERVAL))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(temp32, g_composite.g_UsbDeviceVideoCamera.probeStruct->dwFrameInterval);
            }
            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(probe->dwMaxPayloadTransferSize);
            if ((temp32) && (temp32 < g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    temp32, g_composite.g_UsbDeviceVideoCamera.probeStruct->dwMaxPayloadTransferSize);
            }
            g_composite.g_UsbDeviceVideoCamera.probeStruct->bFormatIndex = probe->bFormatIndex;
            g_composite.g_UsbDeviceVideoCamera.probeStruct->bFrameIndex  = probe->bFrameIndex;
            break;
        case USB_DEVICE_VIDEO_GET_CUR_VS_PROBE_CONTROL:
            request->buffer = (uint8_t *)g_composite.g_UsbDeviceVideoCamera.probeStruct;
            request->length = g_composite.g_UsbDeviceVideoCamera.probeLength;
            break;
        case USB_DEVICE_VIDEO_GET_LEN_VS_PROBE_CONTROL:
            request->buffer = &g_composite.g_UsbDeviceVideoCamera.probeLength;
            request->length = sizeof(g_composite.g_UsbDeviceVideoCamera.probeLength);
            break;
        case USB_DEVICE_VIDEO_GET_INFO_VS_PROBE_CONTROL:
            request->buffer = &g_composite.g_UsbDeviceVideoCamera.probeInfo;
            request->length = sizeof(g_composite.g_UsbDeviceVideoCamera.probeInfo);
            break;
        /* commit request */
        case USB_DEVICE_VIDEO_SET_CUR_VS_COMMIT_CONTROL:
            if ((request->buffer == NULL) || (request->length == 0U))
            {
                return kStatus_USB_InvalidRequest;
            }
            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(commit->dwFrameInterval);
            if ((temp32 >= USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MIN_INTERVAL) &&
                (temp32 <= USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_INTERVAL))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(temp32,
                                               g_composite.g_UsbDeviceVideoCamera.commitStruct->dwFrameInterval);
            }

            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(commit->dwMaxPayloadTransferSize);
            if ((temp32) && (temp32 < g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    temp32, g_composite.g_UsbDeviceVideoCamera.commitStruct->dwMaxPayloadTransferSize);
            }
            g_composite.g_UsbDeviceVideoCamera.commitStruct->bFormatIndex = commit->bFormatIndex;
            g_composite.g_UsbDeviceVideoCamera.commitStruct->bFrameIndex  = commit->bFrameIndex;
            break;
        case USB_DEVICE_VIDEO_GET_CUR_VS_COMMIT_CONTROL:
            request->buffer = (uint8_t *)g_composite.g_UsbDeviceVideoCamera.commitStruct;
            request->length = g_composite.g_UsbDeviceVideoCamera.commitLength;
            break;
        case USB_DEVICE_VIDEO_GET_LEN_VS_COMMIT_CONTROL:
            request->buffer = &g_composite.g_UsbDeviceVideoCamera.commitLength;
            request->length = sizeof(g_composite.g_UsbDeviceVideoCamera.commitLength);
            break;
        case USB_DEVICE_VIDEO_GET_INFO_VS_COMMIT_CONTROL:
            request->buffer = &g_composite.g_UsbDeviceVideoCamera.commitInfo;
            request->length = sizeof(g_composite.g_UsbDeviceVideoCamera.commitInfo);
            break;
        /* still probe request */
        case USB_DEVICE_VIDEO_SET_CUR_VS_STILL_PROBE_CONTROL:
            if ((request->buffer == NULL) || (request->length == 0U))
            {
                return kStatus_USB_InvalidRequest;
            }
            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(still_probe->dwMaxPayloadTransferSize);
            if ((temp32) && (temp32 < g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    temp32, g_composite.g_UsbDeviceVideoCamera.stillProbeStruct->dwMaxPayloadTransferSize);
            }

            g_composite.g_UsbDeviceVideoCamera.stillProbeStruct->bFormatIndex = still_probe->bFormatIndex;
            g_composite.g_UsbDeviceVideoCamera.stillProbeStruct->bFrameIndex  = still_probe->bFrameIndex;
            break;
        case USB_DEVICE_VIDEO_GET_CUR_VS_STILL_PROBE_CONTROL:
            request->buffer = (uint8_t *)g_composite.g_UsbDeviceVideoCamera.stillProbeStruct;
            request->length = g_composite.g_UsbDeviceVideoCamera.stillProbeLength;
            break;
        case USB_DEVICE_VIDEO_GET_LEN_VS_STILL_PROBE_CONTROL:
            request->buffer = &g_composite.g_UsbDeviceVideoCamera.stillProbeLength;
            request->length = sizeof(g_composite.g_UsbDeviceVideoCamera.stillProbeLength);
            break;
        case USB_DEVICE_VIDEO_GET_INFO_VS_STILL_PROBE_CONTROL:
            request->buffer = &g_composite.g_UsbDeviceVideoCamera.stillProbeInfo;
            request->length = sizeof(g_composite.g_UsbDeviceVideoCamera.stillProbeInfo);
            break;
        /* still commit request */
        case USB_DEVICE_VIDEO_SET_CUR_VS_STILL_COMMIT_CONTROL:
            if ((request->buffer == NULL) || (request->length == 0U))
            {
                return kStatus_USB_InvalidRequest;
            }
            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(still_commit->dwMaxPayloadTransferSize);
            if ((temp32) && (temp32 < g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    temp32, g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->dwMaxPayloadTransferSize);
            }

            g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->bFormatIndex = still_commit->bFormatIndex;
            g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->bFrameIndex  = still_commit->bFrameIndex;
            break;
        case USB_DEVICE_VIDEO_GET_CUR_VS_STILL_COMMIT_CONTROL:
            request->buffer = (uint8_t *)g_composite.g_UsbDeviceVideoCamera.stillCommitStruct;
            request->length = g_composite.g_UsbDeviceVideoCamera.stillCommitLength;
            break;
        case USB_DEVICE_VIDEO_GET_LEN_VS_STILL_COMMIT_CONTROL:
            request->buffer = &g_composite.g_UsbDeviceVideoCamera.stillCommitLength;
            request->length = sizeof(g_composite.g_UsbDeviceVideoCamera.stillCommitLength);
            break;
        case USB_DEVICE_VIDEO_GET_INFO_VS_STILL_COMMIT_CONTROL:
            request->buffer = &g_composite.g_UsbDeviceVideoCamera.stillCommitInfo;
            request->length = sizeof(g_composite.g_UsbDeviceVideoCamera.stillCommitInfo);
            break;
        /* still image trigger request */
        case USB_DEVICE_VIDEO_SET_CUR_VS_STILL_IMAGE_TRIGGER_CONTROL:
            g_composite.g_UsbDeviceVideoCamera.stillImageTriggerControl = *(request->buffer);
            break;
        case USB_DEVICE_VIDEO_GET_MAX_VS_PROBE_CONTROL:
        case USB_DEVICE_VIDEO_GET_MIN_VS_PROBE_CONTROL:
        case USB_DEVICE_VIDEO_GET_DEF_VS_PROBE_CONTROL:
            request->buffer = (uint8_t *)g_composite.g_UsbDeviceVideoCamera.probeStruct;
            request->length = sizeof(*g_composite.g_UsbDeviceVideoCamera.probeStruct);
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

/* Prepare next transfer payload */
void USB_DeviceVideoPrepareVideoData(void)
{
    usb_device_video_mjpeg_payload_header_struct_t *payloadHeader;
    uint32_t maxPacketSize;
    uint32_t i;
    uint32_t sendLength;

    g_composite.g_UsbDeviceVideoCamera.currentTime += 10000U;
    g_composite.g_UsbDeviceVideoCamera.imageBuffer = (uint8_t *)(imageBuffer);
    payloadHeader =
        (usb_device_video_mjpeg_payload_header_struct_t *)&g_composite.g_UsbDeviceVideoCamera.imageBuffer[0];
    for (i = 0; i < sizeof(usb_device_video_mjpeg_payload_header_struct_t); i++)
    {
        g_composite.g_UsbDeviceVideoCamera.imageBuffer[i] = 0x00;
    }

    payloadHeader->bHeaderLength                = sizeof(usb_device_video_mjpeg_payload_header_struct_t);
    payloadHeader->headerInfoUnion.bmheaderInfo = 0U;
    payloadHeader->headerInfoUnion.headerInfoBits.frameIdentifier = g_composite.g_UsbDeviceVideoCamera.currentFrameId;

    g_composite.g_UsbDeviceVideoCamera.imageBufferLength = sizeof(usb_device_video_mjpeg_payload_header_struct_t);

    if (g_composite.g_UsbDeviceVideoCamera.stillImageTransmission)
    {
        maxPacketSize = g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->dwMaxPayloadTransferSize;
    }
    else
    {
        maxPacketSize = g_composite.g_UsbDeviceVideoCamera.commitStruct->dwMaxPayloadTransferSize;
    }

    maxPacketSize = maxPacketSize - sizeof(usb_device_video_mjpeg_payload_header_struct_t);

    if (g_composite.g_UsbDeviceVideoCamera.waitForNewInterval)
    {
        if (xSemaphoreTakeFromISR(g_DisplayFull, NULL) == pdFALSE)
        {
            return;
        }
        else
        {
            g_composite.g_UsbDeviceVideoCamera.imagePosition          = 0U;
            g_composite.g_UsbDeviceVideoCamera.currentTime            = 0U;
            g_composite.g_UsbDeviceVideoCamera.waitForNewInterval     = 0U;
            payloadHeader->headerInfoUnion.headerInfoBits.endOfFrame  = 1U;
            g_composite.g_UsbDeviceVideoCamera.stillImageTransmission = 0U;
            g_composite.g_UsbDeviceVideoCamera.currentFrameId ^= 1U;
            if (USB_DEVICE_VIDEO_STILL_IMAGE_TRIGGER_TRANSMIT_STILL_IMAGE ==
                g_composite.g_UsbDeviceVideoCamera.stillImageTriggerControl)
            {
                g_composite.g_UsbDeviceVideoCamera.stillImageTriggerControl =
                    USB_DEVICE_VIDEO_STILL_IMAGE_TRIGGER_NORMAL_OPERATION;
                g_composite.g_UsbDeviceVideoCamera.stillImageTransmission = 1U;
            }
            return;
        }
    };

    if (g_composite.g_UsbDeviceVideoCamera.imagePosition < g_FrameBufferUSBLength)
    {
        sendLength = g_FrameBufferUSBLength - g_composite.g_UsbDeviceVideoCamera.imagePosition;
        if (sendLength > maxPacketSize)
        {
            sendLength = maxPacketSize;
        }

        if (g_FrameBufferUSB != NULL)
        {
            memcpy((imageBuffer + sizeof(usb_device_video_mjpeg_payload_header_struct_t)),
                   (uint8_t *)(g_FrameBufferUSB + g_composite.g_UsbDeviceVideoCamera.imagePosition), sendLength);
        }
        g_composite.g_UsbDeviceVideoCamera.imagePosition += sendLength;

        if (g_composite.g_UsbDeviceVideoCamera.imagePosition >= g_FrameBufferUSBLength)
        {
            g_FrameBufferUSB = NULL;
            g_FrameBufferUSBLength = 0;
            xSemaphoreGiveFromISR(g_DisplayEmpty, NULL);
            g_composite.g_UsbDeviceVideoCamera.waitForNewInterval = 1U;
        }

        g_composite.g_UsbDeviceVideoCamera.imageBufferLength += sendLength;
    }
}

/* USB device Video class callback */
usb_status_t USB_DeviceVideoCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;

    switch (event)
    {
        case kUSB_DeviceVideoEventStreamSendResponse:
            /* Stream data dent */
            if (g_composite.g_UsbDeviceVideoCamera.attach)
            {
                /* Prepare the next stream data */
                USB_DeviceVideoPrepareVideoData();
                error = USB_DeviceVideoSend(g_composite.g_UsbDeviceVideoCamera.videoHandle,
                                            USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_IN,
                                            g_composite.g_UsbDeviceVideoCamera.imageBuffer,
                                            g_composite.g_UsbDeviceVideoCamera.imageBufferLength);
            }
            break;
        case kUSB_DeviceVideoEventClassRequestBuffer:
            if (param && (g_composite.g_UsbDeviceVideoCamera.attach))
            {
                /* Get the class-specific OUT buffer */
                usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;

                if (request->length <= sizeof(usb_device_video_probe_and_commit_controls_struct_t))
                {
                    request->buffer = (uint8_t *)g_composite.g_UsbDeviceVideoCamera.classRequestBuffer;
                    error           = kStatus_USB_Success;
                }
            }
            break;
        default:
            if (param && (event > 0xFFU))
            {
                /* If the event is the class-specific request(Event > 0xFFU), handle the class-specific request */
                error = USB_DeviceVideoRequest(handle, event, param);
            }
            break;
    }

    return error;
}

/* Set to default state */
void USB_DeviceVideoApplicationSetDefault(void)
{
    g_composite.g_UsbDeviceVideoCamera.speed                = USB_SPEED_FULL;
    g_composite.g_UsbDeviceVideoCamera.attach               = 0U;
    g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize = FS_STREAM_IN_PACKET_SIZE;
    g_composite.g_UsbDeviceVideoCamera.imageBuffer          = s_ImageBuffer;
    g_composite.g_UsbDeviceVideoCamera.probeStruct          = &s_ProbeStruct;
    g_composite.g_UsbDeviceVideoCamera.commitStruct         = &s_CommitStruct;
    g_composite.g_UsbDeviceVideoCamera.stillProbeStruct     = &s_StillProbeStruct;
    g_composite.g_UsbDeviceVideoCamera.stillCommitStruct    = &s_StillCommitStruct;
    g_composite.g_UsbDeviceVideoCamera.classRequestBuffer   = &s_ClassRequestBuffer[0];

    g_composite.g_UsbDeviceVideoCamera.probeStruct->bFormatIndex = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FORMAT_INDEX;
    g_composite.g_UsbDeviceVideoCamera.probeStruct->bFrameIndex  = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INDEX;
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_DEFAULT_INTERVAL,
                                   g_composite.g_UsbDeviceVideoCamera.probeStruct->dwFrameInterval);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize,
                                   g_composite.g_UsbDeviceVideoCamera.probeStruct->dwMaxPayloadTransferSize);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_FRAME_SIZE,
                                   g_composite.g_UsbDeviceVideoCamera.probeStruct->dwMaxVideoFrameSize);

    g_composite.g_UsbDeviceVideoCamera.commitStruct->bFormatIndex = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FORMAT_INDEX;
    g_composite.g_UsbDeviceVideoCamera.commitStruct->bFrameIndex  = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INDEX;
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_DEFAULT_INTERVAL,
                                   g_composite.g_UsbDeviceVideoCamera.commitStruct->dwFrameInterval);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize,
                                   g_composite.g_UsbDeviceVideoCamera.commitStruct->dwMaxPayloadTransferSize);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_FRAME_SIZE,
                                   g_composite.g_UsbDeviceVideoCamera.commitStruct->dwMaxVideoFrameSize);

    g_composite.g_UsbDeviceVideoCamera.probeInfo    = 0x03U;
    g_composite.g_UsbDeviceVideoCamera.probeLength  = 26U;
    g_composite.g_UsbDeviceVideoCamera.commitInfo   = 0x03U;
    g_composite.g_UsbDeviceVideoCamera.commitLength = 26U;

    g_composite.g_UsbDeviceVideoCamera.stillProbeStruct->bFormatIndex      = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FORMAT_INDEX;
    g_composite.g_UsbDeviceVideoCamera.stillProbeStruct->bFrameIndex       = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INDEX;
    g_composite.g_UsbDeviceVideoCamera.stillProbeStruct->bCompressionIndex = 0x01U;
    USB_LONG_TO_LITTLE_ENDIAN_DATA(g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize,
                                   g_composite.g_UsbDeviceVideoCamera.stillProbeStruct->dwMaxPayloadTransferSize);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_FRAME_SIZE,
                                   g_composite.g_UsbDeviceVideoCamera.stillProbeStruct->dwMaxVideoFrameSize);

    g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->bFormatIndex = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FORMAT_INDEX;
    g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->bFrameIndex  = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INDEX;
    g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->bCompressionIndex = 0x01U;
    USB_LONG_TO_LITTLE_ENDIAN_DATA(g_composite.g_UsbDeviceVideoCamera.currentMaxPacketSize,
                                   g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->dwMaxPayloadTransferSize);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_FRAME_SIZE,
                                   g_composite.g_UsbDeviceVideoCamera.stillCommitStruct->dwMaxVideoFrameSize);

    g_composite.g_UsbDeviceVideoCamera.stillProbeInfo    = 0x03U;
    g_composite.g_UsbDeviceVideoCamera.stillProbeLength  = sizeof(s_StillProbeStruct);
    g_composite.g_UsbDeviceVideoCamera.stillCommitInfo   = 0x03U;
    g_composite.g_UsbDeviceVideoCamera.stillCommitLength = sizeof(s_StillCommitStruct);

    g_composite.g_UsbDeviceVideoCamera.currentTime                            = 0U;
    g_composite.g_UsbDeviceVideoCamera.currentFrameId                         = 0U;
    g_composite.g_UsbDeviceVideoCamera.currentStreamInterfaceAlternateSetting = 0U;
    g_composite.g_UsbDeviceVideoCamera.imageBufferLength                      = 0U;
    g_composite.g_UsbDeviceVideoCamera.imageIndex                             = 0U;
    g_composite.g_UsbDeviceVideoCamera.waitForNewInterval                     = 0U;
    g_composite.g_UsbDeviceVideoCamera.stillImageTransmission                 = 0U;
    g_composite.g_UsbDeviceVideoCamera.stillImageTriggerControl = USB_DEVICE_VIDEO_STILL_IMAGE_TRIGGER_NORMAL_OPERATION;
}
#endif
