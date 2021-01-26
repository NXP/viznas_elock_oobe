/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_RTVISION_H_
#define _BOARD_RTVISION_H_

//#if RTVISION_BOARD
#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"
#include "sln_pcal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#define BOARD_NAME "SLN-VIZNAS-IOT"

//Flash type
#define HYPER_FLASH 0
#define QSPI_FLASH  1
#ifndef FLASH_TYPE
#define FLASH_TYPE HYPER_FLASH
#endif

//LCD_TYPE
#define LCD_SPI_RIVERDI          0x00
#define LCD_SPI_WZ032HN89V022    0x01   //����Ƽ�

#define LCD_RGB_PJ2805H02        0x80   //PengJi Photoelectricity
#ifndef LCD_TYPE
#define LCD_TYPE  LCD_SPI_RIVERDI
#endif
#define BOARD_SUPPORT_PARALLEL_LCD  ((LCD_TYPE & 0x80) ? 1 : 0)

//Are RGB and IR dual cameras using different i2c bus lines?
#ifndef CAMERA_DIFF_I2C_BUS
#define CAMERA_DIFF_I2C_BUS     (0)
#endif

//screen orientation: portrait  or landscape
#ifndef SCREEN_PORTRAIT_MODE
#define SCREEN_PORTRAIT_MODE    (0)
#endif
//we need 90deg rotated camera module for portrait display.
#define CAMERA_ROTATE_FLAG  (SCREEN_PORTRAIT_MODE ? 1 : 0)

#if FLASH_TYPE == HYPER_FLASH
/*! @brief The board flash size */
#define BOARD_FLASH_SIZE (0x2000000U)
#define BOARD_FLASH_PAGE_SIZE (512)
#define BOARD_FLASH_SECTOR_SIZE (0x40000)
#else
//#define QSPI_FLASH_W25Q128
#define QSPI_FLASH_W25Q256
#ifdef QSPI_FLASH_W25Q256
#define BOARD_FLASH_SIZE (0x2000000U)
#else
#define BOARD_FLASH_SIZE (0x1000000U)
#endif
#define BOARD_FLASH_PAGE_SIZE (256)
#define BOARD_FLASH_SECTOR_SIZE (0x1000)
#endif

/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART3
#define BOARD_DEBUG_UART_INSTANCE 3U

#define BOARD_DEBUG_UART_CLK_FREQ BOARD_DebugConsoleSrcFreq()

#define BOARD_INITPINS_WL_REG_ON_GPIO GPIO5
#define BOARD_INITPINS_WL_REG_ON_PORT GPIO5
#define BOARD_INITPINS_WL_REG_ON_PIN  1U

#define BOARD_UART_IRQ         LPUART3_IRQn
#define BOARD_UART_IRQ_HANDLER LPUART3_IRQHandler

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE (115200U)
#endif /* BOARD_DEBUG_UART_BAUDRATE */

/* @brief BLE PWR Pins*/
#define BOARD_INITPINS_BT_REG_ON_GPIO GPIO2 // former  BOARD_BT_REG_ON_port
#define BOARD_INITPINS_BT_REG_ON_PIN  30U   // former  BOARD_BT_REG_ON_pin
#define BOARD_BT_UART_BASEADDR    LPUART1
#define BOARD_BT_UART_CLK_FREQ    BOARD_DebugConsoleSrcFreq()
#define BOARD_BT_UART_IRQ         LPUART1_IRQn
#define BOARD_BT_UART_IRQ_HANDLER LPUART1_IRQHandler

/*! @brief The USER_LED used for board */
#define LOGIC_LED_ON  (0U)
#define LOGIC_LED_OFF (1U)
#ifndef BOARD_USER_LED_GPIO
#define BOARD_USER_LED_GPIO GPIO1
#endif
#ifndef BOARD_USER_LED_GPIO_PIN
#define BOARD_USER_LED_GPIO_PIN (9U)
#endif

#define USER_LED_INIT(output)                                            \
    GPIO_PinWrite(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, output); \
    BOARD_USER_LED_GPIO->GDIR |= (1U << BOARD_USER_LED_GPIO_PIN) /*!< Enable target USER_LED */
