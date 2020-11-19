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
uint32_t Flash_FacerecFsEraseAllBlock(void);
uint32_t Flash_FacerecFsEraseMapBlock(void);
uint32_t Flash_FacerecFsEraseItemBlock(void);

uint32_t Flash_FacerecFsUpdateMapMagic(int index, FeatureMap *pMap);
uint32_t Flash_FacerecFsUpdateItemMagic(int index, uint8_t flag);
uint32_t Flash_FacerecFsUpdateItem(int index, FeatureItem *pItem);

uint32_t Flash_FacerecFsReadMapMagic(FeatureMap *pMap);
uint32_t Flash_FacerecFsReadItem(int index, FeatureItem *pItem);

#ifdef __cplusplus
}
#endif

#endif /* _FLASHDB_H_ */
