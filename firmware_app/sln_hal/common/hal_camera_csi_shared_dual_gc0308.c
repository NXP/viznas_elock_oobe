/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief camera dev csi shared dual gc0308 hal implementation.
 */
#include <FreeRTOS.h>
#include <task.h>
#include <stdlib.h>
#include <time.h>

#include "fsl_camera.h"
#include "fsl_camera_receiver.h"
#include "fsl_camera_device.h"
#include "fsl_csi.h"
#include "fsl_csi_camera_adapter.h"
#include "fsl_gc0308.h"

#include "fwk_log.h"
#include "fwk_camera_manager.h"

#include "hal_camera_dev.h"
#include "hal_event_descriptor_common.h"

#define CAMERA_NAME             "CSI_DUAL_GC0308"
/* the default pixel format will be set to GRAY which is IR camera sensor */
#define CAMERA_PIXEL_FORMAT     kPixelFormat_UYVY1P422_RGB
#define CAMERA_WIDTH            640
#define CAMERA_HEIGHT           480
#define CAMERA_BYTE_PER_PIXEL   2
#define CAMERA_DEV_BUFFER_COUNT 4

#define CAMERA_RGB_CONTROL_FLAGS (kCAMERA_HrefActiveHigh | kCAMERA_DataLatchOnRisingEdge)

typedef enum _csi_shared_dual_camera_id
{
    CAMERA_RGB = 0,
    CAMERA_IR,
    CAMERA_NUM,
} csi_shared_dual_camera_id;

AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t frameBuffer[CAMERA_DEV_BUFFER_COUNT][CAMERA_HEIGHT][CAMERA_WIDTH * CAMERA_BYTE_PER_PIXEL], 32);

static uint8_t *s_pCurrentFrameBuffer = NULL;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
void BOARD_InitCSICameraResource(void);
void BOARD_PullCSICameraRGBPowerDownPin(bool pullUp);
void BOARD_PullCSICameraIRPowerDownPin(bool pullUp);
status_t BOARD_CSICameraI2CSend(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);
status_t BOARD_CSICameraI2CReceive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);
#if defined(__cplusplus)
}
#endif /* __cplusplus */

static csi_resource_t csiResource = {
    .csiBase = CSI,
};

static csi_private_data_t csiPrivateData;

static camera_receiver_handle_t cameraReceiver = {
    .resource    = &csiResource,
    .ops         = &csi_ops,
    .privateData = &csiPrivateData,
};

/* shared dula gc0308 camera haven't pin out reset. */
void pull_reset_pin(bool pullUp)
{

}

static gc0308_resource_t gc0308Resource[CAMERA_NUM] = {
{
    .i2cSendFunc       = BOARD_CSICameraI2CSend,
    .i2cReceiveFunc    = BOARD_CSICameraI2CReceive,
    .pullResetPin      = pull_reset_pin,
    .pullPowerDownPin  = BOARD_PullCSICameraRGBPowerDownPin,
    .inputClockFreq_Hz = 24000000,
},
{
    .i2cSendFunc       = BOARD_CSICameraI2CSend,
    .i2cReceiveFunc    = BOARD_CSICameraI2CReceive,
    .pullResetPin      = pull_reset_pin,
    .pullPowerDownPin  = BOARD_PullCSICameraIRPowerDownPin,
    .inputClockFreq_Hz = 24000000,
},
};

static camera_device_handle_t cameraDevice[CAMERA_NUM] = {
{
    .resource = &gc0308Resource[CAMERA_RGB],
    .ops      = &gc0308_ops,
},
{
    .resource = &gc0308Resource[CAMERA_IR],
    .ops      = &gc0308_ops,
},
};

static uint8_t s_CurrentRunningCamera = CAMERA_NUM;
static uint8_t s_CurrentFrameValid    = 0;
static uint8_t s_CurRGBExposureMode   = CAMERA_EXPOSURE_MODE_AUTO_LEVEL0;

static void _camera_init_interface(void)
{

}

static void camera_shared_dual_switch(csi_shared_dual_camera_id cameraId)
{
    if (cameraId == CAMERA_RGB)
    {
        CAMERA_DEVICE_Stop(&cameraDevice[CAMERA_IR]);
        CAMERA_DEVICE_Start(&cameraDevice[CAMERA_RGB]);
    }
    else if (cameraId == CAMERA_IR)
    {
        CAMERA_DEVICE_Stop(&cameraDevice[CAMERA_RGB]);
        CAMERA_DEVICE_Start(&cameraDevice[CAMERA_IR]);
    }
    else
    {
        CAMERA_DEVICE_Stop(&cameraDevice[CAMERA_RGB]);
        CAMERA_DEVICE_Stop(&cameraDevice[CAMERA_IR]);
    }
}

