/*
 * Copyright 2021 NXP
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */
#include "board.h"
#include "sln_RT10xx_RGB_LED_driver.h"
#include "fsl_device_registers.h"


status_t RGB_LED_Init(void)
{
#if BOARD_HAS_PCAL
    BOARD_InitPCALResource();
#endif
    return kStatus_Success;
}

void RGB_LED_SetColor(rgbLedColor_t color)
{
#if BOARD_HAS_PCAL
    switch (color)
    {
        case LED_COLOR_OFF:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 1);
            break;
        case LED_COLOR_RED:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 1);
            break;
        case LED_COLOR_GREEN:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 1);
            break;
        case LED_COLOR_BLUE:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 1);
            break;
        case LED_COLOR_WHITE:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 0);
            break;
        case LED_COLOR_PURPLE:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 1);
            break;
        case LED_COLOR_YELLOW:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 0);
            break;
        case LED_COLOR_ORANGE:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 0);
            break;
        case LED_COLOR_CYAN:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 0);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 0);
            break;
        default:
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_RED_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_GREEN_PIN, 1);
            set_iox_port_pin(OUTPUT_PORT2, RGB_LED_BLUE_PIN, 1);
            break;
    }
#endif
}

void RGB_LED_SetBrightnessColor(rgb_led_brightness_t brightness, rgbLedColor_t color)
{
    RGB_LED_SetColor(color);
}
