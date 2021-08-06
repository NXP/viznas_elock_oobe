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

#include <com_nxp_facemanager_Oasis.h>
#include <math.h>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <android/log.h>
#include <pthread.h>
#include <mutex>

#define TAG "OASIS"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, TAG, __VA_ARGS__)

#define FACE_FEATURES 128

#if ENABLE_ANTISPOOFING
#include "face_antispoofing.h"
#include "face_antispoofing_model.h"
#endif
// #include "face_db.h"

#if OASIS_PROFILING
#include "profiling.h"
#endif

#include "oasislite_runtime.h"

#define FACEREC_MINFACE 100
#define FACEREC_MAXFACE 1

#define FACEREC_THRESHOLD 0.85f

#define OASIS_HEIGHT 960
#define OASIS_WIDTH 1280
//using namespace oasis;
using namespace std;

static int isAuthenticated = 0;
static int gOasisReady = 0;
OASISLTInitPara_t init_para;
static int bounding_box_done;

static int OASIS_H = 960;
static int OASIS_W = 960;

pthread_mutex_t copy_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t done_copy = PTHREAD_COND_INITIALIZER;

typedef struct {
    FBox pfaceBox;
    OASISLTEmoID_t emoID;
    OASISLTRegisterRes_t regResult;
    uint16_t faceID;
    std::string name;
    int new_face;
    bool quality;
    std::vector<char> face_feature;
    uint32_t item_size;
    bool done_copying;
    int size_db;
    // int *ids_db;
    // char **features_db;
} UserInfo;

static UserInfo userInfo;

static void EvtHandler(ImageFrame_t **frames, OASISLTEvt_t evt, OASISLTCbPara_t *para, void *user_data);
static void EvtHandler2(ImageFrame_t *frame, OASISLTEvt_t evt, OASISLTCbPara_t *para, void *user_data);
static int GetRegisteredFacesHandler(int start_idx,uint16_t* face_id,void* faces,unsigned int* size);
static int AddNewFaceHandler(uint16_t* face_id, void *face);
static int UpdateFaceHandler(uint16_t face_id, void *face);
static int Oasis_Printf(const char* formatString);

static float similarity_cal(float *v1, std::vector<float> &v2)
{
    double ret = 0.0, mod1 = 0.0, mod2 = 0.0;

    for (int i = 0; i != 128; ++i)
    {
        ret += v1[i] * v2[i];
        mod1 += v1[i] * v1[i];
        mod2 += v2[i] * v2[i];
    }

    float sim = ret / (sqrt(mod1) * sqrt(mod2));
    return sim;
}

int g_IsAuthenticated = 0;
int Oasis_Init(int useHeavyModel, int isElock)
{
    init_para = {
            .height = OASIS_H,
            .width = OASIS_W,
            .img_format = OASIS_IMG_FORMAT_RGB888,
            .min_face = 100,
            .mem_pool =NULL,
            .size = 0,
            .cbs = {EvtHandler,
                    GetRegisteredFacesHandler,
                    AddNewFaceHandler,
                    UpdateFaceHandler,
                    (void *)Oasis_Printf},
            .enable_flags = OASIS_ENABLE_DET | OASIS_ENABLE_REC | OASIS_ENABLE_MULTI_VIEW,
            .emo_mode = OASIS_EMOTION_MODE_INVALID,
    };

    if (useHeavyModel) {
        init_para.mod_class = OASISLT_MODEL_CLASS_HEAVY;
        LOGE("Use Heavy Model\r\n");
    } else {
        init_para.mod_class = OASISLT_MODEL_CLASS_LIGHT;
        LOGE("Use Light Model\r\n");
    }

    if (isElock) {
        init_para.img_type = OASIS_IMG_TYPE_IR_SINGLE;
        LOGE("img_type: IR_SINGLE\r\n");
    } else {
        init_para.img_type = OASIS_IMG_TYPE_RGB_SINGLE;
        LOGE("img_type: RGB_SINGLE\r\n");
    }

    int ret_val = 0;
    ret_val = OASISLT_init(&init_para);
    LOGE("first init ret: %d", ret_val);
    if (ret_val > 0)
    {
        init_para.mem_pool = new char[ret_val];
        init_para.size = ret_val;
        ret_val = OASISLT_init(&init_para);
        LOGE("second init ret: %d", ret_val);
        assert(ret_val == 0);
    }

    return ret_val;
}

