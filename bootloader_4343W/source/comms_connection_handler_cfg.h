/*
 * Copyright 2018, 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/**
 * @file comms_connection_handler_cfg.h
 * @brief This file contains the connections configuration
 */

#ifndef COMMS_CONNECTION_HANDLER_CFG_H_
#define COMMS_CONNECTION_HANDLER_CFG_H_

#include "comms_message_handler_cfg.h"

#if COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTW_UART
#include "board.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define OTW_MAX_BUFFER_SIZE (10000)
#define OTA_TCP_COMMS_PORT  (8889)
#define OTA_MAX_BUFFER_SIZE (10000)

#if COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTW_UART
#define FWUPDATE_OTW_UART_BASEADDR BOARD_DEBUG_UART_BASEADDR
#define FWUPDATE_OTW_UART_BAUDRATE BOARD_DEBUG_UART_BAUDRATE
#define FWUPDATE_OTW_UART_SRCFREQ  BOARD_DebugConsoleSrcFreq()
#endif

#endif /* COMMS_CONNECTION_HANDLER_CFG_H_ */
