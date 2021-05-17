/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef SLN_APP_FWUPDATE_H_
#define SLN_APP_FWUPDATE_H_

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _fwupdate_check_status
{
    FWUPDATE_OK,
    FWUPDATE_JSON_ERROR,
    FWUPDATE_WRONG_MESSAGETYPE,
} fwupdate_check_status_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
/*!
 * @brief Flips the METHOD bit to 0 in the FICA header for OTA is 0
 */
int32_t FICA_set_OTA_bit(void);

/*!
 * @brief Flips the FWUpdate bit to 0 in the FICA header
 */
int32_t FICA_set_FW_bit(void);

/*!
 * @brief Receive a buffer, check if the buffer received is an ota start command
 * @param buffer    Contains the message to be checked
 * @return          fwupdate_check_status
 */
fwupdate_check_status_t FWUpdate_check_start_command(uint8_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /*SLN_APP_FWUPDATE_H_*/
