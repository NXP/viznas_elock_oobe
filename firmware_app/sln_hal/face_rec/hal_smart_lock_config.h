/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief smart lock configuration declaration.
 */

#ifndef _HAL_SMART_LOCK_CONFIG_H_
#define _HAL_SMART_LOCK_CONFIG_H_

#include <stddef.h>

#include "app_config.h"
#include "fwk_sln_config.h"
#include "hal_vision_algo.h"

typedef struct _smart_lock_config
{
    uint8_t mode;      /* oasis application mode */
    uint8_t irPwm;     /* brightness % (0-100) */
    uint8_t whitePwm;  /* brightness % (0-100) */
    uint8_t sleepMode; /* detect no face timeout sleep */
    uint32_t speakerVolume;
    uint8_t password[6];       /* device password */
    uint32_t faceRecThreshold; /* % (0 - 1000); face rec threshold = faceRecThreshold * 1.0 / 1000 */
} smart_lock_config_t;

#if defined(__cplusplus)
extern "C" {
#endif

hal_config_status_t HAL_OutputDev_SmartLockConfig_Init();
oasis_lite_mode_t HAL_OutputDev_SmartLockConfig_GetMode();
hal_config_status_t HAL_OutputDev_SmartLockConfig_SetMode(oasis_lite_mode_t mode);
uint8_t HAL_OutputDev_SmartLockConfig_GetIrPwm();
hal_config_status_t HAL_OutputDev_SmartLockConfig_SetIrPwm(uint8_t brightness);
uint8_t HAL_OutputDev_SmartLockConfig_GetWhitePwm();
hal_config_status_t HAL_OutputDev_SmartLockConfig_SetWhitePwm(uint8_t brightness);
hal_config_status_t HAL_OutputDev_SmartLockConfig_GetPassword(uint8_t *password);
hal_config_status_t HAL_OutputDev_SmartLockConfig_SetPassword(uint8_t *password);
uint32_t HAL_OutputDev_SmartLockConfig_GetSpeakerVolume();
hal_config_status_t HAL_OutputDev_SmartLockConfig_SetSpeakerVolume(uint32_t speakerVolume);
uint8_t HAL_OutputDev_SmartLockConfig_GetSleepMode();
hal_config_status_t HAL_OutputDev_SmartLockConfig_SetSleepMode(uint8_t sleepMode);
hal_config_status_t HAL_OutputDev_SmartLockConfig_GetFaceRecThreshold(unsigned int *pThreshold);
hal_config_status_t HAL_OutputDev_SmartLockConfig_SetFaceRecThreshold(unsigned int threshold);

#if defined(__cplusplus)
}
#endif

#endif /* _HAL_SMART_LOCK_CONFIG_H_ */
