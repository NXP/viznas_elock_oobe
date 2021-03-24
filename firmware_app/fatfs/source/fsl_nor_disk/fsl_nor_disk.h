/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FATFS_DRIVER_FSL_NOR_DISK_H_
#define FATFS_DRIVER_FSL_NOR_DISK_H_

#include "ff.h"
#include "diskio.h"

#define FATFS_FLASH_REGION_SIZE_BYTES (FLASH_SIZE/16)
#define FASFS_FLASH_REGION_OFFSET (FLASH_SIZE - FATFS_FLASH_REGION_SIZE_BYTES)

#define FATFS_SECTOR_SIZE  (FLASH_PAGE_SIZE)
#define FATFS_SECTOR_COUNT (FATFS_FLASH_REGION_SIZE_BYTES/FATFS_SECTOR_SIZE)
#define FATFS_BLOCK_SIZE   (SECTOR_SIZE/FATFS_SECTOR_SIZE)


#define FATFS_BLOCK_SIZE_BYTES (FATFS_BLOCK_SIZE*FATFS_SECTOR_SIZE)

#define FATFS_SECTOR_OFFSET (FASFS_FLASH_REGION_OFFSET/FATFS_SECTOR_SIZE)

DRESULT nor_disk_status(BYTE pdrv);


DRESULT nor_disk_initialize(BYTE pdrv);


DRESULT nor_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);


DRESULT nor_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);


DRESULT nor_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff);


#endif /* FATFS_DRIVER_FSL_NOR_DISK_H_ */
