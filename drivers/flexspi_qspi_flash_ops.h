/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FLEXSPI_QSPI_FLASH_OPS_H_
#define _FLEXSPI_QSPI_FLASH_OPS_H_
#include "sln_flash_config.h"

#if FLASH_TYPE == QSPI_FLASH

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/


status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address);

status_t flexspi_nor_flash_page_program_with_buffer(FLEXSPI_Type *base, uint32_t address, const uint32_t *src);

status_t flexspi_nor_qspi_flash_id(FLEXSPI_Type *base, uint16_t pid);

#endif
#endif /* _FLEXSPI_QSPI_FLASH_OPS_H_ */
