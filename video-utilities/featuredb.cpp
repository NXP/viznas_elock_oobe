/*
 * Copyright 2019-2020 NXP.
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

#include "commondef.h"
#if RTFFI_USE_FATFS
#include "fatfs_op.h"

#define FEATURE_FILE_ID_LENGTH  10
#define FEATURE_FILE_SPLIT_FLAG ,

uint8_t remote_change_fs;

static int isValidFeatureFile(char *filename)
{
    int len    = strlen(filename);
    char *p_fn = filename;

    if (len < FEATURE_FILE_ID_LENGTH)
        return 0;

    for (int i = 0; i < FEATURE_FILE_ID_LENGTH; i++)
    {
        if ((*(p_fn + i) > '9') || (*(p_fn + i) < '0'))
            return 0;
    }

    p_fn += FEATURE_FILE_ID_LENGTH;
    if (*p_fn == ',')
        return 1;
    else
        return 0;
}

FeatureDB::FeatureDB()
{
    std::vector<std::string> filename;

#if 0
    Flash_FacerecFsInit();
#endif

    fatfs_mount_with_mkfs();

    fatfs_mkdir("/snapshot");
    fatfs_write("/snapshot/test", "use for test.", 0, strlen("use for test."));
}

FeatureDB::~FeatureDB()
{
}

int FeatureDB::add_feature(uint16_t id, const std::string name, float *feature)
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    char file_name[255];
    sprintf(file_name, "%010d,%s", id, name.c_str());
    fatfs_write(file_name, (const char *)feature, 0, OASISLT_getFaceItemSize());

    return 0;
}

int FeatureDB::update_feature(uint16_t id, const std::string name, float *feature)
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    char file_name[255];
    sprintf(file_name, "%010d,%s", id, name.c_str());
    fatfs_write(file_name, (const char *)feature, 0, OASISLT_getFaceItemSize());

    return 0;
}

int FeatureDB::del_feature(uint16_t id, std::string name)
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    char file_name[255];
    sprintf(file_name, "%010d,%s", id, name.c_str());
    fatfs_delete(file_name);

    return 0;
}

int FeatureDB::del_feature(const std::string name)
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    // 1.find id
    int ret;
    char file_tmp[255];

    fatfs_opendir("/");

    // find valid item
    for (;;)
    {
        memset(file_tmp, 0, 255);
        ret = fatfs_readdir(file_tmp);
        if (ret == 0)
        {
            break;
        }
        else if (ret == 3)
        {
            if (isValidFeatureFile(file_tmp))
            {
                if (strcmp(file_tmp + 11, name.c_str()) == 0)
                {
                    fatfs_delete(file_tmp);
                }
            }
        }
    }

    fatfs_closedir();

    return 0;
}

int FeatureDB::del_feature_all()
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    int ret;
    char file_tmp[255];

    fatfs_opendir("/");

    // find valid item
    for (;;)
    {
        memset(file_tmp, 0, 255);
        ret = fatfs_readdir(file_tmp);
        if (ret == 0)
        {
            break;
        }
        else if (ret == 3)
        {
            if (isValidFeatureFile(file_tmp))
            {
                fatfs_delete(file_tmp);
            }
        }
    }

    fatfs_closedir();

    return 0;
}

std::vector<std::string> FeatureDB::get_names()
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    int ret;
    char file_tmp[255];
    std::vector<std::string> names;

    fatfs_opendir("/");

    // find valid item
    for (;;)
    {
        memset(file_tmp, 0, 255);
        ret = fatfs_readdir(file_tmp);
        if (ret == 0)
        {
            break;
        }
        else if (ret == 3)
        {
            if (isValidFeatureFile(file_tmp))
            {
                names.push_back(std::string(file_tmp + 11));
            }
        }
    }

    fatfs_closedir();

    return names;
}

int FeatureDB::get_name(uint16_t id, std::string &name)
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    int ret;
    char file_tmp[255];
    char id_str[11] = "";

    sprintf(id_str, "%010d", id);

    fatfs_opendir("/");

    // find valid item
    for (;;)
    {
        memset(file_tmp, 0, 255);
        ret = fatfs_readdir(file_tmp);
        if (ret == 0)
        {
            break;
        }
        else if (ret == 3)
        {
            if (isValidFeatureFile(file_tmp))
            {
                if (strncmp(id_str, file_tmp, 10) == 0)
                {
                    name = std::string(file_tmp + 11);
                }
            }
        }
    }

    fatfs_closedir();

    return 0;
}

std::vector<uint16_t> FeatureDB::get_ids()
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    int ret;
    char file_tmp[255];
    char id_str[11] = "";
    std::vector<uint16_t> ids;

    fatfs_opendir("/");

    // find valid item
    for (;;)
    {
        memset(file_tmp, 0, 255);
        ret = fatfs_readdir(file_tmp);
        if (ret == 0)
        {
            break;
        }
        else if (ret == 3)
        {
            if (isValidFeatureFile(file_tmp))
            {
                strncpy(id_str, file_tmp, 10);
                ids.push_back(atoi(id_str));
            }
        }
    }

    fatfs_closedir();

    return ids;
}

int FeatureDB::ren_name(const std::string oldname, const std::string newname)
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    int ret;
    char file_tmp[255];

    fatfs_opendir("/");

    // find valid item
    for (;;)
    {
        memset(file_tmp, 0, 255);
        ret = fatfs_readdir(file_tmp);
        if (ret == 0)
        {
            break;
        }
        else if (ret == 3)
        {
            if (isValidFeatureFile(file_tmp))
            {
                if (strcmp(file_tmp + 11, oldname.c_str()) == 0)
                {
                    std::string new_filename = std::string(file_tmp).replace(11, oldname.length(), newname);
                    fatfs_rename(file_tmp, new_filename.c_str());
                }
            }
        }
    }

    fatfs_closedir();

    return 0;
}

int FeatureDB::feature_count()
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    int ret;
    int count = 0;
    char file_tmp[255];

    fatfs_opendir("/");

    // find valid item
    for (;;)
    {
        memset(file_tmp, 0, 255);
        ret = fatfs_readdir(file_tmp);
        if (ret == 0)
        {
            break;
        }
        else if (ret == 3)
        {
            if (isValidFeatureFile(file_tmp))
                count++;
        }
    }

    fatfs_closedir();

    return count;
}

int FeatureDB::get_free(int &index)
{
    // discard
    return 0;
}

int FeatureDB::database_save(int count)
{
    // discard
    return 0;
}

int FeatureDB::get_feature(uint16_t id, float *feature)
{
    if (remote_change_fs)
    {
        fatfs_mount_with_mkfs();
        remote_change_fs = 0;
    }

    char file_name[255];
    std::string name;

    get_name(id, name);
    sprintf(file_name, "%010d,%s", id, name.c_str());

    fatfs_read(file_name, (char *)feature, 0, OASISLT_getFaceItemSize());

    return 0;
}

int FeatureDB::load_feature()
{
    return 0;
}

int FeatureDB::erase_feature(int index)
{
    // discard
    return 0;
}

int FeatureDB::save_feature(int index)
{
    // discard
    return 0;
}

int FeatureDB::reassign_feature()
{
    // discard
    return 0;
}

int FeatureDB::get_free_mapmagic()
{
    // discard
    return 0;
}

int FeatureDB::get_remain_map()
{
    // discard
    return 0;
}

#else
#include "flashdb.h"

//#define TEST_DB
#ifdef TEST_DB
#include "test_db_bin.h"
#endif

/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define SDRAM_DB 0

#ifdef TEST_DB
#undef SDRAM_DB
#define SDRAM_DB 1
#define FLASH_SECTOR_SIZE 0x1000
#endif
/*******************************************************************************
 * Variables
 *******************************************************************************/
