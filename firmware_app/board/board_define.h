/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP IoT Solutions Team.
 */

/*
 * @brief  board type define for all needed macros, please all place here before compiling.
 *
 * @Author jianfeng.qin@nxp.com
 */

#ifndef BOARD_DEFINE_H_
#define BOARD_DEFINE_H_

/*----------------------------------------------------------------------------------------------
 *
 * RT106F all definition put here
 * ---------------------------------------------------------------------------------------------*/
/*
 * UI switch define
 */
//#define APP_FFI

/*
 * Debug console definition
 */
#define DEBUG_CONSOLE_UART_INDEX 5

/*
 * Shell command through usb/uart
 */
#define ENABLE_SHELL_USB  1
#define ENABLE_SHELL_UART 0
/*
 * Camera definition
 */
#define ENABLE_CSI_CAMERA             0
#define ENABLE_FLEXIO_CAMERA          0
#define ENABLE_CSI_SHARED_DUAL_CAMERA 1

/*
 * Panel definition
 */
#define ENABLE_LCDIF_RK024HH298       1
#define ENABLE_LCDIF_RK043FN02H       0
#define ENABLE_DISPLAY_UVC            0

#endif
