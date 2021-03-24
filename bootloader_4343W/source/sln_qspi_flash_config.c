/*
 * Copyright 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software
 */

#include "fsl_flexspi.h"
#include "sln_flash_config.h"

#if FLASH_TYPE == QSPI_FLASH
/*******************************************************************************
 * Variables
 ******************************************************************************/
const uint32_t customLUT[NORFLASH_LUT_LENGTH] = {
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_ReadData,
                        kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD,NORFLASH_ADDR_LENGTH),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04,
                        kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_FastReadData,
                        kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD,NORFLASH_ADDR_LENGTH),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, 0x08,
                        kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_FastReadQuad,
                        kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD,NORFLASH_ADDR_LENGTH),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_4PAD, 0x08,
                        kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0x04),

    [4 * NOR_CMD_LUT_SEQ_IDX_WRITEENABLE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_WriteEnable,
                        kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_PageProgramQuad,
                        kFLEXSPI_Command_RADDR_SDR,kFLEXSPI_1PAD, NORFLASH_ADDR_LENGTH),
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0x04,
                        kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    [4 * NOR_CMD_LUT_SEQ_IDX_READJEDECID] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_ManufactDeviceID,
                        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, NORFLASH_ADDR_LENGTH),
    [4 * NOR_CMD_LUT_SEQ_IDX_READJEDECID + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04,
                        kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUSREG1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_ReadStatusReg1,
                        kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUSREG2] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_ReadStatusReg2,
                        kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    [4 * NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG2] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_WriteStatusReg2,
                        kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04),

    [4 * NOR_CMD_LUT_SEQ_IDX_ERASECHIP] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_ChipErase,
                        kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    [4 * NOR_CMD_LUT_SEQ_IDX_ERASESECTOR] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_SectorErase,
                        kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD,NORFLASH_ADDR_LENGTH),

    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_PageProgram,
                        kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, NORFLASH_ADDR_LENGTH),
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04,
                        kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

	[4 * NOR_CMD_LUT_SEQ_IDX_ERASEBLOCK] =
		FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, W25Q_BlockErase,
						kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD,NORFLASH_ADDR_LENGTH),
};
#endif