static FeatureMap s_FeatureMap;


#if SDRAM_DB
static FeatureItem s_FeatureItem[FEATUREDATA_MAX_COUNT];
#endif

/*******************************************************************************
 * Code
 *******************************************************************************/

FeatureDB::FeatureDB()
{
    PRINTF("[DB]:start:0x%x size:0x%x\r\n", FACEREC_FS_FIRST_SECTOR * FLASH_SECTOR_SIZE,
           ((sizeof(FeatureItem) * FEATUREDATA_MAX_COUNT + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE + 1) *
               FLASH_SECTOR_SIZE);
#if 0
    Flash_FacerecFsInit();
#endif
    this->auto_save = false;
    load_feature();
    reassign_feature();
}

FeatureDB::~FeatureDB()
{
}

int FeatureDB::load_feature()
{
#ifdef TEST_DB
    // copy the map
    memcpy(&s_FeatureMap, test_db_bin, sizeof(s_FeatureMap));
    // copy the items
    memcpy(s_FeatureItem, test_db_bin + FLASH_SECTOR_SIZE, sizeof(s_FeatureItem));
#else
    memset(&s_FeatureMap, FEATUREDATA_MAGIC_UNUSE, sizeof(s_FeatureMap));
    Flash_FacerecFsReadMapMagic(&s_FeatureMap);
#if SDRAM_DB
    memset(s_FeatureItem, FEATUREDATA_MAGIC_UNUSE, sizeof(s_FeatureItem));
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        Flash_FacerecFsReadItem(i, &s_FeatureItem[i]);
    }
#endif
#endif
    return 0;
}

