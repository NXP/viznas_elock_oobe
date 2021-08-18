/*
* Copyright 2020 NXP.
* This software is owned or controlled by NXP and may only be used strictly in accordance with the
* license terms that accompany it. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you
* agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
* applicable license terms, then you may not retain, install, activate or otherwise use the software.
*
*/

#ifndef OASIS_LT_2D_RT_H
#define OASIS_LT_2D_RT_H
#include "stdbool.h"
#include "stdint.h"


#define VERSION_MAJOR 4
#define VERSION_MINOR 54
/*this version number only used for hot fix on frozen release or branch*/
#define VERSION_HOTFIX 0


#define INVALID_FACE_ID (0xFFFFUL)


/*these macros are used in OASISLT_init, they can be combined to indicate
 * what functions should be enabled in OASIS LIB.*/
enum {
    /*To control liveness is enable or not*/
    OASIS_ENABLE_LIVENESS = 1U << 0,
    /*This flag is used to enable/disable multi-view check in face registration*/
    OASIS_ENABLE_MULTI_VIEW = 1U << 1,
    /*This flag used for brightness check on face recognitions*/
    OASIS_ENABLE_FACE_REC_BRIGHTNESS_CHECK = 1U << 2,
    /*This flag is only valid for library with mask face support*/
    OASIS_ENABLE_MASK_FACE_REC = 1U << 3,
    /*This flag decide whether face feature smart learning is enabled or not*/
    OASIS_ENABLE_FACE_FEA_SMART_LEARNING = 1U << 4,
    OASIS_ENABLE_INVALID = 0xFF
};


typedef enum {
    OASISLT_OK = 0,
    OASIS_INIT_INVALID_PARAMETERS,
    OASIS_INIT_INVALID_MODEL_CLASS,
    OASIS_INIT_ALREADY_INIT,
    OASIS_INIT_INVALID_FAR,
    OASIS_INIT_INVALID_CB,  //5
    OASIS_INIT_MEMORYPOOL_SMALL,
    OASIS_INIT_INVALID_MEMORYPOOL,
    OASIS_INIT_INVALID_IMAGE_MIN_DIM,
    OASIS_INIT_INVALID_MASK_BUF,
    OASIS_INIT_INVALID_IMG_TYPE_FOR_MASK_FACE, //10
    OASIS_INIT_MASK_REC_NOTSUPORTED,
    OASIS_INIT_INVALID_IMAGE_TYPE,
    OASIS_INIT_INSTANCE_NUM_OVERFLOW,
    OASIS_INIT_AUTHENTICATION_FAIL,
    OASIS_INIT_MEM_ALLOCATE_FAIL,   //15
    OASISLT_SNAPSHOT_INVALID_INPUT_PARAMETERS,
    OASISLT_SNAPSHOT_LIB_UNINIT,
    OASISLT_SNAPSHOT_INVALID_FRAME_NUM,
    OASISLT_SNAPSHOT_IMG_TYPE_NOT_SUPPORT,
    OASISLT_SNAPSHOT_RESIZE_FAILED,  //20

} OASISLTResult_t;



typedef enum {
    OASIS_DET_ONLY = 0,
    OASIS_DET_WITH_QUALITY_CHECK,
    OASIS_DET_REC,
    OASIS_DET_REC_REG,    //face registration
    OASIS_DET_REC_DEREG,  //face deregistration
    OASIS_DET_REC_REG_REMOTE, //registration with pictures
    OASIS_RUN_FLAG_NUM
} OASISRunFlag_t;


typedef enum {
    OASIS_REC_RESULT_KNOWN_FACE,
    OASIS_REC_RESULT_UNKNOWN_FACE,
    OASIS_REC_RESULT_INVALID = 0xFF
} OASISLTRecognizeRes_t;


typedef enum {
    /*these results are used by event OASISLT_EVT_REG_COMPLETE*/
    OASIS_REG_RESULT_OK,
    OASIS_REG_RESULT_DUP,
    OASIS_REG_RESULT_CANCELED,
    OASIS_REG_RESULT_DB_OP_FAILED,
    OASIS_REG_RESULT_INVALID = 0xFF
} OASISLTRegisterRes_t;


typedef enum {
    /*these results are used by event OASISLT_EVT_DEREG_COMPLETE*/
    OASIS_DEREG_RESULT_OK,
    OASIS_DEREG_RESULT_CANCELED,
    OASIS_DEREG_RESULT_DB_OP_FAILED,
    OASIS_DEREG_RESULT_INVALID = 0xFF
} OASISLTDeregisterRes_t;


