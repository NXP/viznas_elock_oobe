/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#ifndef _FLASHDB_H_
#define _FLASHDB_H_

#include "commondef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 *******************************************************************************/
uint32_t Flash_FacerecFsInit(int itemHeaderSize, int itemSize, int itemSpaceSize);

uint32_t Flash_FacerecFsReadItemHeader(int index, void *pItemHeader);
uint32_t Flash_FacerecFsWriteItemHeader(int index, void *pItemHeader);
uint32_t Flash_FacerecFsReadItem(int index, void *pItem);
uint32_t Flash_FacerecFsWriteItem(int index, void *pItem);
uint32_t Flash_FacerecFsReadSector(int index, uint8_t *pBuff);
uint32_t Flash_FacerecFsWriteSector(int index, uint8_t *pBuff);
uint32_t Flash_FacerecFsEraseSector(int index);

uint32_t Flash_FacerecFsGetFeatureAddress(int index, void** featurePointer);

#ifdef __cplusplus
}
#endif

#endif /* _FLASHDB_H_ */