static void camera_receiver_callback(camera_receiver_handle_t *handle, status_t status, void *userData)
{
    camera_dev_t *dev = (camera_dev_t *)userData;

    if (dev->cap.callback != NULL)
    {
        uint8_t fromISR = __get_IPSR();
        dev->cap.callback(dev, kCameraEvent_SendFrame, dev->cap.param, fromISR);
    }
}

static hal_camera_status_t HAL_CameraDev_CsiSharedDualGC0308_Init(camera_dev_t *dev, int width, int height, camera_dev_callback_t callback, void *param)
{
    LOGD("camera_dev_csi_shared_dual_gc0308_init");

    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;
    camera_config_t cameraConfig;

    dev->config.width  = width;
    dev->config.height = height;
    dev->cap.callback  = callback;
    dev->cap.param     = param;

    // init csi receiver
    memset(&cameraConfig, 0, sizeof(cameraConfig));
    cameraConfig.pixelFormat                = kVIDEO_PixelFormatYUYV;
    cameraConfig.bytesPerPixel              = CAMERA_BYTE_PER_PIXEL;
    cameraConfig.resolution                 = FSL_VIDEO_RESOLUTION(width, height);
    cameraConfig.frameBufferLinePitch_Bytes = width * CAMERA_BYTE_PER_PIXEL;
    cameraConfig.interface                  = kCAMERA_InterfaceGatedClock;
    cameraConfig.controlFlags               = CAMERA_RGB_CONTROL_FLAGS;
    cameraConfig.framePerSec                = 30;

    BOARD_InitCSICameraResource();

    _camera_init_interface();

    NVIC_SetPriority(CSI_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);
    CAMERA_RECEIVER_Init(&cameraReceiver, &cameraConfig, camera_receiver_callback, dev);

    // init shared dual camera dev
    /* ir camera */
    CAMERA_DEVICE_Init(&cameraDevice[CAMERA_IR], &cameraConfig);
    CAMERA_DEVICE_Control(&cameraDevice[CAMERA_IR], kCAMERA_DeviceMonoMode, CAMERA_MONO_MODE_ENABLED);
    CAMERA_DEVICE_Stop(&cameraDevice[CAMERA_IR]);

    /* rgb camera */
    CAMERA_DEVICE_Init(&cameraDevice[CAMERA_RGB], &cameraConfig);
    CAMERA_DEVICE_Control(&cameraDevice[CAMERA_RGB], kCAMERA_DeviceMonoMode, CAMERA_MONO_MODE_DISABLED);
    CAMERA_DEVICE_Start(&cameraDevice[CAMERA_RGB]);

    s_CurrentRunningCamera = CAMERA_RGB;

    for (int i = 0; i < CAMERA_DEV_BUFFER_COUNT; i++)
    {
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)frameBuffer[i]);
    }

    return ret;
}

static hal_camera_status_t HAL_CameraDev_CsiSharedDualGC0308_Deinit(camera_dev_t *dev)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;
    return ret;
}

static hal_camera_status_t HAL_CameraDev_CsiSharedDualGC0308_Start(const camera_dev_t *dev)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;

    LOGD("camera_dev_csi_shared_dual_gc0308_start");
    CAMERA_RECEIVER_Start(&cameraReceiver);

    return ret;
}

static hal_camera_status_t HAL_CameraDev_CsiSharedDualGC0308_Enqueue(const camera_dev_t *dev, void *data)
{
    LOGI("++HAL_CameraDev_CsiSharedDualGC0308_Enqueue");

    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;

    if (s_pCurrentFrameBuffer != NULL)
    {
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)s_pCurrentFrameBuffer);
        s_pCurrentFrameBuffer = NULL;
    }

    LOGI("--HAL_CameraDev_CsiSharedDualGC0308_Enqueue");
    return ret;
}

