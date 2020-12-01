/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sln_viznas_iot_flexspi_nor_config.h"
#include "board.h"
/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(XIP_BOOT_HEADER_ENABLE) && (XIP_BOOT_HEADER_ENABLE == 1)
#if defined(__CC_ARM) || defined(__GNUC__)
__attribute__((section(".boot_hdr.conf")))
#elif defined(__ICCARM__)
#pragma location = ".boot_hdr.conf"
#endif
#if FLASH_TYPE == HYPER_FLASH
const flexspi_nor_config_t hyperflash_config = {
    .memConfig =
        {
            .tag = FLEXSPI_CFG_BLK_TAG,
            .version = FLEXSPI_CFG_BLK_VERSION,
            .readSampleClkSrc = kFlexSPIReadSampleClk_ExternalInputFromDqsPad,
            .csHoldTime = 3u,
            .csSetupTime = 3u,
            .columnAddressWidth = 3u,
            // Enable DDR mode, Wordaddassable, Safe configuration, Differential clock
            .controllerMiscOption = (1u << kFlexSpiMiscOffset_DdrModeEnable) |
                                    (1u << kFlexSpiMiscOffset_WordAddressableEnable) |
                                    (1u << kFlexSpiMiscOffset_SafeConfigFreqEnable),
            .sflashPadType = kSerialFlash_8Pads,
            .serialClkFreq = kFlexSpiSerialClk_100MHz,
            .sflashA1Size = 32u * 1024u * 1024u,
            .dataValidTime = {16u, 16u},
            .lookupTable =
                {
                    // Read LUTs
                    FLEXSPI_LUT_SEQ(CMD_DDR, FLEXSPI_8PAD, 0xA0, RADDR_DDR, FLEXSPI_8PAD, 0x18),
                    FLEXSPI_LUT_SEQ(CADDR_DDR, FLEXSPI_8PAD, 0x10, DUMMY_DDR, FLEXSPI_8PAD, 0x06),
                    FLEXSPI_LUT_SEQ(READ_DDR, FLEXSPI_8PAD, 0x04, STOP, FLEXSPI_1PAD, 0x0),
                },
        },
    .pageSize = 512u,
    .sectorSize = 256u * 1024u,
    .blockSize = 256u * 1024u,
    .isUniformBlockSize = true,
};
#elif FLASH_TYPE == QSPI_FLASH
#ifdef QSPI_FLASH_W25Q256
//w25Q256 config
const flexspi_nor_config_t qspiflash_w25q256_config = {
    .memConfig =
        {
            .tag = FLEXSPI_CFG_BLK_TAG,
            .version = FLEXSPI_CFG_BLK_VERSION,
            .readSampleClkSrc = kFlexSPIReadSampleClk_LoopbackFromDqsPad,
            .csHoldTime = 3u,
            .csSetupTime = 3u,
            // Enable DDR mode, Wordaddassable, Safe configuration, Differential clock
            .sflashPadType = kSerialFlash_4Pads,
            .serialClkFreq = kFlexSpiSerialClk_100MHz,
            .sflashA1Size = 32u * 1024u * 1024u,
            .lookupTable =
                {
                    // Read LUTs
                    FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0xEB, RADDR_SDR, FLEXSPI_4PAD, 0x18),
                    FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_4PAD, 0x06, READ_SDR, FLEXSPI_4PAD, 0x04),
                },
        },
    .pageSize = 256u,
    .sectorSize = 4u * 1024u,
    .blockSize = 256u * 1024u,
    .isUniformBlockSize = false,
};
#else
//w25Q128 config
const flexspi_nor_config_t qspiflash_w25q128_config = {
    .memConfig =
        {
            .tag              = FLEXSPI_CFG_BLK_TAG,
            .version          = FLEXSPI_CFG_BLK_VERSION,
            .readSampleClkSrc = kFlexSPIReadSampleClk_LoopbackFromDqsPad,
            .csHoldTime       = 3u,
            .csSetupTime      = 5u,
            // Enable DDR mode, Wordaddassable, Safe configuration, Differential clock
            .sflashPadType = kSerialFlash_4Pads,
            .serialClkFreq = kFlexSpiSerialClk_100MHz,
            .sflashA1Size  = 16u * 1024u * 1024u,
            .lookupTable =
					/*
						{
								// Read LUTs
								FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0xEB, RADDR_SDR, FLEXSPI_4PAD, 0x18),
								FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_4PAD, 0x06, READ_SDR, FLEXSPI_4PAD, 0x04),
						},
					*/
						{
								// Read LUTs
								FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x6B, RADDR_SDR, FLEXSPI_1PAD, 0x18),
								FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_4PAD, 0x08, READ_SDR, FLEXSPI_4PAD, 0x04),
						},
        },
    .pageSize           = 256u,
    .sectorSize         = 4u * 1024u,
    .blockSize          = 64u * 1024u,//256u * 1024u,
    .isUniformBlockSize = false,
};
#endif
#else
#error "***ERROR***"
#endif
#endif /* XIP_BOOT_HEADER_ENABLE */
