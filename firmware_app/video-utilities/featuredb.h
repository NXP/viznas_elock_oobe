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

#pragma once
#ifndef _FEATUREDB_H_
#define _FEATUREDB_H_

#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include <assert.h>

using namespace std;

class FeatureDB
{
   public:
    FeatureDB();
    ~FeatureDB();
    int add_feature(uint16_t id, const std::string name, float *feature);
    int update_feature(uint16_t id, const std::string name, float *feature);
    int del_feature(uint16_t id, std::string name);
    int del_feature(const std::string name);
    int del_feature_all();
    int get_names(std::vector<std::string> &names,int count);
    int get_name(uint16_t id, std::string &name);
    int get_ids(std::vector<uint16_t> &ids);
    int ren_name(const std::string oldname, const std::string newname);
    int get_free(int &index);
    int database_save(int count);
    int get_feature(uint16_t id, float *feature);
    void set_autosave(bool auto_save);
    bool get_autosave();
    int get_count();
    int get_ID_featurePointers(uint16_t* ids, void**featureP, int num);

   private:
    bool auto_save;
    int  itemSize ;   /*Actual size of face item */
    int  itemSpaceSize ; /*Space size reserved for each face item, it must be page aligned */
    void* itemBuff;
    int count;
    void clear_item(void);
    int feature_count();
    int get_idxById(int id, int &index);
    int get_idxByName(const std::string name, int &index);
    int load_feature();
    int erase_feature(int index);
    int save_feature(int index = 0);
    int reassign_feature();

    int get_free_mapmagic();
    int get_remain_map();
};

#endif /* _FEATUREDB_H_ */
