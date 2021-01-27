/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "database.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "generic_list.h"
#include "sln_api.h"
#include "sln_api_internal.h"
#include "sln_oasis_state.h"
#include "sln_dev_cfg.h"
#include "sln_timers.h"
#include "sln_app_fwupdate.h"
#include "sln_pcal.h"

#include "commondef.h"
#include <assert.h>
#include "sln_lpm.h"

#include "fsl_log.h"
#include "sln_pcal.h"
#include "camera.h"
#ifdef __cplusplus
extern "C" {
#include "wifi_credentials.h"
}
#endif
/*******************************************************************************
 * Definitions
 *******************************************************************************/

typedef struct _VIZN_api_context_handle
{
    list_label_t clientListHead;
} VIZN_api_context_handle_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static int VIZN_API_Enrolment(VIZN_api_client_t *pClient, face_info_t face_info);
static int VIZN_API_Recognize(VIZN_api_client_t *pClient, face_info_t face_info);
static int VIZN_API_Detect(VIZN_api_client_t *pClient, int dt);
/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile uint8_t g_AddNewFace         = 0;
volatile uint8_t g_RemoveExistingFace = 0;
volatile uint8_t g_RecFace            = 0;
volatile uint8_t g_FaceSystemLocked   = 0;
std::string g_AddNewFaceName;
VIZN_api_handle_t gApiHandle;

static uint8_t sApiHandleBuffer[VIZN_API_HANDLE_SIZE];

static SemaphoreHandle_t s_cfg_lock;
static client_operations_t api_ops = {
    .detect    = VIZN_API_Detect,
    .recognize = VIZN_API_Recognize,
    .enrolment = VIZN_API_Enrolment,
};
VIZN_API_CLIENT_DEFINE(VIZN_API, (char *)"VIZN_API", &api_ops);

/*******************************************************************************
 * Code
 ******************************************************************************/
static inline char IsValidCharacter(char c)
{
    if (isalnum(c) || (c == '-') || (c == '_'))
    {
        return 1;
    }
    return 0;
}

static int IsValidUserName(char *name)
{
    int i;
    for (i = 0; name[i] != 0; i++)
    {
        if (!IsValidCharacter(name[i]))
        {
            return 0;
        }
    }
    return 1;
}

static vizn_api_status_t Cfg_Lock()
{
    if (s_cfg_lock == NULL)
    {
        return kStatus_API_Sem_NoLock;
    }

    if (pdTRUE != xSemaphoreTake(s_cfg_lock, portMAX_DELAY))
    {
        return kStatus_API_Sem_ERETRY;
    }
    return kStatus_API_Layer_Success;
}

static vizn_api_status_t Cfg_Unlock()
{
    xSemaphoreGive(s_cfg_lock);
    return kStatus_API_Layer_Success;
}

static void VIZN_TimerCallback(TimerHandle_t xTimer)
{
    Cfg_Lock();
    Oasis_TimerCallback(*(uint8_t *)pvTimerGetTimerID(xTimer));
    Cfg_Unlock();
}

static int VIZN_API_Detect(VIZN_api_client_t *pClient, int dt)
{
    assert(pClient == &VIZN_API_CLIENT(VIZN_API));
    Cfg_Lock();
    Oasis_API_Detect(dt);
    Cfg_Unlock();
    return 1;
}

static int VIZN_API_Recognize(VIZN_api_client_t *pClient, face_info_t face_info)
{
    assert(pClient == &VIZN_API_CLIENT(VIZN_API));
    Cfg_Lock();
    Oasis_API_Recognize(face_info);
    Cfg_Unlock();
    return 1;
}

static int VIZN_API_Enrolment(VIZN_api_client_t *pClient, face_info_t face_info)
{
    assert(pClient == &VIZN_API_CLIENT(VIZN_API));

    Cfg_Lock();
    Oasis_API_Enrolment(face_info);
    Cfg_Unlock();
    return 1;
}

/*******************************************************************************
 * Code API
 ******************************************************************************/

