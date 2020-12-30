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

#include "fsl_disp_spi.h"
#include "fsl_stdisp.h"

#define BOARD_LCD_PWR_GPIO GPIO2   /*! LCD power port */
#define BOARD_LCD_PWR_GPIO_PIN 16U /*! LCD power pin */

#define BOARD_LCD_RST_GPIO GPIO2     /*! LCD reset port */
#define BOARD_LCD_RST_GPIO_PIN 7U    /*! LCD reset pin */

#define BOARD_LCD_DC_GPIO GPIO2   /*! LCD data/command port */
#define BOARD_LCD_DC_GPIO_PIN 6U /*! LCD data/command pin */

typedef void (*st7789_send_byte_t)(uint8_t);
typedef void (*st7789_delay_ms_t)(uint32_t);

static void ST7789_Write_Cmd(uint8_t Data)
{
    //GPIO_PortClear(BOARD_LCD_DC_GPIO, 1u << BOARD_LCD_DC_GPIO_PIN);
    GPIO_PinWrite(BOARD_LCD_DC_GPIO,BOARD_LCD_DC_GPIO_PIN,0);
    Disp_Spi_Send_Data(&Data, 1, 0);
}

static void ST7789_Write_Dat(uint8_t Data)
{
    //GPIO_PortSet(BOARD_LCD_DC_GPIO, 1u << BOARD_LCD_DC_GPIO_PIN);
    GPIO_PinWrite(BOARD_LCD_DC_GPIO,BOARD_LCD_DC_GPIO_PIN,1);
    Disp_Spi_Send_Data(&Data, 1, 0);
}

static void ST7789_Write_MDat(uint8_t *pData, int NumItems)
{
    //GPIO_PortSet(BOARD_LCD_DC_GPIO, 1u << BOARD_LCD_DC_GPIO_PIN);
    GPIO_PinWrite(BOARD_LCD_DC_GPIO,BOARD_LCD_DC_GPIO_PIN,1);
    Disp_Spi_Send_Data(pData, NumItems, 0);
}

