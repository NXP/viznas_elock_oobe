/*
 * Copyright 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software
 */

#ifndef _SLN_FLASH_CONFIG_H_
#define _SLN_FLASH_CONFIG_H_

#include "board.h"

#if FLASH_TYPE == HYPER_FLASH
#include "sln_hyper_flash_config.h"
#else
#include "sln_qspi_flash_config.h"
#endif

#endif /* _SLN_FLASH_CONFIG_H_ */
