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

#if RTVISION_BOARD

#include <facerecicon.h>
#include <nxplogo.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "camera.h"
#include "board.h"
#include "pin_mux.h"

#include "fsl_camera.h"
#include "fsl_camera_receiver.h"
#include "fsl_camera_device.h"

#include "fsl_gpio.h"
#include "fsl_csi.h"
#include "fsl_csi_camera_adapter.h"
#if (APP_CAMERA_TYPE == APP_CAMERA_MT9M114)
#include "fsl_mt9m114.h"
#elif (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
#include "fsl_gc0308.h"
#endif
#include "fsl_iomuxc.h"
#include "fsl_log.h"
#include "fsl_pxp.h"

#include "commondef.h"
#include "oasis.h"
#include "util.h"
#include "pxp.h"
#include "font.h"
#include "display.h"

#include "sln_shell.h"
#include "sln_dev_cfg.h"
#include "sln_api.h"
#include "sln_pcal.h"
#include "fsl_common.h"
#include "fsl_qtmr.h"

/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define CAMERA_QTMR_BASEADDR     TMR3
#define CAMERA_QTMR_PWM_CHANNEL  kQTMR_Channel_1
#define CAMERA_QTMR_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_IpgClk)
#define CAMERA_QTMR_PWM_FREQ     3000

#define CAMERA_I2C_SCL_GPIO GPIO1
#define CAMERA_I2C_SCL_PIN  16
#define CAMERA_I2C_SDA_GPIO GPIO1
#define CAMERA_I2C_SDA_PIN  17

#define RGB_IR_FRAME_RATIO 2

/*******************************************************************************
 * Prototypes
 *******************************************************************************/
static void BOARD_PullCameraPowerDownPin(bool pullUp);
static void BOARD_PullCameraIRPowerDownPin(bool pullUp);
static void BOARD_PullCameraResetPin(bool pullUp);
static void Camera_Deinit(void);
static void Camera_RgbIrSwitch(int8_t cameraID);
//static uint32_t Camera_getAnotherRxBuf(uint32_t activeAddr);
static void Camera_Callback(camera_receiver_handle_t *handle, status_t status, void *userData);
static void Camera_CheckOverRun();
static void CameraDevice_Init_Task(void *param);
static void Camera_Init_Task(void *param);
static void Camera_Task(void *param);

/*******************************************************************************
 * Variables
 *******************************************************************************/
/*!< Event handler used for synchronization
   bit 0  sync display
   bit 1  sync camera
   bit 2  sync pxp
*/
EventGroupHandle_t g_SyncVideoEvents;

/*!< Queue used by Camera Task to receive messages*/
static QueueHandle_t CameraMsgQ = NULL;
/*!< Message sent from CameraISR to  Camera task to signal that a frame is available */
static QMsg DQMsg;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
DTC_BSS static StackType_t s_CameraTaskStack[CAMERATASK_STACKSIZE];
DTC_BSS static StaticTask_t s_CameraTaskTCB;

