/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef _SLN_LOG_H_
#define _SLN_LOG_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "fsl_debug_console.h"
#include "fsl_time.h"

#ifdef NXP_MCU_FACE_SECURE_DEBUG
#ifndef LOG_TAG
#define LOG_TAG ""
#endif

#define LOG_VERBOSE 0
#define LOG_INFO    1
#define LOG_DEBUG   2
#define LOG_ERROR   3

#ifndef LOG_NVDEBUG
#define LOG_NVDEBUG 1
#endif

#ifndef LOG_NIDEBUG
#define LOG_NIDEBUG 1
#endif

#ifndef LOG_NDDEBUG
#define LOG_NDDEBUG 0
#endif

//#define printlog(tag, fmt, args...) printf("[%s]:"fmt, tag, ##args)
//#define printlog(tag, fmt, args...) PRINTF("[%d.%d]"fmt, ##args)

#define printlog(tag, fmt, args...)                  \
    {                                                \
        do                                           \
        {                                            \
            uint32_t t = Time_Current();             \
            PRINTF("[%7d.%3d]", t / 1000, t % 1000); \
            PRINTF(fmt, ##args);                     \
        } while (0);                                 \
    }

#ifndef LOG
#define LOG(priority, tag, ...)                                                                                    \
    ({                                                                                                             \
        if (((priority == LOG_VERBOSE) && (LOG_NVDEBUG == 0)) || ((priority == LOG_INFO) && (LOG_NIDEBUG == 0)) || \
            ((priority == LOG_DEBUG) && (LOG_NDDEBUG == 0)) || (priority == LOG_ERROR))                            \
        {                                                                                                          \
            printlog(tag, __VA_ARGS__);                                                                            \
        }                                                                                                          \
    })
#endif

#ifndef LOGV
#define LOGV(...) LOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#endif

#ifndef LOGD
#define LOGD(...) LOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif

#ifndef LOGI
#define LOGI(...) LOG(LOG_INFO, LOG_TAG, __VA_ARGS__)
#endif

#ifndef LOGE
#define LOGE(...) LOG(LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

#else

#define LOGV(...)
#define LOGD(...)
#define LOGI(...)
#define LOGE(...)

#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /*_SLN_LOG_H_*/
