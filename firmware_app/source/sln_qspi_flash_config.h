/*
 * Copyright 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software
 */

#ifndef _SLN_QSPI_FLASH_CONFIG_H_
#define _SLN_QSPI_FLASH_CONFIG_H_

#include "board.h"

#if FLASH_TYPE == QSPI_FLASH

#define EXAMPLE_FLEXSPI FLEXSPI
#define FLASH_SIZE BOARD_FLASH_SIZE
#define EXAMPLE_FLEXSPI_AMBA_BASE FlexSPI_AMBA_BASE
#define FLASH_PAGE_SIZE BOARD_FLASH_PAGE_SIZE
#define SECTOR_SIZE BOARD_FLASH_SECTOR_SIZE
#define EXAMPLE_FLEXSPI_CLOCK kCLOCK_FlexSpi
#define CUSTOM_LUT_LENGTH 60
#define FLASH_QUAD_ENABLE 0x40
#define FLASH_BUSY_STATUS_POL 1
#define FLASH_BUSY_STATUS_OFFSET 0

#define NOR_CMD_LUT_SEQ_IDX_READ_NORMAL                 0
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST                   1
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD              2
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE                 3
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD            4
#define NOR_CMD_LUT_SEQ_IDX_READJEDECID                 5
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG1              6
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG2              7
#define NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG2             8
#define NOR_CMD_LUT_SEQ_IDX_ERASECHIP                   9
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR                 10
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM                 11

#define NORFLASH_SIZE                   (FLASH_SIZE/1024)

#define NORFLASH_LUT_LENGTH             48
#define NORFLASH_ADDR_LENGTH            24

#define W25Q_WriteEnable                0x06
#define W25Q_WriteDisable               0x04
#define W25Q_ReadStatusReg1             0x05
#define W25Q_ReadStatusReg2             0x35
#define W25Q_WriteStatusReg1            0x01
#define W25Q_WriteStatusReg2            0x31
#define W25Q_ReadData                   0x03
#define W25Q_FastReadData               0x0B
#define W25Q_FastReadDual               0x3B
#define W25Q_FastReadQuad               0x6B
#define W25Q_PageProgram                0x02
#define W25Q_PageProgramQuad            0x32
#define W25Q_BlockErase                 0xD8
#define W25Q_SectorErase                0x20
#define W25Q_ChipErase                  0xC7
#define W25Q_PowerDown                  0xB9
#define W25Q_ReleasePowerDown           0xAB
#define W25Q_DeviceID                   0xAB
#define W25Q_ManufactDeviceID           0x90
#define W25Q_JedecDeviceID              0x9F

#endif
#endif /* _SLN_QSPI_FLASH_CONFIG_H_ */
