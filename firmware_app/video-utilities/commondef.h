/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#ifndef _FACEREC_DEF_H_
#define _FACEREC_DEF_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "board.h"
//#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "sln_dev_cfg.h"

#include "oasislite2D_runtime.h"

/*******************************************************************************
 * Definitions
 *******************************************************************************/

#define ENABLE_720P_RESOLUTION 0

#if ENABLE_720P_RESOLUTION
#define APP_CAMERA_HEIGHT 720
#define APP_CAMERA_WIDTH 1280
#else
#define APP_CAMERA_HEIGHT (Cfg_AppDataGetDetectResolutionMode() == DETECT_RESOLUTION_VGA ? 480 : 240)
#define APP_CAMERA_WIDTH  (Cfg_AppDataGetDetectResolutionMode() == DETECT_RESOLUTION_VGA ? 640 : 320)
#endif

#define LCD_WIDTH  (SCREEN_PORTRAIT_MODE ? 240 : 320)
#define LCD_HEIGHT (SCREEN_PORTRAIT_MODE ? 320 : 240)

// Rotation done in PXP
// #define DISPLAY_PITCH_BYTES  (Cfg_AppDataGetOutputMode() == DISPLAY_USB ? LCD_WIDTH * 2:LCD_HEIGHT * 2)
// #define DISPLAY_ROTATION     (Cfg_AppDataGetOutputMode() == DISPLAY_USB ? 0 : 90)

// Rotation done on Riverdi side
#define DISPLAY_PITCH_BYTES (LCD_WIDTH * 2)
#define DISPLAY_ROTATION    (0)

// AX surface
#define CAMERA_SURFACE_SHIFT 0


#define APP_PXP       PXP
#define APP_PS_WIDTH  (LCD_WIDTH - CAMERA_SURFACE_SHIFT)
#define APP_PS_HEIGHT LCD_HEIGHT

#define APP_AS_WIDTH  LCD_WIDTH
#define APP_AS_HEIGHT LCD_HEIGHT

// ROI Box RECT for detect and recognize
#define REC_RECT_HEIGHT (CAMERA_ROTATE_FLAG ? APP_CAMERA_WIDTH  : APP_CAMERA_HEIGHT)
#define REC_RECT_WIDTH  (CAMERA_ROTATE_FLAG ? APP_CAMERA_HEIGHT : APP_CAMERA_WIDTH)

#define CAMERA_HORIZONTAL_POINTS LCD_WIDTH // toolbar width
#define CAMERA_VERTICAL_POINTS   LCD_HEIGHT
#define CAMERA_BYTES_PER_PIXEL   2
#define CAMERA_FRAME_BYTES       (CAMERA_HORIZONTAL_POINTS * CAMERA_VERTICAL_POINTS * CAMERA_BYTES_PER_PIXEL)

// EVENT BITS FOR SYNCRONIZATION
typedef enum
{
    SYNC_VIDEO_DISPLAY_INIT_BIT = 0,
    SYNC_VIDEO_DISPLAY_DEINIT_BIT,
    SYNC_VIDEO_CAMERA_INIT_BIT,
    SYNC_VIDEO_CAMERADEVICE_INIT_BIT,
    SYNC_VIDEO_CAMERA_DEINIT_BIT,
    SYNC_VIDEO_PXP_COMPLET_BIT,
    SYNC_VIDEO_USB_COMPLET_TRANSFER_BIT,
    SYNC_VIDEO_BLUETOOTH_SERVER_START_INIT
} SyncVideoEventsID;

// message queue related definiation
typedef enum
{
    QMSG_CAMERA_DQ = 1,
    QMSG_CAMERA_USERID,
    QMSG_FACEREC_FRAME_REQ,
    QMSG_FACEREC_FRAME_RES,
    QMSG_FACEREC_ENROLMENTMODE,
    QMSG_FACEREC_ADDNEWFACE,
	QMSG_FACEREC_ADDNEWFACEBY_FEA,
    QMSG_FACEREC_DELFACE,
    QMSG_FACEREC_RECFACE,
    QMSG_FACEREC_STOP,
    QMSG_FACEREC_START,
    QMSG_FACEREC_REC_REQ,
    QMSG_FACEREC_REC_RES,
    QMSG_FACEREC_INFO_UPDATE,
    QMSG_DISPLAY_FRAME_REQ,
    QMSG_DISPLAY_FRAME_RES,
    QMSG_DISPLAY_FRAME_SEND,
    QMSG_DISPLAY_INTERFACE,
    QMSG_PXP_DISPLAY,
    QMSG_PXP_FACEREC,
    QMSG_SWITCH_1,
    QMSG_PCAL_SWITCHES,
    QMSG_CMD,
    QMSG_BLE_RESET,
    QMSG_LPM,
    INVALID_MSG_ID
} QMsgID;