typedef enum {
    /*these results are used by event OASISLT_EVT_QUALITY_CHK_COMPLETE*/
    OASIS_QUALITY_RESULT_FACE_OK,
    OASIS_QUALITY_RESULT_FACE_TOO_SMALL,
    OASIS_QUALITY_RESULT_FACE_SIDE_FACE,
    OASIS_QUALITY_RESULT_FACE_BLUR,
    OASIS_QUALITY_RESULT_FAIL_LIVENESS_IR,
    OASIS_QUALITY_RESULT_FAIL_LIVENESS_RGB,  //5
    OASIS_QUALITY_RESULT_FAIL_LIVENESS_3D,
    OASIS_QUALITY_RESULT_FAIL_BRIGHTNESS_DARK,
    OASIS_QUALITY_RESULT_FAIL_BRIGHTNESS_OVEREXPOSURE,
    OASIS_QUALITY_RESULT_INVALID = 0xFF
} OASISLTFaceQualityRes_t;

typedef enum {
    OASIS_GLASSES_CHECK_RESULT_FACE_WITHOUT_GLASSES,
    OASIS_GLASSES_CHECK_RESULT_FACE_WITH_GLASSES,
    OASIS_GLASSES_CHECK_RESULT_INVALID = 0xFF
} OASISLTFaceGlassesCheckRes_t;

typedef enum {
    OASIS_MASK_CHECK_RESULT_FACE_WITHOUT_MASK,
    OASIS_MASK_CHECK_RESULT_FACE_WITH_MASK,
    OASIS_MASK_CHECK_RESULT_INVALID = 0xFF
} OASISLTFaceMaskCheckRes_t;


typedef enum {
    OASIS_FAR_1_10000,
    OASIS_FAR_1_100000,
    OASIS_FAR_1_1000000,
    OASIS_FAR_NUM,
    OASIS_FAR_INVALID = 0xFF
} OASISLTFar_t;

/*Face recognition Model class:
 * light model means a light calculation,fast speed and a lower accuracy.
 * heavy model means a heavy calculation,lower speed and a higher accuracy.
 * */
typedef enum {
    OASISLT_MODEL_CLASS_LIGHT,
    OASISLT_MODEL_CLASS_HEAVY,
    OASISLT_MODEL_CLASS_NUM
} OASISLTModelClass_t;

typedef enum {
    OASIS_IMG_FORMAT_RGB888, // 3 channels
    OASIS_IMG_FORMAT_BGR888, // 3 channels

    //theses formats are used internal only
    OASIS_IMG_FORMAT_GREY888,//3 channels
    OASIS_IMG_FORMAT_GREY8,   // 1 channel
    OASIS_IMG_FORMAT_NUM,
    OASIS_IMG_FORMAT_INVALID = 0xFF
} OASISLTImageFormat_t;

typedef enum {
    OASIS_IMG_TYPE_RGB_SINGLE,  //input only RGB frame
    OASIS_IMG_TYPE_IR_SINGLE,   //input only IR frame
    OASIS_IMG_TYPE_IR_RGB_DUAL, //IR and RGB frames, do face recognition on IR frame
    OASIS_IMG_TYPE_RGB_IR_DUAL, //IR and RGB frames, do face recognition on RGB frame
    OASIS_IMG_TYPE_IR_RGB_3D_TRIPLE,//IR, RGB and 3D frames, do face recognition on IR frame
    OASIS_IMG_TYPE_RGB_IR_3D_TRIPLE,//IR, RGB and 3D frames, do face recognition on RGB frame
    OASIS_IMG_TYPE_NUM,
    OASIS_IMG_TYPE_INVALID = 0xFF
} OASISLTImageType_t;

/*idx of input frames*/
enum {
    OASISLT_INT_FRAME_IDX_RGB,
    OASISLT_INT_FRAME_IDX_IR,
    OASISLT_INT_FRAME_IDX_3D,
    OASISLT_INT_FRAME_IDX_LAST
};

/*Landmark index*/
enum {
    OASISLT_LM_LEFT_EYE_X,
    OASISLT_LM_RIGHT_EYE_X,
    OASISLT_LM_NOSE_X,
    OASISLT_LM_MOUTH_LEFT_CORNER_X,
    OASISLT_LM_MOUTH_RIGHT_CORNER_X,
    OASISLT_LM_LEFT_EYE_Y, /*5*/
    OASISLT_LM_RIGHT_EYE_Y,
    OASISLT_LM_NOSE_Y,
    OASISLT_LM_MOUTH_LEFT_CORNER_Y,
    OASISLT_LM_MOUTH_RIGHT_CORNER_Y,
    OASISLT_LM_IDX_NUM
};


