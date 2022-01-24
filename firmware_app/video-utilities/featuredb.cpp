/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#include "featuredb.h"
#include "fsl_log.h"
#include "sln_shell.h"
#include "commondef.h"
#include "flashdb.h"

//static FeatureItem s_itemTemp;
static uint8_t s_DataCache[FLASH_SECTOR_SIZE];

FeatureDB::FeatureDB()
{
        
	this->itemSize = sizeof(FeatureItem) + OASISLT_getFaceItemSize();
	int page_cnt = (this->itemSize + FACEREC_FS_FLASH_PAGE_SIZE - 1)/FACEREC_FS_FLASH_PAGE_SIZE;
	page_cnt = 1<<(int)ceil(log2(page_cnt));

	this->itemSpaceSize = page_cnt*FACEREC_FS_FLASH_PAGE_SIZE;
	this->itemBuff = pvPortMalloc(itemSpaceSize);

	Flash_FacerecFsInit(sizeof(FeatureItemHeader), this->itemSize, this->itemSpaceSize);

	//This line must below Flash_FacerecFsInit
	this->count = this->feature_count();
}

FeatureDB::~FeatureDB()
{
    this->itemSize = 0;
    this->itemSpaceSize = 0;

    if (this->itemBuff != NULL)
    {
        vPortFree(this->itemBuff);
    }
}

void FeatureDB::clear_item(void)
{
    memset(this->itemBuff, 0x00, this->itemSize);
}

