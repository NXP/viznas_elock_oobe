/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_nor_disk.h"
#include "sln_flash.h"
#include "flexspi_hyper_flash_ops.h"
#include "fsl_debug_console.h"

static BYTE fatfs_cache[FATFS_BLOCK_SIZE_BYTES];

DRESULT nor_disk_status(BYTE pdrv)
{
     DSTATUS stat = RES_OK;

     if( pdrv != NORDISK)
     {
         return STA_NOINIT;
     }

     return stat;
}

DRESULT nor_disk_initialize(BYTE pdrv)
{
    DSTATUS stat = RES_OK;

    if (pdrv != NORDISK)
    {
        return STA_NOINIT;
    }

    SLN_Flash_Init();

    return stat;
}

DRESULT nor_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    DSTATUS stat = RES_OK;

    if (pdrv != NORDISK)
    {
        return RES_PARERR;
    }
    sector += FATFS_SECTOR_OFFSET;
    SLN_Read_Flash_At_Address(sector * FATFS_SECTOR_SIZE, buff, count * FATFS_SECTOR_SIZE);

    return stat;
}

uint32_t is_filed_used(uint32_t addr_beigin, uint32_t addr_end)
{
    char* addr;
    for(addr = (char*)addr_beigin; addr <= (char*)addr_end; addr++) {
        if(*(addr+FlexSPI_AMBA_BASE) != 0xFF)
            return 1;
    }
    return 0;
}

static void ram_memcpy(void *dst, void *src, size_t len)
{
    uint8_t *ptrDst = (uint8_t *)dst;
    uint8_t *ptrSrc = (uint8_t *)src;

    while (len)
    {
        *ptrDst++ = *ptrSrc++;
        len--;
    }
}

DRESULT nor_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    DSTATUS stat = RES_OK;

    if (pdrv != NORDISK)
    {
        return RES_PARERR;
    }

    sector += FATFS_SECTOR_OFFSET;

    //write start addr.
    uint32_t addr_begin = sector * FATFS_SECTOR_SIZE;

    //write size.
    uint32_t write_size = count * FATFS_SECTOR_SIZE;

    uint32_t sector_addr_begin = (addr_begin / FATFS_BLOCK_SIZE_BYTES) * FATFS_BLOCK_SIZE_BYTES;

    if ((addr_begin >= sector_addr_begin) &&
            (addr_begin + write_size - 1) < (sector_addr_begin + FATFS_BLOCK_SIZE_BYTES)) // one sector operate
    {
        uint32_t sector_offset = addr_begin - sector_addr_begin;
        if( !is_filed_used(addr_begin, addr_begin + write_size -1) ) //not need erase
        {
            for(int i = 0; i < write_size; i += FATFS_SECTOR_SIZE)
            {
                if( i + FATFS_SECTOR_SIZE < write_size)
                    SLN_Write_Flash_Page(addr_begin + i, (uint8_t *)buff + i, FATFS_SECTOR_SIZE);
                else
                    SLN_Write_Flash_Page(addr_begin + i, (uint8_t *)buff + i, write_size - i);
            }
        }
        else//need erase
        {
            SLN_Read_Flash_At_Address(sector_addr_begin, fatfs_cache, FATFS_BLOCK_SIZE_BYTES);
            ram_memcpy(fatfs_cache+sector_offset, (uint8_t *)buff, write_size);
            SLN_Erase_Sector(sector_addr_begin);
            for(int i = 0; i < FATFS_BLOCK_SIZE_BYTES; i+=FATFS_SECTOR_SIZE)
            {
                SLN_Write_Flash_Page(sector_addr_begin + i, fatfs_cache + i, FATFS_SECTOR_SIZE);
            }
        }
    }
    else// multiple write
    {
        assert(0);
        PRINTF("multiple write.\r\n");
    }

    return stat;
}


DRESULT nor_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DSTATUS stat = RES_OK;

    if (pdrv != NORDISK)
    {
     return RES_PARERR;
    }

    switch (cmd)
    {
     case GET_SECTOR_COUNT:
         if (buff)
         {
             *(uint32_t *)buff = FATFS_SECTOR_COUNT;
         }
         else
         {
             stat = RES_PARERR;
         }
         break;
     case GET_SECTOR_SIZE:
         if (buff)
         {
             *(uint32_t *)buff = FATFS_SECTOR_SIZE;
         }
         else
         {
             stat = RES_PARERR;
         }
         break;
     case GET_BLOCK_SIZE:
         if (buff)
         {
             *(uint32_t *)buff = FATFS_BLOCK_SIZE;
         }
         else
         {
             stat = RES_PARERR;
         }
         break;
     case CTRL_SYNC:
         stat = RES_OK;
         break;
     default:
         stat = RES_PARERR;
         break;
    }
    return stat;
}


