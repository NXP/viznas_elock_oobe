/*
* Copyright 2018 NXP, Inc.
*
* Author: Dongsheng.zhang@nxp.com
*
*/
//#define LOG_NIDEBUG 0
#include "featuredb.h"
#include<android/log.h>

#define TAG "FaceAddJNI"  
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)   
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) 
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) 
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)  
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)  

/*database related*/
FeatureDB::FeatureDB(float thres)
{
    threshold = thres;
    load_feature();
}

FeatureDB::~FeatureDB()
{
}

int FeatureDB::load_feature()
{
    memset(&gFeatureData, 0, sizeof(FeatureData));
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

int FeatureDB::add_feature(const std::string name, std::vector<float> feature)
{
    int index = FEATUREDATA_MAX_COUNT;
    int freeIndex = FEATUREDATA_MAX_COUNT;
    int opIndex = FEATUREDATA_MAX_COUNT;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++) {
        if (gFeatureData.map.magic[i] == FEATUREDATA_MAGIC && gFeatureData.item[i].magic == FEATUREDATA_MAGIC) {
            if (!strcmp(name.c_str(), gFeatureData.item[i].name)) {
                index = i;
            }
        } else {
            if (freeIndex == FEATUREDATA_MAX_COUNT) {
                freeIndex = i;
            }
        }
    }

    //LOGD("[%d:%d]\r\n", index, freeIndex);
    if (index != FEATUREDATA_MAX_COUNT) {
        // update
        opIndex = index;
    } else {
        // add new
        if (freeIndex != FEATUREDATA_MAX_COUNT) {
            opIndex = freeIndex;           
        } else {
            LOGE("[ERROR]:out of space");
            return -1;
        }
    }

    if (opIndex != FEATUREDATA_MAX_COUNT) {
        gFeatureData.map.magic[opIndex] = FEATUREDATA_MAGIC;
        gFeatureData.item[opIndex].magic = FEATUREDATA_MAGIC;
        strcpy(gFeatureData.item[opIndex].name, name.c_str());
        for (int i = 0; i < FEATUREDATA_FEATURE_SIZE; i++) {
            gFeatureData.item[opIndex].feature[i] = feature[i];
            //LOGD("[%d][0x%08x:%.10f]\r\n", i, *((unsigned int*)((void*)&feature[i])),  feature[i]);
        }
        save_feature(opIndex);
    }
    return 0;
}

std::vector<std::string> FeatureDB::get_names()
{
    std::vector<std::string> names;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++) {
        if (gFeatureData.map.magic[i] == FEATUREDATA_MAGIC && gFeatureData.item[i].magic == FEATUREDATA_MAGIC) {
            names.push_back(std::string(gFeatureData.item[i].name));
        }
    }

    return names;
}

int FeatureDB::del_feature(const std::string name)
{
    int index = FEATUREDATA_MAX_COUNT;

    for (int i = 0; i < FEATUREDATA_MAX_COUNT; i++) {
        if (gFeatureData.map.magic[i] == FEATUREDATA_MAGIC && gFeatureData.item[i].magic == FEATUREDATA_MAGIC) {
            if (!strcmp(name.c_str(), gFeatureData.item[i].name)) {
                index = i;
            }
        }
    }

    if (index != FEATUREDATA_MAX_COUNT) {
        if (0 == erase_feature(index)) {
            gFeatureData.map.magic[index] = 0;
            gFeatureData.item[index].magic = 0;
        }

        return 0;
    } else {
        return -1;
    }
}

std::string FeatureDB::find_name(std::vector<float>& feature)
{
    float largest = 0.0f, similar = 0.0f;
    std::string name = "";
    int index = FEATUREDATA_MAX_COUNT;
    int i = 0;

    for (; i < FEATUREDATA_MAX_COUNT; i++) {
        if (gFeatureData.map.magic[i] == FEATUREDATA_MAGIC && gFeatureData.item[i].magic == FEATUREDATA_MAGIC) {
            similar = cal_similar(feature, gFeatureData.item[i].feature);
            //LOGD("[%d]:%.4f\r\n", i, similar);
            if (similar > largest) {
                largest = similar;
                index = i;
            }
        }
    }

    if (largest > threshold && index != FEATUREDATA_MAX_COUNT) {
        //LOGI(">>find_name:[%s]:[%.4f]\r\n", gFeatureData.item[index].name, largest);
        name = std::string(gFeatureData.item[index].name);
    }
    return name;
}

std::string FeatureDB::find_name(std::vector<float>& feature, float& largest)
{
    float similar;
    largest = 0.0f;
    std::string name = "";
    int index = FEATUREDATA_MAX_COUNT;
    int i = 0;

    for (; i < FEATUREDATA_MAX_COUNT; i++) {
        if (gFeatureData.map.magic[i] == FEATUREDATA_MAGIC && gFeatureData.item[i].magic == FEATUREDATA_MAGIC) {
            similar = cal_similar(feature, gFeatureData.item[i].feature);
            //LOGD("[%d]:%.4f\r\n", i, similar);
            if (similar > largest) {
                largest = similar;
                index = i;
            }
        }
    }

    if (index != FEATUREDATA_MAX_COUNT) {
        //LOGD(">>find_name:[%s]:[%.4f]\r\n", gFeatureData.item[index].name, largest);
        name = std::string(gFeatureData.item[index].name);
    }

    return name;
}

float FeatureDB::cal_similar(std::vector<float>& v1, std::vector<float>& v2)
{
    assert(v1.size() == v2.size());
    double ret = 0.0, mod1 = 0.0, mod2 = 0.0;

    for (unsigned int i = 0; i != v1.size(); ++i) {
        ret += v1[i] * v2[i];
        mod1 += v1[i] * v1[i];
        mod2 += v2[i] * v2[i];
    }

    return (ret / sqrt(mod1) / sqrt(mod2) + 1) / 2.0;
}

float FeatureDB::cal_similar(std::vector<float>& v1, float* v2)
{
    double ret = 0.0, mod1 = 0.0, mod2 = 0.0;

    for (unsigned int i = 0; i != 128; ++i) {
        ret += v1[i] * v2[i];
        mod1 += v1[i] * v1[i];
        mod2 += v2[i] * v2[i];
    }

    return (ret / sqrt(mod1) / sqrt(mod2) + 1) / 2.0;
}