typedef struct FBox_ {
    int rect[4];    // left, top, right, bottom.
    float fld[OASISLT_LM_IDX_NUM];  // 5 landmark point.
} FBox;

typedef struct {
    short height;
    short width;
    OASISLTImageFormat_t fmt; // indicate the format of this frame
    unsigned char* data;
} ImageFrame_t;


typedef struct {
    FBox* faceBoxIR; //face rect and landmark on IR image
    FBox* faceBoxRGB; //face rect and landmark on RGB image
    uint16_t faceID;//only valid when a face recognized or registered
    OASISLTRegisterRes_t regResult; // only valid for registration
    OASISLTDeregisterRes_t deregResult; //only valid for deregistration
    OASISLTRecognizeRes_t recResult;//only valid for face recognition
    OASISLTFaceQualityRes_t qualityResult;//only valid for face quality check event.
    OASISLTFaceMaskCheckRes_t maskResult;//only valid for face mask check event.
    OASISLTFaceGlassesCheckRes_t glassesResult;//only valid for face glasses check event.
#define OASISLT_CB_PARA_RESERVED_INT 16
    int reserved[OASISLT_CB_PARA_RESERVED_INT];//this field is reserved for debugging purpose
} OASISLTCbPara_t;

typedef enum {
    /*indicate the start of face detection, user can update frame data if it is needed.
    * all parameter in callback parameter is invalid.*/
    OASISLT_EVT_DET_START,

    /*The end of face detection.
     *if a face is found, pfaceBox(OASISLTCbPara_t) indicated the rect(left,top,right,bottom point value)
     *info and landmark value of the face.
     *if no face is found,pfaceBox is NULL, following event will not be triggered for current frame.
     *other parameter in callback parameter is invalid */
    OASISLT_EVT_DET_COMPLETE,

    /*Face quality check is done before face recognition*/
    OASISLT_EVT_QUALITY_CHK_START,
    OASISLT_EVT_QUALITY_CHK_COMPLETE,

    /*Start of face recognition*/
    OASISLT_EVT_REC_START,

    /*The end of face recognition.
     * when face feature in current frame is gotten, GetRegisteredFaces callback will be called to get all
     * faces feature registered and OASIS lib will try to search this face in registered faces, if this face
     * is matched, a valid face ID will be set in callback parameter faceID and corresponding simularity(indicate
     * how confidence for the match) also will be set.
     * if no face match, a invalid(INVALID_FACE_ID) will be set.*/
    OASISLT_EVT_REC_COMPLETE,

    /*if user set a registration flag in a call of OASISLT_run and a face is detected, this two events will be notified
     * for auto registration mode, only new face(not recognized) is added(call AddNewFace callback function)
     * for manu registration mode, face will be added forcely.
     * for both cases, face ID of new added face will be set in callback function */
    OASISLT_EVT_REG_START,
    /*when registration start, for each valid frame is handled,this event will be triggered and indicate
     * registration process is going forward a little.
     * */
    OASISLT_EVT_REG_IN_PROGRESS,
    OASISLT_EVT_REG_COMPLETE,

    //for face deregistration
    OASISLT_EVT_DEREG_START,
    OASISLT_EVT_DEREG_COMPLETE,

    OASISLT_EVT_NUM

} OASISLTEvt_t;

typedef void* OASISLTHandler_t;

/*this callback definition is used for calling of OASISLT_run
 *for single frame type, only frames[0] is valid and it points to the single input frame
 *for dual frame type, frames[0] point to RGB frame input while frames[1] point to IR frame input
 *for triple frame type,frames[0] point to RGB frame input while frames[1] point to IR frame input, frame[2] point to 3D frame input
 *frames [input]: frames input.
 *evt [input]: event type
 *para [input]: parameters related to this event.
 *user_data [input]: user data transferring from OASISLT_run_extend
 **/
typedef void (*OASISLTEvtCb)(ImageFrame_t* frames[OASISLT_INT_FRAME_IDX_LAST], OASISLTEvt_t evt, OASISLTCbPara_t* para, void* user_data);


/*OASIS LITE would use this fun to  get all registered face array.
 * face_id[output]: buffer used to save face ID array returned. This buffer size is sizeof(*face_id)*face_num
 * pfaces[output]: buffer used to save pointer array, each pointer point to a face records data
 * this buffer size is *face_num*4
 * face_num[input/output]: as input, it indicates the number "face_id" and "pFaces"
 * as output, it indicates how many items in face_id array and pfaces returned actually.
 * specially, if *face_num is 0, actual face record number should be set in *face_num before return.
 * in this case, no "face_id" and 'pFaces' be returned.
 * user_data [input]: user data transferring from OASISLT_run_extend
 * return 0 if call succeed, otherwise failed.
 *  */
