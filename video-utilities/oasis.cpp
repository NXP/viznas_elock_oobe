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

#include "oasislite_runtime.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "camera.h"
#include "fsl_camera_device.h"
#include "oasis.h"
#include "stdio.h"
#include <vector>
#include <string>
#include "database.h"
#include "oasis.h"
#include "sln_api_internal.h"
#include "sln_shell.h"

/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define OASIS_DETECT_MIN_FACE 100
/* User can modify the pwm range according to the real hardware.
 * To obtain good face brightness in different light conditions.
 * Also balance power consumption and recognition distance.
 */
#define IR_PWM_MIN      20
#define IR_PWM_MAX      100
#define IR_PWM_INTERVAL 10

#define WHITE_PWM_MIN      0
#define WHITE_PWM_MAX      30
#define WHITE_PWM_INTERVAL 5

struct TimeStat
{
    int det_start;
    int det_comp;
    int rec_start;
    int rec_comp;
    int emo_start;
    int emo_comp;

    int det_fps_start;
    int det_fps;

    int rec_fps_start;
    int rec_fps;

} gTimeStat;


/*******************************************************************************
 * Prototypes
 *******************************************************************************/

//__attribute__((section(".bss.$SRAM_OC")))

static void clearFaceInfo(face_info_t *face_info);
static void clearFaceInfoMsg(QUIInfoMsg *info);
static void EvtHandler(ImageFrame_t *frames[], OASISLTEvt_t evt, OASISLTCbPara_t *para, void *user_data);
// static void EvtHandler2(ImageFrame_t* frame,OASISLTEvt_t evt,OASISLTCbPara_t* para,void* user_data);
static int GetRegisteredFacesHandler(uint16_t *face_ids, void **faces, unsigned int *size);
static int AddNewFaceHandler(uint16_t *face_id, void *face, void* snapshot, int snapshot_len);
static int UpdateFaceHandler(uint16_t face_id, void *face, void* snapshot_data, int length, int offset);
static int Oasis_Printf(const char *formatString);
static int Oasis_Exit();
static void Oasis_Task(void *param);
static int Oasis_SetModelClass(OASISLTModelClass_t *model_class);
static int Oasis_SetImgType(OASISLTImageType_t *img_type);

/*******************************************************************************
 * Variables
 *******************************************************************************/
extern uint8_t g_RemoveExistingFace;
extern VIZN_api_client_t VIZN_API_CLIENT(Buttons);
extern std::string g_AddNewFaceName;
static FaceRecBuffer s_FaceRecBuf;
static QMsg gFaceInfoMsg;
static QueueHandle_t gFaceDetMsgQ = NULL;
static QMsg gFaceDetReqMsg;
static OASISLTInitPara_t s_InitPara;
static uint8_t s_lockstatus       = 1;
static OasisState s_CurOasisState = OASIS_STATE_FACE_REC_START;
static uint8_t s_appType;
int g_IsAuthenticated = 0;

