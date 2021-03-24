/*
 * The Clear BSD License
 * Copyright 2020 NXP
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

#include "sln_pcal.h"
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Global variables
 ******************************************************************************/
volatile bool g_MasterCompletionFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief I2C1 initialization
 */
void initLPI2C1()
{
    BOARD_PCAL_I2C_Init();
}

/*
 * PCAL 6524 IOXpander
 */
/* macro */
#define BIT_SET(a, b) ((a) |= (1ULL << (b)))
#define BIT_CLEAR(a, b) ((a) &= ~(1ULL << (b)))

void IOX_I2CTest(void)
{
    PRINTF("*************************************.\r\n");
    PRINTF("I2C verification started.\r\n");
    PRINTF("*************************************.\r\n");
    IOX_gpio_init();
    read_PCAL6524_device_id();
    // set config IOX
    set_iox_pin_config();
    // set_iox_pin();
}
/*
 * function name	: set iox config
 * arguments		: pin number and gpio direction
 * description		: set the pin (0-24) as gpio
 * 					  set bits for output and input
 *
 */

status_t set_iox_reg(uint8_t reg, uint8_t val)
{
    status_t sts;
    sts = BOARD_PCAL_I2C_Send(PCAL_6524_SLAVE_ADDR_7BIT, reg, 1u, &val, 1u);
    return sts;
}

status_t get_iox_reg(uint8_t reg, uint8_t *pVal)
{
    status_t sts;
    sts = BOARD_PCAL_I2C_Receive(PCAL_6524_SLAVE_ADDR_7BIT, reg, 1u, pVal, 1u);
    return sts;
}

void set_iox_pin_config(void)
{
    status_t sts;
    /* Config Port 0:
   CAM_RST			=> CAM_SEL (CSI signal switch) (MSB)
   CAM_IR_CUT_R	=> PWDN_RGB [VF]
   CAM_IR_CUT_F	=> PWDN_IR [VF]
   TFA9984_INT
   TFA9984_RST
   TSL25721_INT	=> IR_WHITEn_SEL (output)
   NPS3000_INT2
   NPS3000_INT1		(LSB)
   */

    sts = set_iox_reg(OUTPUT_PORT0, 0x00);
    sts |= set_iox_reg(CONFIG_PORT0, 0x13);
    sts |= set_iox_reg(PULL_UP_DOWN_EN_REG_PORT0, 0x04);        // Pull_up/Down enable
    sts |= set_iox_reg(PULL_UP_DOWN_SELECTION_REG_PORT0, 0x04); // 100k Pull up (1) / 100k Pull down (0) selection
    sts |= set_iox_reg(INT_MASK_REG_PORT0, 0xFF);               // Interrupt mask: 0 = enabled. To be set later
    //	sts |= set_iox_reg(INT_EDGE_REG_PORT0A, );						// Interrupt edge for P1.3 to P1.0 (2 bits, 01 = rising edge)
    //	sts |= set_iox_reg(INT_EDGE_REG_PORT0B, );						// Interrupt edge for P1.7 to P1.4
    if (sts == I2C_PASS)
    {
        PRINTF("IOX port config done\r\n");
    }
    else
    {
        PRINTF("IOX port config fail\r\n");
        return;
    }

    /* Config Port 1:
     TP48		(MSB)
     TP47
     DISP_DISP_PWDN
     X
     BAT_CHG_ENN
     BATT_FASTCHARGE
     BATT_PPRN
     BAT_CHGN			(LSB)
     */
    sts = set_iox_reg(OUTPUT_PORT1, 0x08); // Disable Battery charger
    //  sts = set_iox_reg(OUTPUT_PORT1, 0x00);                          // Output value. No effect on pins configured as
    //  input
    sts |= set_iox_reg(CONFIG_PORT1, 0xC3); // DISP_BLIGHT not used anymore => set as input
    //  sts |= set_iox_reg(CONFIG_PORT1, 0x03);                         // Input(1) / Output (0) mode
    sts |= set_iox_reg(PULL_UP_DOWN_EN_REG_PORT1, 0x13);        // Pull_up/Down enable
    sts |= set_iox_reg(PULL_UP_DOWN_SELECTION_REG_PORT1, 0x03); // 100k Pull up (1) / 100k Pull down (0) selection
    sts |= set_iox_reg(INT_MASK_REG_PORT1, 0xFF);               // Interrupt mask: 0 = enabled
    //	set_iox_reg(INT_EDGE_REG_PORT1A, 0xFF);							// Interrupt edge for P1.3 to P1.0 (2 bits, 01 = rising edge)
    //	set_iox_reg(INT_EDGE_REG_PORT1B, 0xFF);							// Interrupt edge for P1.7 to P1.4
    if (sts == I2C_PASS)
    {
        PRINTF("IOX port config done\r\n");
    }
    else
    {
        PRINTF("IOX port config fail\r\n");
        return;
    }

    /* Config Port 2:
     FXOS8700_RST		(MSB)
     KEYPAD_SLEEP
     RGBLED_B
     REGLED_G
     REGLED_R
     GPIO_SW4
     GPIO_SW3
     GPIO_SW2			(LSB)
     */
    //	set_iox_reg(OUTPUT_PORT2, 0x18);						// Enable Blue LED
    sts = set_iox_reg(OUTPUT_PORT2, 0x38);                      // Output value. No effect on pins configured as input
    sts |= set_iox_reg(CONFIG_PORT2, 0x07);                     // Input(1) / Output (0) mode
    sts |= set_iox_reg(PULL_UP_DOWN_EN_REG_PORT2, 0x87);        // Pull_up/Down enable
    sts |= set_iox_reg(PULL_UP_DOWN_SELECTION_REG_PORT2, 0x78); // 100k Pull up (1) / 100k Pull down (0) selection
#if BOOTLOADER_OTW_ENABLE
    sts |= set_iox_reg(INT_MASK_REG_PORT2, 0xFF); // Interrupt mask: 0 = enabled
#else
    sts |= set_iox_reg(INT_MASK_REG_PORT2, 0xF8); // Interrupt mask: 0 = enabled
#endif
    sts |= set_iox_reg(INT_EDGE_REG_PORT2A, 0x15); // Interrupt edge for P2.3 to P2.0 (2 bits, 01 = rising edge)
    //	sts |= set_iox_reg(INT_EDGE_REG_PORT2B, 0xFF);					// Interrupt edge for P2.7 to P2.4
    if (sts == I2C_PASS)
    {
        PRINTF("IOX port config done\r\n");
    }
    else
    {
        PRINTF("IOX port config fail\r\n");
        return;
    }
}

