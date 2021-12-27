/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief hal device registration for all needed here.
 */

#include <FreeRTOS.h>
#include <task.h>

#include "app_config.h"
#include "board.h"

#include "fwk_message.h"
#include "fwk_display_manager.h"
#include "fwk_camera_manager.h"
#include "fwk_input_manager.h"
#include "fwk_output_manager.h"
#include "fwk_vision_algo_manager.h"
#include "fwk_log.h"

#include "hal_smart_lock_config.h"
#include "hal_vision_algo.h"

int APP_RegisterHalDevices(void)
{
    int ret = 0;
    ret     = HAL_GfxDev_Pxp_Register();
    if (ret != 0)
    {
        LOGE("HAL_GfxDev_Pxp_Register error %d", ret);
        return ret;
    }

#if HEADLESS_ENABLE
#else
    display_output_t defaultDisplayOutput = FWK_ConfigGetDisplayOutput();
    if ((defaultDisplayOutput >= kDisplayOutput_Panel) && (defaultDisplayOutput < kDisplayOutput_Invalid))
    {
        LOGD("[DisplayOutput]:%d", defaultDisplayOutput);
    }
    else
    {
        LOGE("Invalid display output %d, set to %d", defaultDisplayOutput, kDisplayOutput_Panel);
        defaultDisplayOutput = kDisplayOutput_Panel;
        FWK_ConfigSetDisplayOutput(defaultDisplayOutput);
    }

    if (defaultDisplayOutput == kDisplayOutput_Panel)
    {
#if ENABLE_LCDIF_RK024HH298
        ret = HAL_DisplayDev_LcdifRk024hh298_Register();
#elif ENABLE_LCDIFV2_RK055AHD091
        ret = HAL_DisplayDev_Lcdifv2Rk055ahd091_Register();
#endif

        if (ret != 0)
        {
            LOGE("Display panel register error %d", ret);
            return ret;
        }
    }
    else
    {
#if ENABLE_DISPLAY_UVC
        ret = HAL_DisplayDev_UsbUvc_Register();
        if (ret != 0)
        {
            LOGE("HAL_DisplayDev_UsbUvc_Register error %d", ret);
            return ret;
        }
#endif
    }
#endif

#if ENABLE_FLEXIO_CAMERA
    camera_dev_static_config_t gc0308_static_config = {
        .height   = CAMERA_HEIGHT_FLEXIO_GC0308,
        .width    = CAMERA_WIDTH_FLEXIO_GC0308,
        .pitch    = CAMERA_WIDTH_FLEXIO_GC0308 * 2,
        .left     = 0,
        .top      = 0,
        .right    = CAMERA_WIDTH_FLEXIO_GC0308 - 1,
        .bottom   = CAMERA_HEIGHT_FLEXIO_GC0308 - 1,
        .rotate   = CAMERA_ROTATION_FLEXIO_GC0308,
        .flip     = CAMERA_FLIP_FLEXIO_GC0308,
        .swapByte = CAMERA_SWAPBYTE_FLEXIO_GC0308,
    };

    ret = HAL_CameraDev_FlexioGc0308_Register(&gc0308_static_config);
    if (ret != 0)
    {
        LOGE("HAL_CameraDev_FlexioGc0308_Register error %d", ret);
        return ret;
    }
#endif

#if ENABLE_CSI_CAMERA
    camera_dev_static_config_t csi_gc0308_static_config = {
        .height   = CAMERA_HEIGHT_CSI_GC0308,
        .width    = CAMERA_WIDTH_CSI_GC0308,
        .pitch    = CAMERA_WIDTH_CSI_GC0308 * 2,
        .left     = 0,
        .top      = 0,
        .right    = CAMERA_WIDTH_CSI_GC0308 - 1,
        .bottom   = CAMERA_HEIGHT_CSI_GC0308 - 1,
        .rotate   = CAMERA_ROTATION_CSI_GC0308,
        .flip     = CAMERA_FLIP_CSI_GC0308,
        .swapByte = CAMERA_SWAPBYTE_CSI_GC0308,
    };

    ret = HAL_CameraDev_CsiGc0308_Register(&csi_gc0308_static_config);
    if (ret != 0)
    {
        LOGE("HAL_CameraDev_CsiGc0308_Register error %d", ret);
        return ret;
    }

#endif

#if ENABLE_CSI_SHARED_DUAL_CAMERA
    camera_dev_static_config_t csi_shared_dual_gc0308_static_config = {
        .height   = CAMERA_CSI_SHARED_DUAL_HEIGHT,
        .width    = CAMERA_CSI_SHARED_DUAL_WIDTH,
        .pitch    = CAMERA_CSI_SHARED_DUAL_WIDTH * CAMERA_CSI_SHARED_DUAL_BYTE_PER_PIXEL,
        .left     = 0,
        .top      = 0,
        .right    = CAMERA_CSI_SHARED_DUAL_WIDTH - 1,
        .bottom   = CAMERA_CSI_SHARED_DUAL_HEIGHT - 1,
        .rotate   = CAMERA_CSI_SHARED_DUAL_ROTATION,
        .flip     = CAMERA_CSI_SHARED_DUAL_FLIP,
        .swapByte = CAMERA_CSI_SHARED_DUAL_SWAPBYTE,
    };

    ret = HAL_CameraDev_CsiSharedDualGC0308_Register(&csi_shared_dual_gc0308_static_config);
    if (ret != 0)
    {
        LOGE("HAL_CameraDev_CsiSharedDualGC0308_Register error %d", ret);
        return ret;
    }

#endif

#if defined(APP_FFI)
    ret = HAL_VisionAlgo_OasisLite2D_Register(kOASISLiteMode_FFI);
#else // default APP_SMARTLOCK
    ret = HAL_VisionAlgo_OasisLite2D_Register(kOASISLiteMode_SmartLock);
#endif
    if (ret != 0)
    {
        LOGE("vision_algo_oasis_lite_register error %d", ret);
        return ret;
    }

    ret = HAL_InputDev_PushButtons_Register();
    if (ret != 0)
    {
        LOGE("HAL_InputDev_PushButtons_Register error %d", ret);
        return ret;
    }

#if defined(ENABLE_SHELL_USB) & ENABLE_SHELL_USB
    ret = HAL_InputDev_ShellUsb_Register();
    if (ret != 0)
    {
        LOGE("HAL_InputDev_ShellUsb_Register error %d", ret);
        return ret;
    }
#elif defined(ENABLE_SHELL_UART) && ENABLE_SHELL_UART
    ret = HAL_InputDev_ShellUart_Register();
    if (ret != 0)
    {
        LOGE("HAL_InputDev_ShellUart_Register error %d", ret);
        return ret;
    }
#endif

    ret = HAL_OutputDev_RgbLed_Register();
    if (ret != 0)
    {
        LOGE("HAL_OutputDev_RgbLed_Register error %d", ret);
        return ret;
    }

    ret = HAL_OutputDev_IrWhiteLeds_Register();
    if (ret != 0)
    {
        LOGE("HAL_OutputDev_IrWhiteLeds_Register error %d", ret);
        return ret;
    }

#if HEADLESS_ENABLE
#else
#if defined(APP_FFI)
    ret = HAL_OutputDev_UiFfi_Register();
#else // default APP_SMARTLOCK
    ret = HAL_OutputDev_UiSmartlock_Register();
#endif
    if (ret != 0)
    {
        LOGE("HAL_OutputDev_UiSmartlock_Register error %d", ret);
        return ret;
    }
#endif

    ret = HAL_OutputDev_SmartLockConfig_Register();
    if (ret != 0)
    {
        LOGE("HAL_OutputDev_SmartLockConfig_Register error %d", ret);
        return ret;
    }

    return ret;
}