static void ST7789_Delay_Ms (uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void ST7789_Init(st7789_send_byte_t _writeData, st7789_send_byte_t _writeCommand, st7789_delay_ms_t _delay_ms)
{
//Driving ability Setting
    _writeCommand(0x11);
    _delay_ms(5);//normal cmd need wait 5ms, next sleep in(sleep out) cmd need 120ms.
//------------------------------display and color format setting--------------------------------//
    _writeCommand(0x36);
    _writeData(0x00);
    _writeCommand(0x3a);
    _writeData(0x55);

    _writeCommand(0xb0);
    _writeData(0x00);
    _writeData(0xf8);
//--------------------------------ST7789V Frame rate setting----------------------------------//
    _writeCommand(0xb2);
    _writeData(0x0c);
    _writeData(0x0c);
    _writeData(0x00);
    _writeData(0x33);
    _writeData(0x33);
    _writeCommand(0xb7);
    _writeData(0x35);
//---------------------------------ST7789V Power setting--------------------------------------//
    _writeCommand(0xbb);
    _writeData(0x28);
    _writeCommand(0xc0);
    _writeData(0x2c);
    _writeCommand(0xc2);
    _writeData(0x01);
    _writeCommand(0xc3);
    _writeData(0x10);
    _writeCommand(0xc4);
    _writeData(0x20);
    _writeCommand(0xc6);
    _writeData(0x0f);
    _writeCommand(0xd0);
    _writeData(0xa4);
    _writeData(0xa1);
//--------------------------------ST7789V gamma setting---------------------------------------//
    _writeCommand(0xe0);
    _writeData(0xd0);
    _writeData(0x00);
    _writeData(0x02);
    _writeData(0x07);
    _writeData(0x0a);
    _writeData(0x28);
    _writeData(0x32);
    _writeData(0x44);
    _writeData(0x42);
    _writeData(0x06);
    _writeData(0x0e);
    _writeData(0x12);
    _writeData(0x14);
    _writeData(0x17);
    _writeCommand(0xe1);
    _writeData(0xd0);
    _writeData(0x00);
    _writeData(0x02);
    _writeData(0x07);
    _writeData(0x0a);
    _writeData(0x28);
    _writeData(0x31);
    _writeData(0x54);
    _writeData(0x47);
    _writeData(0x0e);
    _writeData(0x1c);
    _writeData(0x17);
    _writeData(0x1b);
    _writeData(0x1e);

    _writeCommand(0x13);
    _writeCommand(0x29);
}

int STDisp_Init( uint16_t *pDispBuffer )
{
    /* Define the init structure for the data/command output pin */
    gpio_pin_config_t pin_config = {
        kGPIO_DigitalOutput,
        1,
    };

    /* Init power enable GPIO output . */
    GPIO_PinInit(BOARD_LCD_PWR_GPIO, BOARD_LCD_PWR_GPIO_PIN, &pin_config);
    GPIO_PortClear(BOARD_LCD_PWR_GPIO, 1 << BOARD_LCD_PWR_GPIO_PIN);

    /* Init data/command GPIO output . */
    GPIO_PinInit(BOARD_LCD_DC_GPIO, BOARD_LCD_DC_GPIO_PIN, &pin_config);

    /* Init reset GPIO output . */
    GPIO_PinInit(BOARD_LCD_RST_GPIO, BOARD_LCD_RST_GPIO_PIN, &pin_config);
    ST7789_Delay_Ms(5); /* settle down delay after reset */

    GPIO_PortClear(BOARD_LCD_RST_GPIO, 1 << BOARD_LCD_RST_GPIO_PIN);
    ST7789_Delay_Ms(1); /* settle down delay after reset */

    GPIO_PortSet(BOARD_LCD_RST_GPIO, 1 << BOARD_LCD_RST_GPIO_PIN);
    ST7789_Delay_Ms(120); /* settle down delay after reset */

    Disp_Spi_Init(30000000U);

    ST7789_Init(ST7789_Write_Dat, ST7789_Write_Cmd, ST7789_Delay_Ms);

#if 0
    static uint8_t pdata[320*240*2];
    for(int i = 0; i < 320*240; i++)
    {
        pdata[2*i] = 0xf8;
        pdata[2*i+1] = 0x00;
    }

    STDisp_SendFrame((uint16_t *)pdata);

#endif

  return 1;
}

int STDisp_SendFrame(uint16_t* pDispBuffer)
{
    uint8_t xbuf[4] = {0, 0, (((STDISP_WIDTH -  1) >> 8) & 0xff), ((STDISP_WIDTH  - 1) & 0xff)};
    uint8_t ybuf[4] = {0, 0, (((STDISP_HEIGHT - 1) >> 8) & 0xff), ((STDISP_HEIGHT - 1) & 0xff)};

    ST7789_Write_Cmd(0x2a);
    ST7789_Write_Dat(xbuf[0]);
    ST7789_Write_Dat(xbuf[1]);
    ST7789_Write_Dat(xbuf[2]);
    ST7789_Write_Dat(xbuf[3]);

    ST7789_Write_Cmd(0x2b);
    ST7789_Write_Dat(ybuf[0]);
    ST7789_Write_Dat(ybuf[1]);
    ST7789_Write_Dat(ybuf[2]);
    ST7789_Write_Dat(ybuf[3]);

    ST7789_Write_Cmd(0x2c);
    uint8_t *p = (uint8_t *)pDispBuffer;
    int major_loop = STDISP_WIDTH * STDISP_HEIGHT * 2 / RIVERDI_TRANSFER_STEP_B;
    for(int i = 0; i < major_loop; i++) {
        ST7789_Write_MDat(p, RIVERDI_TRANSFER_STEP_B);
        p += RIVERDI_TRANSFER_STEP_B;
    }
    int minor_loop = STDISP_WIDTH * STDISP_HEIGHT * 2  % RIVERDI_TRANSFER_STEP_B;

    ST7789_Write_MDat(p, minor_loop);

    return 1;
}