DTC_BSS static StackType_t s_CameraInitTaskStack[CAMERAINITTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_CameraInitTaskTCB;

DTC_BSS static StackType_t s_CameraDeviceInitTaskStack[CAMERAINITTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_CameraDeviceInitTaskTCB;
#endif

static uint32_t s_ActiveFrameAddr;
static uint32_t s_InactiveFrameAddr;
static uint8_t s_appType;
static unsigned int EQIndex = 0;
static unsigned int DQIndex = 0;

/*!< Pointer to the buffer that stores camera frames */
static uint16_t *s_pBufferQueue = NULL;
uint16_t *g_pRotateBuff         = NULL;

static uint8_t sCurrentLedPwmValue[LED_NUM];

#if (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
static int8_t s_CurrentCameraID = COLOR_CAMERA;
#endif
static uint8_t s_CurExposureMode[2];
static uint8_t gPendingExposureModeSet[2];

static uint8_t gPendingTargetYSet[2] = {0,0};
// > 0: increase targetY; < 0 : decrease targetY; 0xFF: set to default Y
static uint8_t gPendingTargetYValue[2];


static csi_resource_t csiResource = {
    .csiBase = CSI,
};

static csi_private_data_t csiPrivateData;

static camera_receiver_handle_t cameraReceiver = {
    .resource    = &csiResource,
    .ops         = &csi_ops,
    .privateData = &csiPrivateData,
};

#if (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
static gc0308_resource_t gc0308Resource[2] = {
    {   // RGB
        .i2cSendFunc       = BOARD_Camera_I2C_Send,
        .i2cReceiveFunc    = BOARD_Camera_I2C_Receive,
        .pullResetPin      = BOARD_PullCameraResetPin,
        .pullPowerDownPin  = BOARD_PullCameraPowerDownPin,
        .inputClockFreq_Hz = 24000000,
    },
    {   // IR
        .i2cSendFunc       = BOARD_Camera_IR_I2C_Send,
        .i2cReceiveFunc    = BOARD_Camera_IR_I2C_Receive,
        .pullResetPin      = BOARD_PullCameraResetPin,
        .pullPowerDownPin  = BOARD_PullCameraIRPowerDownPin,
        .inputClockFreq_Hz = 24000000,
    }
};
camera_device_handle_t cameraDevice[2]     = {
    {
        .resource = &gc0308Resource[0],
        .ops      = &gc0308_ops,
    },
    {
        .resource = &gc0308Resource[1],
         .ops = &gc0308_ops
    }
};

#else
static mt9m114_resource_t mt9m114Resource[2] = {
    {//RGB
        .i2cSendFunc        = BOARD_Camera_I2C_Send,
        .i2cReceiveFunc     = BOARD_Camera_I2C_Receive,
        .pullResetPin       = BOARD_PullCameraResetPin,
        .pullPowerDownPin   = BOARD_PullCameraPowerDownPin,
        .inputClockFreq_Hz  = 24000000,
        .i2cAddr            = MT9M114_I2C_ADDR,
    },
    {//IR
        .i2cSendFunc        = BOARD_Camera_IR_I2C_Send,
        .i2cReceiveFunc     = BOARD_Camera_IR_I2C_Receive,
        .pullResetPin       = BOARD_PullCameraResetPin,
        .pullPowerDownPin   = BOARD_PullCameraIRPowerDownPin,
        .inputClockFreq_Hz  = 24000000,
        .i2cAddr            = MT9M114_I2C_ADDR_IR,
    }
};
camera_device_handle_t cameraDevice[2] = {
    {.resource = &mt9m114Resource[0], .ops = &mt9m114_ops,},
    {.resource = &mt9m114Resource[1], .ops = &mt9m114_ops}
};

#endif

/*******************************************************************************
 * Code
 *******************************************************************************/
// Allocates an aligned memory buffer
static void *alignedMalloc(size_t size)
{
    uint8_t **adata;
    uint8_t *udata = (uint8_t *)pvPortMalloc(size + sizeof(void *) + FRAME_BUFFER_ALIGN);
    if (!udata)
    {
        return NULL;
    }
    adata     = (uint8_t **)((uint32_t)(udata + FRAME_BUFFER_ALIGN) & (int32_t)(-FRAME_BUFFER_ALIGN));
    adata[-1] = udata;
    return adata;
}

// Deallocates a memory buffer
static void alignedFree(void *ptr)
{
    uint8_t *udata;
    if (ptr)
    {
        udata = ((uint8_t **)ptr)[-1];
        vPortFree(udata);
    }
}

static void Camera_LedTimer_Init()
{
    qtmr_config_t qtmrConfig;

    QTMR_GetDefaultConfig(&qtmrConfig);
    qtmrConfig.primarySource   = kQTMR_ClockDivide_64;
    qtmrConfig.secondarySource = kQTMR_Counter2InputPin;
    QTMR_Init(CAMERA_QTMR_BASEADDR, CAMERA_QTMR_PWM_CHANNEL, &qtmrConfig);

    QTMR_SetupPwm(CAMERA_QTMR_BASEADDR, CAMERA_QTMR_PWM_CHANNEL, CAMERA_QTMR_PWM_FREQ, 0, false,
                  CAMERA_QTMR_SOURCE_CLOCK / 64);

    /* Start the counter */
    QTMR_StartTimer(CAMERA_QTMR_BASEADDR, CAMERA_QTMR_PWM_CHANNEL, kQTMR_PriSrcRiseEdge);
    return;
}

static void Camera_LedTimer_Deinit()
{
    QTMR_StopTimer(CAMERA_QTMR_BASEADDR, CAMERA_QTMR_PWM_CHANNEL);
    QTMR_Deinit(CAMERA_QTMR_BASEADDR, CAMERA_QTMR_PWM_CHANNEL);

    return;
}

static int Camera_SelectLED(uint8_t led)
{
    if (led == LED_WHITE)
    {
        set_iox_port_pin(BOARD_CAMERA_LED_SWITCH_GPIO, BOARD_CAMERA_LED_SWITCH_GPIO_PIN, 1);
    }
    else
    {
        set_iox_port_pin(BOARD_CAMERA_LED_SWITCH_GPIO, BOARD_CAMERA_LED_SWITCH_GPIO_PIN, 0);
    }
    return 0;
}

int Camera_QMsgSetPWM(uint8_t led, uint8_t pulse_width)
{
    int status                     = -1;
    QMsg *pQMsg                    = (QMsg*)pvPortMalloc(sizeof(QMsg));
    pQMsg->id                      = QMSG_CMD;
    pQMsg->msg.cmd.id              = QCMD_SET_PWM;
    pQMsg->msg.cmd.data.led_pwm[0] = led;
    pQMsg->msg.cmd.data.led_pwm[1] = pulse_width;
    status                         = Camera_SendQMsg((void *)&pQMsg);
    return status;
}

void Camera_GetPWM(uint8_t led, uint8_t* pulse_width)
{
//    if( led == LED_IR)
//    {
//        *pulse_width = s_PwmIR;
//    }
//    else
//    {
//        *pulse_width = s_PwmWhite;
//    }
    *pulse_width = sCurrentLedPwmValue[led];
}

static status_t Camera_SetPWM(uint8_t pwm_index, uint8_t pulse_width)
{

    status_t status = kStatus_Fail;
    if (pwm_index == LED_WHITE)
    {
    	return status;
    }
    Camera_SelectLED(pwm_index);
    QTMR_StopTimer(CAMERA_QTMR_BASEADDR, CAMERA_QTMR_PWM_CHANNEL);
    status = QTMR_SetupPwm(CAMERA_QTMR_BASEADDR, CAMERA_QTMR_PWM_CHANNEL, CAMERA_QTMR_PWM_FREQ, pulse_width, false,
                           CAMERA_QTMR_SOURCE_CLOCK / 64);
    QTMR_StartTimer(CAMERA_QTMR_BASEADDR, CAMERA_QTMR_PWM_CHANNEL, kQTMR_PriSrcRiseEdge);
    sCurrentLedPwmValue[pwm_index] = pulse_width;
    return status;
}

/* MT9M114 camera module has PWDN pin, but the pin is not
 * connected internally, MT9M114 does not have power down pin.
 * The reset pin is connected to high, so the module could
 * not be reseted, so at the begining, use GPIO to let camera
 * release the I2C bus.
 */
static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;

    for (i = 0; i < 0x200; i++)
    {
        __NOP();
    }
}

void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i                          = 0;
    const gpio_pin_config_t pin_config = {.direction = kGPIO_DigitalOutput, .outputLogic = 1};

    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_01_GPIO1_IO17, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_00_GPIO1_IO16, 0U);

    GPIO_PinInit(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, &pin_config);
    GPIO_PinInit(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, &pin_config);

