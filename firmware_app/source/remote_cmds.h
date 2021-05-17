/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef _REMOTE_CMD_H_
#define _REMOTE_CMD_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    READY_INDICATION       = 0x00,
    AUTHENTICATION_REQUEST = 0x02,
    AUTHENTICATION_RESPONSE,
    PASSWORD_REQUEST = 0x04,
    PASSWORD_RESPONSE,
    CONFIGURATION_GET_REQUEST = 0x06,
    CONFIGURATION_GET_RESPONSE,
    CONFIGURATION_UPDATE_REQUEST = 0x08,
    CONFIGURATION_UPDATE_RESPONSE,
    FACE_RECORD_GET_REQUEST = 0x0A,
    FACE_RECORD_GET_RESPONSE,
    FACE_RECORD_UPDATE_REQUEST = 0x0C,
    FACE_RECORD_UPDATE_RESPONSE,
    OPEN_DOOR_RESPONSE = 0x5A,
    OPEN_DOOR_REQUEST  = 0xA5,
    UNKNOW_COMMAND     = 0xFF,
} remote_frame_cmd_t;

typedef struct
{
    char prefix[4];
    int timestamp;
    uint8_t version;
    uint8_t cipher;
    uint16_t length;
} remote_frame_header_t;

typedef struct
{
    remote_frame_header_t frame_header;
    remote_frame_cmd_t command;
    void *payload;
    int crc;
} remote_frame_struct_t;

typedef enum
{
    UPDATE_RECORD_ADD     = 1,
    UPDATE_RECORD_DELETE  = 2,
    UPDATE_RECORD_UNKNOWN = -1,
} update_record_op_t;

int Remote_cmd_process(uint8_t *cmd_str);

#ifdef __cplusplus
}
#endif

#endif /*_REMOTE_CMD_H_*/