#define USER_LED_ON() \
    GPIO_PortClear(BOARD_USER_LED_GPIO, 1U << BOARD_USER_LED_GPIO_PIN)                  /*!< Turn off target USER_LED */
#define USER_LED_OFF() GPIO_PortSet(BOARD_USER_LED_GPIO, 1U << BOARD_USER_LED_GPIO_PIN) /*!<Turn on target USER_LED*/
#define USER_LED_TOGGLE()                                       \
    GPIO_PinWrite(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, \
                  0x1 ^ GPIO_PinRead(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN)) /*!< Toggle target USER_LED */

#if 0
/*! @brief Define the port interrupt number for the board switches */
#ifndef BOARD_USER_BUTTON_GPIO
#define BOARD_USER_BUTTON_GPIO GPIO5
#endif
#ifndef BOARD_USER_BUTTON_GPIO_PIN
#define BOARD_USER_BUTTON_GPIO_PIN (0U)
#endif
#define BOARD_USER_BUTTON_IRQ         GPIO5_Combined_0_15_IRQn
#define BOARD_USER_BUTTON_IRQ_HANDLER GPIO5_Combined_0_15_IRQHandler
#define BOARD_USER_BUTTON_NAME        "SW8"
#endif

/*! @brief The GPIO2_IO04 SW1 */
#define SW1_GPIO     GPIO2
#define SW1_GPIO_PIN 4U

/*! @brief The PCAL port where GPIO switches (GPIO SW2, SW3, SW4) are connected */
#define SW_PORT_PCAL OUTPUT_PORT2

/*! @brief PCAL pins where GPIO switches (GPIO SW2, SW3, SW4) are connected */
#define SW2_PIN_PCAL 0U
#define SW3_PIN_PCAL 1U
#define SW4_PIN_PCAL 2U

/*! @brief GPIO (GPIO2_IO13) used by PCAL chip to trigger interrupts */

#define BOARD_HAS_PCAL    1
#define PCAL_INT_GPIO     GPIO2
#define PCAL_INT_GPIO_PIN 13

#define BOARD_USER_BUTTON_IRQ GPIO2_Combined_0_15_IRQn
//#define BOARD_USER_BUTTON_IRQ_HANDLER GPIO2_Combined_0_15_IRQHandler


/*! @brief The ENET PHY address. */
#define BOARD_ENET0_PHY_ADDRESS (0x01U) /* Phy address of enet port 0. */

/* USB PHY condfiguration */
#define BOARD_USB_PHY_D_CAL     (0x0CU)
#define BOARD_USB_PHY_TXCAL45DP (0x06U)
#define BOARD_USB_PHY_TXCAL45DM (0x06U)

#define BOARD_ARDUINO_INT_IRQ           (GPIO1_INT3_IRQn)
#define BOARD_ARDUINO_I2C_IRQ           (LPI2C1_IRQn)
#define BOARD_ARDUINO_I2C_INDEX         (1)
#define BOARD_USDHC1_BASEADDR           USDHC1
#define BOARD_USDHC2_BASEADDR           USDHC2
#define BOARD_USDHC_CD_GPIO_BASE        GPIO2
#define BOARD_USDHC_CD_GPIO_PIN         28
#define BOARD_USDHC_CD_PORT_IRQ         GPIO2_Combined_16_31_IRQn
#define BOARD_USDHC_CD_PORT_IRQ_HANDLER GPIO2_Combined_16_31_IRQHandler

#define BOARD_USDHC_CD_STATUS() (GPIO_PinRead(BOARD_USDHC_CD_GPIO_BASE, BOARD_USDHC_CD_GPIO_PIN))

#define BOARD_USDHC_CD_INTERRUPT_STATUS()    (GPIO_PortGetInterruptFlags(BOARD_USDHC_CD_GPIO_BASE))
#define BOARD_USDHC_CD_CLEAR_INTERRUPT(flag) (GPIO_PortClearInterruptFlags(BOARD_USDHC_CD_GPIO_BASE, flag))

