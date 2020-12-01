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

#ifdef TEST_DB
#define FLASH_SECTOR_SIZE 0x1000
#endif
/*******************************************************************************
 * Variables
 *******************************************************************************/
static FeatureData s_FeatureData;

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
    int ret;
    int index    = 0;
#ifdef TEST_DB
#define FLASH_SECTOR_SIZE 0x1000
    // copy the map
    memcpy(&s_FeatureData.map, test_db_bin, sizeof(s_FeatureData.map));
    // copy the items
    memcpy(&s_FeatureData.item, test_db_bin + FLASH_SECTOR_SIZE, sizeof(s_FeatureData.item));
#endif

    memset(&s_FeatureData, FEATUREDATA_MAGIC_UNUSE, sizeof(FeatureData));

    Flash_FacerecFsReadMapMagic(&s_FeatureData.map);

    int item_max = FEATUREDATA_MAX_COUNT;

    // find valid item
    for (int i = 0; i < item_max; i++)
    {
        if (s_FeatureData.map.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
            ret = Flash_FacerecFsReadItem(i, &s_FeatureData.item[index++]);
            if (ret != FLASH_OK)
                return ret;
        }
    }

    PRINTF("%d valid data.", index);

#if 0
    PRINTF("\r\nRTFS List--------------------------------------------------------");
    PRINTF("\r\nValid -> ");
    for (int i = 0; i < item_max; i++)
    {
        if (s_FeatureData.map.magic[i] == FEATUREDATA_MAGIC_VALID)
        {
            PRINTF("%d,", i);
        }
    }

    PRINTF("\r\nUnuse -> ");
    for (int i = 0; i < item_max; i++)
    {
        if (s_FeatureData.map.magic[i] == FEATUREDATA_MAGIC_UNUSE)
        {
            PRINTF("%d,", i);
        }
    }

    PRINTF("\r\nDelet -> ");
    for (int i = 0; i < item_max; i++)
    {
        if (s_FeatureData.map.magic[i] == FEATUREDATA_MAGIC_DELET)
        {
            PRINTF("%d,", i);
        }
    }
    PRINTF("\r\n-----------------------------------------------------------------\r\n");
#endif

    return index;
}

int FeatureDB::reassign_feature()
{
    int size     = 0;
    int item_max = FEATUREDATA_MAX_COUNT;
    int count, reassign_index;
    // flash occupy size.
    for (int i = 0; i < item_max; i++)
    {
        if (s_FeatureData.map.magic[i] != FEATUREDATA_MAGIC_UNUSE)
        {
            size++;
        }
    }

    if (size < item_max / 2)
    {
        return 0;
    }

    Flash_FacerecFsEraseMapBlock();

    Flash_FacerecFsEraseItemBlock();

    count = feature_count();
    memset(&s_FeatureData.map, FEATUREDATA_MAGIC_UNUSE, sizeof(s_FeatureData.map));
    reassign_index = 0;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (reassign_index >= count)
        {
            break;
        }

        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            s_FeatureData.item[i].index = reassign_index;
            Flash_FacerecFsUpdateItem(reassign_index, &s_FeatureData.item[i]);

            s_FeatureData.map.magic[reassign_index] = FEATUREDATA_MAGIC_VALID;
            Flash_FacerecFsUpdateMapMagic(reassign_index, &s_FeatureData.map);
            reassign_index++;
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
        if (s_FeatureData.map.magic[i] == FEATUREDATA_MAGIC_UNUSE)
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
        if (s_FeatureData.map.magic[i] == FEATUREDATA_MAGIC_UNUSE)
            remain_size++;
    }

    return remain_size;
}

int FeatureDB::save_feature(int index)
{
    int old_map_index = s_FeatureData.item[index].index;

    FeatureItem item_t;
    Flash_FacerecFsReadItem(old_map_index, &item_t);

    if (memcmp(&item_t, &s_FeatureData.item[old_map_index], sizeof(item_t)) == 0)
    {
        return 0;
    }

    // find new map index
    int new_map_index = get_free_mapmagic();

    if (old_map_index != -1)
    {
        // set old map magic to DELET
        s_FeatureData.map.magic[old_map_index] = FEATUREDATA_MAGIC_DELET;
        Flash_FacerecFsUpdateMapMagic(old_map_index, &s_FeatureData.map);

        // set old item magic to DELET
        Flash_FacerecFsUpdateItemMagic(old_map_index, FEATUREDATA_MAGIC_DELET);
    }

    // set new map magic to VALID
    if (new_map_index == -1)
        return -1;
    s_FeatureData.map.magic[new_map_index] = FEATUREDATA_MAGIC_VALID;
    Flash_FacerecFsUpdateMapMagic(new_map_index, &s_FeatureData.map);

    // update new item
    s_FeatureData.item[index].index = new_map_index;
    Flash_FacerecFsUpdateItem(new_map_index, &s_FeatureData.item[index]);

    PRINTF("[RTFS]: %d feature item&map saved.\r\n", index);

    return 0;
}

