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
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

/* C library inclusions */
#include <stdarg.h>
#include <string.h>

/* RT library inclusions */
#include "commondef.h"

/*****************************************************************************/

/* type definitions for EVE HAL library */

#define TRUE        (1)
#define FALSE        (0)

typedef char        bool_t;
typedef char        char8_t;
typedef unsigned char    uchar8_t;
typedef signed char    schar8_t;
typedef float        float_t;

/*****************************************************************************/

#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0

#define RIVERDI_TRANSFER_STEP_B 30720   // 30KB

typedef enum {
  GPIO_CS   = 0,
  GPIO_PD   = 1,
  GPIO_INT  = 2,
  GPIO_CLK  = 3,
  GPIO_SDO  = 4
} gpio_name;

typedef enum {
  GPIO_HIGH = GPIO_PIN_SET,
  GPIO_LOW  = GPIO_PIN_RESET
} gpio_val;

/*****************************************************************************/
bool_t Disp_Spi_Init(uint32_t maxclk);
void Disp_Spi_Deinit(void);

uchar8_t Disp_Spi_Send_Recv_Byte(uchar8_t, uint32_t);
uint16_t Disp_Spi_Send_Data(uchar8_t*, uint16_t, uint32_t);
void Disp_Spi_Recv_Data(uchar8_t*, uint16_t, uint32_t);

/*****************************************************************************/
#endif /*_PLATFORM_H_*/