#define BOARD_USDHC_CD_GPIO_INIT()                                                          \
    {                                                                                       \
        gpio_pin_config_t sw_config = {                                                     \
            kGPIO_DigitalInput,                                                             \
            0,                                                                              \
            kGPIO_IntRisingOrFallingEdge,                                                   \
        };                                                                                  \
        GPIO_PinInit(BOARD_USDHC_CD_GPIO_BASE, BOARD_USDHC_CD_GPIO_PIN, &sw_config);        \
        GPIO_PortEnableInterrupts(BOARD_USDHC_CD_GPIO_BASE, 1U << BOARD_USDHC_CD_GPIO_PIN); \
        GPIO_PortClearInterruptFlags(BOARD_USDHC_CD_GPIO_BASE, ~0);                         \
    }
#define BOARD_HAS_SDCARD              (1U)
#define BOARD_SD_POWER_RESET_GPIO     (GPIO5)
#define BOARD_SD_POWER_RESET_GPIO_PIN (1U)

/* @brief Wifi WL_REG_ON Pins*/
#define BOARD_INITPINS_WL_REG_ON_GPIO GPIO5 // former BOARD_WL_REG_ON_port
#define BOARD_INITPINS_WL_REG_ON_PIN  1U    // former BOARD_WL_REG_ON_pin

#define BOARD_WL_OOB_IRQ_port    GPIO5
#define BOARD_WL_OOB_IRQ_pin     0U
#define BOARD_WL_OOB_IRQ         GPIO5_Combined_0_15_IRQn
#define BOARD_WL_OOB_IRQ_Handler GPIO5_Combined_0_15_IRQHandler

#define BOARD_USDHC_CARD_INSERT_CD_LEVEL (0U)

#define BOARD_USDHC_MMCCARD_POWER_CONTROL(state)

#define BOARD_USDHC_MMCCARD_POWER_CONTROL_INIT()                                            \
    {                                                                                       \
        gpio_pin_config_t sw_config = {                                                     \
            kGPIO_DigitalOutput,                                                            \
            0,                                                                              \
            kGPIO_NoIntmode,                                                                \
        };                                                                                  \
        GPIO_PinInit(BOARD_SD_POWER_RESET_GPIO, BOARD_SD_POWER_RESET_GPIO_PIN, &sw_config); \
        GPIO_PinWrite(BOARD_SD_POWER_RESET_GPIO, BOARD_SD_POWER_RESET_GPIO_PIN, true);      \
    }

#define BOARD_USDHC_SDCARD_POWER_CONTROL_INIT()                                             \
    {                                                                                       \
        gpio_pin_config_t sw_config = {                                                     \
            kGPIO_DigitalOutput,                                                            \
            0,                                                                              \
            kGPIO_NoIntmode,                                                                \
        };                                                                                  \
        GPIO_PinInit(BOARD_SD_POWER_RESET_GPIO, BOARD_SD_POWER_RESET_GPIO_PIN, &sw_config); \
    }

#define BOARD_USDHC_SDCARD_POWER_CONTROL(state) \
    (GPIO_PinWrite(BOARD_SD_POWER_RESET_GPIO, BOARD_SD_POWER_RESET_GPIO_PIN, state))

#define BOARD_USDHC1_CLK_FREQ (CLOCK_GetSysPfdFreq(kCLOCK_Pfd0) / (CLOCK_GetDiv(kCLOCK_Usdhc1Div) + 1U))
#define BOARD_USDHC2_CLK_FREQ (CLOCK_GetSysPfdFreq(kCLOCK_Pfd0) / (CLOCK_GetDiv(kCLOCK_Usdhc2Div) + 1U))

#define BOARD_SD_HOST_BASEADDR BOARD_USDHC1_BASEADDR
#define BOARD_SD_HOST_CLK_FREQ BOARD_USDHC1_CLK_FREQ
#define BOARD_SD_HOST_IRQ      USDHC1_IRQn

#define BOARD_MMC_HOST_BASEADDR BOARD_USDHC2_BASEADDR
#define BOARD_MMC_HOST_CLK_FREQ BOARD_USDHC2_CLK_FREQ
#define BOARD_MMC_HOST_IRQ      USDHC2_IRQn
#define BOARD_MMC_VCCQ_SUPPLY   kMMC_VoltageWindow170to195
#define BOARD_MMC_VCC_SUPPLY    kMMC_VoltageWindows270to360
/* we are using the BB SD socket to DEMO the MMC example,but the
 * SD socket provide 4bit bus only, so we define this macro to avoid
 * 8bit data bus test
 */