#if RTVISION_BOARD
    /* Put A71CH in reset */
    GPIO_PinInit(GPIO1, 11, &pin_config);
    GPIO_PinWrite(GPIO1, 11, 0U);
    i2c_release_bus_delay();
#endif

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_PinWrite(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, 1U);
    i2c_release_bus_delay();
}

static void BOARD_PullCameraResetPin(bool pullUp)
{
    return;
}

static void BOARD_PullCameraPowerDownPin(bool pullUp)
{
    if (pullUp)
    {
        set_iox_port_pin(BOARD_CAMERA_PWD_GPIO, BOARD_CAMERA_PWD_GPIO_PIN, 1);
    }
    else
    {
        set_iox_port_pin(BOARD_CAMERA_PWD_GPIO, BOARD_CAMERA_PWD_GPIO_PIN, 0);
    }
}

static void BOARD_PullCameraIRPowerDownPin(bool pullUp)
{
    if (pullUp)
    {
        set_iox_port_pin(BOARD_CAMERA_IR_PWD_GPIO, BOARD_CAMERA_IR_PWD_GPIO_PIN, 1);
    }
    else
    {
        set_iox_port_pin(BOARD_CAMERA_IR_PWD_GPIO, BOARD_CAMERA_IR_PWD_GPIO_PIN, 0);
    }
}

#if (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
/*!
 * @brief Enables the swap of image data fields in the CSI module.
 *
 * To support the endianess of GC0308 camera data output, this function enables the byte swap in the CSI module.
 *
 */
static void CAMERA_RECEIVER_SwapBytes(camera_receiver_handle_t *handle)
{
    uint32_t regvalue;

    /* in CSICR1 (0x402BC000) set SWAP16_EN & PACK_DIR fields to 1 */
    regvalue = (((csi_resource_t *)handle->resource)->csiBase)->CSICR1;
    *((volatile uint32_t *)(&(((csi_resource_t *)handle->resource)->csiBase)->CSICR1)) = regvalue | 0x80000080;
}
#endif

//static uint32_t Camera_getAnotherRxBuf(uint32_t activeAddr)
//{
//    if (activeAddr == (uint32_t)s_pBufferQueue)
//    {
//        return (uint32_t)(s_pBufferQueue + APP_CAMERA_HEIGHT * APP_CAMERA_WIDTH);
//    }
//    else
//    {
//        return (uint32_t)s_pBufferQueue;
//    }
//}

static void Camera_RgbIrSwitch(int8_t cameraID)
{
    static int8_t last_id = -1;
    if (last_id == cameraID)
        return;
    last_id = cameraID;

    if (cameraID == COLOR_CAMERA || cameraID == IR_CAMERA)
    {
        CAMERA_DEVICE_Stop(&cameraDevice[cameraID^1]);
        set_iox_port_pin(BOARD_CAMERA_SWITCH_GPIO, BOARD_CAMERA_SWITCH_GPIO_PIN, cameraID);
        CAMERA_DEVICE_Start(&cameraDevice[cameraID]);

#if ((APP_CAMERA_TYPE == APP_CAMERA_GC0308) || (APP_CAMERA_TYPE == APP_CAMERA_MT9M114))
        //only need do this if one IIC used for dual camera
        if (gPendingExposureModeSet[cameraID])
        {
            CAMERA_DEVICE_Control(&cameraDevice[cameraID],  kCAMERA_DeviceExposureMode, s_CurExposureMode[cameraID]);
            gPendingExposureModeSet[cameraID] = 0;
        }

        if (gPendingTargetYSet[cameraID])
        {
            CAMERA_DEVICE_Control(&cameraDevice[cameraID], kCAMERA_DeviceBrightnessAdjust, gPendingTargetYValue[cameraID]);
            gPendingTargetYSet[cameraID] = 0;
        }
#endif

    }else
    {
        CAMERA_DEVICE_Stop(&cameraDevice[0]);
        CAMERA_DEVICE_Stop(&cameraDevice[1]);

    }
}

int Camera_SetMonoMode(uint8_t enable)
{
    int status;
    QMsg *pQMsg                       = (QMsg*)pvPortMalloc(sizeof(QMsg));
    if (NULL == pQMsg)
    {
        LOGE("[ERROR]: Camera_SetMonoMode pQMsg pvPortMalloc failed\r\n");
        return -1;
    }
    pQMsg->id                         = QMSG_CMD;
    pQMsg->msg.cmd.id                 = QCMD_SET_LIVENESS_MODE;
    pQMsg->msg.cmd.data.liveness_mode = enable;
    status                            = Camera_SendQMsg((void *)&pQMsg);
    return status;
}

int Camera_SetDispMode(uint8_t displayMode)
{
    int status;
    QMsg *pQMsg                      = (QMsg*)pvPortMalloc(sizeof(QMsg));
    if (NULL == pQMsg)
    {
        LOGE("[ERROR]: Camera_SetDispMode pQMsg pvPortMalloc failed\r\n");
        return -1;
    }
    pQMsg->id                        = QMSG_CMD;
    pQMsg->msg.cmd.id                = QCMD_CHANGE_RGB_IR_DISP_MODE;
    pQMsg->msg.cmd.data.display_mode = displayMode;
    status                           = Camera_SendQMsg((void *)&pQMsg);
    return status;
}


int Camera_SetExposureMode(uint8_t whichCamera, uint8_t mode)
{
#if ((APP_CAMERA_TYPE == APP_CAMERA_GC0308) || (APP_CAMERA_TYPE == APP_CAMERA_MT9M114))

		if (CAMERA_DIFF_I2C_BUS || (s_appType >= APP_TYPE_ELOCK_LIGHT_SINGLE && s_appType <= APP_TYPE_USERID))
		{
			CAMERA_DEVICE_Control(&cameraDevice[whichCamera],  kCAMERA_DeviceExposureMode, mode);

		}else
		{	//delay to do so in Camera_RgbIrSwitch
			s_CurExposureMode[whichCamera] = mode;
			gPendingExposureModeSet[whichCamera] = 1;
		}
#endif
	return 0;
}


