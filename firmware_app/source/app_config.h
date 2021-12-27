/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief app_config macro definitions. Please place each required definition here before compiling.
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#include "board_define.h"
#include "fwk_common.h"
#include "hal_camera_dev.h"

#define AT_NONCACHEABLE_SECTION_ALIGN_DTC(var, alignbytes) \
    __attribute__((section("NonCacheable2,\"aw\",%nobits @"))) var __attribute__((aligned(alignbytes)))

#define AT_CACHEABLE_SECTION_ALIGN_OCRAM(var, alignbytes) \
    __attribute__((section(".bss.$SRAM_OCRAM_CACHED,\"aw\",%nobits @"))) var __attribute__((aligned(alignbytes)))
#define AT_NONCACHEABLE_SECTION_ALIGN_OCRAM(var, alignbytes) \
    __attribute__((section(".bss.$SRAM_OCRAM_NCACHED,\"aw\",%nobits @"))) var __attribute__((aligned(alignbytes)))

#define APP_TASK_ID(n) (kFWKTaskID_APPStart + n)

/*----------------------------------------------------------------------------------------------
 * Put all RT117F/VIZN3D definitions here
 * ---------------------------------------------------------------------------------------------*/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

int HAL_GfxDev_Pxp_Register();
int HAL_DisplayDev_UsbUvc_Register();
int HAL_LpmDev_Register();
int HAL_OutputDev_UiSmartlock_Register();
int HAL_OutputDev_UiFfi_Register();
int HAL_OutputDev_Console_Register();
int HAL_OutputDev_RgbLed_Register();
int HAL_OutputDev_IrWhiteLeds_Register();
int HAL_InputDev_PushButtons_Register();
int HAL_OutputDev_SmartLockConfig_Register();

/* App task ID offset definition here. App task ID will start from kFWKTaskID_APPStart */

// for vision_algo_oasis_lite2D device
// please include oasis2D/oasislite_runtime.h
// please link oasis2D/liboasis_lite2D_DEFAULT_117f_ae.a
int HAL_VisionAlgo_OasisLite2D_Register(int mode); // mode=0 smartlock; mode=1 ffi

#if ENABLE_CSI_CAMERA
#define CAMERA_PIXEL_FORMAT_CSI_GC0308    kPixelFormat_UYVY1P422_Gray
#define CAMERA_WIDTH_CSI_GC0308           640
#define CAMERA_HEIGHT_CSI_GC0308          480
#define CAMERA_BYTES_PER_PIXEL_CSI_GC0308 2
#define CAMERA_BUFFER_COUNT_CSI_GC0308    4
#define CAMERA_ROTATION_CSI_GC0308        kCWRotateDegree_90
#define CAMERA_FLIP_CSI_GC0308            kFlipMode_None
#define CAMERA_SWAPBYTE_CSI_GC0308        1

int HAL_CameraDev_CsiGc0308_Register(camera_dev_static_config_t *format);
#endif

#if ENABLE_FLEXIO_CAMERA
#define CAMERA_PIXEL_FORMAT_FLEXIO_GC0308    kPixelFormat_UYVY1P422_RGB
#define CAMERA_WIDTH_FLEXIO_GC0308           640
#define CAMERA_HEIGHT_FLEXIO_GC0308          480
#define CAMERA_BYTES_PER_PIXEL_FLEXIO_GC0308 2
#define CAMERA_BUFFER_COUNT_FLEXIO_GC0308    4
#define CAMERA_ROTATION_FLEXIO_GC0308        kCWRotateDegree_90
#define CAMERA_FLIP_FLEXIO_GC0308            kFlipMode_None
#define CAMERA_SWAPBYTE_FLEXIO_GC0308        1

int HAL_CameraDev_FlexioGc0308_Register(camera_dev_static_config_t *config);
#endif

#if ENABLE_CSI_SHARED_DUAL_CAMERA
#define CAMERA_PIXEL_FORMAT_CSI_SHARED_DUAL_GC0308 kPixelFormat_UYVY1P422_RGB
#define CAMERA_CSI_SHARED_DUAL_HEIGHT              480
#define CAMERA_CSI_SHARED_DUAL_WIDTH               640
#define CAMERA_CSI_SHARED_DUAL_BYTE_PER_PIXEL      2
#define CAMERA_CSI_SHARED_DUAL_ROTATION            kCWRotateDegree_270
#define CAMERA_CSI_SHARED_DUAL_FLIP                kFlipMode_None
#define CAMERA_CSI_SHARED_DUAL_SWAPBYTE            1

int HAL_CameraDev_CsiSharedDualGC0308_Register(camera_dev_static_config_t *config);
#endif

#if ENABLE_SHELL_USB
int HAL_InputDev_ShellUsb_Register();
#endif
#if ENABLE_SHELL_UART
int HAL_InputDev_ShellUart_Register();
#endif

#if ENABLE_LCDIF_RK024HH298
int HAL_DisplayDev_LcdifRk024hh298_Register();
#endif

#if ENABLE_DISPLAY_OVER_USBCDC
int HAL_DisplayDev_UsbCdc2D_Register();
#endif

/*
 * The UI will dependent on the display resolution
 */
#if ENABLE_LCDIFV2_RK055AHD091
#define UI_BUFFER_WIDTH  480
#define UI_BUFFER_HEIGHT 640
#else
#define UI_BUFFER_WIDTH  240
#define UI_BUFFER_HEIGHT 320
#endif

int HAL_FlashDev_Littlefs_Init();

#if ENABLE_DISPLAY_OVER_LPUART
int HAL_DisplayDev_Lpuart2D_Register();
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* APP_CONFIG_H_ */