int Oasis_Exit()
{
    int ret = 0;
    ret = OASISLT_uninit();

    if (ret)
    {
        LOGE("[ERROR]:OASISLT_uninit failed\n");
    }

    return ret;
}

/*
 * Class:     com_nxp_faceadd_Oasis
 * Method:    faceRecognize
 * Signature: ([B)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_nxp_facemanager_Oasis_faceRecognize(JNIEnv *env, jobject obj,
                                                                       jbyteArray data, jint width,
                                                                       jint height, jintArray box,
                                                                       jint isEditUser,
                                                                       jintArray id,
                                                                       jbyteArray feature,
                                                                       jintArray id_db,
                                                                       jobjectArray feature_db,
                                                                       jintArray result)
{
    LOGD(">>faceRecognize");
    jstring name = (*env).NewStringUTF("");
    jint ret = 0;

    if (!gOasisReady) {
        LOGE("[ERROR]:Oasis is not Init");
        return name;
    }

    userInfo.size_db = 0;
    userInfo.size_db = env->GetArrayLength(id_db);
    // userInfo.features_db = new char*[userInfo.size_db];
    // for(int i = 0; i < userInfo.size_db; i++) {
    //     userInfo.features_db[i] = new char[OASISLT_getFaceItemSize()];
    // }

    LOGD("feature db size: %d.\r\n", userInfo.size_db);
    // if( userInfo.size_db > 0){
    //     userInfo.ids_db = env->GetIntArrayElements(id_db, NULL);
    //     for (jint i = 0; i <  userInfo.size_db; i++) {
    //         jarray obj = (jbyteArray)env->GetObjectArrayElement(feature_db, i);
    //         jbyte* byteArray = env->GetByteArrayElements((jbyteArray)obj, NULL);
    //         memcpy(userInfo.features_db[i], byteArray, OASISLT_getFaceItemSize());
    //         env->ReleaseByteArrayElements((jbyteArray)obj, byteArray, NULL);
    //     }
    // }

    jbyte *imageData = env->GetByteArrayElements(data, NULL);
    if (NULL == imageData) {
        env->ReleaseByteArrayElements(data, imageData, 0);
        return name;
    }

    ImageFrame_t frame;
    frame.height = height;
    frame.width = width;
    frame.data = (unsigned char *) imageData;

    userInfo.name = "";

    jbyte *featureData = env->GetByteArrayElements(feature, NULL);
    char *temp_feature = (char *) featureData;
    if (NULL == featureData) {
        env->ReleaseByteArrayElements(feature, featureData, JNI_COMMIT);
        return name;
    }

    userInfo.item_size = OASISLT_getFaceItemSize();

    userInfo.quality = 0;

//    userInfo.face_feature.resize(userInfo.item_size);
    userInfo.face_feature.resize((unsigned) userInfo.item_size);
    LOGD("feature capacity before: %u", userInfo.face_feature.capacity());
    int flag = isEditUser ? OASIS_DET_REC : OASIS_DET_REC | OASIS_REG_MODE;
//    userInfo.done_copying = false;
    ret = OASISLT_run(&frame, flag, FACEREC_MINFACE, NULL);
    assert(ret == 0);
    LOGD("feature size after: %u", userInfo.face_feature.size());
//    for (float f: userInfo.face_feature){
//        LOGE("Feature data after: %f", f);
//    }

//    pthread_mutex_lock(&copy_lock);
//    if (!userInfo.done_copying){
//        pthread_cond_wait(&done_copy, &copy_lock);
//    }
    for (int i = 0; i < userInfo.face_feature.size(); i++) {
        temp_feature[i] = userInfo.face_feature[i];
    }
//    pthread_mutex_unlock(&copy_lock);

    userInfo.face_feature.clear();

    FBox *faceBox = &userInfo.pfaceBox;

    jint *boxData = env->GetIntArrayElements(box, NULL);
    jint *userId = env->GetIntArrayElements(id, NULL);
    jint *res = env->GetIntArrayElements(result, NULL);

    if (NULL == boxData) {
        env->ReleaseIntArrayElements(box, boxData, 0);
        return name;
    }

    int *pBox = (int *) boxData;

    LOGD("[box]:[%d, %d, %d, %d]", faceBox->rect[0], faceBox->rect[1], faceBox->rect[2],
         faceBox->rect[3]);
    if (userInfo.quality) {
        pBox[0] = faceBox->rect[0];
        pBox[1] = faceBox->rect[1];
        pBox[2] = faceBox->rect[2];
        pBox[3] = faceBox->rect[3];
    } else {
        pBox[0] = -1;
        pBox[1] = -1;
        pBox[2] = -1;
        pBox[3] = -1;
    }
    userId[0] = userInfo.faceID;
    res[0] = userInfo.regResult;
    env->ReleaseIntArrayElements(box, boxData, 0);
    env->ReleaseIntArrayElements(id, userId, 0);
    env->ReleaseIntArrayElements(result, res, 0);
    env->ReleaseByteArrayElements(feature, featureData, JNI_COMMIT);

    // for(int i = 0; i < userInfo.size_db; i++) {
    //     delete[] userInfo.features_db[i];
    // }
    // delete[] userInfo.features_db;

    // TODO: Get username from db with id
//    userInfo.name = "temp";
    char *temp_1 = &(userInfo.name[0]);
    LOGD("[name]:[%s]", userInfo.name.c_str());
    name = (env)->NewStringUTF(userInfo.name.c_str());

//    name = isEditUser? (env)->NewStringUTF((const char*)&userInfo.name):name;

    LOGD("<<faceRecognize");
    return name;
}

/*
 * Class:     com_nxp_faceadd_Oasis
 * Method:    Init
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_nxp_facemanager_Oasis_Init(JNIEnv *env, jobject obj, jint useHeavyModel, jint isElock)
{
    LOGE(">>Oasis_Init:%d", gOasisReady);
    jint ret = 0;
    if (gOasisReady == 0)
    {
        LOGE("Do Init");

        ret = Oasis_Init(useHeavyModel, isElock);

        if (ret)
        {
            LOGE("[ERROR]:oasis init %d", ret);
            return ret;
        }
        // ret = face_db_init(FACEREC_THRESHOLD);
        // if (ret)
        // {
        //     LOGE("[ERROR]:face_db_init %d", ret);
        //     return ret;
        // }
    }
    gOasisReady = 1;
    LOGE("<<Oasis_Init");
    return ret;
}

JNIEXPORT jint JNICALL Java_com_nxp_facemanager_Oasis_Exit(JNIEnv *env, jobject obj)
{
    LOGD(">>Oasis_Exit:%d", gOasisReady);
    jint ret = 0;
    gOasisReady = 0;
#if 0
    ret = face_detect_exit();
    if (ret) {
        LOGE("[ERROR]:face_detect_exit %d", ret);
        return ret;
    }
    ret = face_recognize_exit();
    if (ret) {
        LOGE("[ERROR]:face_recognize_exit %d", ret);
        return ret;
    }
    ret = face_db_exit();
    if (ret) {
        LOGE("[ERROR]:face_db_exit %d", ret);
        return ret;
    }
#else
    ret = Oasis_Exit();
#endif
    LOGD("<<Oasis_Exit");
    return ret;
}

/*
 * Class:     com_nxp_faceadd_Oasis
 * Method:    faceAdd
 * Signature: (Ljava/lang/String;[B)I
 */