//whichCamera, 0 indicate RGB, 1 indicate IR.
//upOrDown, 0 indicate down, 1 indicate up, 0xFF indicate to default value
int Camera_SetTargetY(uint8_t whichCamera,uint8_t upOrDown)
{
#if ((APP_CAMERA_TYPE == APP_CAMERA_GC0308) || (APP_CAMERA_TYPE == APP_CAMERA_MT9M114))

    if (CAMERA_DIFF_I2C_BUS || ((s_appType >= APP_TYPE_ELOCK_LIGHT_SINGLE) && (s_appType <= APP_TYPE_USERID)))
    {
    	CAMERA_DEVICE_Control(&cameraDevice[whichCamera],  kCAMERA_DeviceBrightnessAdjust, upOrDown);

    }else
    {
		//delay to do so in Camera_RgbIrSwitch
		gPendingTargetYSet[whichCamera] = 1;
		gPendingTargetYValue[whichCamera] = upOrDown;
    }
    UsbShell_DbgPrintf(VERBOSE_MODE_L2, "Camera_SetTargetY,id:%d upOrDown:%d \r\n",whichCamera,upOrDown);
#endif
	return 0;
}

//int Camera_QMsgSetExposureMode(uint8_t mode)
//{
//    int status = -1;
//    QMsg* pQMsg = (QMsg*)pvPortMalloc(sizeof(QMsg));
//    pQMsg->id = QMSG_CMD;
//    pQMsg->msg.cmd.id = QCMD_CHANGE_RGB_EXPOSURE_MODE;
//    pQMsg->msg.cmd.data.exposure_mode = mode;
//    status = Camera_SendQMsg((void*)&pQMsg);
//    return status;
//}

int Camera_ChangeInterfaceMode(uint8_t mode)
{
    int status = -1;
    QMsg* pQMsg                        = (QMsg *)pvPortMalloc(sizeof(QMsg));
    if (NULL == pQMsg)
    {
        LOGE("[ERROR]: Camera_ChangeInterfaceMode pQMsg pvPortMalloc failed\r\n");
        return -1;
    }
    pQMsg->id                          = QMSG_CMD;
    pQMsg->msg.cmd.id                  = QCMD_CHANGE_INFO_DISP_MODE;
    pQMsg->msg.cmd.data.interface_mode = mode;
    status                             = Camera_SendQMsg((void *)&pQMsg);
    return status;
}

void BOARD_InitCameraResource(void)
{
    BOARD_Camera_I2C_Init();

#if CAMERA_DRIVE_STRENGTH_LOW
    /* CSI MCLK select 24M. */
    /*
     * CSI clock source:
     *
     * 00 derive clock from osc_clk (24M)
     * 01 derive clock from PLL2 PFD2
     * 10 derive clock from pll3_120M
     * 11 derive clock from PLL3 PFD1
     */
    CLOCK_SetMux(kCLOCK_CsiMux, 2);
    /*
     * CSI clock divider:
     *
     * 000 divide by 1
     * 001 divide by 2
     * 010 divide by 3
     * 011 divide by 4
     * 100 divide by 5
     * 101 divide by 6
     * 110 divide by 7
     * 111 divide by 8
     */
    CLOCK_SetDiv(kCLOCK_CsiDiv, 7);
#else
    CLOCK_SetMux(kCLOCK_CsiMux, 0);
    CLOCK_SetDiv(kCLOCK_CsiDiv, 0);
#endif

    /*
     * For RT1060, there is not dedicate clock gate for CSI MCLK, it use CSI
     * clock gate.
     */

    CLOCK_EnableClock(kCLOCK_Iomuxc); /* iomuxc clock (iomuxc_clk_enable): 0x03u */

    /* DISP_EXTCOMIN pin */
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_07_QTIMER3_TIMER1, /* GPIO_B0_05 is configured as QTIMER2_TIMER2 */
                     0U); /* Software Input On Field: Input Path is determined by functionality */

    IOMUXC_GPR->GPR6 =
        ((IOMUXC_GPR->GPR6 & (~(IOMUXC_GPR_GPR6_QTIMER3_TRM1_INPUT_SEL_MASK))) /* Mask bits to zero which are setting */
         | IOMUXC_GPR_GPR6_QTIMER3_TRM1_INPUT_SEL(0x00u)                       /* QTIMER2 TMR2 input select: 0x00u */
        );

    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_07_QTIMER3_TIMER1, /* GPIO_B0_05 PAD functional properties : */
                        0x10B0u);                         /* Slew Rate Field: Slow Slew Rate
                                                           Drive Strength Field: R0/6
                                                           Speed Field: medium(100MHz)
                                                           Open Drain Enable Field: Open Drain Disabled
                                                           Pull / Keep Enable Field: Pull/Keeper Enabled
                                                           Pull / Keep Select Field: Keeper
                                                           Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                           Hyst. Enable Field: Hysteresis Disabled */

    Camera_LedTimer_Init();
}

static void Camera_Callback(camera_receiver_handle_t *handle, status_t status, void *userData)
{
    BaseType_t HigherPriorityTaskWoken = pdFALSE;
    QMsg *pQMsg                        = &DQMsg;
    xQueueSendToBackFromISR(CameraMsgQ, (void *)&pQMsg, &HigherPriorityTaskWoken);
    portYIELD_FROM_ISR(HigherPriorityTaskWoken);

    // LOGD("[DQ:%d]\r\n", DQIndex);
    if (DQIndex < EQIndex)
    {
        LOGE("Camera BQ overrun [%d:%d]\r\n", EQIndex, DQIndex);
    }

    DQIndex++;
}