static hal_camera_status_t HAL_CameraDev_CsiSharedDualGC0308_Dequeue(const camera_dev_t *dev, void **data, pixel_format_t *format)
{
    LOGI("++HAL_CameraDev_CsiSharedDualGC0308_Dequeue");

    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;

    while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, (uint32_t *)&s_pCurrentFrameBuffer))
    {
    }

    if (s_CurrentRunningCamera == CAMERA_RGB && s_CurrentFrameValid == 1)
    {
        *data   = s_pCurrentFrameBuffer;
        *format = kPixelFormat_UYVY1P422_RGB;
        camera_shared_dual_switch(CAMERA_IR);
        s_CurrentFrameValid = 0;
        s_CurrentRunningCamera = CAMERA_IR;
    }
    else if (s_CurrentRunningCamera == CAMERA_IR && s_CurrentFrameValid == 1)
    {
        *data   = s_pCurrentFrameBuffer;
        *format = kPixelFormat_UYVY1P422_Gray;
        camera_shared_dual_switch(CAMERA_RGB);
        s_CurrentFrameValid = 0;
        s_CurrentRunningCamera = CAMERA_RGB;
    }
    else if (s_CurrentFrameValid != 1)
    {
        s_CurrentFrameValid = 1;
    }

    LOGI("--HAL_CameraDev_CsiSharedDualGC0308_Dequeue");
    return ret;
}

static uint8_t _HAL_CameraDev_GetTargetExposureMode(uint8_t curMode, uint8_t direction)
{
    uint8_t mode;
    uint8_t modeMin      = CAMERA_EXPOSURE_MODE_AUTO_LEVEL0;
    uint8_t modeMax      = CAMERA_EXPOSURE_MODE_AUTO_LEVEL3;
    uint8_t modeInterval = 1;
    if (direction)
    {
        mode = curMode + modeInterval;
        if (mode >= modeMax)
            mode = modeMax;
    }
    else
    {
        if (curMode <= (modeInterval + modeMin))
        {
            mode = modeMin;
        }
        else
        {
            mode = curMode - modeInterval;
        }
    }
    LOGI("Camera exposure [curMode:%d][targetMode:%d]", curMode, mode);
    return mode;
}

static hal_camera_status_t HAL_CameraDev_CsiSharedDualGC0308_InputNotify(const camera_dev_t *dev, void *data)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;

    LOGI("++HAL_CameraDev_CsiSharedDualGC0308_InputNotify");

    event_base_t eventBase = *(event_base_t *)data;
    switch (eventBase.eventId)
    {
        case kEventID_ControlRGBCamExposure:
        {
            if (s_CurrentRunningCamera == CAMERA_RGB)
            {
                event_common_t event = *(event_common_t *)data;
                uint8_t mode;
                if (event.brightnessControl.enable == true)
                {
                    mode = _HAL_CameraDev_GetTargetExposureMode(s_CurRGBExposureMode, event.brightnessControl.direction);
                }
                else
                {
                    mode = CAMERA_EXPOSURE_MODE_AUTO_LEVEL0;
                }
                if (mode != s_CurRGBExposureMode)
                {
                    CAMERA_DEVICE_Control(&cameraDevice[CAMERA_RGB], kCAMERA_DeviceExposureMode, (int32_t)mode);
                    s_CurRGBExposureMode = mode;
                }
            }
        }
        break;
        default:
            break;
    }
    LOGI("--HAL_CameraDev_CsiSharedDualGC0308_InputNotify");

    return ret;
}

const static camera_dev_operator_t s_CameraDev_CsiSharedDualGC0308Ops = {
    .init         = HAL_CameraDev_CsiSharedDualGC0308_Init,
    .deinit       = HAL_CameraDev_CsiSharedDualGC0308_Deinit,
    .start        = HAL_CameraDev_CsiSharedDualGC0308_Start,
    .enqueue      = HAL_CameraDev_CsiSharedDualGC0308_Enqueue,
    .dequeue      = HAL_CameraDev_CsiSharedDualGC0308_Dequeue,
    .inputNotify  = HAL_CameraDev_CsiSharedDualGC0308_InputNotify,
};

static camera_dev_t s_CameraDev_CsiSharedDualGC0308 = {
    .id   = 1,
    .ops  = &s_CameraDev_CsiSharedDualGC0308Ops,
    .name = CAMERA_NAME,
    .cap  = {
            .callback = NULL,
            .param    = NULL,
    },
};

int HAL_CameraDev_CsiSharedDualGC0308_Register(camera_dev_static_config_t *config)
{
    int error = 0;
    LOGD("camera_dev_csi_shared_dual_gc0308_register");

    memcpy(&(s_CameraDev_CsiSharedDualGC0308.config), config, sizeof(camera_dev_static_config_t));

    error = FWK_CameraManager_DeviceRegister(&s_CameraDev_CsiSharedDualGC0308);
    return error;
}