void set_iox_port_pin(uint8_t port, uint8_t pin, uint8_t set)
{
    status_t sts = kStatus_Fail;
    uint8_t data_read;

    sts = get_iox_reg(port, &data_read);

    if (sts == I2C_PASS)
    {
        // PRINTF("IOX pin config done\r\n");
        // PRINTF("############PCAL6524 data read : %x\n\r", data_read);
    }
    else
    {
        PRINTF("IOX pin config fail\r\n");
        return;
    }

    if (set)
    {
        data_read = data_read | ((uint8_t)1 << pin);
    }
    else
    {
        data_read = data_read & ~((uint8_t)1 << pin);
    }

    // PRINTF("############PCAL6524 data byte with bit : %x\n\r", data_read);

    sts = set_iox_reg(port, data_read);

    if (sts == I2C_PASS)
    {
        // PRINTF("IOX pin config done\r\n");
    }
    else
    {
        PRINTF("IOX pin config fail\r\n");
        return;
    }
}

int get_iox_port_pin(uint8_t port, uint8_t pin)
{
    status_t sts;
    uint8_t data_read;

    sts = get_iox_reg(port, &data_read);

    if (sts == I2C_PASS)
    {
        // PRINTF("############PCAL6524 pin get data read : %x\n\r", data_read);
    }
    else
    {
        PRINTF("IOX pin get fail\r\n");
        return PCAL_PIN_ERROR;
    }

    return (data_read >> pin) & 1 ? PCAL_PIN_HIGH : PCAL_PIN_LOW;
}