static void CameraDevice_Init_Task(void *param)
{
#if (CAMERA_DIFF_I2C_BUS || (APP_CAMERA_TYPE == APP_CAMERA_MT9M114))
    if (APP_TYPE_USERID != s_appType)
    {
        camera_config_t *cameraConfig = (camera_config_t *)param;
        // IR camera
        int st = CAMERA_DEVICE_Init(&cameraDevice[1], cameraConfig);
        if (st != kStatus_Success)
        {
            LOGE("Camera device init error:%d\r\n", st);
        }
        CAMERA_DEVICE_Control(&cameraDevice[1], kCAMERA_DeviceMonoMode, CAMERA_MONO_MODE_ENABLED);
        CAMERA_DEVICE_Stop(&cameraDevice[1]);
    }
    xEventGroupSetBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_CAMERADEVICE_INIT_BIT);
    vTaskDelete(NULL);
#endif
}

static void Camera_Deinit(void)
{
    Camera_SelectLED(LED_IR);
    Camera_SetPWM(LED_IR,0);

    Camera_SelectLED(LED_WHITE);
    Camera_SetPWM(LED_WHITE,0);

    Camera_LedTimer_Deinit();
    DisableIRQ(CSI_IRQn);

    CAMERA_DEVICE_Stop(&cameraDevice[0]);
    CAMERA_DEVICE_Deinit(&cameraDevice[0]);

    CAMERA_DEVICE_Stop(&cameraDevice[1]);
    CAMERA_DEVICE_Deinit(&cameraDevice[1]);

    CAMERA_RECEIVER_Stop(&cameraReceiver);
    CAMERA_RECEIVER_Deinit(&cameraReceiver);
    xEventGroupClearBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_CAMERA_INIT_BIT);
    xEventGroupSetBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_CAMERA_DEINIT_BIT);
    vTaskSuspend(NULL);
}

static void Camera_Init_Task(void *param)
{
    int32_t i = 0;
    camera_config_t cameraConfig;
    memset(&cameraConfig, 0, sizeof(cameraConfig));
    cameraConfig.pixelFormat   = kVIDEO_PixelFormatYUYV; // kVIDEO_PixelFormatRGB888;//kVIDEO_PixelFormatRGB565;
    cameraConfig.bytesPerPixel = APP_BPP;
    cameraConfig.resolution    = FSL_VIDEO_RESOLUTION(
        APP_CAMERA_WIDTH,
        APP_CAMERA_HEIGHT); // kVIDEO_ResolutionQVGA;//FSL_VIDEO_RESOLUTION(APP_CAMERA_WIDTH, APP_CAMERA_HEIGHT);
    cameraConfig.frameBufferLinePitch_Bytes = APP_CAMERA_WIDTH * APP_BPP;
    cameraConfig.interface                  = kCAMERA_InterfaceGatedClock;
    cameraConfig.controlFlags               = APP_CAMERA_CONTROL_FLAGS;
#if (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
    cameraConfig.framePerSec = 15; // 20
#else
    cameraConfig.framePerSec = 15;
#endif
    NVIC_SetPriority(CSI_IRQn, LIB_CSI_PRI);
    CAMERA_RECEIVER_Init(&cameraReceiver, &cameraConfig, Camera_Callback, NULL);
#if (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
    CAMERA_RECEIVER_SwapBytes(&cameraReceiver);
#endif

#if (CAMERA_DIFF_I2C_BUS || (APP_CAMERA_TYPE == APP_CAMERA_MT9M114))

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(CameraDevice_Init_Task, "CameraDevice_Init_Task", CAMERAINITTASK_STACKSIZE, &cameraConfig, CAMERAINITTASK_PRIORITY,
                                    s_CameraDeviceInitTaskStack, &s_CameraDeviceInitTaskTCB))
#else
    if (xTaskCreate(CameraDevice_Init_Task, "CameraDevice_Init_Task", CAMERAINITTASK_STACKSIZE, &cameraConfig,
                                    CAMERAINITTASK_PRIORITY, NULL) != pdPASS)
#endif
    {
        LOGE("[ERROR]CameraDevice  Init created failed\r\n");

        while (1)
            ;
    }
    // RGB camera
    CAMERA_DEVICE_Init(&cameraDevice[0], &cameraConfig);
    CAMERA_DEVICE_Control(&cameraDevice[0], kCAMERA_DeviceMonoMode, CAMERA_MONO_MODE_DISABLED);
    // RGB camera on
    CAMERA_DEVICE_Start(&cameraDevice[0]);
    xEventGroupWaitBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_CAMERADEVICE_INIT_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
#else
    // RGB camera off
    CAMERA_DEVICE_Stop(&cameraDevice[0]);
    // IR camera
    CAMERA_DEVICE_Init(&cameraDevice[1], &cameraConfig);
    CAMERA_DEVICE_Control(&cameraDevice[1], kCAMERA_DeviceMonoMode, CAMERA_MONO_MODE_ENABLED);
    // IR camera off
    CAMERA_DEVICE_Stop(&cameraDevice[1]);
    // RGB camera
    CAMERA_DEVICE_Init(&cameraDevice[0], &cameraConfig);
    CAMERA_DEVICE_Control(&cameraDevice[0], kCAMERA_DeviceMonoMode, CAMERA_MONO_MODE_DISABLED);
    // RGB camera on
    CAMERA_DEVICE_Start(&cameraDevice[0]);
