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

#include "face_db.h"

#include<android/log.h>

#define TAG "OASIS"  
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)   
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) 
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) 
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)  
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)  

static FeatureDB *gDB = NULL;

int face_db_init(float thres)
{
    if (gDB == NULL) {
        gDB = new FeatureDB(thres);
    }
    if (gDB == NULL) {
        return -100;
    }
    return 0;
}

int face_db_add(std::string name, std::vector<float> &feature)
{
    if (gDB != NULL) {
        gDB->add_feature(name, feature);
    }
    return 1;
}
/*
int face_db_find(std::vector<Face>& faces)
{
    std:vector<float> feature_vec;
    if (gDB != NULL) {
        int fcount = faces.size();
        for (int i = 0; i < fcount; i ++) {
            feature_vec.assign(faces[i].feature,faces[i].feature + sizeof(faces[i].feature)/
                                                                   sizeof(faces[i].feature[0]));
            std::string str_name = gDB->find_name(feature_vec);
            memcpy(faces[i].name,str_name.c_str(),str_name.length());
            //faces[i].name = gDB->find_name(feature_vec);
        }
        return fcount;
    }
    return 0;
}
*/
int face_db_exit()
{
    if (gDB) {
        delete gDB;
        gDB = NULL;
    }
    return 0;
}
