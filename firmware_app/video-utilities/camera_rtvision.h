/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#ifndef _CAMERA_RTVISION_H_
#define _CAMERA_RTVISION_H_

#if RTVISION_BOARD

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Definitions
 *******************************************************************************/

/* Camera definition. */
#define APP_CAMERA_CONTROL_FLAGS (kCAMERA_HrefActiveHigh | kCAMERA_DataLatchOnRisingEdge)

#define APP_CAMERA_MT9M114 1
#define APP_CAMERA_GC0308  2

#ifndef APP_CAMERA_TYPE
#define APP_CAMERA_TYPE APP_CAMERA_MT9M114
#endif

/* Frame buffer data alignment. */
#define FRAME_BUFFER_ALIGN 64

#define APP_FRAME_BUFFER_COUNT 4

/* Pixel format RGB565, bytesPerPixel is 2. */
#define APP_BPP 2

#define CAMERA_MSG_Q_COUNT 10

#define COLOR_CAMERA 0
#define IR_CAMERA    1

/*******************************************************************************
 * API
 *******************************************************************************/
void BOARD_I2C_ReleaseBus(void);
void BOARD_InitCameraResource(void);
void Camera_ELCDIF_Init(void);
void Camera_InitPxp(void);
int Camera_Start();
int Camera_SendQMsg(void *msg);
int Camera_QMsgSetPWM(uint8_t led, uint8_t pulse_width);
void Camera_GetPWM(uint8_t led, uint8_t *pulse_width);
int Camera_SetMonoMode(uint8_t enable);
int Camera_SetDispMode(uint8_t displayMode);
uint8_t Camera_GetRGBExposureMode(void);
int Camera_SetRGBExposureMode(uint8_t mode);
int Camera_ChangeInterfaceMode(uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif
#endif /* _CAMERA_RTVISION_H_ */
