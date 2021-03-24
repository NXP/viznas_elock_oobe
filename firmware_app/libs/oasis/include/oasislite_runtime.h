/*
* Copyright 2020 NXP.
* This software is owned or controlled by NXP and may only be used strictly in accordance with the
* license terms that accompany it. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you
* agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
* applicable license terms, then you may not retain, install, activate or otherwise use the software.
*
*/

#ifndef OASIS_LT_RT_H
#define OASIS_LT_RT_H
#include "stdbool.h"
#include "stdint.h"


#define VERSION_MAJOR 4
#define VERSION_MINOR 34
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


#define OASIS_REG_MODE_SHIFT (5)
typedef enum {
    OASISLT_OK = 0,
    OASIS_INIT_INVALID_PARAMETERS,
    OASIS_INIT_INVALID_MODEL_CLASS,
    OASIS_INIT_ALREADY_INIT,
    OASIS_INIT_INVALID_FAR,
    OASIS_INIT_INVALID_CB,
    OASIS_INIT_MEMORYPOOL_SMALL,
    OASIS_INIT_INVALID_MEMORYPOOL,
    OASIS_INIT_INVALID_IMAGE_MIN_DIM,
    OASIS_INIT_INVALID_MASK_BUF,
	OASIS_INIT_INVALID_IMG_TYPE_FOR_MASK_FACE,
    OASIS_INIT_MASK_REC_NOTSUPORTED,
    OASIS_INIT_INVALID_IMAGE_TYPE,
	OASISLT_SNAPSHOT_INVALID_INPUT_PARAMETERS,
	OASISLT_SNAPSHOT_LIB_UNINIT,
	OASISLT_SNAPSHOT_INVALID_FRAME_NUM,
	OASISLT_SNAPSHOT_IMG_TYPE_NOT_SUPPORT,
	OASISLT_SNAPSHOT_RESIZE_FAILED,

} OASISLTResult_t;



typedef enum {
    OASIS_DET_ONLY = 0,
    OASIS_DET_REC,
    OASIS_RUN_FLAG_NUM,

    //reg mode can be ored with OASIS_DET_REC,OASIS_DET_REC_EMO flags if needed
    OASIS_REG_MODE = 1 << OASIS_REG_MODE_SHIFT,
    OASIS_INVALID_RUN_FLAG = 0xFF
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
    OASIS_REG_RESULT_WITHMASK,
    OASIS_REG_RESULT_DB_OP_FAILED,
    OASIS_REG_RESULT_INVALID = 0xFF
} OASISLTRegisterRes_t;


typedef enum {
    /*these results are used by event OASISLT_EVT_QUALITY_CHK_COMPLETE*/
    OASIS_QUALITY_RESULT_FACE_OK,
    OASIS_QUALITY_RESULT_FACE_TOO_SMALL,
    OASIS_QUALITY_RESULT_FACE_SIDE_FACE,
    OASIS_QUALITY_RESULT_FACE_BLUR,
    OASIS_QUALITY_RESULT_FAIL_LIVENESS_IR,
    OASIS_QUALITY_RESULT_FAIL_LIVENESS_RGB,
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
    OASIS_IMG_FORMAT_GREY,   // 1 channel
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
    //this int is used for internal purpose, must be set as 0 on initialization
    int reserved0;
    unsigned char* data;
} ImageFrame_t;


typedef struct {
    FBox* faceBoxIR; //face rect and landmark on IR image
    FBox* faceBoxRGB; //face rect and landmark on RGB image
    uint16_t faceID;//only valid when a face recognized or registered
    OASISLTRegisterRes_t regResult; // only valid for registration
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

    /*Face mask check is done after quality check and before face recognition
     * These events is only valid for library with mask face support*/
    OASISLT_EVT_MASK_CHK_START,
    OASISLT_EVT_MASK_CHK_COMPLETE,

    /*Face glasses check is done after quality check and  before face recognition*/
    OASISLT_EVT_GLASSES_CHK_START,
    OASISLT_EVT_GLASSES_CHK_COMPLETE,

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
    OASISLT_EVT_NUM

} OASISLTEvt_t;


/*this callback definition is used for calling of OASISLT_run
 *for single frame type, only frames[0] is valid and it points to the single input frame
 *for dual frame type, frames[0] point to RGB frame input while frames[1] point to IR frame input
 *for triple frame type,frames[0] point to RGB frame input while frames[1] point to IR frame input, frame[2] point to 3D frame input
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
 * return 0 if call succeed, otherwise failed.
 *  */
typedef int (*GetRegisteredFaces)(uint16_t* face_id, void** pFaces, unsigned int* face_num);

/*in register mode, OASIS LITE would call this function to add new face data to database.
 *face_data:[input]pointer to the faces data, the data length can be get by OASISLT_getFaceItemSize()
 *face_id: [output]this id identifies this face uniquely, it should be generated by the caller and
           return back to library.it is used in following face update/recognition procedure.
 *snapshot:[input]snapshot data corresponding to this face. it can be saved for face feature generating
           purpose. if caller don't need it,ignore it.
 *snapshot_length:[input]snapshot data length.
 *return 0 if succeed; otherwise failed*/