int FeatureDB::reassign_feature()
{
    int unuse_size     = 0;
    int valid_size     = 0;
    int delet_size     = 0;
    int other_size     = 0;
    int item_max = FEATUREDATA_MAX_COUNT;

    for (int i = 0; i < item_max; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_UNUSE)
        {
            unuse_size++;
        }
        else if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
            valid_size++;
        }
        else if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_DELET)
        {
            delet_size++;
        }
        else
        {
            other_size++;
        }
    }

    if (other_size > 0)
    {
        Flash_FacerecFsEraseAllBlock();
        LOGD("Flash is not clear, need to erase flash first!\r\n");
        return 0;
    }

    if (unuse_size > 0)
    {
        LOGD("space is enough, no need to reassign_feature!\r\n");
        return 0;
    }

    if (delet_size == 0)
    {
        LOGD("no deleted db, no need to reassign_feature!\r\n");
        return 0;
    }

    //remove the deleted db
    for (int i = 0; i < item_max; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_DELET)
        {
            s_FeatureMap.magic[i] = FEATUREDATA_MAGIC_UNUSE;
            FeatureItem item_t;
            memset(&item_t, FEATUREDATA_MAGIC_UNUSE, sizeof(item_t));
#if SDRAM_DB
            memset(&s_FeatureItem[i], FEATUREDATA_MAGIC_UNUSE, sizeof(item_t));
#endif
            Flash_FacerecFsUpdateItem(i, &item_t, true);
            Flash_FacerecFsUpdateMapMagic(i, &s_FeatureMap, true);
        }
    }
    return 0;
}

int FeatureDB::get_free_mapmagic()
{
    int item_max = FEATUREDATA_MAX_COUNT;
    // find new map index
    for (int i = 0; i < item_max; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_UNUSE)
            return i;
    }

    return -1;
}

int FeatureDB::get_remain_map()
{
    int item_max    = FEATUREDATA_MAX_COUNT;
    int remain_size = 0;

    // find new map index
    for (int i = 0; i < item_max; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_UNUSE)
            remain_size++;
    }

    return remain_size;
}

int FeatureDB::save_feature(int index)
{
    return 0;
}

int FeatureDB::erase_feature(int index)
{
    return 0;
}

int FeatureDB::ren_name(const std::string oldname, const std::string newname)
{
    int ret;
    int index = FEATUREDATA_MAX_COUNT;
    FeatureItem item_t;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
#if SDRAM_DB
            memcpy(&item_t, &s_FeatureItem[i], sizeof(item_t));
#else
            Flash_FacerecFsReadItem(i,&item_t);
#endif
            if (!strcmp(oldname.c_str(), item_t.name))
            {
                index = i;
                break;
            }
        }
    }

    if (index == FEATUREDATA_MAX_COUNT)
    {
        return -1;
    }

    ret = del_feature(item_t.id, item_t.name);
    if (ret == -1)
    {
        return ret;
    }
    strcpy(item_t.name, newname.c_str());

    ret = add_feature(item_t.id, item_t.name, item_t.feature);

    return ret;
}

std::vector<std::string> FeatureDB::get_names()
{
    FeatureItem item_t;
    std::vector<std::string> names;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
#if SDRAM_DB
            memcpy(&item_t, &s_FeatureItem[i], sizeof(item_t));
#else
            Flash_FacerecFsReadItemHeader(i,&item_t);
#endif
            names.push_back(std::string(item_t.name));
        }
    }
    return names;
}

int FeatureDB::feature_count()
{
    int count = 0;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
            count++;
        }
    }

    return count;
}

int FeatureDB::get_free(int &index)
{
    index = FEATUREDATA_MAX_COUNT;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_UNUSE)
        {
            index = i;
            break;
        }
    }

    if (index != FEATUREDATA_MAX_COUNT)
    {
        return 0;
    }

    return -1;
}

int FeatureDB::del_feature(uint16_t id, std::string name)
{
    int index = FEATUREDATA_MAX_COUNT;
    FeatureItem item_t;
 
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
#if SDRAM_DB
            memcpy(&item_t, &s_FeatureItem[i], sizeof(item_t));
#else
            Flash_FacerecFsReadItem(i,&item_t);
#endif
            if ((item_t.id == id) && (!strcmp(name.c_str(), item_t.name)))
            {
                index = i;
                break;
            }
        }
    }

    if (index == FEATUREDATA_MAX_COUNT)
        return -1;

    s_FeatureMap.magic[index] = FEATUREDATA_MAGIC_DELET;
    Flash_FacerecFsUpdateMapMagic(index, &s_FeatureMap, false);
    return 0;
}

