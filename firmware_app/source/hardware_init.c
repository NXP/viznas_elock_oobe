/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP IoT Solutions Team.
 */

/*
 * @brief  board rt117F VIZN3 hardware configure
 *
 * @Author jianfeng.qin@nxp.com
 */

#include "board_define.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "sln_time.h"
#include "task.h"
#include "fsl_lpi2c_freertos.h"
#include "fsl_lpspi_freertos.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "fsl_clock.h"
#include "fsl_dc_fb_elcdif.h"

#include "fwk_log.h"

uint32_t LPI2C_GetInstance(LPI2C_Type *base);

static IRQn_Type const lpi2c_irqs[] = LPI2C_IRQS;
static lpi2c_rtos_handle_t master_rtos_handle[sizeof(lpi2c_irqs) / sizeof(lpi2c_irqs[0])];

#define WEAK __attribute__((weak))
WEAK void INPUT_DEV_PUSH_BUTTONS_IRQHandler(GPIO_Type *base, uint32_t intPin)
{
}
/*----------------------------------------------------------------------------------------
 * Use sub #define to identify vizn3 model selection and complete all pin configure here
 * --------------------------------------------------------------------------------------*/

void BOARD_LPI2C_Init(LPI2C_Type *base, uint32_t clkSrc_Hz)
{
    status_t status;
    uint32_t instance = LPI2C_GetInstance(base);
    lpi2c_master_config_t masterConfig;
    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * masterConfig.baudRate_Hz = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    LPI2C_MasterGetDefaultConfig(&masterConfig);

#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
    status = LPI2C_RTOS_Init(&master_rtos_handle[instance], base, &masterConfig, clkSrc_Hz);
#else
    status        = LPI2C_MasterInit(base, &masterConfig, clkSrc_Hz);
#endif
    NVIC_SetPriority(lpi2c_irqs[instance], configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);

    if (status != kStatus_Success)
    {
        LOGE("LPI2C %d master: Error initializing LPI2C!", instance);
    }
}

status_t BOARD_LPI2C_Send(LPI2C_Type *base,
                          uint8_t deviceAddress,
                          uint32_t subAddress,
                          uint8_t subAddressSize,
                          uint8_t *txBuff,
                          uint8_t txBuffSize)
{
    status_t status;
    uint32_t instance = LPI2C_GetInstance(base);
    lpi2c_master_transfer_t xfer;

    xfer.flags          = kLPI2C_TransferDefaultFlag;
    xfer.slaveAddress   = deviceAddress;
    xfer.direction      = kLPI2C_Write;
    xfer.subaddress     = subAddress;
    xfer.subaddressSize = subAddressSize;
    xfer.data           = txBuff;
    xfer.dataSize       = txBuffSize;

#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
    status = LPI2C_RTOS_Transfer(&master_rtos_handle[instance], &xfer);
#else
    status        = LPI2C_MasterTransferBlocking(base, &xfer);
#endif

    return status;
}

status_t BOARD_LPI2C_Receive(LPI2C_Type *base,
                             uint8_t deviceAddress,
                             uint32_t subAddress,
                             uint8_t subAddressSize,
                             uint8_t *rxBuff,
                             uint8_t rxBuffSize)
{
    status_t status;
    uint32_t instance = LPI2C_GetInstance(base);
    lpi2c_master_transfer_t xfer;

    xfer.flags          = kLPI2C_TransferDefaultFlag;
    xfer.slaveAddress   = deviceAddress;
    xfer.direction      = kLPI2C_Read;
    xfer.subaddress     = subAddress;
    xfer.subaddressSize = subAddressSize;
    xfer.data           = rxBuff;
    xfer.dataSize       = rxBuffSize;

#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
    status = LPI2C_RTOS_Transfer(&master_rtos_handle[instance], &xfer);
#else
    status        = LPI2C_MasterTransferBlocking(base, &xfer);
#endif

    return status;
}

#if defined(SDK_SW_SPI) && SDK_SW_SPI
#define CS_L  (BOARD_ELCDIF_DISPLAY_SPI_CS_GPIO->DR &= ~(1 << BOARD_ELCDIF_DISPLAY_SPI_CS_PIN))
#define CS_H  (BOARD_ELCDIF_DISPLAY_SPI_CS_GPIO->DR |= 1 << BOARD_ELCDIF_DISPLAY_SPI_CS_PIN)
#define SCL_L (BOARD_ELCDIF_DISPLAY_SPI_SCL_GPIO->DR &= ~(1 << BOARD_ELCDIF_DISPLAY_SPI_SCL_PIN))
#define SCL_H (BOARD_ELCDIF_DISPLAY_SPI_SCL_GPIO->DR |= 1 << BOARD_ELCDIF_DISPLAY_SPI_SCL_PIN)
#define SDA_L (BOARD_ELCDIF_DISPLAY_SPI_SDA_GPIO->DR &= ~(1 << BOARD_ELCDIF_DISPLAY_SPI_SDA_PIN))
#define SDA_H (BOARD_ELCDIF_DISPLAY_SPI_SDA_GPIO->DR |= 1 << BOARD_ELCDIF_DISPLAY_SPI_SDA_PIN)
#define SPI_SW_Delay(x)
// SDK_DelayAtLeastUs(x, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY)//vTaskDelay(x)