#endif
    /* Submit the empty frame buffers to buffer queue. */
    for (i = 0; i < APP_FRAME_BUFFER_COUNT; i++)
    {
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver,
                                          (uint32_t)(s_pBufferQueue + i * APP_CAMERA_HEIGHT * APP_CAMERA_WIDTH));
    }

    CAMERA_RECEIVER_Start(&cameraReceiver);

    LOGD("[Camera]:running\r\n");

    xEventGroupSetBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_CAMERA_INIT_BIT);
    vTaskDelete(NULL);
}

static void Camera_CheckOverRun()
{
    if (DQIndex < EQIndex)
    {
        LOGE("Camera BQ overrun [%d:%d]\r\n", EQIndex, DQIndex);
    }
}

// Send msg to Dispaly Task by Camera task to signal that a frame is available for display.
static int Camera_SendDResMsg(void)
{
    QMsg *pDResMsg = (QMsg*)pvPortMalloc(sizeof(QMsg));
    if (NULL == pDResMsg)
    {
        LOGE("[ERROR]: pDResMsg pvPortMalloc failed\r\n");
        return -1;
    }
    pDResMsg->id = QMSG_DISPLAY_FRAME_RES;

    return Display_SendQMsg((void *)&pDResMsg);
}

// Send msg to Oasis Task by Camera task to signal that a frame is available for face rec.
static int Camera_SendFResMsg(void)
{
    QMsg *pFResMsg = (QMsg*)pvPortMalloc(sizeof(QMsg));
    if (NULL == pFResMsg)
    {
        LOGE("[ERROR]: pFResMsg pvPortMalloc failed\r\n");
        return -1;
    }
    pFResMsg->id = QMSG_FACEREC_FRAME_RES;

    return Oasis_SendQMsg((void *)&pFResMsg);
}

// Send msg to PXP Task by Camera task to signal that a frame is available for display .
static int Camera_SendDPxpMsg(uint32_t in_buffer, QUIInfoMsg *info, uint32_t out_buffer)
{
    QMsg *pDPxpMsg = (QMsg*)pvPortMalloc(sizeof(QMsg));
    if (NULL == pDPxpMsg)
    {
        LOGE("[ERROR]: pDPxpMsg pvPortMalloc failed\r\n");
        return -1;
    }
    pDPxpMsg->id = QMSG_PXP_DISPLAY;
    pDPxpMsg->msg.pxp.in_buffer = in_buffer;
    pDPxpMsg->msg.pxp.out_buffer = out_buffer;
    pDPxpMsg->msg.pxp.user_data = info;

    return PXP_SendQMsg((void *)&pDPxpMsg);
}

// Send msg to PXP Task by Camera task to signal that a frame is available for face rec .
static int Camera_SendFPxpMsg(uint32_t in_buffer, uint32_t out_buffer, void* user_data)
{
    QMsg *pFPxpMsg = (QMsg*)pvPortMalloc(sizeof(QMsg));
    if (NULL == pFPxpMsg)
    {
        LOGE("[ERROR]: FPxpMsg pvPortMalloc failed\r\n");
        return -1;
    }
    pFPxpMsg->id = QMSG_PXP_FACEREC;
    pFPxpMsg->msg.pxp.in_buffer = in_buffer;
    pFPxpMsg->msg.pxp.out_buffer = out_buffer;
    pFPxpMsg->msg.pxp.user_data = user_data;
    return PXP_SendQMsg((void *)&pFPxpMsg);
}

// Send msg to PXP Task by Camera task to signal that interface mode is changed .
static int Camera_SendPxpInfoModeMsg(uint8_t mode)
{
    QMsg *pInfoDispMode = (QMsg*)pvPortMalloc(sizeof(QMsg));
    if (NULL == pInfoDispMode)
    {
        LOGE("[ERROR]: pInfoDispMode pvPortMalloc failed\r\n");
        return -1;
    }
    pInfoDispMode->id = QMSG_DISPLAY_INTERFACE;
    pInfoDispMode->msg.cmd.data.interface_mode = mode;
    return PXP_SendQMsg((void *)&pInfoDispMode);
}

// Send msg to Display Task by Camera task to signal that interface mode is changed .
static int Camera_SendDispInfoModeMsg(uint8_t mode)
{
    QMsg *pInfoDispMode = (QMsg*)pvPortMalloc(sizeof(QMsg));
    if (NULL == pInfoDispMode)
    {
        LOGE("[ERROR]: pInfoDispMode pvPortMalloc failed\r\n");
        return -1;
    }
    pInfoDispMode->id = QMSG_DISPLAY_INTERFACE;
    pInfoDispMode->msg.cmd.data.interface_mode = mode;
    return Display_SendQMsg((void *)&pInfoDispMode);
}