int FeatureDB::del_feature(const std::string name)
{
    int index = FEATUREDATA_MAX_COUNT;
    FeatureItem item_t;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
#if SDRAM_DB
            memcpy(&item_t, &s_FeatureItem[i], sizeof(item_t));
#else
            Flash_FacerecFsReadItem(i,&item_t);
#endif
            if (!strcmp(name.c_str(), item_t.name))
            {
                index = i;
                break;
            }
        }
    }


    if (index == FEATUREDATA_MAX_COUNT)
        return -1;

    s_FeatureMap.magic[index] = FEATUREDATA_MAGIC_DELET;
    Flash_FacerecFsUpdateMapMagic(index, &s_FeatureMap, false);
    return 0;
}

int FeatureDB::del_feature_all()
{
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
            s_FeatureMap.magic[i] = FEATUREDATA_MAGIC_DELET;
            Flash_FacerecFsUpdateMapMagic(i, &s_FeatureMap, false);
        }
    }

    return 0;
}

int FeatureDB::database_save(int count)
{
    return 0;
}

int FeatureDB::add_feature(uint16_t id, const std::string name, float *feature)
{
    reassign_feature();

    int index = get_free_mapmagic();
    if( index == -1)
    {
        LOGD("[ERROR]:Database space is full");
        return -1;
    }
    s_FeatureMap.magic[index] = FEATUREDATA_MAGIC_VALID;

    FeatureItem item_t;
    item_t.id = id;
    item_t.index = index;
    strcpy(item_t.name, name.c_str());
    memcpy(item_t.feature, feature, OASISLT_getFaceItemSize());
#if SDRAM_DB
    memcpy(&s_FeatureItem[index], &item_t, sizeof(item_t));
#endif
    Flash_FacerecFsUpdateItem(index,&item_t, false);
    Flash_FacerecFsUpdateMapMagic(index, &s_FeatureMap, false);
    return 0;
}

int FeatureDB::update_feature(uint16_t id, const std::string name, float *feature)
{
    int ret;
    ret = del_feature(id, name);
    if (ret == -1)
    {
        return ret;
    }
    ret = add_feature(id, name, feature);
    return ret;
}

int FeatureDB::get_feature(uint16_t id, float *feature)
{
    int index = FEATUREDATA_MAX_COUNT;
    FeatureItem item_t;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
#if SDRAM_DB
            memcpy(&item_t, &s_FeatureItem[i], sizeof(item_t));
#else
            Flash_FacerecFsReadItem(i,&item_t);
#endif
            if (id == item_t.id)
            {
                index = i;
                memcpy(feature, item_t.feature,OASISLT_getFaceItemSize());
                break;
            }
        }
    }

    if (index == FEATUREDATA_MAX_COUNT)
    {
        return -1;
    }
    else
    {
        return index;
    }
}


int FeatureDB::get_ID_featurePointers(uint16_t* ids, void**featureP, int num)
{
    int index = 0;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
    	if (index >= num)
    	{
    		break;
    	}

        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
#if SDRAM_DB
            ids[index] = s_FeatureItem[i].id;
            featureP[index] = s_FeatureItem[i].feature;
#else
            Flash_FacerecFsReadIDFeaturePointer(index,&ids[index],&featureP[index]);
#endif
            index++;
        }
    }

    return 0;
}

std::vector<uint16_t> FeatureDB::get_ids()
{
    FeatureItem item_t;
    std::vector<uint16_t> ids;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
#if SDRAM_DB
            memcpy(&item_t, &s_FeatureItem[i], sizeof(item_t));
#else
            Flash_FacerecFsReadItemHeader(i,&item_t);
#endif
            ids.push_back(item_t.id);
        }
    }
    return ids;
}

int FeatureDB::get_name(uint16_t id, std::string &name)
{
    FeatureItem item_t;
    int index = FEATUREDATA_MAX_COUNT;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureMap.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
#if SDRAM_DB
            memcpy(&item_t, &s_FeatureItem[i], sizeof(item_t));
#else
            Flash_FacerecFsReadItemHeader(i,&item_t);
#endif
            if (id == item_t.id)
            {
                index = i;
                name = std::string(item_t.name);
                break;
            }
        }
    }

    if (index == FEATUREDATA_MAX_COUNT)
    {
        name.clear();
        return -1;
    }
    else
    {
        return index;
    }
}

void FeatureDB::set_autosave(bool auto_save)
{
    this->auto_save = auto_save;
}

bool FeatureDB::get_autosave()
{
    return this->auto_save;
}
#endif