typedef enum
{
    QCMD_SET_LIVENESS_MODE,
    QCMD_CHANGE_RGB_IR_DISP_MODE,
    QCMD_CHANGE_RGB_EXPOSURE_MODE,
    QCMD_SET_PWM,
    QCMD_CHANGE_INFO_DISP_MODE,
    QCMD_DEINIT_CAMERA,
    INVALID_QCMD_ID
} QCmdID;

typedef struct
{
    void *IR_frame_data;
    void *RGB_frame_data;
} QRawMsg;

typedef struct
{
    int id;
    union
    {
        uint8_t liveness_mode;
        uint8_t display_mode;
        uint8_t led_pwm[2];
        uint8_t exposure_mode;
        uint8_t enrolment_mode;
        uint8_t rec_face;
        uint8_t del_face;
        struct{
			uint8_t add_newface;
			char new_face_name[31];
			void* feature;
        }add_face;
        uint8_t interface_mode;
        uint16_t control_status;
    } data;
} QCmdMsg;

typedef struct
{
    uint16_t *pFaceData;
    char name[64];
    float similar;
    int rect[4];
    int rect2[4];
    int dt;
    int rt;
    int registeredFaces;
    //unsigned char emotion;
    float fps;
    float detect_fps;
    float recognize_fps;
    int faceCount;
    uint8_t rgbLive;  // real = 1 or fake = 0 or unkown = 0xff
    uint8_t blur;  // blur = 1 or clean = 0 or unkown = 0xff
    uint8_t front;   // front =  1  or side = 0  or unknown = 0xff
    uint8_t irLive;    // real = 1  or fake = 0   or unknown = 0xff
    uint8_t irBrightness;
    uint8_t irPwm;
    uint8_t rgbBrightness;
    uint8_t rgbPwm;
} QUIInfoMsg;

typedef struct
{
    uint32_t in_buffer;
    uint32_t out_buffer;
    void* user_data;
} QPXPMsg;

typedef struct
{
    QMsgID id;
    union
    {
        QRawMsg raw;
        QUIInfoMsg info;
        QCmdMsg cmd;
        QPXPMsg pxp;
        uint8_t key_num;
    } msg;
} QMsg;

#define RAM_ADDRESS_ALIGNMENT(n) __attribute__((aligned(n)))
#define OCRAM_CACHED_BSS         __attribute__((section(".ocram_cacheable_data, \"aw\", %nobits @")))
#define DTC_BSS                  __attribute__((section(".dtc_data, \"aw\", %nobits @")))
#define DTC_DATA                 __attribute__((section(".dtc_data_init")))

#define WIFIINITTASK_PRIORITY  (configMAX_PRIORITIES - 1UL)
#define WIFIINITTASK_STACKSIZE 1024

#define BLEINITTASK_PRIORITY  (configMAX_PRIORITIES - 1UL)
#define BLEINITTASK_STACKSIZE 256

#define CAMERAINITTASK_PRIORITY  (configMAX_PRIORITIES - 1UL)
#define CAMERAINITTASK_STACKSIZE 256

#define DISPLAYINITTASK_PRIORITY  (configMAX_PRIORITIES - 1UL)
#define DISPLAYINITTASK_STACKSIZE 256

#define DISPLAYTASK_PRIORITY  (configMAX_PRIORITIES - 2UL)
#define DISPLAYTASK_STACKSIZE 1024

#define CAMERATASK_PRIORITY  (configMAX_PRIORITIES - 1UL)
#define CAMERATASK_STACKSIZE 2 * 1024

#define PXPTASK_PRIORITY  (configMAX_PRIORITIES - 2UL)
#define PXPTASK_STACKSIZE 1024

#define OASISDETTASK_PRIORITY  (configMAX_PRIORITIES - 3UL)
#define OASISDETTASK_STACKSIZE 10 * 1024

#define host_task_PRIORITY ((configMAX_PRIORITIES)-5UL)
#define BLETASK_STACKSIZE  1024

#define blackbox_task_PRIORITY ((configMAX_PRIORITIES)-4UL)
#define BLACKTASK_STACKSIZE    1024

#define SWITCHTASK_PRIORITY  (configMAX_PRIORITIES - 2UL)
#define SWITCHTASK_STACKSIZE 256

#define UARTTASK_STACKSIZE 1024
#define USBTASK_STACKSIZE  1024

#define TCPTASK_PRIORITY  (configMAX_PRIORITIES - 2UL)
#define TCPTASK_STACKSIZE 2 * 1024

#define LPMTASK_PRIORITY (configMAX_PRIORITIES - 1UL)
#define LPMTASK_STACKSIZE 256
////////////////
#define BLE_LPUART      LPUART5
#define BLE_LPUART_IRQn LPUART5_IRQn

#define gHybridApp_d 1

#if gHybridApp_d
#define gFsciMaxVirtualInterfaces_c 2
#endif