vizn_api_status_t VIZN_API_Init(VIZN_api_handle_t *apiHandle)
{
    *apiHandle = &sApiHandleBuffer[0];
    sln_cfg_data_t cfg;
    VIZN_api_context_handle_t *apiContextHandle = (VIZN_api_context_handle_t *)(*apiHandle);
    // The API is always a client of the engine.
    apiContextHandle->clientListHead.max = MAX_VIZN_CLIENTS + 1;
    Cfg_AppDataRead(&cfg);
    Oasis_State_Init(cfg.app_type);
    s_cfg_lock = xSemaphoreCreateMutex();
    if (s_cfg_lock == NULL)
    {
        return kStatus_API_Layer_Error;
    }

    if (SLN_Init_Timers(VIZN_TimerCallback) != 0)
    {
        return kStatus_API_Layer_Error;
    }

    VIZN_RegisterClient(gApiHandle, &VIZN_API_CLIENT(VIZN_API));
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_RegisterClient(VIZN_api_handle_t apiHandle, VIZN_api_client_t *clientHandle)
{
    VIZN_api_context_handle_t *apiContextHandle = (VIZN_api_context_handle_t *)apiHandle;
    list_status_t status;
    assert(clientHandle);
    assert(apiHandle);
    /* memory set for shellHandle */
    (void)memset(&clientHandle->link, 0, sizeof(clientHandle->link));

    status = LIST_AddTail(&apiContextHandle->clientListHead, &clientHandle->link);
    if (status != kLIST_Ok)
    {
        if (status == kLIST_Full)
            return kStatus_API_RegisterClient_LimitReached;
        else
            return kStatus_API_Layer_Error;
    }
    else
    {
        return kStatus_API_Layer_Success;
    }
}

vizn_api_status_t VIZN_UnregisterClient(VIZN_api_handle_t apiHandle, VIZN_api_client_t *clientHandle)
{
    list_status_t status;
    assert(clientHandle);
    status = LIST_RemoveElement(&clientHandle->link);

    /* memory set for shellHandle */
    (void)memset(&clientHandle->link, 0, sizeof(clientHandle->link));

    if (status != kLIST_Ok)
        return kStatus_API_Layer_Error;
    else
        return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_GetRegisteredUsers(VIZN_api_client_t *clientHandle, std::vector<std::string> *nameList)
{
    uint8_t status = DB_GetNames(nameList);
    if (status < 0)
        return kStatus_API_Layer_Error;
    else
        return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_AddUser(VIZN_api_client_t *clientHandle)
{
    vizn_api_status_t status;
    Cfg_Lock();
    if (g_AddNewFace)
    {
        status = kStatus_API_Layer_AddUser_MultipleAddCommand;
    }
    else
    {
        if (g_RemoveExistingFace)
        {
            StopDeregistrationProcess(kEvents_API_Layer_DeregCanceled);
        }
        else if (g_RecFace)
        {
            StopRecognitionProcess(kEvents_API_Layer_RecCanceled);
        }
        StartRegistrationProcess();
        status = kStatus_API_Layer_Success;
    }
    Cfg_Unlock();
    return status;
}

vizn_api_status_t VIZN_AddUser(VIZN_api_client_t *clientHandle, char *userName)
{
    Cfg_Lock();
    if (!strcmp(userName, "-s"))
    {
        if (!g_AddNewFace)
        {
            Cfg_Unlock();
            return kStatus_API_Layer_AddUser_NoAddCommand;
        }
        StopRegistrationProcess(kEvents_API_Layer_RegCanceled);
        StartRecognitionProcess();
        Cfg_Unlock();
        return kStatus_API_Layer_AddUser_AddCommandStopped;
    }

    if (g_AddNewFace)
    {
        Cfg_Unlock();
        return kStatus_API_Layer_AddUser_MultipleAddCommand;
    }

    if (!IsValidUserName(userName))
    {
        Cfg_Unlock();
        return kStatus_API_Layer_AddUser_InvalidUserName;
    }

    g_AddNewFaceName.assign(userName);
    if (g_RemoveExistingFace)
    {
        StopDeregistrationProcess(kEvents_API_Layer_DeregCanceled);
    }
    else if (g_RecFace)
    {
        StopRecognitionProcess(kEvents_API_Layer_RecCanceled);
    }
    StartRegistrationProcess();
    Cfg_Unlock();
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_DelUser(VIZN_api_client_t *clientHandle, char *userName)
{
    if (!strcmp(userName, "-s"))
    {
        Cfg_Lock();
        if (g_RemoveExistingFace)
        {
            StopDeregistrationProcess(kEvents_API_Layer_DeregCanceled);
            StartRecognitionProcess();
        }
        Cfg_Unlock();
        return kStatus_API_Layer_DelUser_DeregistrationStopped;
    }

    if (!strcmp(userName, "-n"))
    {
        Cfg_Lock();
        if (g_RemoveExistingFace == 0)
        {
            if (g_AddNewFace)
            {
                StopRegistrationProcess(kEvents_API_Layer_RegCanceled);
            }
            else if (g_RecFace)
            {
                StopRecognitionProcess(kEvents_API_Layer_RecCanceled);
            }
            StartDeregistrationProcess();
        }
        else
        {
            Cfg_Unlock();
            return kStatus_API_Layer_DelUser_MultipleDelCommand;
        }
        Cfg_Unlock();
        return kStatus_API_Layer_DelUser_DeregistrationStarted;
    }

    if (!IsValidUserName(userName))
    {
        return kStatus_API_Layer_DelUser_InvalidUserName;
    }

    if (DB_Del(std::string(userName)) != 0)
    {
        return kStatus_API_Layer_DelUser_FailedDelete;
    }
    else
    {
        return kStatus_API_Layer_Success;
    }
}

vizn_api_status_t VIZN_DelUser(VIZN_api_client_t *clientHandle)
{
    int32_t status;
    status = DB_DelAll();
    if (0 != status)
    {
        return kStatus_API_Layer_DelUser_FailedDeleteAll;
    }
    else
    {
        return kStatus_API_Layer_Success;
    }
}

vizn_api_status_t VIZN_RenameUser(VIZN_api_client_t *clientHandle, char *oldUserName, char *newUserName)
{
    int32_t status;
    if (!IsValidUserName(oldUserName) || !IsValidUserName(newUserName))
    {
        return kStatus_API_Layer_RenameUser_InvalidUserName;
    }
    status = DB_Ren(std::string(oldUserName), std::string(newUserName));
    if (status == 0)
    {
        return kStatus_API_Layer_Success;
    }
    else if (status == -2)
    {
        return kStatus_API_Layer_RenameUser_AlreadyExists;
    }
    else
    {
        return kStatus_API_Layer_RenameUser_FailedRename;
    }
}

vizn_api_status_t VIZN_SetVerboseMode(VIZN_api_client_t *clientHandle, cfg_verbose_t mode)
{
    uint32_t status;
    sln_cfg_data_t cfg;
    Cfg_Lock();
    Cfg_AppDataRead(&cfg);
    cfg.usb_shell_verbose = (uint8_t)mode;
    status                = Cfg_AppDataSave(&cfg);
    Cfg_Unlock();
    if (SLN_FLASH_MGMT_OK != status)
    {
        return kStatus_API_Layer_SetVerboseMode_SaveFailed;
    }

    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_SaveDB(VIZN_api_client_t *clientHandle, int count)
{
    uint8_t status = DB_Save(count);
    if (status < 0)
        return kStatus_API_Layer_Error;
    else
        return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_SystemReset(VIZN_api_client_t *clientHandle)
{
    NVIC_SystemReset();
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_SetDetResolutionMode(VIZN_api_client_t *clientHandle, cfg_detresolution_t mode)
{
    uint32_t status;
    bool already = false;

    sln_cfg_data_t cfg;
    Cfg_Lock();
    Cfg_AppDataRead(&cfg);

    if (cfg.detect_resolution_mode == (uint8_t)mode)
        already = true;
    else
    {
        cfg.detect_resolution_mode = (uint8_t)mode;
        status                     = Cfg_AppDataSave(&cfg);
    }
    Cfg_Unlock();

    if (already)
    {
        return kStatus_API_Layer_SetDetResolutionMode_Same;
    }
    if (SLN_FLASH_MGMT_OK != status)
    {
        return kStatus_API_Layer_SetDetResolutionMode_SaveFailed;
    }
    else
    {
        return kStatus_API_Layer_Success;
    }
}

vizn_api_status_t VIZN_SetPulseWidth(VIZN_api_client_t *clientHandle, cfg_led_t led, uint8_t pulse_width)
{
    uint32_t status;
    sln_cfg_data_t cfg;
    if (pulse_width > 100)
        return kStatus_API_Layer_SetCameraPulseWidth_WrongValue;
    Cfg_Lock();
    Cfg_AppDataRead(&cfg);
    if (led == LED_WHITE)
        cfg.camera_white_pulse_width = pulse_width;
    else
        cfg.camera_ir_pulse_width = pulse_width;
    status = Cfg_AppDataSave(&cfg);
    Cfg_Unlock();
    Camera_QMsgSetPWM(led, pulse_width);
    if (status != SLN_FLASH_MGMT_OK)
    {
        return kStatus_API_Layer_SetCameraPulseWidth_SaveFailed;
    }
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_GetPulseWidth(VIZN_api_client_t *clientHandle, cfg_led_t led, uint8_t *pulse_width)
{
    if (led == LED_WHITE)
        *pulse_width = Cfg_AppDataGetCameraWhitePulseWidth();
    else
        *pulse_width = Cfg_AppDataGetCameraIRPulseWidth();
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_SetEmotionTypes(VIZN_api_client_t *clientHandle, uint8_t types)
{
    uint32_t status;

    // 0 : disable the emotion recognition
    // 2 : 2 types emotion recognition
    // 4 : 4 types emotion recognition
    // 7 : 7 types emotion recognition
    if ((types == 2) || (types == 4) || (types == 7) || (types == 0))
    {
        sln_cfg_data_t cfg;
        Cfg_Lock();
        Cfg_AppDataRead(&cfg);

        cfg.emotion_types = (uint8_t)types;
        status            = Cfg_AppDataSave(&cfg);
        Cfg_Unlock();

        if (SLN_FLASH_MGMT_OK != status)
        {
            return kStatus_API_Layer_SetEmotionTypes_SaveFailed;
        }
        else
        {
            if (types == 0)
            {
                return kStatus_API_Layer_SetEmotionTypes_Disabled;
            }
            else
            {
                return kStatus_API_Layer_Success;
            }
        }
    }
    else
    {
        return kStatus_API_Layer_SetEmotionTypes_InvalidTypes;
    }
}

vizn_api_status_t VIZN_GetEmotionTypes(VIZN_api_client_t *clientHandle, uint8_t *types)
{
    *types = Cfg_AppDataGetEmotionRecTypes();
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_SetLivenessMode(VIZN_api_client_t *clientHandle, cfg_liveness_t mode)
{
    uint32_t status;
    sln_cfg_data_t cfg;
    Cfg_Lock();
    Cfg_AppDataRead(&cfg);
    if (cfg.liveness_mode == mode)
    {
        Cfg_Unlock();
        return kStatus_API_Layer_SetLivenessMode_Same;
    }
    else
    {
        cfg.liveness_mode = (uint8_t)mode;
        status            = Cfg_AppDataSave(&cfg);
    }
    Cfg_Unlock();
    Camera_SetMonoMode(cfg.liveness_mode);
    if (SLN_FLASH_MGMT_OK != status)
    {
        return kStatus_API_Layer_SetLivenessMode_SaveFailed;
    }

    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_GetLivenessMode(VIZN_api_client_t *clientHandle, cfg_liveness_t *mode)
{
    *mode = (cfg_liveness_t)Cfg_AppDataGetLivenessMode();
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_SetDispMode(VIZN_api_client_t *clientHandle, cfg_displaymode_t mode)
{
    uint32_t status;
    bool already = false;

    sln_cfg_data_t cfg;
    Cfg_Lock();
    Cfg_AppDataRead(&cfg);

    if (cfg.display_mode == (uint8_t)mode)
    {
        already = true;
    }
    else
    {
        status           = Camera_SetDispMode((uint8_t)mode);
        cfg.display_mode = (uint8_t)mode;
        status |= Cfg_AppDataSave(&cfg);
    }
    Cfg_Unlock();

    if (already)
    {
        return kStatus_API_Layer_SetDisplayMode_Same;
    }
    if (SLN_FLASH_MGMT_OK != status)
    {
        return kStatus_API_Layer_SetDisplayMode_SaveFailed;
    }
    else
    {
        return kStatus_API_Layer_Success;
    }
}

vizn_api_status_t VIZN_SetDispOutputDevice(VIZN_api_client_t *clientHandle, cfg_displayoutput_t output_mode)
{
#if !(RTFFI_RV_DISP_DRIVER)
    return kStatus_API_Layer_SetDispOutputDevice_NotSupported;
#else
    uint32_t status;
    bool already = false;
    sln_cfg_data_t cfg;
    Cfg_Lock();
    Cfg_AppDataRead(&cfg);

    if (cfg.output_mode == (uint8_t)output_mode)
    {
        already = true;
    }
    else
    {
        cfg.output_mode = (uint8_t)output_mode;
        status          = Cfg_AppDataSave(&cfg);
    }
    Cfg_Unlock();

    if (already)
    {
        return kStatus_API_Layer_SetDispOutputDevice_Same;
    }
    if (SLN_FLASH_MGMT_OK != status)
    {
        return kStatus_API_Layer_SetDispOutputDevice_SaveFailed;
    }
    else
    {
        return kStatus_API_Layer_Success;
    }
#endif
}

vizn_api_status_t VIZN_SetDispOutputInterface(VIZN_api_client_t *clientHandle, cfg_displayinterface_t interface_mode)
{
    uint32_t status;
    sln_cfg_data_t cfg;
    bool already = false;

    if (interface_mode >= DISPLAY_LAST_INTERFACE)
        return kStatus_API_Layer_SetDispOutputInterface_NotSupported;

    Cfg_Lock();
    Cfg_AppDataRead(&cfg);

    if (cfg.display_interface == (uint8_t)interface_mode)
    {
        already = true;
    }
    else
    {
        cfg.display_interface = (uint8_t)interface_mode;
        status                = Cfg_AppDataSave(&cfg);
        Camera_ChangeInterfaceMode(interface_mode);
    }
    Cfg_Unlock();

    if (already)
    {
        return kStatus_API_Layer_SetDispOutputInterface_Same;
    }
    if (SLN_FLASH_MGMT_OK != status)
    {
        return kStatus_API_Layer_SetDispOutputInterface_SaveFailed;
    }
    else
    {
        return kStatus_API_Layer_Success;
    }
}

vizn_api_status_t VIZN_SetAppTypeResetDatabase(VIZN_api_client_t *clientHandle, cfg_apptype_t type)
{
    uint32_t status;
    sln_cfg_data_t cfg;
    bool currentAppIsSame = false;

    if (type >= APP_TYPE_SUPPORT_LAST)
        return kStatus_API_Layer_SetAppType_NotSupported;

    Cfg_Lock();
    Cfg_AppDataRead(&cfg);

    if (type == cfg.app_type)
    {
        currentAppIsSame = true;
    }
    else
    {
        cfg.app_type = type;
        status       = Cfg_AppDataSave(&cfg);
    }

    Cfg_Unlock();

    if (currentAppIsSame)
        return kStatus_API_Layer_SetAppType_Same;

    if (status != SLN_FLASH_MGMT_OK)
        return kStatus_API_Layer_SetAppType_SaveFailed;

    status = DB_DelAll();
    if (status != DB_MGMT_OK)
        return kStatus_API_Layer_SetAppType_ResetDatabaseFailed;

    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_GetAppType(VIZN_api_client_t *clientHandle, cfg_apptype_t *type)
{
    *type = (cfg_apptype_t)Cfg_AppDataGetApplicationType();
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_SetLowPowerMode(VIZN_api_client_t *clientHandle, cfg_lowpower_t mode)
{
    uint32_t status;
    sln_cfg_data_t cfg;
    Cfg_Lock();
    Cfg_AppDataRead(&cfg);
    if (cfg.low_power_mode == mode)
    {
        Cfg_Unlock();
        return kStatus_API_Layer_SetLowPowerMode_Same;
    }
    else
    {
        cfg.low_power_mode = (uint8_t)mode;
        status             = Cfg_AppDataSave(&cfg);
    }
    Cfg_Unlock();
    if (SLN_FLASH_MGMT_OK != status)
    {
        return kStatus_API_Layer_SetLowPowerMode_SaveFailed;
    }
    if (mode == LOW_POWER_MODE_OFF)
    {
        DB_SetAutoSave(false);
    }
    else
    {
        DB_SetAutoSave(true);
    }
    /* Restart recognition process after change to low power mode setting */
    if (VIZN_StartRecognition(clientHandle) != kStatus_API_Layer_Success)
    {
        return kStatus_API_Layer_SetLowPowerMode_SaveFailed;
    }
    LPM_SendControlStatus(LPM_LockState, mode);
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_GetLowPowerMode(VIZN_api_client_t *clientHandle, cfg_lowpower_t *mode)
{
    *mode = (cfg_lowpower_t)Cfg_AppDataGetLowPowerMode();
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_WiFiCredentials_Erase(VIZN_api_client_t *clientHandle)
{
    wifi_cred_t wifi_cred;
    status_t status;
    memset(&wifi_cred, 0, sizeof(wifi_cred_t));
    status = wifi_credentials_flash_set(&wifi_cred);
    if (0 != status)
    {
        return kStatus_API_Layer_Error;
    }
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_WiFiCredentials_Get(VIZN_api_client_t *clientHandle, char *pSSID, char *pPassword)
{
    wifi_cred_t wifi_cred;
    status_t status = wifi_credentials_flash_get(&wifi_cred);
    if (0 != status)
    {
        return kStatus_API_Layer_Error;
    }
    status = check_valid_credentials(&wifi_cred);
    if (0 != status)
    {
        return kStatus_API_Layer_WIFI_Credentials_NotFound;
    }
    memcpy(pSSID, wifi_cred.ssid.value, wifi_cred.ssid.length + 1);
    memcpy(pPassword, wifi_cred.password.value, wifi_cred.password.length + 1);
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_WiFiCredentials_Set(VIZN_api_client_t *clientHandle, char *new_SSID, char *new_Password)
{
    wifi_cred_t wifi_cred;
    status_t status;
    uint32_t name_len = strlen(new_SSID);
    uint32_t pass_len = strlen(new_Password);
    memset(&wifi_cred, 0, sizeof(wifi_cred_t));
    wifi_cred.password.length = pass_len;
    memcpy(wifi_cred.password.value, new_Password, pass_len + 1);
    wifi_cred.ssid.length = name_len;
    memcpy(wifi_cred.ssid.value, new_SSID, name_len + 1);
    status = wifi_credentials_flash_set(&wifi_cred);
    if (0 != status)
    {
        return kStatus_API_Layer_Error;
    }
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_FWUpdate_Set(VIZN_api_client_t *clientHandle, cfg_fwupdate_method_t update_method)
{
    vizn_api_status_t status = kStatus_API_Layer_Success;

    Cfg_Lock();
    if (FWUPDATE_OTA == update_method)
    {
        /* if it is OTA set the method flag and the update flag */

        if (0 != FICA_set_OTA_bit())
        {
            Cfg_Unlock();
            return kStatus_API_Layer_Error;
        }
    }
    else if (FWUPDATE_OTW == update_method)
    {
        /* if it is OTW just set the update flag in the FICA */
        if (0 != FICA_set_FW_bit())
        {
            Cfg_Unlock();
            return kStatus_API_Layer_Error;
        }
    }
    else
    {
        Cfg_Unlock();
        return kStatus_API_Layer_Error;
    }

    /*Wait to print on the console or send response */
    vTaskDelay(500);
    /* Reset to enter update procedure */
    VIZN_SystemReset(clientHandle);
    Cfg_Unlock();
    return status;
}

vizn_api_status_t VIZN_EnrolmentEvent(VIZN_api_handle_t apiHandle, face_info_t faceInfo)
{
    VIZN_api_context_handle_t *apiContextHandle = (VIZN_api_context_handle_t *)apiHandle;
    assert(apiHandle);
    list_element_handle_t p = apiContextHandle->clientListHead.head;
    while (p != NULL)
    {
        VIZN_api_client_t *client = VIZN_CLIENT_POINTER(p);
        if (client->ops->enrolment)
            client->ops->enrolment(client, faceInfo);
        p = LIST_GetNext(p);
    }
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_DetectEvent(VIZN_api_handle_t apiHandle, int dt)
{
    VIZN_api_context_handle_t *apiContextHandle = (VIZN_api_context_handle_t *)apiHandle;
    assert(apiHandle);
    list_element_handle_t p = apiContextHandle->clientListHead.head;

    while (p != NULL)
    {
        VIZN_api_client_t *client = VIZN_CLIENT_POINTER(p);
        if (client->ops->detect)
            client->ops->detect(client, dt);
        p = LIST_GetNext(p);
    }
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_RecognizeEvent(VIZN_api_handle_t apiHandle, face_info_t faceInfo)
{
    VIZN_api_context_handle_t *apiContextHandle = (VIZN_api_context_handle_t *)apiHandle;
    assert(apiHandle);
    list_element_handle_t p = apiContextHandle->clientListHead.head;

    while (p != NULL)
    {
        VIZN_api_client_t *client = VIZN_CLIENT_POINTER(p);
        if (client->ops->recognize)
            client->ops->recognize(client, faceInfo);
        p = LIST_GetNext(p);
    }
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_GetEvents(VIZN_api_handle_t *apiHandle, uint32_t *events)
{
    *events = Oasis_Get_Events();
    return kStatus_API_Layer_Success;
}

vizn_api_status_t VIZN_StartRecognition(VIZN_api_client_t *clientHandle)
{
    vizn_api_status_t status;
    Cfg_Lock();
    if (g_AddNewFace)
    {
        StopRegistrationProcess(kEvents_API_Layer_RegCanceled);
    }
    else if (g_RemoveExistingFace)
    {
        StopDeregistrationProcess(kEvents_API_Layer_DeregCanceled);
    }
    StartRecognitionProcess();
    status = kStatus_API_Layer_Success;
    Cfg_Unlock();
    return status;
}

vizn_api_status_t VIZN_EnrolmentAddNewFace(VIZN_api_client_t *clientHandle, uint16_t *face_id, void *face)
{
    vizn_api_status_t status = kStatus_API_Layer_Success;
    float *feature_data      = (float *)face;
    int ret;

    Cfg_Lock();

    if (!g_AddNewFace)
    {
        status = kStatus_API_Layer_Error;
    }

    if (status == kStatus_API_Layer_Success)
    {
        ret = DB_GenID(face_id);
        if (ret < 0)
        {
            status = kStatus_API_Layer_Error;
        }
    }

    if (status == kStatus_API_Layer_Success)
    {
        if (g_AddNewFaceName != "")
        {
            ret = DB_Add(*face_id, g_AddNewFaceName, feature_data);
        }
        else
        {
            ret = DB_Add(*face_id, feature_data);
        }

        if (ret < 0)
        {
            if (ret == DB_MGMT_NOSPACE)
            {
                status = kStatus_API_Layer_EnrolmentAddNewFace_NoMemory;
            }
            else
            {
                status = kStatus_API_Layer_Error;
            }
        }
        else
        {
            StopRegistrationProcess(kEvents_API_Layer_RegSuccess);
            status = kStatus_API_Layer_Success;
        }
    }
    Cfg_Unlock();
    return status;
}
