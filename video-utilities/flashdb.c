/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */
#include "flashdb.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "commondef.h"
#include "sln_flash.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PAGE_ADDR(x)   ((x)*FACEREC_FS_FLASH_PAGE_SIZE)
#define SECTOR_ADDR(x) ((x)*FLASH_SECTOR_SIZE)

/*******************************************************************************
 * Variables
 *******************************************************************************/
static uint8_t s_DataCache[FLASH_SECTOR_SIZE];

/*******************************************************************************
 * Code
 ******************************************************************************/
uint32_t Flash_FacerecFsEraseMapBlock(void)
{
    status_t status = SLN_Erase_Sector(FACEREC_FS_MAP_ADDR);
    if (status != FLASH_OK) {
        return FLASH_ERR;
    }

    return FLASH_OK;
}

uint32_t Flash_FacerecFsEraseItemBlock(void)
{
    status_t status;

    for (int i = 0; i < FACEREC_FS_SECTORS; i++) {
        status = SLN_Erase_Sector(FACEREC_FS_ITEM_ADDR + SECTOR_ADDR(i));
        if (status != FLASH_OK) {
            return FLASH_ERR;
        }
    }

    return FLASH_OK;
}

uint32_t Flash_FacerecFsEraseAllBlock(void)
{
    status_t status;
    status = Flash_FacerecFsEraseMapBlock();
    if (status != FLASH_OK) {
        return FLASH_ERR;
    }
    status = Flash_FacerecFsEraseItemBlock();

    if (status != FLASH_OK) {
        return FLASH_ERR;
    }

    return FLASH_OK;
}

uint32_t Flash_FacerecFsReadMapMagic(FeatureMap *pMap)
{
    status_t status;

    status = SLN_Read_Flash_At_Address(FACEREC_FS_MAP_ADDR, (uint8_t *)pMap, sizeof(FeatureMap));
    if (status != FLASH_OK)
    {
        return FLASH_ERR;
    }

    return FLASH_OK;
}

uint8_t Flash_FacerecFsReadMagicValue(int index)
{
    status_t status;
    uint8_t magic_temp[4];

    status = SLN_Read_Flash_At_Address(FACEREC_FS_MAP_ADDR + (index & ~0x03), magic_temp, 4);
    if (status != FLASH_OK)
    {
        return -1;
    }

    return magic_temp[index % 4];
}

uint32_t Flash_FacerecFsReadItem(int index, FeatureItem *pItem)
{
    status_t status;

    int size_page = (sizeof(FeatureItem) % FACEREC_FS_FLASH_PAGE_SIZE) ?
                        (sizeof(FeatureItem) / FACEREC_FS_FLASH_PAGE_SIZE + 1) :
                        (sizeof(FeatureItem) / FACEREC_FS_FLASH_PAGE_SIZE);
    int offset_page = index * size_page;

    status = SLN_Read_Flash_At_Address(FACEREC_FS_ITEM_ADDR + PAGE_ADDR(offset_page), (uint8_t *)pItem,
                                       size_page * FACEREC_FS_FLASH_PAGE_SIZE);
    if (status != FLASH_OK)
    {
        return FLASH_ERR;
    }

    return FLASH_OK;
}

uint32_t Flash_FacerecFsReadItemHeader(int index, FeatureItem *pItem)
{
    status_t status;
    int offset = offsetof(FeatureItem,feature);
    int size_page = (sizeof(FeatureItem) % FACEREC_FS_FLASH_PAGE_SIZE) ?
                        (sizeof(FeatureItem) / FACEREC_FS_FLASH_PAGE_SIZE + 1) :
                        (sizeof(FeatureItem) / FACEREC_FS_FLASH_PAGE_SIZE);
    int offset_page = index * size_page;

    status = SLN_Read_Flash_At_Address(FACEREC_FS_ITEM_ADDR + PAGE_ADDR(offset_page), (uint8_t *)pItem,
    		offset);
    if (status != FLASH_OK)
    {
        return FLASH_ERR;
    }

    return FLASH_OK;
}


uint32_t Flash_FacerecFsReadIDFeaturePointer(int index, uint16_t *id, void** featurePointer)
{
    status_t status;
    int offset = offsetof(FeatureItem,id);
    int size_page = (sizeof(FeatureItem) % FACEREC_FS_FLASH_PAGE_SIZE) ?
                        (sizeof(FeatureItem) / FACEREC_FS_FLASH_PAGE_SIZE + 1) :
                        (sizeof(FeatureItem) / FACEREC_FS_FLASH_PAGE_SIZE);
    int offset_page = index * size_page;

    status = SLN_Read_Flash_At_Address(FACEREC_FS_ITEM_ADDR + PAGE_ADDR(offset_page) + offset, (uint8_t *)id,
    								sizeof(*id));
    *featurePointer =
    		(void*)SLN_Flash_Get_Read_Address(FACEREC_FS_ITEM_ADDR + PAGE_ADDR(offset_page) + offsetof(FeatureItem,feature));
    if (status != FLASH_OK)
    {
        return FLASH_ERR;
    }

    return FLASH_OK;
}