static void Camera_Task(void *param)
{
    BaseType_t ret;
    QMsg *pQMsg;
    QUIInfoMsg infoMsgIn, infoMsgOut;
    uint8_t *pDetIR     = NULL;
    uint8_t *pDetRGB    = NULL;
    /*first bit indicate RGB frame is ready or not, 2nd bit indicate IR frame is ready or not*/
    uint32_t oasis_frames_ready = 0;
    uint16_t *pDispData = NULL;
    uint8_t dispMode    = Cfg_AppDataGetDisplayMode();
    memset(&infoMsgIn, 0x0, sizeof(infoMsgIn));
    memset(&infoMsgOut, 0x0, sizeof(infoMsgOut));

    xEventGroupWaitBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_CAMERA_INIT_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    while (1)
    {
        /* pick up message */
        ret = xQueueReceive(CameraMsgQ, (void *)&pQMsg, portMAX_DELAY);

        if (ret == pdTRUE)
        {
            switch (pQMsg->id)
            {
                case QMSG_CAMERA_USERID:
                {
                    /* This is used for single camera apptype userid */
                    if (kStatus_Success == CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &s_ActiveFrameAddr))
                    {
                        // RBG frame
                        if (pDispData)
                        {
                            memcpy(&infoMsgOut, &infoMsgIn, sizeof(QUIInfoMsg));
                            if (infoMsgOut.rect2[0] > 0) // means can detect face in the RGB frame, so use RGB facebox to display
                            {
                                memcpy(infoMsgOut.rect, infoMsgOut.rect2, sizeof(infoMsgOut.rect));
                            }
                            Camera_SendDPxpMsg(s_ActiveFrameAddr, &infoMsgOut, (uint32_t)pDispData);
                            pDispData = NULL;
                        }

                        if (pDetRGB)
                        {
                            Camera_SendFPxpMsg(s_ActiveFrameAddr, (uint32_t)pDetRGB, (void*)COLOR_CAMERA);
                            pDetRGB = NULL;
                        }

                        s_InactiveFrameAddr = s_ActiveFrameAddr;
                        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, s_InactiveFrameAddr);
                        Camera_CheckOverRun();
                        EQIndex++;
                    }
                }
                break;
                case QMSG_CAMERA_DQ:
                {
                    if (kStatus_Success == CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &s_ActiveFrameAddr))
                    {
#if (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
                        if (COLOR_CAMERA == s_CurrentCameraID)
#else
                        if (EQIndex % (RGB_IR_FRAME_RATIO + 1) < RGB_IR_FRAME_RATIO)
#endif
                        { // RBG frame
                            //UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[camera]:RBG frame!\r\n");
#if (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
                            // need to switch to IR camera
                            if (pDetIR || (dispMode == DISPLAY_MODE_IR))
                            {
                                Camera_RgbIrSwitch(IR_CAMERA);
                                s_CurrentCameraID = IR_CAMERA;
                            }
#endif
                            if ((dispMode == DISPLAY_MODE_RGB) && pDispData)
                            {
                                memcpy(&infoMsgOut, &infoMsgIn, sizeof(QUIInfoMsg));
                                if (infoMsgOut.rect2[0] > 0) // means can detect face in the RGB frame, so use RGB facebox to display
                                {
                                    memcpy(infoMsgOut.rect, infoMsgOut.rect2, sizeof(infoMsgOut.rect));
                                }
                                Camera_SendDPxpMsg(s_ActiveFrameAddr, &infoMsgOut, (uint32_t)pDispData);
                                pDispData = NULL;
                            }

                            if (pDetRGB)
                            {
                            	//there is RGB data request is pending
                                Camera_SendFPxpMsg(s_ActiveFrameAddr, (uint32_t)pDetRGB, (void*)COLOR_CAMERA);
                                pDetRGB = NULL;
                            }
                        }
#if (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
                        else if (IR_CAMERA == s_CurrentCameraID)
#else
                        else if (EQIndex % (RGB_IR_FRAME_RATIO + 1) == RGB_IR_FRAME_RATIO)
#endif
                        { // IR frame
                            //UsbShell_DbgPrintf(VERBOSE_MODE_L2, "[camera]:IR frame!\r\n");
#if (APP_CAMERA_TYPE == APP_CAMERA_GC0308)
                            // need to switch to RGB camera
                            if (pDetRGB || (dispMode == DISPLAY_MODE_RGB))
                            {
                                Camera_RgbIrSwitch(COLOR_CAMERA);
                                s_CurrentCameraID = COLOR_CAMERA;
                            }
#endif
                            if ((dispMode == DISPLAY_MODE_IR) && pDispData)
                            {
                                memcpy(&infoMsgOut, &infoMsgIn, sizeof(QUIInfoMsg));
                                Camera_SendDPxpMsg(s_ActiveFrameAddr, &infoMsgOut, (uint32_t)pDispData);
                                pDispData = NULL;
                            }

                            if (pDetIR)
                            {
                            	//IR frame req is pending
                                Camera_SendFPxpMsg(s_ActiveFrameAddr, (uint32_t)pDetIR,(void*)IR_CAMERA);
                                pDetIR = NULL;
                            }
                        }

#if (APP_CAMERA_TYPE == APP_CAMERA_MT9M114)
                        DisableIRQ(CSI_IRQn);
                        if (DQIndex % (RGB_IR_FRAME_RATIO + 1) == RGB_IR_FRAME_RATIO)
                        {
                            Camera_RgbIrSwitch(IR_CAMERA);
                        }
                        else
                        {
                            Camera_RgbIrSwitch(COLOR_CAMERA);
                        }
                        EnableIRQ(CSI_IRQn);
#endif

                        s_InactiveFrameAddr = s_ActiveFrameAddr;
                        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, s_InactiveFrameAddr);
                        Camera_CheckOverRun();
                        EQIndex++;
                    }
                    else
                    {
                        LOGE("[ERROR]:Camera DQ buffer\r\n");
                    }
                }
                break;

                case QMSG_PXP_DISPLAY:
                {
                    Camera_SendDResMsg();
                }
                break;

                case QMSG_PXP_FACEREC:
                {
                    uint32_t mask;
                    if (s_appType == APP_TYPE_USERID || s_appType == APP_TYPE_DOOR_ACCESS_LIGHT_SINGLE || s_appType == APP_TYPE_DOOR_ACCESS_HEAVY_SINGLE)
                    {
                        mask = (1UL<<COLOR_CAMERA);
                    }
                    else if (s_appType == APP_TYPE_ELOCK_LIGHT_SINGLE || s_appType == APP_TYPE_ELOCK_HEAVY_SINGLE)
                    {
                        mask = (1UL<<IR_CAMERA);
                    }
                    else // dual camera application
                    {
                        mask = (1UL<<IR_CAMERA) | (1UL<<COLOR_CAMERA);
                    }
                    oasis_frames_ready |= 1UL<<(uint32_t)pQMsg->msg.pxp.user_data;
                    if ((oasis_frames_ready&mask) == mask)
                    {
                        Camera_SendFResMsg();
                        oasis_frames_ready = 0;
                    }
                }
                break;

                case QMSG_FACEREC_FRAME_REQ:
                {

                    pDetIR = (uint8_t *)pQMsg->msg.raw.IR_frame_data;
                    pDetRGB = (uint8_t *)pQMsg->msg.raw.RGB_frame_data;

                }
                break;

                case QMSG_DISPLAY_FRAME_REQ:
                {
                    pDispData = (uint16_t *)pQMsg->msg.raw.IR_frame_data;
                }
                break;

                case QMSG_FACEREC_INFO_UPDATE:
                {
                    memcpy(&infoMsgIn, &pQMsg->msg.info, sizeof(QUIInfoMsg));
                    // LOGD("[rect:%d/%d/%d/%d]\r\n", infoMsg.rect[0], infoMsg.rect[1], infoMsg.rect[2],
                    // infoMsg.rect[3]); LOGD("[name:%s/%f]\r\n", pQMsg->msg.info.name, pQMsg->msg.info.similar);
                }
                break;

                case QMSG_CMD:
                {
                    if (pQMsg->msg.cmd.id == QCMD_DEINIT_CAMERA)
                    {
                        Camera_Deinit();
                    }
                    else if (pQMsg->msg.cmd.id == QCMD_CHANGE_RGB_IR_DISP_MODE)
                    {
                        dispMode = pQMsg->msg.cmd.data.display_mode;
                    }
                    else if (pQMsg->msg.cmd.id == QCMD_SET_PWM)
                    {
                    	Camera_SetPWM(pQMsg->msg.cmd.data.led_pwm[0],pQMsg->msg.cmd.data.led_pwm[1]);

                    }
                    else if(pQMsg->msg.cmd.id == QCMD_CHANGE_RGB_EXPOSURE_MODE)
                    {
                        s_CurExposureMode[COLOR_CAMERA] = pQMsg->msg.cmd.data.exposure_mode;
                    }
					else if (pQMsg->msg.cmd.id == QCMD_CHANGE_INFO_DISP_MODE)
                    {
                        uint8_t display_interface = pQMsg->msg.cmd.data.interface_mode;
                        Camera_SendPxpInfoModeMsg(display_interface);
                        Camera_SendDispInfoModeMsg(display_interface);
                    }
                }
                break;

                default:
                    break;
            }
        }
        if ((pQMsg->id != QMSG_CAMERA_USERID) && (pQMsg->id != QMSG_CAMERA_DQ))
            vPortFree(pQMsg);
    }
    // end if while (1)
    if (s_pBufferQueue)
        alignedFree(s_pBufferQueue);