__attribute__((section(".ramfunc.$SRAM_ITC_cm7")))
void board_elcdif_rk024hh298_spi_send_cmd(uint8_t cmd)
{
    uint8_t n, m;

    __disable_irq();

    m = 0x80;

    CS_L;
    SPI_SW_Delay(1);
    SCL_L;
    SDA_L;
    SPI_SW_Delay(1);
    SCL_H;
    SPI_SW_Delay(2);
    for (n = 0; n < 8; n++)
    {
        if (cmd & m)
        {
            SCL_L;
            SPI_SW_Delay(1);
            SDA_H;
            SPI_SW_Delay(1);
            SCL_H;
            SPI_SW_Delay(1);
        }
        else
        {
            SCL_L;
            SPI_SW_Delay(1);
            SDA_L;
            SPI_SW_Delay(1);
            SCL_H;
            SPI_SW_Delay(1);
        }
        m = m >> 1;
        SPI_SW_Delay(1);
    }
    SCL_L;
    SDA_H;
    SPI_SW_Delay(2);
    CS_H;
    SPI_SW_Delay(10);

    __enable_irq();
}

__attribute__((section(".ramfunc.$SRAM_ITC_cm7")))
void board_elcdif_rk024hh298_spi_send_data(uint8_t data)
{
    uint8_t n, m;

    __disable_irq();

    m = 0x80;

    CS_L;
    SPI_SW_Delay(1);
    SCL_L;
    SDA_H;
    SPI_SW_Delay(1);
    SCL_H;
    SPI_SW_Delay(2);
    for (n = 0; n < 8; n++)
    {
        if (data & m)
        {
            SCL_L;
            SPI_SW_Delay(1);
            SDA_H;
            SPI_SW_Delay(1);
            SCL_H;
            SPI_SW_Delay(1);
        }
        else
        {
            SCL_L;
            SPI_SW_Delay(1);
            SDA_L;
            SPI_SW_Delay(1);
            SCL_H;
            SPI_SW_Delay(1);
        }
        m = m >> 1;
        SPI_SW_Delay(1);
    }
    SCL_L;
    SDA_H;
    SPI_SW_Delay(2);
    CS_H;
    SPI_SW_Delay(10);

    __enable_irq();
}
#endif

void BOARD_CSICameraI2CInit(void)
{
    CLOCK_SetMux(kCLOCK_Lpi2cMux, BOARD_CSI_CAMERA_I2C_CLOCK_SOURCE);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, BOARD_CSI_CAMERA_I2C_CLOCK_DIVIDER);

    BOARD_LPI2C_Init(BOARD_CSI_CAMERA_I2C_BASE, BOARD_CSI_CAMERA_I2C_CLOCK_FREQ);
}

status_t BOARD_CSICameraI2CSend(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_LPI2C_Send(BOARD_CSI_CAMERA_I2C_BASE, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                            txBuffSize);
}

status_t BOARD_CSICameraI2CReceive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_CSI_CAMERA_I2C_BASE, deviceAddress, subAddress, subAddressSize, rxBuff,
                               rxBuffSize);
}

void BOARD_PullCSICameraRGBPowerDownPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_CSI_RGB_CAMERA_PWDN_GPIO, BOARD_CSI_RGB_CAMERA_PWDN_PIN, 1U);
    }
    else
    {
        GPIO_PinWrite(BOARD_CSI_RGB_CAMERA_PWDN_GPIO, BOARD_CSI_RGB_CAMERA_PWDN_PIN, 0U);
    }
}

void BOARD_PullCSICameraIRPowerDownPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_CSI_IR_CAMERA_PWDN_GPIO, BOARD_CSI_IR_CAMERA_PWDN_PIN, 1U);
    }
    else
    {
        GPIO_PinWrite(BOARD_CSI_IR_CAMERA_PWDN_GPIO, BOARD_CSI_IR_CAMERA_PWDN_PIN, 0U);
    }
}

void BOARD_FlexioCameraI2CInit(void)
{
    CLOCK_SetMux(kCLOCK_Lpi2cMux, BOARD_FLEXIO_CAMERA_I2C_CLOCK_SOURCE);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, BOARD_FLEXIO_CAMERA_I2C_CLOCK_DIVIDER);

    BOARD_LPI2C_Init(BOARD_FLEXIO_CAMERA_I2C_BASE, BOARD_FLEXIO_CAMERA_I2C_CLOCK_FREQ);
}

