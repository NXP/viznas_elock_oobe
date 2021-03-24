/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#ifndef FATFS_OP_H_
#define FATFS_OP_H_
#include "fsl_debug_console.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#define FATFS_PRINTF(args) \
    do                     \
    {                      \
        PRINTF args;       \
    } while (0)

int fatfs_mount_with_mkfs(void);
int fatfs_unmount(void);
int fatfs_write(const char *file_name, const char *buf, int offset, int bytes);
int fatfs_write_append(const char *file_name, const char *buf, int bytes);

int fatfs_read(char *file_name, char *buf, int offset, int bytes);

int fatfs_opendir(const char *dir_name);
int fatfs_readdir(char *file_name);
int fatfs_closedir(void);

int fatfs_delete(const char *file_name);
int fatfs_getsize(char *file_name);

int fatfs_rename(const char *old_name, const char *new_name);
int fatfs_mkdir(const char *dir_name);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* FATFS_OP_H_ */
