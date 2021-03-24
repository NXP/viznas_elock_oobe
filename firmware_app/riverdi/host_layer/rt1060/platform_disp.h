/*
 * Copyright (c) NXP 2019
 * Copyright (c) Riverdi Sp. z o.o. sp. k. <riverdi@riverdi.com>
 * Copyright (c) Skalski Embedded Technologies <contact@lukasz-skalski.com>
 */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#if RTVISION_BOARD

/* C library inclusions */
#include <stdarg.h>
#include <string.h>

/* RT library inclusions */
#include "commondef.h"

/*****************************************************************************/

/* type definitions for EVE HAL library */

#define TRUE		(1)
#define FALSE		(0)

typedef char		bool_t;
typedef char		char8_t;
typedef unsigned char	uchar8_t;
typedef signed char	schar8_t;
typedef float		float_t;

/* Predefined Riverdi modules */
#include "modules.h"

/* EVE inclusions */
#include "Gpu_Hal.h"
#include "Gpu.h"
#include "CoPro_Cmds.h"
#include "Hal_Utils.h"

/*****************************************************************************/

#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0

#define RTVISION_BOARD_COCMD_BURST

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

bool_t platform_init (Gpu_HalInit_t*);
void platform_sleep_ms (uint32_t);

bool_t platform_spi_init (Gpu_Hal_Context_t*, uint32_t maxclk);
void platform_spi_deinit (Gpu_Hal_Context_t*);

uchar8_t platform_spi_send_recv_byte (Gpu_Hal_Context_t*, uchar8_t, uint32_t);
uint16_t platform_spi_send_data (Gpu_Hal_Context_t*, uchar8_t*, uint16_t, uint32_t);
void platform_spi_recv_data (Gpu_Hal_Context_t*, uchar8_t*, uint16_t, uint32_t);

bool_t platform_gpio_init (Gpu_Hal_Context_t*, gpio_name);
bool_t platform_gpio_value (Gpu_Hal_Context_t*, gpio_name, gpio_val);

/*****************************************************************************/
#endif
#endif /*_PLATFORM_H_*/
