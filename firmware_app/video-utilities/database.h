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

#ifndef _DATA_BASE_H_
#define _DATA_BASE_H_

#include <string.h>
#include <string>
#include <vector>

using namespace std;

/*******************************************************************************
 * Definitions
 *******************************************************************************/

/* Define error codes */
typedef enum _database_mgmt_status
{
    DB_MGMT_OK      = 0x00,
    DB_MGMT_FAILED  = -0x01,
    DB_MGMT_NODB    = -0x02,
    DB_MGMT_ENOLOCK = -0x03,
    DB_MGMT_ERETRY  = -0x04,
    DB_MGMT_NOSPACE = -0x05,

} database_mgmt_status_t;

/*******************************************************************************
 * API
 *******************************************************************************/

int DB_Del(uint16_t id, std::string name);
int DB_Del(string name);
int DB_DelAll();
int DB_Ren(const std::string oldname, const std::string newname);
int DB_GetFree(int &index);
int DB_GetNames(std::vector<std::string> *names, int count);
int DB_Count(int *count);
int DB_Save(int count);
int DB_GetFeature(uint16_t id, float *feature);
int DB_Add(uint16_t id, float *feature);
int DB_Add(uint16_t id, std::string name, float *feature);
int DB_Update(uint16_t id, float *feature);
int DB_GetIDs(std::vector<uint16_t> &ids);
int DB_GetName(uint16_t id, std::string &names);
int DB_GenID(uint16_t *id);
int DB_SetAutoSave(bool auto_save);
int DB_GetID_FeaturePointers(uint16_t* ids,void**pFeatures,int num);

#endif /*_DATA_BASE_H_*/