status_t BOARD_FlexioCameraI2CSend(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_LPI2C_Send(BOARD_FLEXIO_CAMERA_I2C_BASE, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                            txBuffSize);
}

status_t BOARD_FlexioCameraI2CReceive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_FLEXIO_CAMERA_I2C_BASE, deviceAddress, subAddress, subAddressSize, rxBuff,
                               rxBuffSize);
}

void BOARD_PullFlexioCameraResetPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_FLEXIO_CAMERA_RESET_GPIO, BOARD_FLEXIO_CAMERA_RESET_PIN, 1U);
    }
    else
    {
        GPIO_PinWrite(BOARD_FLEXIO_CAMERA_RESET_GPIO, BOARD_FLEXIO_CAMERA_RESET_PIN, 0U);
    }
}

void BOARD_PullFlexioCameraPowerDownPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_FLEXIO_CAMERA_PWDN_GPIO, BOARD_FLEXIO_CAMERA_PWDN_PIN, 1U);
    }
    else
    {
        GPIO_PinWrite(BOARD_FLEXIO_CAMERA_PWDN_GPIO, BOARD_FLEXIO_CAMERA_PWDN_PIN, 0U);
    }
}

void board_pull_elcdif_rk024hh298_reset_pin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_ELCDIF_DISPLAY_RESET_GPIO, BOARD_ELCDIF_DISPLAY_RESET_PIN, 1U);
    }
    else
    {
        GPIO_PinWrite(BOARD_ELCDIF_DISPLAY_RESET_GPIO, BOARD_ELCDIF_DISPLAY_RESET_PIN, 0U);
    }
}

void board_pull_elcdif_rk024hh298_backlight_pin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_ELCDIF_DISPLAY_BACKLIGHT_GPIO, BOARD_ELCDIF_DISPLAY_BACKLIGHT_PIN, 1U);
    }
    else
    {
        GPIO_PinWrite(BOARD_ELCDIF_DISPLAY_BACKLIGHT_GPIO, BOARD_ELCDIF_DISPLAY_BACKLIGHT_PIN, 0U);
    }
}

void BOARD_InitLedResource(void)
{
    BOARD_InitLedPins();
}

void BOARD_InitElcdifRk024hh298Resource(void)
{
    BOARD_InitElcdifRk024hh298LcdPins();

    /* GPIO configuration of PERI_PWREN on PMIC_STBY_REQ (pin L7) */
    gpio_pin_config_t pin_config = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };

    pin_config.outputLogic = 1;

    /* reset = 1, lcd work normally. */
    GPIO_PinInit(BOARD_ELCDIF_DISPLAY_RESET_GPIO, BOARD_ELCDIF_DISPLAY_RESET_PIN, &pin_config);
    GPIO_PinInit(BOARD_ELCDIF_DISPLAY_SPI_CS_GPIO, BOARD_ELCDIF_DISPLAY_SPI_CS_PIN, &pin_config);
    GPIO_PinInit(BOARD_ELCDIF_DISPLAY_SPI_SDA_GPIO, BOARD_ELCDIF_DISPLAY_SPI_SDA_PIN, &pin_config);

    pin_config.outputLogic = 0;
    GPIO_PinInit(BOARD_ELCDIF_DISPLAY_SPI_SCL_GPIO, BOARD_ELCDIF_DISPLAY_SPI_SCL_PIN, &pin_config);

    /* backlight = 0, lcd work normally. Before splash screen ready, set 1 to power down backlight. */
    pin_config.outputLogic = 1;
    GPIO_PinInit(BOARD_ELCDIF_DISPLAY_BACKLIGHT_GPIO, BOARD_ELCDIF_DISPLAY_BACKLIGHT_PIN, &pin_config);
}

void board_pull_elcdif_rk043fn02h_backlight_pin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_ELCDIF_DISPLAY_BACKLIGHT_GPIO, BOARD_ELCDIF_DISPLAY_BACKLIGHT_PIN, 1U);
    }
    else
    {
        GPIO_PinWrite(BOARD_ELCDIF_DISPLAY_BACKLIGHT_GPIO, BOARD_ELCDIF_DISPLAY_BACKLIGHT_PIN, 0U);
    }
}

void BOARD_InitElcdifRk043fn02hResource(void)
{
    BOARD_InitElcdifRk043fn02hLcdPins();

    /* GPIO configuration of PERI_PWREN on PMIC_STBY_REQ (pin L7) */
    gpio_pin_config_t pin_config = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };

    /* backlight = 1, lcd work normally. */
    pin_config.outputLogic = 1;
    GPIO_PinInit(BOARD_ELCDIF_DISPLAY_BACKLIGHT_GPIO, BOARD_ELCDIF_DISPLAY_BACKLIGHT_PIN, &pin_config);
}