void read_PCAL6524_device_id(void)
{
    status_t sts = kStatus_Fail;
    uint8_t device_id[3];
    uint8_t data[3];
    data[0] = 0xF8; // as per data sheet
    data[1] = 0x22; // write addr as per ckt diagram
    data[2] = 0xF9; // as per data sheet

    sts = BOARD_PCAL_I2C_Send(PCAL_6524_SLAVE_ADDR_7BIT, 0, 0u, data, 3u);
    sts = BOARD_PCAL_I2C_Receive(PCAL_6524_SLAVE_ADDR_7BIT, 0, 0u, device_id, 3u);
    // PRINTF("PCAL6524 device id : %x %x %x\n\r",device_id[0],device_id[1],device_id[2]);
    if (sts == I2C_PASS)
    {
        // PRINTF("IOX device id read done\r\n");
    }
    else
    {
        PRINTF("IOX device id read fail\r\n");
        return;
    }
}

/* Set pin direction, pin numbers are from 0 to 47, where for instance pin 8 is P1.0
 * pin direction is PCAL_PIN_DIR_OUTPUT or PCAL_PIN_DIR_INPUT*/
void set_iox_pin_dir(int pin, int direction)
{
    status_t sts;
    uint8_t port_reg;
    uint8_t port_value;

    // selection of port according to pin
    if ((pin < 0) || (pin >= PCAL_PINS))
    {
        PRINTF("PCAL pin config 2 invalid pin number: %d\r\n", pin);
        return;
    }

    if (!((direction == PCAL_PIN_DIR_OUTPUT) || (direction == PCAL_PIN_DIR_INPUT)))
    {
        PRINTF("PCAL pin config 2 invalid pin dir: %d\r\n", direction);
        return;
    }

    port_reg = CONFIG_PORT0 + (pin >> 3);

    sts = get_iox_reg(port_reg, &port_value);

    if (sts == I2C_PASS)
    {
        // PRINTF("############PCAL6524 config 2 data read : %x\n\r", port_value);
    }
    else
    {
        PRINTF("IOX pin config 2 fail\r\n");
        return;
    }

    if (direction == PCAL_PIN_DIR_INPUT)
    {
        port_value = port_value | ((uint8_t)1 << (pin % 8));
    }
    else
    {
        port_value = port_value & ~((uint8_t)1 << (pin % 8));
    }

    sts = set_iox_reg(port_reg, port_value);

    if (sts == I2C_PASS)
    {
        // PRINTF("IOX port config done\r\n");
    }
    else
    {
        PRINTF("IOX port config fail\r\n");
        return;
    }
}

/* Set an output pin value, pin numbers are from 0 to 47, where for instance pin 8 is P1.0
 * pin value is PCAL_PIN_SET to set a pin to 1 or PCAL_PIN_CLEAR to set a pin to 0*/

void set_iox_pin(int pin, int set)
{
    // Selection of port according to pin
    if ((pin < 0) || (pin >= PCAL_PINS))
    {
        PRINTF("PCAL set_iox_pin invalid pin number: %d\r\n", pin);
        return;
    }

    if (!((set == PCAL_PIN_HIGH) || (set == PCAL_PIN_LOW)))
    {
        PRINTF("PCAL set_iox_pin invalid pin value: %d\r\n", set);
        return;
    }

    set_iox_port_pin(OUTPUT_PORT0 + (pin >> 3), pin % 8, set);
}

/* Get an input pin value, pin numbers are from 0 to 47, where for instance pin 8 is P1.0
 * returned pin value is PCAL_PIN_HIGH if the pin is set/1/up or PCAL_PIN_LOW if the pin is low or PCAL_ERROR if any
 * error */

int get_iox_pin(int pin)
{
    // selection of port according to pin
    if ((pin < 0) || (pin >= PCAL_PINS))
    {
        PRINTF("PCAL get_iox_pin invalid pin number: %d\r\n", pin);
        return PCAL_PIN_INVALID;
    }

    return get_iox_port_pin(OUTPUT_PORT0 + (pin >> 3), pin % 8);
}

void IOX_gpio_init(void)
{
    gpio_pin_config_t IOX_rst_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(IOX_RST_PIN_GPIO, IOX_PST_PIN, &IOX_rst_config); // config the gpio as output
    GPIO_PinWrite(IOX_RST_PIN_GPIO, IOX_PST_PIN, 1U);             // high the rst pin
}
void BOARD_InitPCALResource()
{
    initLPI2C1();
    IOX_gpio_init();
    set_iox_pin_config();
}