uint32_t Flash_FacerecFsUpdateMapMagic(int index, FeatureMap *pMap, bool needErase)
{
    status_t status;
//    int size_page = (sizeof(FeatureMap) % FACEREC_FS_FLASH_PAGE_SIZE) ?
//                    (sizeof(FeatureMap) / FACEREC_FS_FLASH_PAGE_SIZE + 1) :
//                    (sizeof(FeatureMap) / FACEREC_FS_FLASH_PAGE_SIZE);
//    int size_sector = (size_page % FLASH_NPAGE_PER_SECTOR) ?
//                    (size_page / FLASH_NPAGE_PER_SECTOR + 1) :
//                    (size_page / FLASH_NPAGE_PER_SECTOR);
    int offset_page = index / FACEREC_FS_FLASH_PAGE_SIZE;
    int offset_sector = offset_page / FLASH_NPAGE_PER_SECTOR;

    if (needErase)
    {
        SLN_Read_Flash_At_Address(FACEREC_FS_MAP_ADDR + SECTOR_ADDR(offset_sector), s_DataCache, FLASH_SECTOR_SIZE);
        memcpy(s_DataCache, pMap + SECTOR_ADDR(offset_sector), sizeof(FeatureMap) - SECTOR_ADDR(offset_sector));
        SLN_Erase_Sector(FACEREC_FS_MAP_ADDR + SECTOR_ADDR(offset_sector));

        for (int i = 0; i < FLASH_SECTOR_SIZE / FACEREC_FS_FLASH_PAGE_SIZE; i++)
        {
            status = SLN_Write_Flash_Page(FACEREC_FS_MAP_ADDR + SECTOR_ADDR(offset_sector) + PAGE_ADDR(i),
                                         (uint8_t *)s_DataCache + i * FACEREC_FS_FLASH_PAGE_SIZE,
                                         FACEREC_FS_FLASH_PAGE_SIZE);
            {
                return FLASH_ERR;
            }
        }

    }
    else
    {
        status = SLN_Write_Flash_Page(FACEREC_FS_MAP_ADDR + PAGE_ADDR(offset_page), (uint8_t *)pMap + PAGE_ADDR(offset_page), FACEREC_FS_FLASH_PAGE_SIZE);
        if (status != FLASH_OK)
        {
            return FLASH_ERR;
        }
    }
    return FLASH_OK;
}

uint32_t Flash_FacerecFsUpdateItem(int index, FeatureItem *pItem, bool needErase)
{
    status_t status;
    int size_page = (sizeof(FeatureItem) % FACEREC_FS_FLASH_PAGE_SIZE) ?
                        (sizeof(FeatureItem) / FACEREC_FS_FLASH_PAGE_SIZE + 1) :
                        (sizeof(FeatureItem) / FACEREC_FS_FLASH_PAGE_SIZE);
    int offset_page   = index * size_page;
    int offset_sector = offset_page / FLASH_NPAGE_PER_SECTOR;

    if (needErase)
    {
        int offset_in_sector = (offset_page - offset_sector * FLASH_NPAGE_PER_SECTOR) * FACEREC_FS_FLASH_PAGE_SIZE;

        SLN_Read_Flash_At_Address(FACEREC_FS_ITEM_ADDR + SECTOR_ADDR(offset_sector), s_DataCache, FLASH_SECTOR_SIZE);

        memcpy(&s_DataCache[offset_in_sector], pItem, sizeof(FeatureItem));

        SLN_Erase_Sector(FACEREC_FS_ITEM_ADDR + SECTOR_ADDR(offset_sector));

        for (int i = 0; i < FLASH_SECTOR_SIZE / FACEREC_FS_FLASH_PAGE_SIZE; i++)
        {
            status =
                SLN_Write_Flash_Page(FACEREC_FS_ITEM_ADDR + SECTOR_ADDR(offset_sector) + PAGE_ADDR(i),
                                     (uint8_t *)s_DataCache + i * FACEREC_FS_FLASH_PAGE_SIZE,
                                     FACEREC_FS_FLASH_PAGE_SIZE);
            if (status != FLASH_OK)
            {
                return FLASH_ERR;
            }
        }
    }
    else
    {
        for (int i = 0; i < size_page; i++)
        {
            status =
                SLN_Write_Flash_Page(FACEREC_FS_ITEM_ADDR + PAGE_ADDR(offset_page + i),
                                     (uint8_t *)pItem + i * FACEREC_FS_FLASH_PAGE_SIZE, FACEREC_FS_FLASH_PAGE_SIZE);
            if (status != FLASH_OK)
            {
                return FLASH_ERR;
            }
        }
    }
    return FLASH_OK;
}