void BOARD_InitFlexioCameraResource(void)
{
    BOARD_InitFlexioCameraPins();

    /* Set the pins for CSI reset and power down. */
    gpio_pin_config_t pinConfig = {
        kGPIO_DigitalOutput,
        0,
        kGPIO_NoIntmode,
    };

    /* GC0308 Sensor PWDN
     * 0: normal work,
     * 1: standby     */
    GPIO_PinInit(BOARD_FLEXIO_CAMERA_PWDN_GPIO, BOARD_FLEXIO_CAMERA_PWDN_PIN, &pinConfig);

    CLOCK_SetMux(kCLOCK_Flexio2Mux, 3);
    CLOCK_SetDiv(kCLOCK_Flexio2PreDiv, 1);
    CLOCK_SetDiv(kCLOCK_Flexio2Div, 1);

#if 0
    /* GC0308 Sensor Reset
     * 0: chip reset
     * 1: normal work. */
    pinConfig.outputLogic = 1;
    GPIO_PinInit(BOARD_FLEXIO_CAMERA_RESET_GPIO, BOARD_FLEXIO_CAMERA_RESET_PIN, &pinConfig);

    pinConfig.outputLogic = 0;
    GPIO_PinInit(BOARD_FLEXIO_CAMERA_RESET_GPIO, BOARD_FLEXIO_CAMERA_RESET_PIN, &pinConfig);

    pinConfig.outputLogic = 1;
    GPIO_PinInit(BOARD_FLEXIO_CAMERA_RESET_GPIO, BOARD_FLEXIO_CAMERA_RESET_PIN, &pinConfig);
#endif

    BOARD_FlexioCameraI2CInit();
}

void BOARD_InitCSICameraResource(void)
{
    BOARD_InitCsiCameraPins();

    /* Set the pins for CSI reset and power down. */
    gpio_pin_config_t pinConfig = {
        kGPIO_DigitalOutput,
        1,
        kGPIO_NoIntmode,
    };

    /* GC0308 Sensor PWDN
     * 0: normal work,
     * 1: standby     */
    /* shared dual camera share same i2c and csi line, we need power down both device after power on. */
    GPIO_PinInit(BOARD_CSI_RGB_CAMERA_PWDN_GPIO, BOARD_CSI_RGB_CAMERA_PWDN_PIN, &pinConfig);
    GPIO_PinInit(BOARD_CSI_IR_CAMERA_PWDN_GPIO, BOARD_CSI_IR_CAMERA_PWDN_PIN, &pinConfig);

    /* csi mux osc_clock 24Mh/div0 = 24Mhz */
    CLOCK_SetMux(kCLOCK_CsiMux, 0);
    CLOCK_SetDiv(kCLOCK_CsiDiv, 1);

    BOARD_CSICameraI2CInit();
}

void BOARD_InitEDMA()
{
    edma_config_t edmaConfig;
    DMAMUX_Init(DMAMUX);
    EDMA_GetDefaultConfig(&edmaConfig);
    edmaConfig.enableDebugMode = true;
    EDMA_Init(DMA0, &edmaConfig);
}

void BOARD_InitFlashResources()
{
    BOARD_InitFlexSPIPins();
    /*Clock setting for flexspi1*/

    CLOCK_SetMux(kCLOCK_FlexspiMux, 3);
    CLOCK_SetDiv(kCLOCK_FlexspiDiv, 1);
}

void BOARD_InitHardware(void)
{
#if RELOCATE_VECTOR_TABLE
    BOARD_RelocateVectorTableToRam();
#endif
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitEDMA();
    Time_Init(1);
}

void GPIO2_Combined_16_31_IRQHandler(void)
{
    uint32_t intPin        = 0x00;
    uint32_t pushButtonInt = 0x00;
    // Get interrupt flag for the GPIO
    intPin = GPIO_PortGetInterruptFlags(GPIO2);
    pushButtonInt = (1 << BOARD_BUTTON_SW1_PIN) | (1 << BOARD_BUTTON_SW2_PIN) | (1 << BOARD_BUTTON_SW3_PIN);

    // Check for the interrupt pin on the GPIO for USER_BUTTON (SW7)
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        BaseType_t HigherPriorityTaskWoken = pdFALSE;

        // Check if interrupt was triggered by a push button
        if (intPin & pushButtonInt)
        {
            INPUT_DEV_PUSH_BUTTONS_IRQHandler(GPIO2, intPin);
            /* clear the interrupt status */
            GPIO_PortClearInterruptFlags(GPIO2, pushButtonInt);

            portYIELD_FROM_ISR(HigherPriorityTaskWoken);
        }
        else
        {
            // TODO
            portYIELD_FROM_ISR(HigherPriorityTaskWoken);
        }
    }
}

