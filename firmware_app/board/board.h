/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"

#include "board_define.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE kSerialPort_Uart
#define BOARD_DEBUG_UART_CLK_FREQ BOARD_DebugConsoleSrcFreq()

#if DEBUG_CONSOLE_UART_INDEX == 1
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART1
#define BOARD_DEBUG_UART_INSTANCE 1U
#define BOARD_UART_IRQ LPUART1_IRQn
#define BOARD_UART_IRQ_HANDLER LPUART1_IRQHandler
#elif DEBUG_CONSOLE_UART_INDEX == 2
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART2
#define BOARD_DEBUG_UART_INSTANCE 2U
#define BOARD_UART_IRQ LPUART2_IRQn
#define BOARD_UART_IRQ_HANDLER LPUART2_IRQHandler
#elif DEBUG_CONSOLE_UART_INDEX == 3
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART3
#define BOARD_DEBUG_UART_INSTANCE 3U
#define BOARD_UART_IRQ LPUART3_IRQn
#define BOARD_UART_IRQ_HANDLER LPUART3_IRQHandler
#elif DEBUG_CONSOLE_UART_INDEX == 5
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART5
#define BOARD_DEBUG_UART_INSTANCE 5U
#define BOARD_UART_IRQ LPUART5_IRQn
#define BOARD_UART_IRQ_HANDLER LPUART5_IRQHandler
#else
#error "Unsupported UART"
#endif

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE (115200U)
#endif /* BOARD_DEBUG_UART_BAUDRATE */

/* USB PHY condfiguration */
#define BOARD_USB_PHY_D_CAL (0x0CU)
#define BOARD_USB_PHY_TXCAL45DP (0x06U)
#define BOARD_USB_PHY_TXCAL45DM (0x06U)

#ifndef BOARD_USER_BUTTON_GPIO
#define BOARD_USER_BUTTON_GPIO GPIO5
#endif
#ifndef BOARD_USER_BUTTON_GPIO_PIN
#define BOARD_USER_BUTTON_GPIO_PIN (0U)
#endif
#define BOARD_USER_BUTTON_IRQ GPIO5_Combined_0_15_IRQn
#define BOARD_USER_BUTTON_IRQ_HANDLER GPIO5_Combined_0_15_IRQHandler
#define BOARD_USER_BUTTON_NAME "SW4"

/* FlexIO Camera configurations */
#define BOARD_FLEXIO_CAMERA_INST FLEXIO2
#define BOARD_FLEXIO_CAMERA_DATA_PIN_START_INDEX 24
#define BOARD_FLEXIO_CAMERA_DATA_PIN_END_INDEX 31
#define BOARD_FLEXIO_CAMERA_XCLK_PIN_INDEX 0
#define BOARD_FLEXIO_CAMERA_PCLK_PIN_INDEX 1
#define BOARD_FLEXIO_CAMERA_HREF_PIN_INDEX 2

#define BOARD_CAMERA_VSYNC_GPIO_BASE GPIO3
#define BOARD_CAMERA_VSYNC_PIN_INDEX 31u
#define BOARD_CAMERA_VSYNC_IRQn GPIO3_Combined_16_31_IRQn
#define BOARD_CAMERA_VSYNC_IRQHandler GPIO3_Combined_16_31_IRQHandler

#define BOARD_FLEXIO_CAMERA_I2C_BASE LPI2C1
#define BOARD_FLEXIO_CAMERA_I2C_CLOCK_SOURCE  (0U)  /* Select USB1 PLL (480 MHz) as LPI2C's clock source */
#define BOARD_FLEXIO_CAMERA_I2C_CLOCK_DIVIDER (5U) /* Divider = 12, LPI2C clock frequency 2M. */
#define BOARD_FLEXIO_CAMERA_I2C_CLOCK_FREQ \
    (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8 / (BOARD_FLEXIO_CAMERA_I2C_CLOCK_DIVIDER + 1U))

#define BOARD_FLEXIO_CAMERA_PWDN_GPIO GPIO1
#define BOARD_FLEXIO_CAMERA_PWDN_PIN 13U
#define BOARD_FLEXIO_CAMERA_RESET_GPIO GPIO1
#define BOARD_FLEXIO_CAMERA_RESET_PIN 25U

/* CSI Camera configurations */
#define BOARD_CSI_CAMERA_I2C_BASE LPI2C4
#define BOARD_CSI_CAMERA_I2C_CLOCK_SOURCE  (0U) /* Select USB1 PLL (480 MHz) as LPI2C's clock source */
#define BOARD_CSI_CAMERA_I2C_CLOCK_DIVIDER (5U) /* Divider = 12, LPI2C clock frequency 2M. */
#define BOARD_CSI_CAMERA_I2C_CLOCK_FREQ \
    (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8 / (BOARD_CSI_CAMERA_I2C_CLOCK_DIVIDER + 1U))

#define BOARD_CSI_RGB_CAMERA_PWDN_GPIO GPIO2
#define BOARD_CSI_RGB_CAMERA_PWDN_PIN  25U

