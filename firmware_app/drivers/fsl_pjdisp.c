/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#include "fsl_pjdisp.h"
#if defined(FSL_RTOS_FREE_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif


/*
CS GPIO3_IO27
SCL GPIO2_IO20
SDA GPIO2_IO21
RESET GPIO1_IO14
*/
#define CS_HI   GPIO3->DR |= (1U<<27)
#define CS_LO   GPIO3->DR &= ~(1U<<27)
#define SCL_HI  GPIO2->DR |= (1U<<20)
#define SCL_LO  GPIO2->DR &= ~(1U<<20)
#define SDA_HI  GPIO2->DR |= (1U<<21)
#define SDA_LO  GPIO2->DR &= ~(1U<<21)
#define RST_HI  GPIO1->DR |= (1U<<14)
#define RST_LO  GPIO1->DR &= ~(1U<<14)
#define BL_HI   GPIO3->DR |= (1U<<2)
#define BL_LO   GPIO3->DR &= ~(1U<<2)

typedef void (*ILI9341V_send_byte_t)(uint8_t);
typedef void (*ILI9341V_delay_ms_t)(uint32_t);

static void Delay(uint32_t ticks)
{
    while(ticks--)
    {
    }
}
static void ILI9341V_Write_Cmd(uint8_t cmd)
{
    uint8_t i;
    CS_LO;
    Delay(10);
    SCL_LO;
    SDA_LO;
    Delay(10);
    SCL_HI;
    Delay(10);
    for(i=0;i<8;i++)
    {
        if(cmd&0x80)
            SDA_HI;
        else
            SDA_LO;
        Delay(10);
        SCL_LO;
        cmd <<= 1;
        Delay(10);
        SCL_HI;
        Delay(10);
    }
    Delay(10);
    SDA_HI;
    CS_HI;
    Delay(100);
}
static void ILI9341V_Write_Dat(uint8_t data)
{
    uint8_t i;
    CS_LO;
    Delay(10);
    SCL_LO;
    SDA_HI;
    Delay(10);
    SCL_HI;
    Delay(10);
    for(i=0;i<8;i++)
    {
        if(data&0x80)
            SDA_HI;
        else
            SDA_LO;
        Delay(10);
        SCL_LO;
        data <<= 1;
        Delay(10);
        SCL_HI;
        Delay(10);
    }
    Delay(10);
    SDA_HI;
    CS_HI;
    Delay(100);
}


static void ILI9341V_Delay_Ms (uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void ILI9341V_Init(ILI9341V_send_byte_t _writeData, ILI9341V_send_byte_t _writeCommand, ILI9341V_delay_ms_t _delay_ms)
{
    _writeCommand(0xE8);
    _writeData(0x84);
    _writeData(0x00);
    _writeData(0x7a);

    _writeCommand(0xCF);
    _writeData(0x00);
    _writeData(0x81);
    _writeData(0X30);

    _writeCommand(0xEA);
    _writeData(0x00);
    _writeData(0x00);

    _writeCommand(0xC0);   //Power control
    _writeData(0x21);	   //VRH[5:0]

    _writeCommand(0xC1);   //Power control
    _writeData(0x11);	   //SAP[2:0];BT[3:0]

    _writeCommand(0xC5);   //VCM control
    _writeData(0x41);
    _writeData(0x3c);

    _writeCommand(0xC7);    //VCM control2
    _writeData(0xac);

    _writeCommand(0x36);    // Memory Access Control
    _writeData(0x48);

    _writeCommand(0x3a);    //
    _writeData(0x66);

    _writeCommand(0xF2);    // 3Gamma Function Disable
    _writeData(0x00);

    _writeCommand(0x26);    //Gamma curve selected
    _writeData(0x01);

    _writeCommand(0xE0);    //Set Gamma
    _writeData(0x0F);
    _writeData(0x38);
    _writeData(0x32);
    _writeData(0x0C);
    _writeData(0x0e);
    _writeData(0x07);
    _writeData(0x4f);
    _writeData(0X98);
    _writeData(0x39);
    _writeData(0x07);
    _writeData(0x11);
    _writeData(0x04);
    _writeData(0x13);
    _writeData(0x11);
    _writeData(0x00);

    _writeCommand(0xE1);    //Set Gamma
    _writeData(0x00);
    _writeData(0x0d);
    _writeData(0x0e);
    _writeData(0x04);
    _writeData(0x12);
    _writeData(0x08);
    _writeData(0x31);
    _writeData(0x78);
    _writeData(0x46);
    _writeData(0x08);
    _writeData(0x10);
    _writeData(0x0d);
    _writeData(0x2b);
    _writeData(0x33);
    _writeData(0x0F);

    _writeCommand(0xb0);    //Set
    _writeData(0xc0);

    _writeCommand(0xF6);    //Set
    _writeData(0x01);
    _writeData(0x00);
    _writeData(0x06);

    _writeCommand(0x11);    //Exit Sleep
    _delay_ms(10);
    _writeCommand(0x29);    //Display on
    _delay_ms(20);
}

int PJDisp_Init(void)
{
    RST_LO;
    ILI9341V_Delay_Ms(1);
    RST_HI;
    ILI9341V_Delay_Ms(20);

    ILI9341V_Init(ILI9341V_Write_Dat, ILI9341V_Write_Cmd, ILI9341V_Delay_Ms);
    return 1;
}

int PJDisp_SendFrame(uint32_t DispBuffer)
{
    LCDIF->NEXT_BUF = DispBuffer;
    return 1;
}

int PJDisp_TurnOnBacklight(void)
{
    BL_HI;
    return 0;
}

int PJDisp_TurnOffBacklight(void)
{
    BL_LO;
    return 0;
}
