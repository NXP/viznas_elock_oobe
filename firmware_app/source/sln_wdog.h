/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef _SLN_WDOG_H_
#define _SLN_WDOG_H_

#include <stdint.h>

#define SLN_WDOG_TIMEOUT_SEC (10U)
#define SLN_WDOG_REFRESH_SEC (8U)

#define SLN_WDOG_TIMEOUT_MIN_SEC (1U)
#define SLN_WDOG_TIMEOUT_MAX_SEC (128U)

/*! @brief WDOG error code */
typedef enum _wdog_status_t
{
    kStatus_WDOG_Enabled = 0x00U, /*< Watchdog Timer succesfully started */
    kStatus_WDOG_WrongTo,         /*< Invalid WDOG timeout */
    kStatus_WDOG_WrongRefresh     /*< Invalid WDOG refresh rate */
} wdog_status_t;

/**
 * @brief Start the WatchdogTimer in order to prevent the system hang
 *
 * It prevents the system hang by executing a System Reset
 * if the WDOG counter is not refreshed befor the time-out is reached.
 *
 * @param timeoutSec    The WDOG timeout in seconds
 * @param refreshSec    The WDOG refresh rate in seconds (must be smaller than the timeout)
 *
 * @return              Status of WDOG
 */
wdog_status_t SLN_WatchdogEnable(uint8_t timeoutSec, uint8_t refreshSec);

#endif /*_SLN_WDOG_H_*/
