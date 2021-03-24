/*
 * Copyright 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef SLN_TCP_CONNECTION_h
#define SLN_TCP_CONNECTION_h

#define TCP_COMMS_PORT      (8889)
#define TCP_MAX_BUFFER_SIZE (1024)
#define TCP_COMMS_TIMEOUT   (3000)

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _tcp_connection_status
{
    kCommon_Success = 0,
    kCommon_Failed,
    kCommon_ToManyBytes,
    kCommon_NoDataRead,
    kCommon_ConnectionLost,
} tcp_connection_status_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void TCP_Connection_Start(void);

#ifdef __cplusplus
}
#endif
#endif
