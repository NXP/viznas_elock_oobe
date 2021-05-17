/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "sln_wdog.h"
#include "fsl_wdog.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SLN_WDOG_BASE       WDOG1
#define SLN_WDOG_IRQ        WDOG1_IRQn
#define SLN_WDOG_IRQHandler WDOG1_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void SLN_WDOG_IRQHandler(void)
{
    WDOG_Refresh(SLN_WDOG_BASE);
    WDOG_ClearInterruptStatus(SLN_WDOG_BASE, kWDOG_InterruptFlag);
}

wdog_status_t SLN_WatchdogEnable(uint8_t timeoutSec, uint8_t refreshSec)
{
    wdog_config_t config;
    volatile uint16_t wt;
    volatile uint16_t wict;

    if (!(timeoutSec >= SLN_WDOG_TIMEOUT_MIN_SEC && timeoutSec <= SLN_WDOG_TIMEOUT_MAX_SEC))
    {
        return kStatus_WDOG_WrongTo;
    }

    if (refreshSec >= timeoutSec)
    {
        return kStatus_WDOG_WrongRefresh;
    }

    /*! WDOG_WCR - WT
     *  0b00000000..- 0.5 Seconds
     *  0b00000001..- 1.0 Seconds.
     *  0b00000010..- 1.5 Seconds.
     *  0b00000011..- 2.0 Seconds.
     *  0b11111111..- 128 Seconds.
     */
    wt = timeoutSec * 2 - 1;

    /*! WDOG_WICR - WICT
     *  0b00000000..WICT[7:0] = Time duration between interrupt and time-out is 0 seconds.
     *  0b00000001..WICT[7:0] = Time duration between interrupt and time-out is 0.5 seconds.
     *  0b00000100..WICT[7:0] = Time duration between interrupt and time-out is 2 seconds (Default).
     *  0b11111111..WICT[7:0] = Time duration between interrupt and time-out is 127.5 seconds.
     */
    wict = (timeoutSec - refreshSec) * 2;

    /*
     * wdogConfig->enableWdog = true;
     * wdogConfig->workMode.enableWait = false;
     * wdogConfig->workMode.enableStop = false;
     * wdogConfig->workMode.enableDebug = false;
     * wdogConfig->enableInterrupt = false;
     * wdogConfig->enablePowerdown = false;
     * wdogConfig->resetExtension = flase;
     * wdogConfig->timeoutValue = 0xFFU;
     * wdogConfig->interruptTimeValue = 0x04u;
     */
    WDOG_GetDefaultConfig(&config);

    config.workMode.enableWait  = true;
    config.workMode.enableStop  = true;
    config.workMode.enableDebug = true;
    config.enableInterrupt = true;
    config.timeoutValue = wt;
    config.interruptTimeValue = wict;
    WDOG_Init(SLN_WDOG_BASE, &config);

    return kStatus_WDOG_Enabled;
}