volatile int g_OASISLT_heap_debug;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
DTC_BSS static StackType_t s_OasisTaskStack[OASISDETTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_OasisTaskTCB;
OCRAM_CACHED_BSS RAM_ADDRESS_ALIGNMENT(4) static uint8_t s_OasisMemPool[750 * 1024];
#endif

/*******************************************************************************
 * Code
 *******************************************************************************/

static void clearFaceInfoMsg(QUIInfoMsg *info)
{
    std::vector<std::string> featurenames;
    for (int i = 0; i < 4; i++)
    {
        info->rect[i] = -1;
    }
    DB_GetNames(&featurenames);

    memset(info->name, 0x0, 64);
    info->similar         = 1.0f;
    info->dt              = 0;
    info->rt              = 0;
    info->registeredFaces = featurenames.size();
    info->emotion         = 0;
    info->blur            = 0xFF;
    info->rgbLive         = 0xFF;
    info->front           = 0xFF;
    info->irLive          = 0xFF;
    info->irBrightness = 0xFF;
    info->irPwm = 0xFF;
    info->rgbBrightness = 0xFF;
    info->rgbPwm = 0xFF;
}

static void clearFaceInfo(face_info_t *face_info)
{
    face_info->dt               = 0;
    face_info->rt               = 0;
    face_info->enrolment        = false;
    face_info->recognize        = false;
    face_info->enrolment_mode   = 0;
    face_info->enrolment_result = 0;
    face_info->face_id          = 0;
}

static void EvtHandler(ImageFrame_t *frames[], OASISLTEvt_t evt, OASISLTCbPara_t *para, void *user_data)
{
    face_info_t face_info;
    QMsg *pQMsg;
    struct TimeStat *timeState;

    pQMsg = &gFaceInfoMsg;

    /*The memset is corupting string pointers use clearFaceInfo function.
     The MCUX11 optimize copy trait. face_info.name = std::string(name) won't make a copy. Not seen with MCUX10.*/
    // memset(&face_info, 0, sizeof(face_info_t));
    clearFaceInfo(&face_info);
    // UsbShell_Printf("[OASIS]:evt:%d\r\n",evt);

    timeState = (struct TimeStat *)user_data;
    switch (evt)
    {
        case OASISLT_EVT_DET_START:
        {
            timeState->det_start = Time_Now();
        }
        break;

        case OASISLT_EVT_DET_COMPLETE:
        {
            timeState->det_comp = Time_Now();
            pQMsg->msg.info.dt  = timeState->det_start - timeState->det_comp;
            if (para->faceBoxIR == NULL && para->faceBoxRGB == NULL)
            {
                memset(pQMsg->msg.info.name, 0x0, sizeof(pQMsg->msg.info.name));
            }

            if (para->faceBoxIR == NULL)
            {
                memset(pQMsg->msg.info.rect, -1, sizeof(para->faceBoxIR->rect));
            }
            else
            {
                memcpy(pQMsg->msg.info.rect, para->faceBoxIR->rect, sizeof(para->faceBoxIR->rect));
            }

            if (para->faceBoxRGB == NULL)
            {
                memset(pQMsg->msg.info.rect2, -1, sizeof(para->faceBoxRGB->rect));
            }
            else
            {
                // UsbShell_Printf("[EVT]:RGB detected:%d\r\n",para->faceBoxRGB->rect[0]);
                memcpy(pQMsg->msg.info.rect2, para->faceBoxRGB->rect, sizeof(para->faceBoxRGB->rect));
            }
#ifdef SHOW_FPS
            /*pit timer unit is us*/
            timeState->det_fps++;
            int diff = abs(timeState->det_fps_start - timeState->det_comp);
            if (diff > 1000000 / PIT_TIMER_UNIT && diff < 5000000 / PIT_TIMER_UNIT)
            {
                // update fps
                pQMsg->msg.info.detect_fps = timeState->det_fps * 1000.0f / diff;
                timeState->det_fps         = 0;
                timeState->det_fps_start   = timeState->det_comp;
            }
#endif
            VIZN_DetectEvent(gApiHandle,
                             (para->faceBoxIR == NULL && para->faceBoxRGB == NULL) ? -1 : pQMsg->msg.info.dt);
        }

        break;

        case OASISLT_EVT_QUALITY_CHK_START:
            break;
        case OASISLT_EVT_QUALITY_CHK_COMPLETE:
        {
            UsbShell_Printf("[OASIS]:quality chk res:%d\r\n", para->qualityResult);

            pQMsg->msg.info.irLive  = para->reserved[5];
            pQMsg->msg.info.front   = para->reserved[1];
            pQMsg->msg.info.blur    = para->reserved[3];
            pQMsg->msg.info.rgbLive = para->reserved[7];
            pQMsg->msg.info.irBrightness = para->reserved[10];
            pQMsg->msg.info.rgbBrightness = para->reserved[12];
            Camera_GetPWM(LED_IR,&pQMsg->msg.info.irPwm);
            Camera_GetPWM(LED_WHITE,&pQMsg->msg.info.rgbPwm);

            UsbShell_DbgPrintf(VERBOSE_MODE_L2,"[irBrightness]:%d\r\n",pQMsg->msg.info.irBrightness);
            UsbShell_DbgPrintf(VERBOSE_MODE_L2,"[rgbBrightness]:%d\r\n",pQMsg->msg.info.rgbBrightness);
            if (para->qualityResult == OASIS_QUALITY_RESULT_FACE_OK)
            {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]:ok!\r\n");
            }
            else if (OASIS_QUALITY_RESULT_FACE_SIDE_FACE == para->qualityResult)
            {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]:side face!\r\n");
            }
            else if (para->qualityResult == OASIS_QUALITY_RESULT_FACE_TOO_SMALL)
            {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]:Small Face!\r\n");
            }
            else if (para->qualityResult == OASIS_QUALITY_RESULT_FACE_BLUR)
            {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]: Blurry Face!\r\n");
            }
            else if (para->qualityResult == OASIS_QUALITY_RESULT_FAIL_LIVENESS_IR)
            {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]: IR Fake Face!\r\n");
            }
            else if (para->qualityResult == OASIS_QUALITY_RESULT_FAIL_LIVENESS_RGB)
            {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]: RGB Fake Face!\r\n");
            }else if (para->qualityResult == OASIS_QUALITY_RESULT_FAIL_BRIGHTNESS_DARK
                  || para->qualityResult == OASIS_QUALITY_RESULT_FAIL_BRIGHTNESS_OVEREXPOSURE) {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]: Face Brightness unfit!\r\n");
            }
        }
        break;

        case OASISLT_EVT_MASK_CHK_START:
        break;

        case OASISLT_EVT_MASK_CHK_COMPLETE: {
            if (para->maskResult == OASIS_MASK_CHECK_RESULT_FACE_WITH_MASK) {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2,"[EVT]:Face with mask!\r\n");
            } else {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2,"[EVT]:Face without mask!\r\n");
            }
        }
        break;

        case OASISLT_EVT_GLASSES_CHK_START:
        break;

        case OASISLT_EVT_GLASSES_CHK_COMPLETE: {
            if (para->glassesResult == OASIS_GLASSES_CHECK_RESULT_FACE_WITH_GLASSES) {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2,"[EVT]:Face with glasses!\r\n");
            } else {
                UsbShell_DbgPrintf(VERBOSE_MODE_L2,"[EVT]:Face without glasses!\r\n");
            }
        }
        break;

        case OASISLT_EVT_REC_START:
        {
            timeState->rec_start = Time_Now();
        }
        break;
        case OASISLT_EVT_REC_COMPLETE:
        {
            int diff;
            unsigned id                     = para->faceID;
            OASISLTRecognizeRes_t recResult = para->recResult;

            timeState->rec_comp = Time_Now();
            pQMsg->msg.info.rt  = timeState->rec_start - timeState->rec_comp;
            face_info.rt        = pQMsg->msg.info.rt;
#ifdef SHOW_FPS
            /*pit timer unit is us*/
            timeState->rec_fps++;
            diff = abs(timeState->rec_fps_start - timeState->rec_comp);
            if (diff > 1000000 / PIT_TIMER_UNIT)
            {
                // update fps
                pQMsg->msg.info.recognize_fps = timeState->rec_fps * 1000.0f / diff;
                timeState->rec_fps            = 0;
                timeState->rec_fps_start      = timeState->rec_comp;
            }
#endif
            memset(pQMsg->msg.info.name, 0x0, sizeof(pQMsg->msg.info.name));

            if (recResult == OASIS_REC_RESULT_KNOWN_FACE)
            {
                std::string name;
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[OASIS]:face id:%d\r\n", id);
                DB_GetName(id, name);
                memcpy(pQMsg->msg.info.name, name.c_str(), name.size());
                face_info.recognize = true;
                face_info.name      = std::string(name);
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[OASIS]:face id:%d name:%s\r\n", id, pQMsg->msg.info.name);
            }
            else
            {
                // face is not recognized, do nothing
                UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[OASIS]:face unrecognized\r\n");
                face_info.recognize = false;
            }

            VIZN_RecognizeEvent(gApiHandle, face_info);
        }
        break;

        case OASISLT_EVT_REG_START:
        {
        }
        break;

        case OASISLT_EVT_REG_IN_PROGRESS:
            break;
        case OASISLT_EVT_REG_COMPLETE:
        {
            unsigned id              = para->faceID;
            OASISLTRegisterRes_t res = para->regResult;
            UsbShell_Printf("[OASIS]:registration complete:%d\r\n", res);
            face_info.enrolment_result = res;
            memset(pQMsg->msg.info.name, 0x0, sizeof(pQMsg->msg.info.name));
            if ((res == OASIS_REG_RESULT_OK) || (res == OASIS_REG_RESULT_DUP))
            {
                std::string name;
                int count;
                DB_GetName(id, name);
                memcpy(pQMsg->msg.info.name, name.c_str(), name.size());
                DB_Count(&count);
                // gFaceInfoMsg.msg.info.registeredFaces = featurenames.size();
                pQMsg->msg.info.registeredFaces = count;
                // pQMsg->msg.info.updateFlag |= DISPLAY_INFO_UPDATE_NAME_SIM_RT|DISPLAY_INFO_UPDATE_NEW_REG_FACE;
                face_info.dt        = pQMsg->msg.info.dt;
                face_info.rt        = pQMsg->msg.info.rt;
                face_info.name      = std::string(pQMsg->msg.info.name);
                face_info.enrolment = true;
            }
            else
            {
                face_info.enrolment = false;
            }
            VIZN_EnrolmentEvent(gApiHandle, face_info);
        }
        break;
        default:
            break;
    }

    if (evt == OASISLT_EVT_DET_COMPLETE || evt == OASISLT_EVT_REC_COMPLETE ||
        evt == OASISLT_EVT_QUALITY_CHK_COMPLETE || evt == OASISLT_EVT_REG_COMPLETE)
    {
        pQMsg->msg.info.similar = para->reserved[0];
        Camera_SendQMsg((void *)&pQMsg);
        if (evt == OASISLT_EVT_REC_COMPLETE || evt == OASISLT_EVT_REG_COMPLETE)
        {
            UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]:sim:[%d]\r\n", para->reserved[0]);
            UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]:FF[%d][%d]\r\n", para->reserved[1], para->reserved[2]);
            UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]:Blur[%d][%d]\r\n", para->reserved[3], para->reserved[4]);
            UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[EVT]:Liveness[%d][%d]\r\n", para->reserved[5], para->reserved[6]);
        }
    }
}