typedef int (*FaceOperationAdd)(uint16_t* face_id, void* face_data, void* snapshot, int snapshot_length);

/*When library beleives a face data/snapshot need to be update during face recognition procedure,
this function will be called to update corresponding data in database.
*face_id:[input] which face data should be update.
*face_data:[input] new face data which should replace original face data in whole.
*snapshot_data:[input] new data which should be replace orignal snapshot.
*data_length:[input] snapshot data length.
*offset:[input] in most cases, not whole snapshot need update, offset indicate start position of
        snapshot where snapshot_data should be writen.
*/
typedef int (*FaceOperationUpdate)(uint16_t face_id, void* face_data, void* snapshot_data, int data_length, 
int offset);

/*Using for print out ANSI string in self test API*/
typedef void (*StringPrint)(const char* str);

/*Used to get current system millisecond number*/
typedef uint32_t (*GetSystemCurrentMS)(void);

/*Used to dynamically adjust face brightness
  * frame_idx: which frame is need to be adjusted on, OASISLT_INT_FRAME_IDX_RGB or OASISLT_INT_FRAME_IDX_IR ?
  * direction: 1: up 0: down*/
typedef void (*FaceBrightnessAdjust)(uint8_t frame_idx, uint8_t direction);


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

    /*By this function, caller can know RGB and IR image's brightness according input parameters */
    FaceBrightnessAdjust AdjustBrightness;

    //internal debugging use only
    void* reserved;

} InfCallbacks_t;

typedef struct {
    //max input image height, width and channel, min_face: minimum face can be detected
    int height;
    int width;

    //only valid for RGB images; for IR image, always GREY888 format
    OASISLTImageFormat_t img_format;
    OASISLTImageType_t img_type;

    //min_face should not smaller than 40
    int min_face;

    /*memory pool pointer, this memory pool should only be used by OASIS LIB*/
    char* mem_pool;

    /*memory pool size*/
    int size;

    /*output parameter,indicate authenticated or not*/
    int auth;

    /*callback functions provided by caller*/
    InfCallbacks_t cbs;

    /*what functions should be enabled in OASIS LIB*/
    uint8_t enable_flags;

    /*false accept rate*/
    OASISLTFar_t false_accept_rate;

    /*model class */
    OASISLTModelClass_t mod_class;

} OASISLTInitPara_t;



#ifdef __cplusplus
extern "C" {
#endif
/* Initialize OASIS LITE lib, it should be called before any other APIs.
 * para: initializing parameter. refer to OASISLTInitPara_t for detail information.
 * */
OASISLTResult_t OASISLT_init(OASISLTInitPara_t* para);
OASISLTResult_t OASISLT_uninit(void);

/*return version information string of OASIS LITE library, please note that:
 * string buffer size input should not less than 64 bytes
 * verStrBuf: buffer used to save version string.
 * length: verStrBuf lenght, unit: byte, it should not less than 64*/
void OASISLT_getVersion(char* verStrBuf, int length);


/*this API can be used to replace OASISLT_run and OASISLT_run2D API with a more flexiable input parameters.
 * user can input RGB/IR/3D frame with different combinations according image types in intializing.
 * */
int OASISLT_run_extend(ImageFrame_t* frames[OASISLT_INT_FRAME_IDX_LAST], uint8_t flag, int minFace, void* userData);


enum {
    OASISLT_RUN_IDENTIFY_RESULT_OK,
    OASISLT_RUN_IDENTIFY_RESULT_NO_FACE_ON_BOTH,
    OASISLT_RUN_IDENTIFY_RESULT_NO_FACE_ON_INPUT,
    OASISLT_RUN_IDENTIFY_RESULT_NO_FACE_ON_TARGET,
    OASISLT_RUN_IDENTIFY_RESULT_PARAM_ERROR,
    OASISLT_RUN_IDENTIFY_RESULT_LIB_UNINIT,
    OASISLT_RUN_IDENTIFY_RESULT_FAIL_UNKNOW,

};
/*Used to compare and get similarity of faces in input image and target image.
 * OASIS_IMG_FORMAT_RGB888 and OASIS_IMG_FORMAT_BGR888 are supported*/
int OASISLT_run_identification(ImageFrame_t* input, ImageFrame_t* target, float* sim);


/*Used to extract face data(features) from a snapshot.
 * snapshot:[input] point to snapshot packet.
 * snapshot_length:[input] length of snapshot packet in bytes.
 * face_data:[output] point to a buffer which is used for saving of face data extracted. The length of
 * this buffer should not less than OASISLT_getFaceItemSize().
 * OASIS_IMG_FORMAT_RGB888 and OASIS_IMG_FORMAT_BGR888 are supported*/
OASISLTResult_t OASISLT_snapshot2feature(const void* snapshot, int snapshot_lenght,void* face_data);


/*return minimum free memory size since OASISLT initialization.
 **/
unsigned int OASISHeapGetMinimumEverFreeHeapSize(void);

/*return the size of each face feature. unit: byte*/
uint32_t OASISLT_getFaceItemSize(void);


/*OASIS LITE runtime library self test. Internal use only*/
int OASISLT_selftest(void* mempool, int size, StringPrint print, GetSystemCurrentMS getMS);


#ifdef __cplusplus
}
#endif

#endif