#define BOARD_MMC_SUPPORT_8BIT_BUS (1U)

#define BOARD_SD_HOST_SUPPORT_SDR104_FREQ (200000000U)
#define BOARD_SD_HOST_SUPPORT_HS200_FREQ  (180000000U)

/* define for SD/MMC config IO driver strength dynamic */
#define BOARD_SD_PIN_CONFIG(speed, strength)                                                      \
    {                                                                                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_USDHC1_CMD,                                      \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_01_USDHC1_CLK,                                      \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(0) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_USDHC1_DATA0,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_USDHC1_DATA1,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_04_USDHC1_DATA2,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_05_USDHC1_DATA3,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
    }

#define BOARD_MMC_PIN_CONFIG(speed, strength)                                                     \
    {                                                                                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC2_CMD,                                      \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_04_USDHC2_CLK,                                      \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(0) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_03_USDHC2_DATA0,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_02_USDHC2_DATA1,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_01_USDHC2_DATA2,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_00_USDHC2_DATA3,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_08_USDHC2_DATA4,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_09_USDHC2_DATA5,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_10_USDHC2_DATA6,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_11_USDHC2_DATA7,                                    \
                            IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | \
                                IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |   \
                                IOMUXC_SW_PAD_CTL_PAD_DSE(strength));                             \
    }

/*! @brief The WIFI-QCA shield pin. */
#define BOARD_INITGT202SHIELD_PWRON_GPIO      GPIO1               /*!< GPIO device name: GPIO */
#define BOARD_INITGT202SHIELD_PWRON_PORT      1U                  /*!< PORT device index: 1 */
#define BOARD_INITGT202SHIELD_PWRON_GPIO_PIN  3U                  /*!< PIO4 pin index: 3 */
#define BOARD_INITGT202SHIELD_PWRON_PIN_NAME  GPIO1_3             /*!< Pin name */
#define BOARD_INITGT202SHIELD_PWRON_LABEL     "PWRON"             /*!< Label */
#define BOARD_INITGT202SHIELD_PWRON_NAME      "PWRON"             /*!< Identifier name */
#define BOARD_INITGT202SHIELD_PWRON_DIRECTION kGPIO_DigitalOutput /*!< Direction */

#define BOARD_INITGT202SHIELD_IRQ_GPIO      GPIO1              /*!< GPIO device name: GPIO */
#define BOARD_INITGT202SHIELD_IRQ_PORT      1U                 /*!< PORT device index: 1 */
#define BOARD_INITGT202SHIELD_IRQ_GPIO_PIN  19U                /*!< PIO1 pin index: 19 */
#define BOARD_INITGT202SHIELD_IRQ_PIN_NAME  GPIO1_19           /*!< Pin name */
#define BOARD_INITGT202SHIELD_IRQ_LABEL     "IRQ"              /*!< Label */
#define BOARD_INITGT202SHIELD_IRQ_NAME      "IRQ"              /*!< Identifier name */
#define BOARD_INITGT202SHIELD_IRQ_DIRECTION kGPIO_DigitalInput /*!< Direction */

#define BOARD_INITSILEX2401SHIELD_PWRON_GPIO      GPIO1               /*!< GPIO device name: GPIO */
#define BOARD_INITSILEX2401SHIELD_PWRON_PORT      1U                  /*!< PORT device index: 1 */
#define BOARD_INITSILEX2401SHIELD_PWRON_GPIO_PIN  9U                  /*!< PIO4 pin index: 9 */
#define BOARD_INITSILEX2401SHIELD_PWRON_PIN_NAME  GPIO1_9             /*!< Pin name */
#define BOARD_INITSILEX2401SHIELD_PWRON_LABEL     "PWRON"             /*!< Label */
#define BOARD_INITSILEX2401SHIELD_PWRON_NAME      "PWRON"             /*!< Identifier name */
#define BOARD_INITSILEX2401SHIELD_PWRON_DIRECTION kGPIO_DigitalOutput /*!< Direction */

