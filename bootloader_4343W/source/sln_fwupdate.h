/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef SLN_FWUPDATE_H_
#define SLN_FWUPDATE_H_

#define METHOD_OTA 0
#define METHOD_OTW 1

/*!
 * @brief TBD
 */
#define selfTestCleanup() \
    {                     \
    }

/*!
 * @brief Starts the firmaware update over UART
 */

void otwAppStart(void);

/*!
 * @brief Starts the firmaware update over TCP
 */

void otaAppStart(void);

#endif /* SLN_FWUPDATE_H_ */