JNIEXPORT jint JNICALL Java_com_nxp_facemanager_Oasis_faceAdd(JNIEnv *env, jobject obj, jstring name, jbyteArray feature)
{
    jint ret = 0;
    int iret = 0;
    LOGD(">>faceAdd");
    if (!gOasisReady)
    {
        LOGE("[ERROR]:Oasis is not Init");
        return ret;
    }

    jbyte *featureData = env->GetByteArrayElements(feature, NULL);
    if (NULL == featureData)
    {
        env->ReleaseByteArrayElements(feature, featureData, 0);
        ret = -100;
        return ret;
    }

    LOGD("<<faceAdd");
    return ret;
}

/*
 * Class:     com_nxp_faceadd_Oasis
 * Method:    faceDel
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_nxp_facemanager_Oasis_faceDel(JNIEnv *env, jobject obj, jstring name)
{
    jint ret = 0;
    LOGD(">>faceDel");
    if (!gOasisReady)
    {
        LOGE("[ERROR]:Oasis is not Init");
        return ret;
    }
    //TODO: delete user in db based on user ID
    LOGD("<<faceDel");
    return ret;
}

/*
 * Class:     com_nxp_faceadd_Oasis
 * Method:    faceDelAll
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_nxp_facemanager_Oasis_faceDelAll(JNIEnv *env, jobject obj)
{
    jint ret = 0;
    LOGD(">>faceDelAll");
    if (!gOasisReady)
    {
        LOGE("[ERROR]:Oasis is not Init");
        return ret;
    }
    LOGD("<<faceDelAll");
    return ret;
}

/*Below are all the call-back functions for OASISLT_run
 */