#define BOARD_CSI_IR_CAMERA_PWDN_GPIO GPIO2
#define BOARD_CSI_IR_CAMERA_PWDN_PIN  27U

/* buttons configurations */
#define BOARD_BUTTON_SW1_GPIO GPIO2
#define BOARD_BUTTON_SW1_PIN (23U)
#define BOARD_BUTTON_SW2_GPIO GPIO2
#define BOARD_BUTTON_SW2_PIN (24U)
#define BOARD_BUTTON_SW3_GPIO GPIO2
#define BOARD_BUTTON_SW3_PIN (26U)
#define BOARD_BUTTON_SW1_IRQ GPIO2_Combined_16_31_IRQn
#define BOARD_BUTTON_SW2_IRQ GPIO2_Combined_16_31_IRQn
#define BOARD_BUTTON_SW3_IRQ GPIO2_Combined_16_31_IRQn

/* @Brief Board LCD configuration */
#define LPSPI_CLOCK_SOURCE_ROOT kCLOCK_Root_Lpspi1
#define LPSPI_CLOCK_SOURCE_SELECT (3U) /* Select SYS PLL2 PFD2 (328 MHz) as lpspi clock source */
#define LPSPI_CLOCK_SOURCE_DIVIDER (2U)
#define LPSPI_MASTER_CLK_FREQ (CLOCK_GetFreqFromObs(CCM_OBS_LPSPI1_CLK_ROOT))

#define BOARD_ELCDIF_DISPLAY_SPI_BASE LPSPI1

#define BOARD_ELCDIF_DISPLAY_RESET_GPIO GPIO1
#define BOARD_ELCDIF_DISPLAY_RESET_PIN 14U
#define BOARD_ELCDIF_DISPLAY_BACKLIGHT_GPIO GPIO3
#define BOARD_ELCDIF_DISPLAY_BACKLIGHT_PIN 02U

#define BOARD_ELCDIF_DISPLAY_SPI_CS_GPIO GPIO3
#define BOARD_ELCDIF_DISPLAY_SPI_CS_PIN 27U
#define BOARD_ELCDIF_DISPLAY_SPI_SCL_GPIO GPIO2
#define BOARD_ELCDIF_DISPLAY_SPI_SCL_PIN 20U
#define BOARD_ELCDIF_DISPLAY_SPI_SDA_GPIO GPIO2
#define BOARD_ELCDIF_DISPLAY_SPI_SDA_PIN 21U

/* @Brief Board LED configuration */
#define BOARD_LED_PORT GPIO3
#define BOARD_LED_PIN  03U

/* @Brief Board BLE configuration */
#define BOARD_BLE_QN9090_QN_WAKEUP_GPIO GPIO2
#define BOARD_BLE_QN9090_QN_WAKEUP_PIN 13U

#define BOARD_FLASH_SIZE 0x1000000U
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
uint32_t BOARD_DebugConsoleSrcFreq(void);

void BOARD_InitDebugConsole(void);

void BOARD_ConfigMPU(void);
#if defined(SDK_I2C_BASED_COMPONENT_USED) && SDK_I2C_BASED_COMPONENT_USED
void BOARD_LPI2C_Init(LPI2C_Type *base, uint32_t clkSrc_Hz);
status_t BOARD_LPI2C_Send(LPI2C_Type *base,
                          uint8_t deviceAddress,
                          uint32_t subAddress,
                          uint8_t subaddressSize,
                          uint8_t *txBuff,
                          uint8_t txBuffSize);
status_t BOARD_LPI2C_Receive(LPI2C_Type *base,
                             uint8_t deviceAddress,
                             uint32_t subAddress,
                             uint8_t subaddressSize,
                             uint8_t *rxBuff,
                             uint8_t rxBuffSize);
status_t BOARD_LPI2C_SendSCCB(LPI2C_Type *base,
                              uint8_t deviceAddress,
                              uint32_t subAddress,
                              uint8_t subaddressSize,
                              uint8_t *txBuff,
                              uint8_t txBuffSize);
status_t BOARD_LPI2C_ReceiveSCCB(LPI2C_Type *base,
                                 uint8_t deviceAddress,
                                 uint32_t subAddress,
                                 uint8_t subaddressSize,
                                 uint8_t *rxBuff,
                                 uint8_t rxBuffSize);
void BOARD_Accel_I2C_Init(void);
status_t BOARD_Accel_I2C_Send(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff);
status_t BOARD_Accel_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);
void BOARD_Codec_I2C_Init(void);
status_t BOARD_Codec_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);
status_t BOARD_Codec_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);
void BOARD_Camera_I2C_Init(void);
status_t BOARD_Camera_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);
status_t BOARD_Camera_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

status_t BOARD_Camera_I2C_SendSCCB(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);
status_t BOARD_Camera_I2C_ReceiveSCCB(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

void BOARD_MIPIPanelTouch_I2C_Init(void);
status_t BOARD_MIPIPanelTouch_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);
status_t BOARD_MIPIPanelTouch_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);
#endif /* SDK_I2C_BASED_COMPONENT_USED */

void BOARD_RelocateVectorTableToRam(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
