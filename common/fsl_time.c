/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "fsl_pit.h"

static int TimeInited = 0;

/*! @brief Time_Init.
 *
 * Init the PIT timer 1.
 */
void Time_Init(uint32_t unit_us)
{
    if (!TimeInited)
    {
        pit_config_t pitConfig;
        pit_chnl_t pit_c0, pit_c1;
        uint8_t index = 1;
        assert(index == 0 || index == 1);
        assert(unit_us > 0 && unit_us < 10000UL);
        /* Set PERCLK_CLK source to OSC_CLK*/
        CLOCK_SetMux(kCLOCK_PerclkMux, 1U);
        /* Set PERCLK_CLK divider to 1 */
        CLOCK_SetDiv(kCLOCK_PerclkDiv, 0U);
        /*
         * pitConfig.enableRunInDebug = false;
         */
        PIT_GetDefaultConfig(&pitConfig);
        /* Init pit module */
        PIT_Init(PIT, &pitConfig);
        if (index == 0)
        {
            pit_c0 = kPIT_Chnl_0;
            pit_c1 = kPIT_Chnl_1;
        }
        else
        {
            pit_c0 = kPIT_Chnl_2;
            pit_c1 = kPIT_Chnl_3;
        }
        PIT_SetTimerPeriod(PIT, pit_c0, USEC_TO_COUNT(unit_us, CLOCK_GetFreq(kCLOCK_OscClk)));
        PIT_SetTimerChainMode(PIT, pit_c1, 1);
        PIT_SetTimerPeriod(PIT, pit_c1, 0xFFFFFFFF);
        PIT_StartTimer(PIT, pit_c1);
        PIT_StartTimer(PIT, pit_c0);
        TimeInited = 1;
    }
}

/*! @brief TIME_Now.
 *
 * Get the PIT timer 1.
 */
uint32_t Time_Now()
{
    uint32_t Count = 0xFFFFFFFF;
    if (TimeInited)
    {
        Count = PIT_GetCurrentTimerCount(PIT, kPIT_Chnl_3);
    }
    return Count;
}

uint32_t Time_Current()
{
    return (0xFFFFFFFF - Time_Now());
}
