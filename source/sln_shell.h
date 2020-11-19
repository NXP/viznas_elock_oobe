/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef _SLN_SHELL_H_
#define _SLN_SHELL_H_

#include "fsl_shell.h"
#include "sln_dev_cfg.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define USB_SHELL_PROMPT       "SHELL>> "
#define USB_SHELL_PROMPT_DEBUG "SHELL_DEBUG>> "

#define USB_SHELL_PROMPT_INDEX       0
#define USB_SHELL_PROMPT_DEBUG_INDEX 0

#define NonCachedNonInit __attribute__((section("NonCacheable, \"aw\", %nobits @")))
#define NonCached        __attribute__((section("NonCacheable.init")))

/* The maximum number of shell parameters processed by Shell Command Processing Task */
#define USB_SHELL_PARAMS_COUNT 4
/* The maximum shell parameters length processed by Shell Command Processing Task */
#define USB_SHELL_PARAMS_SIZE 24
/* The Queue Length used by the Shell Command Processing Task */
#define USB_SHELL_CMDQUEUE_SIZE 10

/* Filter by verbosity level and print the log messages to the USB shell debug */
#define UsbShell_DbgPrintf(dbgLvl, formatString, ...)                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        uint8_t verbLvl = Cfg_AppDataGetVerbosity();                                                                   \
        if (dbgLvl != 0 && dbgLvl <= verbLvl)                                                                          \
        {                                                                                                              \
            uint32_t t = Time_Current();                                                                               \
            SHELL_Printf(usb_shellHandle[USB_SHELL_PROMPT_DEBUG_INDEX], "[%7d.%3d][L%d]", t / 1000, t % 1000, dbgLvl); \
            SHELL_Printf(usb_shellHandle[USB_SHELL_PROMPT_DEBUG_INDEX], formatString, ##__VA_ARGS__);                  \
        }                                                                                                              \
    } while (0)

/* Print the log message to the USB shell debug using the verbosity level 1 */
#define UsbShell_Printf(formatString, ...) UsbShell_DbgPrintf(VERBOSE_MODE_L1, formatString, ##__VA_ARGS__)

/*******************************************************************************
 * GLobal  Declarations
 ******************************************************************************/
/* The Queue Message processed by Shell Command Processing Task */
typedef struct UsbShellCmdQueue
{
    char shellCommand;                                        /* The shell command to be processed */
    shell_handle_t shellContextHandle;                        /* The current shell handle */
    char argv[USB_SHELL_PARAMS_COUNT][USB_SHELL_PARAMS_SIZE]; /* The shell command parameters */
    int argc;                                                 /* The shell command number of paramters */
} UsbShellCmdQueue_t;

extern shell_handle_t usb_shellHandle[];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void uart_shell_init(void);
void usb_shell_init(void);
char *getAppBank(void);

#ifdef __cplusplus
}
#endif

#endif /*_SLN_SHELL_H_*/
