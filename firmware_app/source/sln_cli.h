/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef SLN_CLI_H_
#define SLN_CLI_H_

#include "fsl_shell.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __shell_event
{
    SHELL_EV_HELP = 0,
    SHELl_EV_EXIT,
    SHELL_EV_FFI_CLI_LIST,
    SHELL_EV_FFI_CLI_MODE,
    SHELL_EV_FFI_CLI_ADD,
    SHELL_EV_FFI_CLI_DEL,
    SHELL_EV_FFI_CLI_RENAME,
    SHELL_EV_FFI_CLI_VERBOSE,
    SHELL_EV_FFI_CLI_SAVE,
    SHELL_EV_FFI_CLI_RESET,
    SHELL_EV_FFI_CLI_CAMERA,
    SHELL_EV_FFI_CLI_VERSION,
    SHELL_EV_FFI_CLI_FWUPDATE_OTW,
    SHELL_EV_FFI_CLI_EMOTION_TYPES,
    SHELL_EV_FFI_CLI_LIVENESS,
    SHELL_EV_FFI_CLI_DET_RESOLUTION,
    SHELL_EV_FFI_CLI_DISPLAY,
    SHELL_EV_FFI_CLI_WIFI_ERASE,
    SHELL_EV_FFI_CLI_WIFI_CREDENTIALS,
    SHELL_EV_FFI_CLI_WIFI,
    SHELL_EV_FFI_CLI_APP,
    SHELL_EV_FFI_CLI_LOW_POWER,
    SHELL_EV_FFI_CLI_ALGO_START,
    SHELL_EV_FFI_CLI_LAST_EVENT // All future commands to be placed before this.

} shell_event_t;

shell_status_t RegisterFFICmds(shell_handle_t shellContextHandle);
void UsbShell_CmdProcess_Task(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* SLN_CLI_H_ */
