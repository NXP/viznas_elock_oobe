/*
* Copyright 2018 NXP, Inc.
*
* Author: Dongsheng.zhang@nxp.com
*
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

#define FEATUREDATA_FLASH_PAGE_SIZE 256

#define FEATUREDATA_MAGIC 0x79
#define FEATUREDATA_MAX_COUNT 100
#define FEATUREDATA_NAME_MAX_LEN 31
#define FEATUREDATA_FEATURE_SIZE 128

typedef union {
    struct {
        unsigned char magic;
        char name[FEATUREDATA_NAME_MAX_LEN];
        float feature[FEATUREDATA_FEATURE_SIZE];
    };
    unsigned char raw[FEATUREDATA_FLASH_PAGE_SIZE * 3];
} FeatureItem;

typedef union {
    struct {
        char magic[FEATUREDATA_MAX_COUNT];
    };
    unsigned char raw[FEATUREDATA_FLASH_PAGE_SIZE];
} FeatureMap;

typedef struct {
    FeatureMap map;
    FeatureItem item[FEATUREDATA_MAX_COUNT];
} FeatureData;

class FeatureDB
{
public:
    FeatureDB(float thres);
    ~FeatureDB();
    int add_feature(const std::string name, std::vector<float> feature);
    int del_feature(const std::string name);
    std::string find_name(std::vector<float>& feature);
    std::string find_name(std::vector<float>& feature, float& largest);
    std::vector<std::string> get_names();

private:
    int load_feature();
    int erase_feature(int index);
    int save_feature(int index = 0);

    float cal_similar(std::vector<float>& v1, std::vector<float>& v2);
    float cal_similar(std::vector<float>& v1, float* v2);

private:
    float threshold;    
    FeatureData gFeatureData;
};

#endif /* _FEATUREDB_H_ */