typedef int (*GetRegisteredFaces)(uint16_t* face_id, void** pFaces, uint32_t* face_num, void* userData);

/*in register mode, OASIS LITE would call this function to add new face data to database.
 *face_data:[input]pointer to the faces data, the data length can be get by OASISLT_getFaceItemSize()
 *face_id: [output]this id identifies this face uniquely, it should be generated by the caller and
           return back to library.it is used in following face update/recognition procedure.
 *snapshot:[input]snapshot data corresponding to this face. it can be saved for face feature generating
           purpose. if caller don't need it,ignore it.
 *snapshot_length:[input]snapshot data length.
 *user_data [input]: user data transferring from OASISLT_run_extend/OASISLT_registration_by_feature
 *return 0 if succeed; otherwise failed*/
typedef int (*FaceOperationAdd)(uint16_t* face_id, void* face_data, void* snapshot, int snapshot_length, void* userData);

/*When library beleives a face data/snapshot need to be update during face recognition procedure,
this function will be called to update corresponding data in database.
*face_id:[input] which face data should be update.
*face_data:[input] new face data which should replace original face data in whole.
*snapshot_data:[input] new data which should be replace orignal snapshot.
*data_length:[input] snapshot data length.
*offset:[input] in most cases, not whole snapshot need update, offset indicate start position of
        snapshot where snapshot_data should be writen.
*user_data [input]: user data transferring from OASISLT_run_extend
*return 0 if successful, other value means failed.
*/
typedef int (*FaceOperationUpdate)(uint16_t face_id, void* face_data, void* snapshot_data, int data_length,
                                   int offset, void* userData);


/*when library is going to delete a face feature, this function will be called
 * return 0 if successful, other value means failed.
 * */
typedef int (*FaceOperationDelete)(uint16_t face_id, void* userData);

/*Using for print out ANSI string in self test API*/
typedef void (*StringPrint)(const char* str);

typedef void (*EnterCriticalArea)(void);
typedef void (*ExitCriticalArea)(void);

/*Used to dynamically adjust face brightness
  * frame_idx: which frame is need to be adjusted on, OASISLT_INT_FRAME_IDX_RGB or OASISLT_INT_FRAME_IDX_IR ?
  * direction: 1: up 0: down*/
typedef void (*FaceBrightnessAdjust)(uint8_t frame_idx, uint8_t direction, void* userData);


typedef struct {
    const uint8_t* model;
    const uint8_t* model_data;
    uint32_t outputID;

    //input HWC
    int input_h, input_w, input_c;

    //face recognition threshold
    float th;
    //how many features is output?
    int output_c;
} OASISLTCustFaceRec_t;


typedef struct {
    const uint8_t* model;
    const uint8_t* model_data;
    uint32_t outputID;

    //input HWC
    int input_h, input_w, input_c;
    //liveness threshold
    float th;
} OASISLTCustLiveness_t;


typedef struct {
    /*This callback function is called when any event start/complete inside library.
     * It can be NULL if caller does not care about any event.*/
    OASISLTEvtCb EvtCb;
    /*By this function, library can get how many face are registered or get a part of registered face items.
     * It can not be NULL.*/
    GetRegisteredFaces GetFaces;
    /*By this function, library can save a face item record to somewhere( it depend on caller, can be ram/disk)
     * If is NULL, no face item should be saved.*/
    FaceOperationAdd AddFace;

    /*By this function,library can update a face item. if is NULL, no update will be done.*/
    FaceOperationUpdate UpdateFace;

    FaceOperationDelete DeleteFace;

    /*By this function, caller can know RGB and IR image's brightness according input parameters */
    FaceBrightnessAdjust AdjustBrightness;

    /*These 2 callback functions are used for multi-thread support. can be set to NULL for non-multi-thread environment*/
    EnterCriticalArea lock;
    ExitCriticalArea unlock;

    //internal debugging use only
    void* reserved;

} InfCallbacks_t;

typedef struct {
    //max input image height, width and channel, min_face: minimum face can be detected
    int height;
    int width;

    OASISLTImageType_t img_type;

    //min_face should not smaller than 40
    int min_face;

    /*memory pool pointer, this memory pool should only be used by OASIS LIB*/
    char* mem_pool;

    /*memory pool size*/
    int size;

    /*callback functions provided by caller*/
    InfCallbacks_t cbs;

    /*what functions should be enabled in OASIS LIB*/
    uint8_t enable_flags;

    /*false accept rate*/
    OASISLTFar_t false_accept_rate;

    /*model class */
    OASISLTModelClass_t mod_class;

} OASISLTInitPara_t;


