/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include <stdint.h>

#include "sln_system_state.h"
#include "FreeRTOS.h"
#include "fsl_pit.h"
#include "sln_RT10xx_RGB_LED_driver.h"
#include "FreeRTOSConfig.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define PIT_SYSSTATE_IRQ_ID     PIT_IRQn
#define PIT_SYSSTATE_CHANNEL    kPIT_Chnl_0
#define PIT_SYSSTATE_CLOCK_FREQ CLOCK_GetFreq(kCLOCK_OscClk)
#if configMAX_SYSCALL_INTERRUPT_PRIORITY > 0
#define PIT_SYSSTATE_IRQ_PRIORITY (configMAX_SYSCALL_INTERRUPT_PRIORITY - 1)
#else
#error "configMAX_SYSCALL_INTERRUPT_PRIORITY must be greater than 0"
#endif

#define RBG_PATTERN_MAX_COUNT 8U

struct
{
    system_states_t current_state;
    bool led_toggle_en        = 0;
    uint16_t led_toggle_count = 0;
    rgbLedColor_t led_colors[RBG_PATTERN_MAX_COUNT];
    uint8_t led_index;
    uint8_t led_count;
} sysstate_settings;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Start Implementation
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif
void PIT_IRQHandler(void)
{
    if (0x1 == PIT_GetStatusFlags(PIT, PIT_SYSSTATE_CHANNEL))
    {
        /* Clear interrupt flag.*/
        PIT_ClearStatusFlags(PIT, PIT_SYSSTATE_CHANNEL, kPIT_TimerFlag);

        if (sysstate_settings.led_index >= sysstate_settings.led_count)
        {
            sysstate_settings.led_index = 0;
            if (1 == sysstate_settings.led_toggle_en)
            {
                sysstate_settings.led_toggle_count--;
            }
        }

        if ((1 == sysstate_settings.led_toggle_en) && (0 == sysstate_settings.led_toggle_count))
        {
            PIT_StopTimer(PIT, PIT_SYSSTATE_CHANNEL);
            RGB_LED_SetColor(LED_COLOR_OFF);
        }
        else
        {
            RGB_LED_SetColor(sysstate_settings.led_colors[sysstate_settings.led_index]);
        }

        sysstate_settings.led_index++;
    }
}
#if defined(__cplusplus)
}
#endif

void SysState_Init()
{
    memset(&sysstate_settings, 0, sizeof(sysstate_settings));
    RGB_LED_SetColor(LED_COLOR_OFF);

    /* Structure of initialize PIT */
    pit_config_t pitConfig;

    /* Enable clock gate for GPIO1 */
    CLOCK_EnableClock(kCLOCK_Gpio1);
    /* Set PERCLK_CLK source to OSC_CLK*/
    CLOCK_SetMux(kCLOCK_PerclkMux, 1U);
    /* Set PERCLK_CLK divider to 1 */
    CLOCK_SetDiv(kCLOCK_PerclkDiv, 0U);

    PIT_GetDefaultConfig(&pitConfig);

    /* Init pit module */
    PIT_Init(PIT, &pitConfig);

    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(PIT, PIT_SYSSTATE_CHANNEL, USEC_TO_COUNT(1000000U, PIT_SYSSTATE_CLOCK_FREQ));

    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(PIT, PIT_SYSSTATE_CHANNEL, kPIT_TimerInterruptEnable);

    /* Enable at the NVIC */
    NVIC_SetPriority(PIT_SYSSTATE_IRQ_ID, PIT_SYSSTATE_IRQ_PRIORITY);
    NVIC_EnableIRQ(PIT_SYSSTATE_IRQ_ID);
}

void SysState_DeInit()
{
    RGB_LED_SetColor(LED_COLOR_OFF);
    NVIC_DisableIRQ(PIT_SYSSTATE_IRQ_ID);
    PIT_StopTimer(PIT, PIT_SYSSTATE_CHANNEL);
    PIT_DisableInterrupts(PIT, PIT_SYSSTATE_CHANNEL, kPIT_TimerInterruptEnable);
    PIT_Deinit(PIT);
}

void SysState_Set(system_states_t state)
{
    if (sysstate_settings.current_state == state)
    {
        return;
    }

    sysstate_settings.current_state = state;
    if (sysStateIdle == state)
    {
        // Play toggling blue/off led every 100 milliseconds
        PIT_StopTimer(PIT, PIT_SYSSTATE_CHANNEL);
        sysstate_settings.led_index        = 0;
        sysstate_settings.led_count        = 2;
        sysstate_settings.led_toggle_count = 0;
        sysstate_settings.led_toggle_en    = 0;

        sysstate_settings.led_colors[0] = LED_COLOR_BLUE;
        sysstate_settings.led_colors[1] = LED_COLOR_OFF;

        PIT_SetTimerPeriod(PIT, PIT_SYSSTATE_CHANNEL, USEC_TO_COUNT(100000U, PIT_SYSSTATE_CLOCK_FREQ));
        PIT_StartTimer(PIT, PIT_SYSSTATE_CHANNEL);
    }
    else if (sysStateDetectedNoUser == state)
    {
        PIT_StopTimer(PIT, PIT_SYSSTATE_CHANNEL);
        RGB_LED_SetColor(LED_COLOR_OFF);
    }
    else if (sysStateDetectedUnknownUser == state)
    {
        PIT_StopTimer(PIT, PIT_SYSSTATE_CHANNEL);
        RGB_LED_SetColor(LED_COLOR_RED);
    }
    else if (sysStateDetectedKnownUser == state)
    {
        PIT_StopTimer(PIT, PIT_SYSSTATE_CHANNEL);
        RGB_LED_SetColor(LED_COLOR_GREEN);
    }
}
