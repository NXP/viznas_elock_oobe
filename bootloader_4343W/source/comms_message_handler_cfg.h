/*
 * Copyright 2018, 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/**
 * @file comms_message_handler_cfg.h
 * @brief This file contains the configuration for software update
 */

#ifndef COMMS_MESSAGE_HANDLER_CFG_H_
#define COMMS_MESSAGE_HANDLER_CFG_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifndef COMMS_MESSAGE_HANDLER_AIS_SUPPORTED
#define COMMS_MESSAGE_HANDLER_AIS_SUPPORTED (0)
#endif

#ifndef COMMS_MESSAGE_HANDLER_FWUPDATE

#ifdef BOOTLOADER_FWUPDATE_ENABLE
#define COMMS_MESSAGE_HANDLER_FWUPDATE BOOTLOADER_FWUPDATE_ENABLE
#else
#define COMMS_MESSAGE_HANDLER_FWUPDATE (0)
#endif /*BOOTLOADER_FWUPDATE_ENABLE*/

#endif /*COMMS_MESSAGE_HANDLER_FWUPDATE*/

#if COMMS_MESSAGE_HANDLER_FWUPDATE

#ifndef COMMS_MESSAGE_HANDLER_FWUPDATE_INITIATION_SUPPORTED
#define COMMS_MESSAGE_HANDLER_FWUPDATE_INITIATION_SUPPORTED (0)
#endif /*COMMS_MESSAGE_HANDLER_FWUPDATE_INITIATION_SUPPORTED*/

#ifndef COMMS_MESSAGE_HANDLER_FWUPDATE_SERVER
#define COMMS_MESSAGE_HANDLER_FWUPDATE_SERVER (1)
#endif /*COMMS_MESSAGE_HANDLER_FWUPDATE_SERVER*/

#ifndef COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTA_TCP

#ifdef BOOTLOADER_TCP_OTA_ENABLE
#define COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTA_TCP (BOOTLOADER_TCP_OTA_ENABLE)
#else
#define COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTA_TCP (0)
#endif /*BOOTLOADER_TCP_OTA_ENABLE*/

#endif /*COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTA_TCP*/

#ifndef COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTW_UART

#ifdef BOOTLOADER_OTW_ENABLE
#define COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTW_UART (BOOTLOADER_OTW_ENABLE)
#else
#define COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTW_UART (0)
#endif /*BOOTLOADER_OTW_ENABLE*/

#endif /*COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTW_UART*/

#if (COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTA_TCP == 0) && (COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTW_UART == 0)
#warning "OTW and OTA are both disabled while fwupdate is enabled"
#endif

#endif /*COMMS_MESSAGE_HANDLER_FWUPDATE*/

#endif /* COMMS_MESSAGE_HANDLER_CFG_H_ */
