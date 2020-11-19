/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "sln_app_fwupdate.h"
#include "fsl_common.h"
#include "sln_flash.h"
#include "sln_flash_config.h"
#include "cJSON.h"

#define FWUPDATE_MESSAGETYPE_START    (2)
#define FICA_START_ADDRESS            0x01FC0000
#define FICA_OFFSET_ICA_COMM          8
#define FICA_COMM_FWUPDATE_BIT        0x00000020
#define FICA_COMM_FWUPDATE_METHOD_BIT 0x00000040

/* Set the a configuration in the flash */
static int32_t FICA_set_bit(uint32_t bit)
{
    uint8_t current_bit;
    uint32_t config;
    int32_t ret = -1;
    uint8_t tempPage[FLASH_PAGE_SIZE];

    ret = SLN_Read_Flash_At_Address(FICA_START_ADDRESS, tempPage, FLASH_PAGE_SIZE);
    if (ret != 0)
    {
        return -1;
    }
    config      = *(uint32_t *)(tempPage + FICA_OFFSET_ICA_COMM);
    current_bit = ((config & bit) > 0); // 0 or 1

    /* Check if the bit is already 0.
     * The bit must be set to 0 on active due to flash restrictions
     * We can flip a bit to 0 from 1 without a sector erase
     */
    if (current_bit == 0)
    {
        return 0;
    }

    config &= ~bit;
    *(uint32_t *)(tempPage + FICA_OFFSET_ICA_COMM) = config;

    ret = SLN_Write_Flash_At_Address(FICA_START_ADDRESS, tempPage);
    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

int32_t FICA_set_OTA_bit()
{
    /* set update and method at the same time to avoid one failling */

    return FICA_set_bit(FICA_COMM_FWUPDATE_BIT | FICA_COMM_FWUPDATE_METHOD_BIT);
}

int32_t FICA_set_FW_bit()
{
    return FICA_set_bit(FICA_COMM_FWUPDATE_BIT);
}

/*!
 * @brief Receive a buffer, check if the buffer received is an ota start command
 */
fwupdate_check_status_t FWUpdate_check_start_command(uint8_t *buffer)
{
    fwupdate_check_status_t status = FWUPDATE_OK;
    cJSON *json                    = NULL;
    cJSON *messageType             = NULL;

    if (buffer == NULL)
    {
        return FWUPDATE_JSON_ERROR;
    }

    json = cJSON_Parse((const char *)buffer);

    if (!json)
    {
        status = FWUPDATE_JSON_ERROR;
    }

    if (0 == status)
    {
        /* Check if it contains messagetype JSON Object */
        messageType = cJSON_GetObjectItemCaseSensitive(json, "messageType");
        if (!cJSON_IsNumber(messageType))
        {
            status = FWUPDATE_JSON_ERROR;
        }
        else
        {
            /* Check if messagetype is the one used for start fwupdate command  */
            if (messageType->valueint != FWUPDATE_MESSAGETYPE_START)
            {
                status = FWUPDATE_WRONG_MESSAGETYPE;
            }
        }
    }

    /* Free heap memory allocated with cJSON_Parse() */
    cJSON_Delete(json);

    return status;
}