#define BOARD_INITSILEX2401SHIELD_IRQ_GPIO      GPIO1              /*!< GPIO device name: GPIO */
#define BOARD_INITSILEX2401SHIELD_IRQ_PORT      1U                 /*!< PORT device index: 1 */
#define BOARD_INITSILEX2401SHIELD_IRQ_GPIO_PIN  11U                /*!< PIO1 pin index: 11 */
#define BOARD_INITSILEX2401SHIELD_IRQ_PIN_NAME  GPIO1_11           /*!< Pin name */
#define BOARD_INITSILEX2401SHIELD_IRQ_LABEL     "IRQ"              /*!< Label */
#define BOARD_INITSILEX2401SHIELD_IRQ_NAME      "IRQ"              /*!< Identifier name */
#define BOARD_INITSILEX2401SHIELD_IRQ_DIRECTION kGPIO_DigitalInput /*!< Direction */

/* @Brief Board accelerator sensor configuration */
#define BOARD_ACCEL_I2C_BASEADDR LPI2C1
/* Select USB1 PLL (480 MHz) as LPI2C's clock source */
#define BOARD_ACCEL_I2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for LPI2C clock source */
#define BOARD_ACCEL_I2C_CLOCK_SOURCE_DIVIDER (5U)
#define BOARD_ACCEL_I2C_CLOCK_FREQ           (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8 / (BOARD_ACCEL_I2C_CLOCK_SOURCE_DIVIDER + 1U))
#define BOARD_ACCEL_I2C_INSTANCE            (1U)

#define BOARD_CODEC_I2C_BASEADDR             LPI2C1
#define BOARD_CODEC_I2C_CLOCK_SOURCE_SELECT  (0U)
#define BOARD_CODEC_I2C_CLOCK_SOURCE_DIVIDER (5U)
#define BOARD_CODEC_I2C_CLOCK_FREQ \
    ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (BOARD_CODEC_I2C_CLOCK_SOURCE_DIVIDER + 1U))
#define BOARD_CODEC_I2C_INSTANCE             (1U)

/* @Brief Board CAMERA configuration */
#define BOARD_CAMERA_I2C_BASEADDR             LPI2C1
#define BOARD_CAMERA_I2C_INSTANCE             (1U)
#define BOARD_CAMERA_I2C_CLOCK_SOURCE_DIVIDER (5U)
#define BOARD_CAMERA_I2C_CLOCK_SOURCE_SELECT  (0U) /* Select USB1 PLL (480 MHz) as LPI2C's clock source */
#define BOARD_CAMERA_I2C_CLOCK_FREQ \
    (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8 / (BOARD_CAMERA_I2C_CLOCK_SOURCE_DIVIDER + 1U))

#if CAMERA_DIFF_I2C_BUS
#define BOARD_CAMERA_IR_I2C_BASEADDR          LPI2C4
#define BOARD_CAMERA_IR_I2C_INSTANCE          (4U)
#else
#define BOARD_CAMERA_IR_I2C_BASEADDR          BOARD_CAMERA_I2C_BASEADDR
#define BOARD_CAMERA_IR_I2C_INSTANCE          BOARD_CAMERA_I2C_INSTANCE
#endif

/* @Brief Board PCAL configuration */
#define BOARD_PCAL_I2C_BASEADDR             LPI2C1
#define BOARD_PCAL_I2C_CLOCK_SOURCE_DIVIDER (5U)
#define BOARD_PCAL_I2C_CLOCK_SOURCE_SELECT  (0U) /* Select USB1 PLL (480 MHz) as LPI2C's clock source */
#define BOARD_PCAL_I2C_CLOCK_FREQ           (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8 / (BOARD_PCAL_I2C_CLOCK_SOURCE_DIVIDER + 1U))
#define BOARD_PCAL_I2C_INSTANCE             (1U)

/* @Brief Board Bluetooth HCI UART configuration */
#define BOARD_BT_UART_BASEADDR    LPUART1
#define BOARD_BT_UART_CLK_FREQ    BOARD_DebugConsoleSrcFreq()
#define BOARD_BT_UART_IRQ         LPUART1_IRQn
#define BOARD_BT_UART_IRQ_HANDLER LPUART1_IRQHandler

#define BOARD_CAMERA_PWD_GPIO     OUTPUT_PORT0
#define BOARD_CAMERA_PWD_GPIO_PIN 6U

#define BOARD_CAMERA_IR_PWD_GPIO     OUTPUT_PORT0
#define BOARD_CAMERA_IR_PWD_GPIO_PIN 5U