int FeatureDB::get_idxById(int id, int &index)
{
	int i;
    FeatureItemHeader *pItemHeader = (FeatureItemHeader *)this->itemBuff;
    for (i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {

        Flash_FacerecFsReadItemHeader(i, pItemHeader);
        if(pItemHeader->magic == FEATUREDATA_MAGIC_VALID)
        {
            if (pItemHeader->id == id)
            {
                index = i;
                break;
            }
        }
    }

    if (i < FEATUREDATA_MAX_COUNT)
    {
        return 0;
    }

    return -1;
}

int FeatureDB::get_idxByName(const std::string name, int &index)
{
    int i;
    FeatureItemHeader *pItemHeader = (FeatureItemHeader *)this->itemBuff;
    for (i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {

        Flash_FacerecFsReadItemHeader(i, pItemHeader);
        if(pItemHeader->magic == FEATUREDATA_MAGIC_VALID)
        {
            if (memcmp(pItemHeader->name, name.c_str(), strlen(pItemHeader->name)) == 0)
            {
                index = i;
                break;
            }
        }
    }

    if (i < FEATUREDATA_MAX_COUNT)
    {
        return 0;
    }

    return -1;
}

int FeatureDB::get_free(int &index)
{
	int idx_first_deleted = -1;
	int i;
    FeatureItemHeader *pItemHeader = (FeatureItemHeader *)this->itemBuff;
    for (i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {

        Flash_FacerecFsReadItemHeader(i, pItemHeader);
        //record the 1st deleted index
        if (idx_first_deleted == -1 && pItemHeader->magic == FEATUREDATA_MAGIC_DELET)
        {
        	idx_first_deleted = i;
        }

        if(pItemHeader->magic == FEATUREDATA_MAGIC_UNUSE)
        {
            index = i;
            break;
        }
    }

    if (i < FEATUREDATA_MAX_COUNT)
    {
    	//we got a unused slot
        return 0;
    }

    if (idx_first_deleted != -1)
    {
    	//it's time to do reassign
		reassign_feature();
		index = idx_first_deleted;
		return 0;
    }else
    {
    	//no deleted item
    	return -2;

    }
}

int FeatureDB::add_feature(uint16_t id, const std::string name, float *feature)
{
    int index = -1;

    if (get_free(index))
    {
        return -1;
    }

    clear_item();
    FeatureItem *pItem = (FeatureItem *)this->itemBuff;
    pItem->magic = FEATUREDATA_MAGIC_VALID;
    pItem->id = id;
    pItem->index = index;
    memcpy(pItem->name, name.c_str(), name.length() + 1);
    memcpy(pItem->feature, feature, OASISLT_getFaceItemSize());

    if (Flash_FacerecFsWriteItem(index, pItem) != FLASH_OK)
    {
        return -2;
    }

    this->count++;

    return 0;
}

int FeatureDB::del_feature(const std::string name)
{
    int index = -1;
    if (get_idxByName(name, index))
    {
        return -1;
    }

    FeatureItemHeader *pItemHeader = (FeatureItemHeader *)this->itemBuff;
    if (Flash_FacerecFsReadItemHeader(index, pItemHeader) != FLASH_OK)
    {
        return -2;
    }

    pItemHeader->magic = FEATUREDATA_MAGIC_DELET;
    if (Flash_FacerecFsWriteItemHeader(index, pItemHeader) != FLASH_OK)
    {
        return -3;
    }

    this->count--;

    return 0;
}



int FeatureDB::del_feature_all()
{
    FeatureItemHeader *pItemHeader = (FeatureItemHeader *)this->itemBuff;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {

        if (Flash_FacerecFsReadItemHeader(i, pItemHeader) != FLASH_OK)
        {
            return -1;
        }

        if (pItemHeader->magic == FEATUREDATA_MAGIC_VALID)
        {

        	pItemHeader->magic = FEATUREDATA_MAGIC_DELET;
			if (Flash_FacerecFsWriteItemHeader(i, pItemHeader) != FLASH_OK)
			{
				return -2;
			}
        }
    }

    this->count = 0;

    reassign_feature();

    return 0;
}

int FeatureDB::update_feature(uint16_t id, float *feature)
{
    int indexId = -1;//, indexName = -1;
    if (get_idxById(id, indexId))
    {
        return -1;
    }

    FeatureItem *pItem = (FeatureItem *)this->itemBuff;
    if (Flash_FacerecFsReadItem(indexId, pItem) != FLASH_OK)
    {
        return -3;
    }

    //delete old item firstly
    pItem->magic = FEATUREDATA_MAGIC_DELET;
    if (Flash_FacerecFsWriteItemHeader(indexId, pItem) != FLASH_OK)
    {
        return -4;
    }


    int indexFree = -1;
    if (get_free(indexFree))
    {
        return -2;
    }

    pItem->magic = FEATUREDATA_MAGIC_VALID;
    pItem->index = indexFree;
    memcpy(pItem->feature, feature, OASISLT_getFaceItemSize());
    if (Flash_FacerecFsWriteItem(indexFree, pItem) != FLASH_OK)
    {
        return -5;
    }

    return 0;
}

int FeatureDB::get_names(std::vector<std::string> &names,int count)
{

    FeatureItemHeader * pItemHeader = (FeatureItemHeader *)this->itemBuff;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {

        if (Flash_FacerecFsReadItemHeader(i, pItemHeader) != FLASH_OK)
        {
        	return -1;
        }

        //UsbShell_DbgPrintf(VERBOSE_MODE_L1, "item:%d magic:%x\r\n", i,pItemHeader->magic);

        if (pItemHeader->magic == FEATUREDATA_MAGIC_VALID)
        {
            names.push_back(std::string(pItemHeader->name));
            if (names.size() >= count)
            {
                break;
            }
        }
    }

    return 0;
}

int FeatureDB::get_name(uint16_t id, std::string &name)
{
    FeatureItemHeader * pItemHeader = (FeatureItemHeader *)this->itemBuff;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {

        if (Flash_FacerecFsReadItemHeader(i, pItemHeader) != FLASH_OK)
        {
            return -1;
        }

        if (pItemHeader->magic == FEATUREDATA_MAGIC_VALID)
        {
            if (pItemHeader->id == id)
            {
                name = std::string(pItemHeader->name);
                return 0;
            }
        }
    }

    return -1;
}

int FeatureDB::get_ids(std::vector<uint16_t> &ids)
{
    FeatureItemHeader * pItemHeader = (FeatureItemHeader *)this->itemBuff;
	ids.clear();
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (Flash_FacerecFsReadItemHeader(i, pItemHeader) != FLASH_OK)
        {
            return -1;

        }

        if (pItemHeader->magic == FEATUREDATA_MAGIC_VALID)
        {
            ids.push_back(pItemHeader->id);
        }
    }

    return 0;
}

int FeatureDB::ren_name(const std::string oldname, const std::string newname)
{
    int indexOld = -1, indexNew = -1;
    if (get_idxByName(oldname, indexOld))
    {
        return -1;
    }

    //check for duplicate new name
    if (get_idxByName(newname, indexNew) == 0)
    {
        return -2;
    }

    FeatureItem *pItem = (FeatureItem *)this->itemBuff;
    if (Flash_FacerecFsReadItem(indexOld, pItem) != FLASH_OK)
    {
        return -4;
    }

    pItem->magic = FEATUREDATA_MAGIC_DELET;
    if (Flash_FacerecFsWriteItemHeader(indexOld, pItem) != FLASH_OK)
    {
        return -5;
    }

    if (get_free(indexNew))
    {
        return -3;
    }

    pItem->magic = FEATUREDATA_MAGIC_VALID;
    pItem->index = indexNew;
    memcpy(pItem->name, newname.c_str(), sizeof(pItem->name));
    if (Flash_FacerecFsWriteItem(indexNew, pItem) != FLASH_OK)
    {
        return -5;
    }

    return 0;
}

int FeatureDB::get_count()
{
	return this->count;
}

int FeatureDB::feature_count()
{
    int count = 0;
    FeatureItemHeader * pItemHeader = (FeatureItemHeader *)this->itemBuff;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {

        if (Flash_FacerecFsReadItemHeader(i, pItemHeader) != FLASH_OK)
        {
            return -1;
        }

        if (pItemHeader->magic == FEATUREDATA_MAGIC_VALID)
        {
            count++;
        }
    }

    return count;
}

int FeatureDB::get_feature(uint16_t id, float *feature)
{
    FeatureItem *pItem = (FeatureItem *)this->itemBuff;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {

        if (Flash_FacerecFsReadItem(i, pItem) != FLASH_OK)
        {
            return -1;
        }

        if (pItem->magic == FEATUREDATA_MAGIC_VALID)
        {
            if (pItem->id == id)
            {
                memcpy(feature, pItem->feature, OASISLT_getFaceItemSize());
                return 0;
            }
        }
    }

    return -1;
}


//check each sector status.
//all item  is unused/valid, no action
//all item is deleted, sector erase only
//unused + deleted, sector erase only
//valid + deleted, sector erase,write
//valid + deleted + unused, sector erase, write
int FeatureDB::reassign_feature()
{
    int itemsPerSector = FLASH_SECTOR_SIZE / this->itemSpaceSize;
    int sectors = (FEATUREDATA_MAX_COUNT % itemsPerSector) ?
            (FEATUREDATA_MAX_COUNT / itemsPerSector + 1) :
            (FEATUREDATA_MAX_COUNT / itemsPerSector);

    FeatureItem *pItem = (FeatureItem *)this->itemBuff;

    for (int i = 0; i < sectors; i++)
    {
        int ItemDeleted = 0;
        int ItemUnused = 0;
        int ItemValid = 0;

        if (Flash_FacerecFsReadSector(i , s_DataCache) != FLASH_OK)
        {
            return -1;
        }


        for (int j = 0; j < itemsPerSector; j ++)
        {
            int index = i * itemsPerSector + j;

            if (index >= FEATUREDATA_MAX_COUNT)
            {
            	break;
            }

            if (Flash_FacerecFsReadItem(index, pItem) != FLASH_OK)
            {
                return -2;
            }

        	//UsbShell_DbgPrintf(VERBOSE_MODE_L1, "sector:%d item:%d magic:%x\r\n", i, index,pItem->magic);

            if (pItem->magic == FEATUREDATA_MAGIC_DELET)
            {
                memset(s_DataCache + j * this->itemSpaceSize, 0xff, this->itemSpaceSize);
                ItemDeleted++;
            }else if (pItem->magic == FEATUREDATA_MAGIC_VALID)
            {
            	ItemValid++;

            }else if(pItem->magic == FEATUREDATA_MAGIC_UNUSE)
            {
            	ItemUnused++;
            }
        }

        if (ItemDeleted == 0)
        {
        	//do nothing
        }else if (ItemValid == 0)
        {
        	//erase only
            if (Flash_FacerecFsEraseSector(i) != FLASH_OK)
            {
                return -3;
            }
        }else
        {
        	//erase and write
            if (Flash_FacerecFsWriteSector(i, s_DataCache) != FLASH_OK)
            {
                return -4;
            }
        }

    }

    return 0;
}

int FeatureDB::save_feature(int index)
{
    return 0;
}

int FeatureDB::erase_feature(int index)
{
    return 0;
}

int FeatureDB::load_feature()
{
    return 0;
}

void FeatureDB::set_autosave(bool auto_save)
{
    this->auto_save = auto_save;
}

bool FeatureDB::get_autosave()
{
    return this->auto_save;
}

int FeatureDB::get_ID_featurePointers(uint16_t* ids, void**featureP, int num)
{
    int index = 0;
    FeatureItemHeader *pItemHeader = (FeatureItemHeader *)this->itemBuff;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (index >= num)
        {
            break;
        }

        if (Flash_FacerecFsReadItemHeader(i, pItemHeader) != FLASH_OK)
        {
            return -1;
        }

        if (pItemHeader->magic == FEATUREDATA_MAGIC_VALID)
        {
            ids[index] = pItemHeader->id;
            if (Flash_FacerecFsGetFeatureAddress(i, &featureP[index]) != FLASH_OK)
            {
                return -1;
            }

            index++;
        }
    }

    return 0;
}

int FeatureDB::database_save(int count)
{
    return 0;
}