enum {
    OASISLT_RUN_IDENTIFY_RESULT_OK,
    OASISLT_RUN_IDENTIFY_RESULT_NO_FACE_ON_BOTH,
    OASISLT_RUN_IDENTIFY_RESULT_NO_FACE_ON_INPUT,
    OASISLT_RUN_IDENTIFY_RESULT_NO_FACE_ON_TARGET,
    OASISLT_RUN_IDENTIFY_RESULT_PARAM_ERROR,
    OASISLT_RUN_IDENTIFY_RESULT_LIB_UNINIT,
    OASISLT_RUN_IDENTIFY_RESULT_FAIL_UNKNOW,

};

#ifdef __cplusplus
extern "C" {
#endif
//=====================================================================
/*These APIs are used for all libraries.*/
//=====================================================================

/* Initialize OASIS LITE lib, it should be called before any other APIs.
 * para: initializing parameter. refer to OASISLTInitPara_t for detail information.
 * */
OASISLTResult_t OASISLT_init(OASISLTInitPara_t* para);

OASISLTResult_t OASISLT_uninit();

/*return version information string of OASIS LITE library, please note that:
 * string buffer size input should not less than 64 bytes
 * verStrBuf: buffer used to save version string.
 * length: verStrBuf lenght, unit: byte, it should not less than 64*/
/*return minimum free memory size since OASISLT initialization.
 **/
unsigned int OASISHeapGetMinimumEverFreeHeapSize();

/*return the size of each face feature. unit: byte*/
uint32_t OASISLT_getFaceItemSize(void);

void OASISLT_getVersion(char* verStrBuf, int length);


//=====================================================================
/*These APIs are used only for RT106F and RT117F libraries.*/
//=====================================================================
/*this API can be used to replace OASISLT_run and OASISLT_run2D API with a more flexiable input parameters.
 * user can input RGB/IR/3D frame with different combinations according image types in intializing.
 * this API also can be used to extract feature from a given image, user can get face feature by AddFace
 * callback.
 * */
//int OASISLT_run_extend(ImageFrame_t* frames[OASISLT_INT_FRAME_IDX_LAST], uint8_t flag, int minFace, void* userData);
int OASISLT_run_extend(ImageFrame_t* frames[OASISLT_INT_FRAME_IDX_LAST],
                       uint8_t flag, int minFace, void* userData);



/*Used to compare and get similarity of faces in input image and target image.
 * OASIS_IMG_FORMAT_RGB888 and OASIS_IMG_FORMAT_BGR888 are supported*/
int OASISLT_run_identification(ImageFrame_t* input,
                               ImageFrame_t* target,
                               float* sim);



/*This function is used for registration by a face feature generated by OASISLT_run_extend and AddFace
 * callback.
 * face_data: [input] face feature is trying to be added to face database.
 * user_data:[input] this user data is going to be transfered to getFace/addFace/updateFace directly.
 * id:[input/output] if *id equal to INVALID_FACE_ID, it means registration by given feature, when registration
 * successfully, corresponding id will be returned; if *id is not INVALID_FACE_ID, it means a feature update,
 * face data of corresponding "id" will be updated with given face data
 * if registration successful, it is a unique ID allocated for the input feature
 * snapshot/snapshot_len:[input] if there is snapshot, it can be also saved.
 * */
OASISLTRegisterRes_t OASISLT_registration_by_feature(void* face_data,
        void* snapshot, int snapshot_len, uint16_t* id, void* user_data);




//=====================================================================
/*These APIs are used only for Linux64 and Android libraries.*/
//=====================================================================
OASISLTRegisterRes_t OASISLT_MT_registration_by_feature(OASISLTHandler_t handler, void* face_data,
        void* snapshot, int snapshot_len, uint16_t* id, void* user_data);
int OASISLT_MT_run_identification(OASISLTHandler_t handler,
                                  ImageFrame_t* input,
                                  ImageFrame_t* target,
                                  float* sim);
int OASISLT_MT_run_extend(OASISLTHandler_t handler, ImageFrame_t* frames[OASISLT_INT_FRAME_IDX_LAST],
                          uint8_t flag, int minFace, void* userData);
/*These 2 callback functions are used for multi-thread support. can be set to NULL for non-multi-thread environment*/
OASISLTResult_t OASISLT_CreateInstance(OASISLTHandler_t* pHandler);
OASISLTResult_t OASISLT_DeleteInstance(OASISLTHandler_t handler);

#ifdef __cplusplus
}
#endif

#endif