#define BOARD_CAMERA_SWITCH_GPIO     OUTPUT_PORT0
#define BOARD_CAMERA_SWITCH_GPIO_PIN 7U

#define BOARD_CAMERA_LED_SWITCH_GPIO     OUTPUT_PORT0
#define BOARD_CAMERA_LED_SWITCH_GPIO_PIN 2U

/* @Brief Board TFA9894 amplifier SAI configuration */
#define BOARD_AUDIO_CODEC_TFA9894           1
#define BOARD_AMP_SAI SAI1
/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define BOARD_AMP_SAI_CLOCK_SOURCE_SELECT (2U)
/* Clock pre divider for sai1 clock source */
#define BOARD_AMP_SAI_CLOCK_SOURCE_PRE_DIVIDER (0U)
/* Clock divider for sai1 clock source */
#define BOARD_AMP_SAI_CLOCK_SOURCE_DIVIDER (63U)
/* Get frequency of sai1 clock */
#define BOARD_AMP_SAI_CLK_FREQ                                                       \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (BOARD_AMP_SAI_CLOCK_SOURCE_DIVIDER + 1U) / \
     (BOARD_AMP_SAI_CLOCK_SOURCE_PRE_DIVIDER + 1U))

#define BOARD_AMP_SAI_IRQ         SAI1_IRQn
#define BOARD_AMP_SAI_IRQ_Handler SAI1_IRQHandler

#define BOARD_AMP_SAI_EDMA_TX_CH  2U
#define BOARD_AMP_SAI_EDMA_RX_CH  3U
#define BOARD_AMP_SAI_EDMA_TX_REQ kDmaRequestMuxSai1Tx
#define BOARD_AMP_SAI_EDMA_RX_REQ kDmaRequestMuxSai1Rx
#define BOARD_AMP_SAI_EDMA_TX_IRQ DMA2_DMA18_IRQn
#define BOARD_AMP_SAI_EDMA_RX_IRQ DMA3_DMA19_IRQn

#define NUM_TFA_DEVICE 1
#define TFA_LEFT_AMP   0x34
#define TFA_RIGHT_AMP  0x35
#define DEFAULT_CFG_VALUES \
    {                      \
        0                  \
    }

#define RGB_LED_RED_PIN_GPIO   OUTPUT_PORT2
#define RGB_LED_GREEN_PIN_GPIO OUTPUT_PORT2
#define RGB_LED_BLUE_PIN_GPIO  OUTPUT_PORT2

#define RGB_LED_RED_PIN   3
#define RGB_LED_GREEN_PIN 4
#define RGB_LED_BLUE_PIN  5

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
uint32_t BOARD_DebugConsoleSrcFreq(void);
void BOARD_DeInitDebugConsole(void);

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
status_t BOARD_Camera_IR_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);
status_t BOARD_Camera_IR_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);
void BOARD_PCAL_I2C_Init(void);
status_t BOARD_PCAL_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);
status_t BOARD_PCAL_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);
#endif /* SDK_I2C_BASED_COMPONENT_USED */

#if defined(SDK_SAI_BASED_COMPONENT_USED) && SDK_SAI_BASED_COMPONENT_USED
#include "fsl_sai.h"
#include "fsl_sai_edma.h"
#include "fsl_dmamux.h"
typedef struct
{
    edma_handle_t *amp_dma_tx_handle;
    edma_handle_t *amp_dma_rx_handle;
    sai_edma_handle_t *amp_sai_tx_handle;
    sai_edma_handle_t *amp_sai_rx_handle;
    sai_edma_callback_t sai_tx_callback;
    sai_edma_callback_t sai_rx_callback;
} sai_init_handle_t;

void BOARD_SAI_Init(sai_init_handle_t saiInitHandle);
void BOARD_SAI_Deinit(void);
void BOARD_Get_Calibration_State(uint8_t *cur_state);
void BOARD_Set_Calibration_State(uint8_t new_state);
void *BOARD_GetBoardCodecConfig(void);
#endif
void BOARD_MMC_Pin_Config(uint32_t speed, uint32_t strength);
void BOARD_RelocateVectorTableToRam(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif
//#endif /* _BOARD_RTVISION_H_ */
