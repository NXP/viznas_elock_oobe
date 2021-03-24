/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#define SDK_I2C_BASED_COMPONENT_USED 1
#define SDK_I2C_FREERTOS 1
#define USE_RTOS_SPI 1
#define DISP_USE_SPI_DMA 1
#define WDOG_ENABLE 1


#ifndef APP_VERSION_NUMBER
#define APP_MAJ_VER 2
#define APP_MIN_VER 0
#define APP_BLD_VER 128  //month,day
#define APP_VERSION_NUMBER (unsigned int)(((APP_MAJ_VER & 0xFFU ) << 24U) | ((APP_MIN_VER & 0xFFU) << 16U) | (APP_BLD_VER & 0xFFFFU ))
#endif

#if RTVISION_BOARD
    #define WIFI_ENABLE      0
    #define VOICE_PROMPT     0
	#define SDK_SAI_BASED_COMPONENT_USED 1
	#include "board_rtvision.h"
#elif RT106F_ELOCK_BOARD
    #define WIFI_ENABLE      0
    #define VOICE_PROMPT     0
    #define SDK_SAI_BASED_COMPONENT_USED 0
	#include "board_rt106f_elock.h"

#else
	#error "***ERROR***"
#endif


#endif /* _BOARD_H_ */