#if CAMERA_ROTATE_FLAG
    if (g_pRotateBuff)
        alignedFree(g_pRotateBuff);
#endif
}

int Camera_Start()
{
    LOGD("[Camera]:starting...\r\n");
    // BOARD_InitCameraResource();
    s_appType = Cfg_AppDataGetApplicationType();
    if (APP_TYPE_USERID == s_appType)
    {
        DQMsg.id = QMSG_CAMERA_USERID;
    }
    else
    {
        DQMsg.id = QMSG_CAMERA_DQ;
    }
    CameraMsgQ = xQueueCreate(CAMERA_MSG_Q_COUNT, sizeof(QMsg *));

    int buffersize = 0;

    if (CameraMsgQ == NULL)
    {
        LOGE("[ERROR]:xQueueCreate camera queue\r\n");
        return -1;
    }
    g_SyncVideoEvents = xEventGroupCreate();
    if (g_SyncVideoEvents == NULL)
    {
        LOGE("[ERROR]Event Group failed\r\n");
        while (1)
            ;
    }

    buffersize     = APP_FRAME_BUFFER_COUNT * APP_CAMERA_HEIGHT * APP_CAMERA_WIDTH;
    s_pBufferQueue = (uint16_t *)alignedMalloc(buffersize * sizeof(uint16_t));
    if (s_pBufferQueue == NULL)
    {
        while (1)
            ;
    }
#if CAMERA_ROTATE_FLAG
    buffersize    = APP_CAMERA_HEIGHT * APP_CAMERA_WIDTH;
    g_pRotateBuff = (uint16_t *)alignedMalloc(buffersize * sizeof(uint16_t));
    if (g_pRotateBuff == NULL)
    {
        LOGE("get rotate buff failed\n");
        while (1)
            ;
    }
#else
    g_pRotateBuff = NULL;
#endif
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(Camera_Init_Task, "Camera_Init_Task", CAMERAINITTASK_STACKSIZE, NULL, CAMERAINITTASK_PRIORITY,
                                    s_CameraInitTaskStack, &s_CameraInitTaskTCB))
#else
    if (xTaskCreate(Camera_Init_Task, "Camera_Init_Task", CAMERAINITTASK_STACKSIZE, NULL, CAMERAINITTASK_PRIORITY, NULL) != pdPASS)
#endif
    {
        LOGE("[ERROR]Camera Init created failed\r\n");

        while (1)
            ;
    }

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(Camera_Task, "Camera_Task", CAMERATASK_STACKSIZE, NULL, CAMERATASK_PRIORITY,
                                    s_CameraTaskStack, &s_CameraTaskTCB))
#else
    if (xTaskCreate(Camera_Task, "Camera Task", CAMERATASK_STACKSIZE, NULL, CAMERATASK_PRIORITY, NULL) != pdPASS)
#endif
    {
        LOGE("[ERROR]Camera Task created failed\r\n");

        while (1)
            ;
    }

    LOGD("[Camera]:started\r\n");
    return 0;
}

int Camera_SendQMsg(void *msg)
{
    BaseType_t ret;

    ret = xQueueSend(CameraMsgQ, msg, (TickType_t)0);

    if (ret != pdPASS)
    {
        LOGE("[ERROR]:Camera_SendQMsg failed %d\r\n", ret);
        return -1;
    }

    return 0;
}

#endif
