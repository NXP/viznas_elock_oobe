/*
 * The Clear BSD License
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SLN_PCAL_H_
#define _SLN_PCAL_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))
#define LPI2C_BAUDRATE 100000U
#define LPI2C1_BUS LPI2C1
#define LPI2C_MASTER_CLOCK_FREQUENCY LPI2C_CLOCK_FREQUENCY

//IO Expander chip slave address
#define PCAL_6524_SLAVE_ADDR_7BIT 	0x22 //as per ckt diagram
#define PCAL_6524_WRITE_ADDR		0x44 //as per ckt diagram
#define PCAL_6524_READ_ADDR			0x45 //as per ckt diagram
#define INPUT_PORT0	 	0x00
#define INPUT_PORT1  	0x01
#define INPUT_PORT2  	0x02
// 03 is reserved
#define OUTPUT_PORT0	0x04
#define OUTPUT_PORT1	0x05
#define OUTPUT_PORT2	0x06
// 07 is reserved
#define POLARITY_INVERSION_PORT0 0x08
#define POLARITY_INVERSION_PORT1 0x09
#define POLARITY_INVERSION_PORT2 0x0A
// 0B is reserved
#define CONFIG_PORT0	0x0C
#define CONFIG_PORT1	0x0D
#define CONFIG_PORT2	0x0E
// 0Fh to 3Fh reserved
#define OUTPUT_DRIVE_STRENGTH_REGISTER_PORT_0A 0x40
#define OUTPUT_DRIVE_STRENGTH_REGISTER_PORT_0B 0x41
#define OUTPUT_DRIVE_STRENGTH_REGISTER_PORT_1A 0x42
#define OUTPUT_DRIVE_STRENGTH_REGISTER_PORT_1B 0x43
#define OUTPUT_DRIVE_STRENGTH_REGISTER_PORT_2A 0x44
#define OUTPUT_DRIVE_STRENGTH_REGISTER_PORT_2B 0x45
// 46,47 are reserved
#define INPUT_LATCH_REGISTER_PORT0 0x48
#define INPUT_LATCH_REGISTER_PORT1 0x49
#define INPUT_LATCH_REGISTER_PORT2 0x4A
//4B is reserved
#define PULL_UP_DOWN_EN_REG_PORT0 0x4C
#define PULL_UP_DOWN_EN_REG_PORT1 0x4D
#define PULL_UP_DOWN_EN_REG_PORT2 0x4E
//4Fh is reserved
#define PULL_UP_DOWN_SELECTION_REG_PORT0 0x50
#define PULL_UP_DOWN_SELECTION_REG_PORT1 0x51
#define PULL_UP_DOWN_SELECTION_REG_PORT2 0x52
//53h reserved
#define INT_MASK_REG_PORT0 0x54
#define INT_MASK_REG_PORT1 0x55
#define INT_MASK_REG_PORT2 0x56
//57h reserved
#define INT_STATUS_REG_PORT0 0x58
#define INT_STATUS_REG_PORT1 0x59
#define INT_STATUS_REG_PORT2 0x5A
//5Bh reserved
#define OUTPUT_PORT_CONFIG_REG_RW 0x5C
//5Dh, 5Eh,5Fh reserved
#define INT_EDGE_REG_PORT0A 0x60
#define INT_EDGE_REG_PORT0B 0x61
#define INT_EDGE_REG_PORT1A 0x62
#define INT_EDGE_REG_PORT1B 0x63
#define INT_EDGE_REG_PORT2A 0x64
#define INT_EDGE_REG_PORT2B 0x65
//66h, 67hreserved
#define INT_CLEAR_REG_PORT0 0x68
#define INT_CLEAR_REG_PORT1 0x69
#define INT_CLEAR_REG_PORT2 0x6A
//6Bh reserved
#define INT_STATUS_PORT0 0x6C
#define INT_STATUS_PORT1 0x6D
#define INT_STATUS_PORT2 0x6E
//6Fh reserved
#define INDIVIDUALPIN_OUTPUT_PORT0 0x70
#define INDIVIDUALPIN_OUTPUT_PORT1 0x71
#define INDIVIDUALPIN_OUTPUT_PORT2 0x72
//73h reserved
#define SW_DEBOUNCE_EN0_RW 0x74
#define SW_DEBOUNCE_EN1_RW 0x75
#define SW_DEBOUNCE_EN2_RW 0x76
//77h to ffh reserved
#define OUTPUT_PORT_VALUE 0x00
// RGB LED VALUES
#define RED_LED 0x30
#define GREEN_LED 0x28
#define BLUE_LED 0x18

// PCAL total number of GPIO pins

#define PCAL_PINS 48

#define PCAL_PIN_DIR_OUTPUT 0
#define PCAL_PIN_DIR_INPUT 1

#define PCAL_PIN_HIGH 1
#define PCAL_PIN_LOW 0
#define PCAL_PIN_ERROR -1
#define PCAL_PIN_INVALID -2

#define IOX_RST_PIN_GPIO GPIO2
#define IOX_PST_PIN 14

#define IOX_INT_PIN_GPIO GPIO2
#define IOX_INT_PIN 13

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * ENUMS
 ******************************************************************************/
enum i2cstatus {
	I2C_FAIL = -1,
	I2C_PASS = 0,
};

/*******************************************************************************
 * Prototype
 ******************************************************************************/
void initLPI2C1();

void IOX_I2CTest( void );
void read_PCAL6524_device_id( void );
void set_iox_pin_config( void );
void set_iox_port_pin(uint8_t port, uint8_t pin, uint8_t set);
int get_iox_port_pin(uint8_t port, uint8_t pin);
void set_iox_pin_dir(int pin, int direction);
void set_iox_pin(int pin, int set);
int get_iox_pin(int pin);

void BOARD_InitPCALResource(void);
void IOX_gpio_init(void);
status_t set_iox_reg(uint8_t reg, uint8_t val);
status_t get_iox_reg(uint8_t reg, uint8_t * pVal);
#if defined(__cplusplus)
}
#endif
#endif /* _SLN_PCAL_H_ */