static void EvtHandler(ImageFrame_t **frames, OASISLTEvt_t evt, OASISLTCbPara_t *para, void *user_data)
{
    if ((frames[OASISLT_INT_FRAME_IDX_RGB] == NULL && frames[OASISLT_INT_FRAME_IDX_IR] == NULL)
        || para == NULL)
    {
        LOGD("Illegal parameter: %d", evt);
        return;
    }

    UserInfo *uInfo = &userInfo;
    LOGD("event: %d", evt);

    switch (evt)
    {
    case OASISLT_EVT_DET_COMPLETE:
    {
        LOGE("on detection complete");

        if (para->faceBoxRGB == NULL && para->faceBoxIR == NULL)
        {
            LOGE("no face detected");
            memset(userInfo.pfaceBox.rect, -1, sizeof(para->faceBoxRGB->rect));
            userInfo.done_copying = true;
        }
        else
        {
            //    (OASISLTCbPara_t *)user_data = para;
            int *rect = (para->faceBoxRGB)?para->faceBoxRGB->rect:para->faceBoxIR->rect;
            LOGD("bounding box: %d %d %d %d", rect[0], rect[1], rect[2], rect[3]);
            memcpy(userInfo.pfaceBox.rect, rect, sizeof(userInfo.pfaceBox.rect));
            bounding_box_done = 1;
        }
        break;
    }

    case OASISLT_EVT_REC_COMPLETE:
    {
        unsigned id = para->faceID;
        if (id != INVALID_FACE_ID)
        {
            std::string name;
            //TODO: Talk to db and get name from id
            name = "temp_rec";
//            DB_GetName(id, name);
            userInfo.name = name;
            uInfo->new_face = 0;
        }
        else
        {
            LOGD("Face unrecognized");
            uInfo->new_face = 1;
        }
    }
    break;

    case OASISLT_EVT_REG_COMPLETE:
    {
        unsigned id = para->faceID;
        OASISLTRegisterRes_t regResult = para->regResult;
        uInfo->regResult = para->regResult;
        LOGD("[OASIS]:reg complete:%d:%d\r\n", id, regResult);

        if(regResult == OASIS_REG_RESULT_OK) {
            uInfo->new_face = 1;
            uInfo->quality = 1;
            uInfo->faceID = id;
            LOGD("[OASIS] New face id:%d, name:%d.\r\n", id, regResult);
        }
        else if(regResult == OASIS_REG_RESULT_DUP) {
            LOGD("[OASIS] Dup face id:%d, name:%d.\r\n", id, regResult);
            uInfo->quality = 0;
            userInfo.done_copying = true;
        } else {
            LOGD("low face quality, face registration failed...");
            uInfo->quality = 0;
            userInfo.done_copying = true;
        }
    }
    break;
    case OASISLT_EVT_QUALITY_CHK_START:
            break;
    case OASISLT_EVT_QUALITY_CHK_COMPLETE: {
        LOGD("[OASIS]:face quality check res:%d\r\n", para->qualityResult);
        break;
    }
    case OASISLT_EVT_EMO_REC_COMPLETE:
    {
        uInfo->emoID = para->emoID;
    }
    break;

    case OASISLT_EVT_DET_START:
        break;
    case OASISLT_EVT_REC_START:
        break;

    case OASISLT_EVT_REG_START:
        break;
    case OASISLT_EVT_EMO_REC_START:
        break;

    case OASISLT_EVT_REG_IN_PROGRESS: {
        OASISLTFaceQualityRes_t regResult = para->qualityResult;
        uInfo->regResult = para->regResult;
        LOGD("[OASIS]:reg in process : %d\r\n", regResult);

        if(regResult == OASIS_QUALITY_RESULT_FACE_OK)
        {
            LOGD("[EVT_REG]: Valid Face!\r\n");
        }
        else if(regResult == OASIS_QUALITY_RESULT_FACE_TOO_SMALL)
        {
            LOGD("[EVT_REG]:Small Face!\r\n");
        }
        else if(regResult == OASIS_QUALITY_RESULT_FACE_BLUR)
        {
            LOGD("[EVT_REG]: Burry Face!\r\n");
        }
        else if(regResult == OASIS_QUALITY_RESULT_FAIL_LIVENESS_IR)
        {
            LOGD("[EVT_REG]: IR Fake Face!\r\n");
        }
        else if(regResult == OASIS_QUALITY_RESULT_FAIL_LIVENESS_RGB)
        {
            LOGD("[EVT_REG]: RGB Fake Face!\r\n");
        }
    }
        break;
    default:
        assert(0);
    }
}
static void EvtHandler2(ImageFrame_t *frame, OASISLTEvt_t evt, OASISLTCbPara_t *para, void *user_data){

}
// TODO: Talk to db for all call-backs

