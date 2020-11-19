/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef _PDM_PCM_DEFINITIONS_H_
#define _PDM_PCM_DEFINITIONS_H_

/*******************************************************************************
 * App Config Definitions
 ******************************************************************************/

/*******************************************************************************
 * PCM Stream Sample Definitions
 ******************************************************************************/
#define PCM_SAMPLE_SIZE_BYTES    (2U)
#define PCM_SINGLE_CH_SMPL_COUNT (160U)
#define PCM_SAMPLE_RATE_HZ       (16000U)
#define PCM_AMP_SAMPLE_RATE_HZ   (48000U)
#define PCM_AMP_SAMPLE_COUNT     (PCM_SINGLE_CH_SMPL_COUNT * (PCM_AMP_SAMPLE_RATE_HZ / PCM_SAMPLE_RATE_HZ))
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* _PDM_PCM_DEFINITIONS_H_ */

/*******************************************************************************
 * API
 ******************************************************************************/