int FeatureDB::erase_feature(int index)
{
    int map_index = s_FeatureData.item[index].index;
    // user is not in flash, do nothing and return with no error.
    if (map_index == -1)
    {
        return 0;
    }
    // update index item magic to delet
    Flash_FacerecFsUpdateItemMagic(map_index, FEATUREDATA_MAGIC_DELET);

    // update index map magic to delet
    s_FeatureData.map.magic[map_index] = FEATUREDATA_MAGIC_DELET;
    Flash_FacerecFsUpdateMapMagic(map_index, &s_FeatureData.map);

    PRINTF("[RTFS]: %d feature magic deleted.\r\n", index);

    return 0;
}

int FeatureDB::ren_name(const std::string oldname, const std::string newname)
{
    int index    = FEATUREDATA_MAX_COUNT;
    int newIndex = FEATUREDATA_MAX_COUNT;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            if (!strcmp(oldname.c_str(), s_FeatureData.item[i].name))
            {
                index = i;
            }
            if (!strcmp(newname.c_str(), s_FeatureData.item[i].name))
            {
                newIndex = i;
            }
        }
    }

    if (index == FEATUREDATA_MAX_COUNT)
    {
        return -1;
    }

    if (newIndex != FEATUREDATA_MAX_COUNT)
    {
        // nothing to do if the name is the same
        if (newIndex == index)
        {
            return 0;
        }
        // skip renaming if the newname already exists
        return -2;
    }
    // rename the oldname entry
    strcpy(s_FeatureData.item[index].name, newname.c_str());
    return 0;
}

std::vector<std::string> FeatureDB::get_names()
{
    std::vector<std::string> names;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            names.push_back(std::string(s_FeatureData.item[i].name));
        }
    }

    return names;
}

int FeatureDB::feature_count()
{
    int count = 0;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
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
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_UNUSE)
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

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            if ((s_FeatureData.item[i].id == id) && (!strcmp(name.c_str(), s_FeatureData.item[i].name)))
            {
                index = i;
            }
        }
    }

    if (index != FEATUREDATA_MAX_COUNT)
    {
        if (0 == erase_feature(index))
        {
            memset(&s_FeatureData.item[index], 0xFF, sizeof(s_FeatureData.item[index]));
        }

        return 0;
    }
    else
    {
        return -1;
    }
}

int FeatureDB::del_feature(const std::string name)
{
    int index = FEATUREDATA_MAX_COUNT;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            if (!strcmp(name.c_str(), s_FeatureData.item[i].name))
            {
                index = i;
            }
        }
    }

    if (index != FEATUREDATA_MAX_COUNT)
    {
        if (0 == erase_feature(index))
        {
            memset(&s_FeatureData.item[index], 0xFF, sizeof(s_FeatureData.item[index]));
        }

        return 0;
    }
    else
    {
        return -1;
    }
}

int FeatureDB::del_feature_all()
{
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            if (0 == erase_feature(i))
            {
                memset(&s_FeatureData.item[i], 0xFF, sizeof(s_FeatureData.item[i]));
            }
        }
    }

    return 0;
}

int FeatureDB::database_save(int count)
{
    int ret;

    ret = feature_count();

    if (count > ret)
    {
        count = ret;
    }

    if (count == 0)
    {
        return 0;
    }

    // find the save index position.
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            // save item, read context to confirm if any different
            FeatureItem item_t;
            FeatureMap map_t;
            int map_index = s_FeatureData.item[i].index;
            Flash_FacerecFsReadItem(map_index, &item_t);

            if (memcmp(&item_t, &s_FeatureData.item[i], sizeof(item_t)) != 0)
            {
                int old_map_index = s_FeatureData.item[i].index;

                // find new map index
                int new_map_index = get_free_mapmagic();

                // flash database full
                if (new_map_index == -1)
                {
                    reassign_feature();
                    return 0;
                }

                if (old_map_index != -1)
                {
                    // set old map magic to DELET
                    s_FeatureData.map.magic[old_map_index] = FEATUREDATA_MAGIC_DELET;
                    Flash_FacerecFsUpdateMapMagic(old_map_index, &s_FeatureData.map);

                    // set old item magic to DELET
                    Flash_FacerecFsUpdateItemMagic(old_map_index, FEATUREDATA_MAGIC_DELET);
                }

                // set new map magic to VALID
                s_FeatureData.map.magic[new_map_index] = FEATUREDATA_MAGIC_VALID;
                Flash_FacerecFsUpdateMapMagic(new_map_index, &s_FeatureData.map);

                // update new item
                s_FeatureData.item[i].index = new_map_index;
                Flash_FacerecFsUpdateItem(new_map_index, &s_FeatureData.item[i]);
            }

            // save map
            Flash_FacerecFsReadMapMagic(&map_t);

            if (memcmp(&map_t, &s_FeatureData.map, sizeof(map_t)) != 0)
            {
                ret = Flash_FacerecFsUpdateMapMagic(i, &s_FeatureData.map);
                if (ret != 0)
                    return ret;
            }

            PRINTF("[RTFS]: %d feature item&map saved.\r\n", i);

            count--;
            if (count == 0)
                return 0;
        }
    }

    return 0;
}