static int GetRegisteredFacesHandler(uint16_t *face_ids, void **faces, unsigned int *size)
{
    /*caller ask for the total records numbers*/
    if (*size == 0)
    {
        if (APP_TYPE_USERID == s_appType)
        {
            *size = 1;
        }
        else
        {
        	DB_Count((int*)size);
        }

        return 0;
    }

    DB_GetID_FeaturePointers(face_ids,faces,*size);

    return 0;
}

static int AddNewFaceHandler(uint16_t *face_id, void *face,void* snapshot, int snapshot_len)
{
    vizn_api_status_t status;

    status = VIZN_EnrolmentAddNewFace(NULL, face_id, face);
    if (status != kStatus_API_Layer_Success)
    {
        if (status == kStatus_API_Layer_EnrolmentAddNewFace_NoMemory)
        {
            UsbShell_Printf("Maximum number of users reached\r\n");
        }
        return -1;
    }

    return 0;
}

static int UpdateFaceHandler(uint16_t face_id, void *face,void* snapshot_data, int length, int offset)
{
    int ret;
    float *feature_data = (float *)face;

    ret = DB_Update(face_id, feature_data);

    if (ret >= 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/* PWM control LED's light intensity to adjust face brightness.
 * led: which led needs to be adjusted?  LED_IR or LED_WHITE.
 * curPWM:  current pwm value.
 * direction: 1: up, need to increase brightness;  0: down, need to reduce brightness.
 */
static void Oasis_PWMControl(uint8_t led, uint8_t curPWM, uint8_t direction)
{
    uint8_t pwm;
    uint8_t pwm_min      = ((led == LED_IR) ? IR_PWM_MIN : WHITE_PWM_MIN);
    uint8_t pwm_max      = ((led == LED_IR) ? IR_PWM_MAX : WHITE_PWM_MAX);
    uint8_t pwm_interval = ((led == LED_IR) ? IR_PWM_INTERVAL : WHITE_PWM_INTERVAL);
    UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[LED:%d][curPWM:%d][dir:%d]\r\n", led, curPWM, direction);

    if (direction)
    {
        pwm = curPWM + pwm_interval;
        if (pwm >= pwm_max)
            pwm = pwm_max;
        Camera_QMsgSetPWM(led, pwm);
    }
    else
    {
        if (curPWM <= (pwm_interval + pwm_min))
            pwm = pwm_min;
        else
            pwm = curPWM - pwm_interval;
        Camera_QMsgSetPWM(led, pwm);
    }
}

//For GC0308, we can combine maunal exposure and pwm to adjust rgb face brightness.
//For MT9M114, only use pwm to adjust rgb face brightness.
static void Oasis_RGBControl(uint8_t direction)
{
    uint8_t mode = Camera_GetRGBExposureMode();
    uint8_t pwm;
    VIZN_GetPulseWidth(NULL, LED_WHITE, &pwm);
    //Camera_GetPWM(LED_WHITE,&pwm);
    UsbShell_Printf("[OASIS]:AdjustBrightnessHandler,RGB dir:%d pwm:%d mode:%d\r\n",direction, pwm, mode);
    //Oasis_PWMControl(LED_WHITE, pwm, direction);
    if (direction)
    {
        Camera_QMsgSetPWM(LED_WHITE, pwm);
        mode = (mode < CAMERA_EXPOSURE_MODE_AUTO_LEVEL3)? (mode + 1):CAMERA_EXPOSURE_MODE_AUTO_LEVEL3;
    }else
    {
        Camera_QMsgSetPWM(LED_WHITE,0);
        mode = (mode > CAMERA_EXPOSURE_MODE_AUTO_LEVEL0)? (mode-1):CAMERA_EXPOSURE_MODE_AUTO_LEVEL0;
    }
    Camera_SetRGBExposureMode(mode);
}

/* Used to dynamically adjust face brightness, user can adjust brightness by modifing LED's light intensity or using manual exposure.
 *  frame_idx: which frame needs to be adjusted on, OASISLT_INT_FRAME_IDX_RGB or OASISLT_INT_FRAME_IDX_IR ?
 *  direction: 1: up, need to increase brightness;  0: down, need to reduce brightness.
 */
static void AdjustBrightnessHandler(uint8_t frame_idx, uint8_t direction)
{
    uint8_t pwm;
    if (frame_idx == OASISLT_INT_FRAME_IDX_IR)
    {
        Camera_GetPWM(LED_IR, &pwm);
        Oasis_PWMControl(LED_IR, pwm, direction);
    }
    else
    {
        //There is a HW limitation to control LED_WHITE and LED_IR at the same time, so diable RGB part.
#if RT106F_ELOCK_BOARD
        Oasis_RGBControl(direction);
#endif
    }
}

static int Oasis_Printf(const char *formatString)
{
    UsbShell_DbgPrintf(VERBOSE_MODE_L2, formatString);
    return 0;
}

static void Oasis_SetState(OasisState state)
{
    s_CurOasisState = state;
}
static void Oasis_Task(void *param)
{
    BaseType_t ret;
    QMsg *rxQMsg              = NULL;
    QMsg *sentQMsg            = NULL;
    QMsg *infoQMsg            = NULL;
    OASISLTInitPara_t *init_p = (OASISLTInitPara_t *)param;
    ImageFrame_t frameRGB     = {(short)init_p->height, (short)init_p->width, 0, NULL};
    ImageFrame_t frameIR      = {(short)init_p->height, (short)init_p->width, 0, NULL};
    ImageFrame_t *frames[]    = {&frameRGB, &frameIR, NULL};
    uint8_t reg_mode          = 0;
    uint8_t run_flag          = OASIS_DET_REC;
    UsbShell_Printf("[OASIS DETECT]:running\r\n");

    // ask for the first frame
    gFaceInfoMsg.id = QMSG_FACEREC_INFO_UPDATE;
    infoQMsg        = &gFaceInfoMsg;
    clearFaceInfoMsg(&infoQMsg->msg.info);
    Camera_SendQMsg((void *)&infoQMsg);

    gFaceDetReqMsg.id            = QMSG_FACEREC_FRAME_REQ;
    gFaceDetReqMsg.msg.raw.data  = s_FaceRecBuf.dataIR;
    gFaceDetReqMsg.msg.raw.data2 = s_FaceRecBuf.dataRGB;
    sentQMsg                     = &gFaceDetReqMsg;
    Camera_SendQMsg((void *)&sentQMsg);

    memset(&gTimeStat, 0, sizeof(gTimeStat));

    VIZN_StartRecognition(NULL);
    while (1)
    {
#if AUTO_CALIBRATION
        // force to do liveness check in registration process
        g_AddNewFace = 1;
        s_lockstatus = 1;
#endif
        // pick up one response message.
        ret = xQueueReceive(gFaceDetMsgQ, (void *)&rxQMsg, portMAX_DELAY);

        if (ret)
        {
            switch (rxQMsg->id)
            {
                case QMSG_FACEREC_FRAME_RES:
                {
                    if (s_lockstatus != 0)
                    {
                        g_OASISLT_heap_debug = OASISHeapGetMinimumEverFreeHeapSize();
                        frameRGB.data        = s_FaceRecBuf.dataRGB;
                        frameIR.data         = s_FaceRecBuf.dataIR;
                        // if user request to add new user, enable reg mode, it not, use default mode (get enrolment
                        // mode)
                        int ret = OASISLT_run_extend(frames, run_flag, init_p->min_face, &gTimeStat);
                        if (ret)
                        {
                            UsbShell_Printf("N:%d %d\r\n", ret, g_OASISLT_heap_debug);
                            while (1)
                                ;
                        }

                        sentQMsg = &gFaceDetReqMsg;
                        Camera_SendQMsg((void *)&sentQMsg);
                    }
                }
                break;

                case QMSG_FACEREC_RECFACE:
                {
                    if (rxQMsg->msg.cmd.data.rec_face)
                    {
                        Oasis_SetState(OASIS_STATE_FACE_REC_START);
                    }
                    else
                    {
                        Oasis_SetState(OASIS_STATE_FACE_REC_STOP);
                    }
                }
                break;

                case QMSG_FACEREC_DELFACE:
                {
                    if (rxQMsg->msg.cmd.data.del_face)
                    {
                        Oasis_SetState(OASIS_STATE_FACE_DEREG_START);
                    }
                    else
                    {
                        Oasis_SetState(OASIS_STATE_FACE_DEREG_STOP);
                    }
                }
                break;

                case QMSG_FACEREC_ADDNEWFACE:
                {
                    run_flag &= ~(OASIS_REG_MODE);
                    reg_mode = (rxQMsg->msg.cmd.data.add_newface) ? OASIS_REG_MODE : 0;
                    run_flag |= reg_mode;
                    if (reg_mode == OASIS_REG_MODE)
                    {
                        Oasis_SetState(OASIS_STATE_FACE_REG_START);
                    }
                    else
                    {
                        Oasis_SetState(OASIS_STATE_FACE_REG_STOP);
                    }
                }
                break;

                case QMSG_FACEREC_STOP:
                {
                    s_lockstatus = 0;
                    //Camera_QMsgSetPWM(LED_IR, 0);
                    Camera_QMsgSetPWM(LED_WHITE, 0);
                    Camera_SetRGBExposureMode(CAMERA_EXPOSURE_MODE_AUTO_LEVEL0);
                }
                break;

                case QMSG_FACEREC_START:
                {
                    uint8_t pwm  = 0;
                    s_lockstatus = 1;
                    UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[OASIS]:QMSG_FACEREC_START!\r\n");
                    VIZN_GetPulseWidth(NULL, LED_IR, &pwm);
                    Camera_QMsgSetPWM(LED_IR, pwm);
                    infoQMsg = &gFaceInfoMsg;
                    clearFaceInfoMsg(&infoQMsg->msg.info);
                    Camera_SendQMsg((void *)&infoQMsg);
                    sentQMsg = &gFaceDetReqMsg;
                    Camera_SendQMsg((void *)&sentQMsg);
                }
                break;

                default:
                    assert(0);
            }
        }
    }
}

static int Oasis_SetImgType(OASISLTImageType_t *img_type)
{
    if (s_appType == APP_TYPE_USERID)
    {
        *img_type = OASIS_IMG_TYPE_RGB_SINGLE;
    }
    else if (s_appType == APP_TYPE_ELOCK_LIGHT || s_appType == APP_TYPE_ELOCK_HEAVY)
    {
        *img_type = OASIS_IMG_TYPE_IR_RGB_DUAL;
    }
    else if (s_appType == APP_TYPE_DOOR_ACCESS_LIGHT || s_appType == APP_TYPE_DOOR_ACCESS_HEAVY)
    {
        *img_type = OASIS_IMG_TYPE_RGB_IR_DUAL;
    }
    else
    {
        return -1;
    }
    return 0;
}

static int Oasis_SetModelClass(OASISLTModelClass_t *model_class)
{
    if (s_appType == APP_TYPE_USERID)
    {
        *model_class = OASISLT_MODEL_CLASS_LIGHT;
    }
    if (s_appType == APP_TYPE_ELOCK_LIGHT || s_appType == APP_TYPE_DOOR_ACCESS_LIGHT)
    {
        *model_class = OASISLT_MODEL_CLASS_LIGHT;
    }
    else if (s_appType == APP_TYPE_ELOCK_HEAVY || s_appType == APP_TYPE_DOOR_ACCESS_HEAVY)
    {
        *model_class = OASISLT_MODEL_CLASS_HEAVY;
    }
    else
    {
        return -1;
    }

    return 0;
}

int Oasis_Start()
{
    //uint8_t mode = Cfg_AppDataGetEmotionRecTypes();
    s_appType    = Cfg_AppDataGetApplicationType();
    int ret      = 0;

    if (s_appType != APP_TYPE_USERID)
    {
        s_FaceRecBuf.dataIR = (uint8_t *)pvPortMalloc(REC_RECT_WIDTH * REC_RECT_HEIGHT * 3);
        if (s_FaceRecBuf.dataIR == NULL)
        {
            return -1;
        }
    }

    s_FaceRecBuf.dataRGB = (uint8_t *)pvPortMalloc(REC_RECT_WIDTH * REC_RECT_HEIGHT * 3);
    if (s_FaceRecBuf.dataRGB == NULL)
    {
        return -1;
    }

    memset(&s_InitPara, 0, sizeof(s_InitPara));

    s_InitPara.img_format = OASIS_IMG_FORMAT_BGR888;

    Oasis_SetImgType(&s_InitPara.img_type);
    Oasis_SetModelClass(&s_InitPara.mod_class);

    s_InitPara.min_face = OASIS_DETECT_MIN_FACE;
    s_InitPara.cbs      = {EvtHandler,        GetRegisteredFacesHandler, AddNewFaceHandler,
                      UpdateFaceHandler, AdjustBrightnessHandler,   (void *)Oasis_Printf};

    s_InitPara.enable_flags = 0;
    if (s_appType != APP_TYPE_USERID)
    {
        s_InitPara.enable_flags |= OASIS_ENABLE_MULTI_VIEW;
    }
    s_InitPara.false_accept_rate = OASIS_FAR_1_1000000;
    s_InitPara.enable_flags |= (Cfg_AppDataGetLivenessMode() == LIVENESS_MODE_ON) ? OASIS_ENABLE_LIVENESS : 0;


    s_InitPara.height = REC_RECT_HEIGHT;
    s_InitPara.width  = REC_RECT_WIDTH;

    ret = OASISLT_init(&s_InitPara);

    if (ret == OASIS_INIT_INVALID_MEMORYPOOL)
    {
#if (configSUPPORT_STATIC_ALLOCATION == 1)
        if (s_InitPara.size <= sizeof(s_OasisMemPool))
        {
            s_InitPara.mem_pool = (char *)s_OasisMemPool;
            s_InitPara.size = sizeof(s_OasisMemPool);
        }
        else
        {
            s_InitPara.mem_pool = NULL;
        }
#else
        s_InitPara.mem_pool = (char *)pvPortMalloc(s_InitPara.size);
#endif

        if (s_InitPara.mem_pool == NULL)
        {
            UsbShell_Printf("[ERROR]: Unable to allocate memory for oasis mem pool\r\n");
            while (1)
                ;
        }
        ret             = OASISLT_init(&s_InitPara);
    }

    assert(ret == 0);

    /*get authentication result*/
    g_IsAuthenticated = s_InitPara.auth;

    gFaceDetMsgQ = xQueueCreate(FACEREC_MSG_Q_COUNT, sizeof(QMsg *));

    if (gFaceDetMsgQ == NULL)
    {
        UsbShell_Printf("[ERROR]:xQueueCreate facedet queue\r\n");
        return -1;
    }

    UsbShell_Printf("[OASIS]:start\r\n");

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(Oasis_Task, "Oasis Task", OASISDETTASK_STACKSIZE, &s_InitPara, OASISDETTASK_PRIORITY,
                                  s_OasisTaskStack, &s_OasisTaskTCB))
#else
    if (xTaskCreate(Oasis_Task, "Oasis Task", OASISDETTASK_STACKSIZE, &s_InitPara, OASISDETTASK_PRIORITY, NULL) !=
        pdPASS)
#endif
    {
        UsbShell_Printf("[ERROR]:oasis Task created failed\r\n");
        Oasis_Exit();
        while (1)
            ;
    }

    UsbShell_Printf("[OASIS]:starting\r\n");
    return ret;
}

static int Oasis_Exit()
{
    int ret = OASISLT_uninit();
    if (s_appType != APP_TYPE_USERID)
    {
        vPortFree(s_FaceRecBuf.dataIR);
    }
#if !(configSUPPORT_STATIC_ALLOCATION == 1)
    vPortFree(s_InitPara.mem_pool);
#endif
    vPortFree(s_FaceRecBuf.dataRGB);

    return ret;
}

int Oasis_SendQMsg(void *msg)
{
    BaseType_t ret;
    ret = xQueueSend(gFaceDetMsgQ, msg, (TickType_t)0);

    if (ret != pdPASS)
    {
        UsbShell_Printf("[ERROR]:FaceDet_SendQMsg failed\r\n");
        return -1;
    }

    return 0;
}