#define THR_FSCI_IF  0
#define BLE_FSCI_IF  1
#define SHELL_PROMPT "fs$ "

/* Prerequisites for QCA driver */
#define A_LITTLE_ENDIAN

#define FEATURE_BORDER_ROUTER 0

/* Flash Database related */
#define FEATUREDATA_MAX_COUNT 100

#define FLASH_OK  0
#define FLASH_ERR 1

#define FACEREC_FS_FLASH_SIZE      (BOARD_FLASH_SIZE/1024) /* in KB */
#define FACEREC_FS_FLASH_PAGE_SIZE BOARD_FLASH_PAGE_SIZE
#define FLASH_SECTOR_SIZE          BOARD_FLASH_SECTOR_SIZE
#define FLASH_NPAGE_PER_SECTOR     (FLASH_SECTOR_SIZE / FACEREC_FS_FLASH_PAGE_SIZE)

#if FLASH_TYPE == HYPER_FLASH
#define FACEREC_FS_MAP_ADDR  (0xB00000U)
#else
//0xB00000 to 0xB20000 are used by configuration file
#define FACEREC_FS_MAP_ADDR  (0xB20000U)
#endif

#define FACEREC_FS_ITEM_ADDR  (FACEREC_FS_MAP_ADDR + FLASH_SECTOR_SIZE)
#define FACEREC_FS_FIRST_SECTOR ((FACEREC_FS_MAP_ADDR) / (FLASH_SECTOR_SIZE))
#define FACEREC_FS_SECTORS (((FEATUREDATA_MAX_COUNT * 1024)+ (FLASH_SECTOR_SIZE -1)) / (FLASH_SECTOR_SIZE))
#define FEATUREDATA_FLASH_PAGE_SIZE FACEREC_FS_FLASH_PAGE_SIZE

#define FEATUREDATA_MAGIC_UNUSE 0xFF
#define FEATUREDATA_MAGIC_VALID 0x79
#define FEATUREDATA_MAGIC_DELET 0x00

#define FEATUREDATA_NAME_MAX_LEN 31
#define FEATUREDATA_FEATURE_SIZE (OASISLT_getFaceItemSize() / sizeof(float))

/********************************************************
 * item        |  address   |  size/sector |
 * FeatureMap  | 0x60B00000 |      1       |
 * FeatureItem | 0x60B40000 |      1       |
 */
/********************************************************
 * item        |  address   |  size/sector | for RT106F_ELOCK_BOARD
 * FeatureMap  | 0x60B20000 |      1      |
 * FeatureItem | 0x60B21000~0x60B3A000 |       25      |
 */
typedef struct
{
    /*put char/unsigned char together to avoid padding*/
    unsigned char magic;
    char name[FEATUREDATA_NAME_MAX_LEN];
    int index;
    // this id identify a feature uniquely,we should use it as a handler for feature add/del/update/rename
    uint16_t id;
    uint16_t pad;
    /*put feature in the last so, we can take it as dynamic, size limitation:
     * (FEATUREDATA_FLASH_PAGE_SIZE * 2 - 1 - FEATUREDATA_NAME_MAX_LEN - 4 - 4)/4*/
    float feature[0];
} FeatureItemHeader;


typedef union
{
    struct
    {
        /*put char/unsigned char together to avoid padding*/
        unsigned char magic;
        char name[FEATUREDATA_NAME_MAX_LEN];
        int index;
        // this id identify a feature uniquely,we should use it as a handler for feature add/del/update/rename
        uint16_t id;
        uint16_t pad;
        /*put feature in the last so, we can take it as dynamic, size limitation:
         * (FEATUREDATA_FLASH_PAGE_SIZE * 2 - 1 - FEATUREDATA_NAME_MAX_LEN - 4 - 4)/4*/
        float feature[0];
    };
#if FLASH_TYPE == HYPER_FLASH
    unsigned char raw[FEATUREDATA_FLASH_PAGE_SIZE * 2];
#else
    unsigned char raw[FEATUREDATA_FLASH_PAGE_SIZE * 4];
#endif
} FeatureItem; // 1kB

typedef union
{
    struct
    {
        char magic[FEATUREDATA_MAX_COUNT];
    };
    unsigned char raw[((FEATUREDATA_MAX_COUNT + (FEATUREDATA_FLASH_PAGE_SIZE - 1)) / FEATUREDATA_FLASH_PAGE_SIZE)
                      * FEATUREDATA_FLASH_PAGE_SIZE];
} FeatureMap;


/* BLE/Network remote operation command definition */
enum
{
    OP_ADD_FACE      = 0x11,
    OP_DEL_FACE      = 0x12,
    OP_KEY_ADD_START = 0x13,
    OP_KEY_ADD_DONE  = 0x14,
};

#endif /* _FACEREC_DEF_H_ */