int FeatureDB::add_feature(uint16_t id, const std::string name, float *feature)
{
    int index     = FEATUREDATA_MAX_COUNT;
    int freeIndex = FEATUREDATA_MAX_COUNT;
    int opIndex   = FEATUREDATA_MAX_COUNT;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            if (id == s_FeatureData.item[i].id)
            {
                index = i;
            }
        }
        else if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_UNUSE)
        {
            if (freeIndex == FEATUREDATA_MAX_COUNT)
            {
                freeIndex = i;
            }
        }
    }

    // LOGD("[%d:%d]\r\n", index, freeIndex);
    if (index != FEATUREDATA_MAX_COUNT)
    {
        // update
        opIndex = index;
    }
    else if (freeIndex != FEATUREDATA_MAX_COUNT)
    {
        // add new
        opIndex = freeIndex;
    }

    if (opIndex != FEATUREDATA_MAX_COUNT)
    {
        s_FeatureData.item[opIndex].magic = FEATUREDATA_MAGIC_VALID;
        strcpy(s_FeatureData.item[opIndex].name, name.c_str());
        s_FeatureData.item[opIndex].id = id;
        memcpy(s_FeatureData.item[opIndex].feature, feature, OASISLT_getFaceItemSize());
        if(this->auto_save)
        {
            save_feature(opIndex);
        }
        return opIndex;
    }

    LOGD("[ERROR]:Database out of space");
    return -1;
}

int FeatureDB::update_feature(uint16_t id, const std::string name, float *feature)
{
    int index = FEATUREDATA_MAX_COUNT;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            if (id == s_FeatureData.item[i].id)
            {
                index = i;
            }
        }
    }

    if (index != FEATUREDATA_MAX_COUNT)
    {
        s_FeatureData.item[index].magic = FEATUREDATA_MAGIC_VALID;
        s_FeatureData.item[index].id    = id;
        memcpy(s_FeatureData.item[index].name, name.c_str(), min(name.size(), sizeof(s_FeatureData.item[index].name)));
        s_FeatureData.item[index].name[sizeof(s_FeatureData.item[index].name) - 1] = 0;
        memcpy(s_FeatureData.item[index].feature, feature, OASISLT_getFaceItemSize());
        if(this->auto_save)
        {
            save_feature(index);
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

int FeatureDB::get_feature(uint16_t id, float *feature)
{
    int index = FEATUREDATA_MAX_COUNT;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            if (id == s_FeatureData.item[i].id)
            {
                index = i;
            }
        }
    }

    if (index != FEATUREDATA_MAX_COUNT)
    {
        memcpy(feature, s_FeatureData.item[index].feature, OASISLT_getFaceItemSize());
        return 0;
    }
    else
    {
        memset(feature, 0x0, OASISLT_getFaceItemSize());
        return -1;
    }
}

std::vector<uint16_t> FeatureDB::get_ids()
{
    std::vector<uint16_t> ids;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            ids.push_back(s_FeatureData.item[i].id);
        }
    }
    return ids;
}

int FeatureDB::get_name(uint16_t id, std::string &name)
{
    int index = FEATUREDATA_MAX_COUNT;
    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++)
    {
        if (s_FeatureData.item[i].magic == FEATUREDATA_MAGIC_VALID)
        {
            if (id == s_FeatureData.item[i].id)
            {
                index = i;
            }
        }
    }

    if (index != FEATUREDATA_MAX_COUNT)
    {
        name = std::string(s_FeatureData.item[index].name);
        return 0;
    }
    else
    {
        name.clear();
        return -1;
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