static int GetRegisteredFacesHandler(int start_idx, uint16_t* face_id, void* faces, unsigned int* size)
{
    int face_item_size = OASISLT_getFaceItemSize();
    // if(start_idx < 0 || start_idx >= userInfo.size_db) {
    //     *size = 0;
    //     return 0;
    // } else {
    //     if( userInfo.size_db <= *size + start_idx) {
    //         *size = userInfo.size_db - start_idx;
    //     }

    //     for (int i = 0; i < *size; i++) {
    //         face_id[i] = userInfo.ids_db[start_idx+i];
    //         memcpy(((char *)faces + i*face_item_size), userInfo.features_db[start_idx+i], face_item_size);
    //     }
    // }
    *size = 0;
    return 0;
}
static int AddNewFaceHandler(uint16_t* face_id, void *face)
{
    LOGE(">>Add new face handler");
    int ret;
    char *feature_data = (char *)face;

    userInfo.faceID = *face_id;
    std::string name = "user_"+to_string(*face_id);
    userInfo.name = name;

//    userInfo.item_size = OASISLT_getFaceItemSize();
    int count = 0;
    for (int i=0; i< userInfo.item_size; i++){
        count++;
        userInfo.face_feature[i] = feature_data[i];
    }
    LOGD("count %d", count);
    userInfo.done_copying = true;
    LOGE("Done adding new face<<");
    return 0;
}

static int UpdateFaceHandler(uint16_t face_id, void *face)
{
    int ret;
    char *feature_data = (char *)face;
    userInfo.faceID = face_id;
    //TODO: Update db
    ret = 0;
//    ret = face_db_update(face_id, feature_data);
    if (ret >= 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

static int Oasis_Printf(const char* formatString){
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "%s", formatString);
    return 0;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_nxp_facemanager_Oasis_getOasisHeight(JNIEnv *env, jobject instance) {

    // TODO
    return OASIS_H;

}
extern "C"
JNIEXPORT jint JNICALL
Java_com_nxp_facemanager_Oasis_getOasisWidth(JNIEnv *env, jobject instance) {

    // TODO
    return OASIS_W;

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_nxp_facemanager_Oasis_getFaceItemSize(JNIEnv *env, jobject instance) {

// TODO
//    return OASISLT_getFaceItemSize() & 0x00000000ffffffff;
    return OASISLT_getFaceItemSize();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_nxp_facemanager_Oasis_cancelOp(JNIEnv *env, jobject thiz) {
    ImageFrame_t frame;
    frame.height = OASIS_H;
    frame.width = OASIS_W;
    frame.data = (unsigned char *)1;
    int flag = OASIS_DET_REC;
    userInfo.done_copying = false;
    int ret = OASISLT_run(&frame, flag, FACEREC_MINFACE, NULL);
    LOGE("op cancelled");

    return ret;
}