/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef __RVDISPLAY_H__
#define __RVDISPLAY_H__

#if RTVISION_BOARD
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if 0
/* Master related */
#define EXAMPLE_LPSPI_MASTER_BASEADDR (LPSPI4)
#define EXAMPLE_LPSPI_MASTER_IRQN LPSPI4_IRQn
#define EXAMPLE_LPSPI_MASTER_IRQHandler LPSPI4_IRQHandler

/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER (7U)

#define LPSPI_MASTER_CLK_FREQ (CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk) / (EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER + 1U))
#define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 500k */

#define EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT (kLPSPI_Pcs0)
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)
#endif

#define CS_PIN_GPIO GPIO2
#define CS_PIN 0U

/* Hardware related display defines */

/* The QTMR instance/channel used for display com inversion */
#define DISP_QTMR_BASEADDR TMR2
#define DISP_QTMR_PWM_CHANNEL kQTMR_Channel_2
/* Get source clock for the display QTMR driver */
#define DISP_QTMR_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_IpgClk)

/*GPIO for DISP_DISP signal */
#define DISP_DISP_PIN_GPIO GPIO1
#define DISP_DISP_PIN 1U

/*GPIO for DISP_EXTMODE signal */
#define DISP_EXTMODE_PIN_GPIO GPIO1
#define DISP_EXTMODE_PIN 8U

/* Display colors for 4 bit mode, the LSB bit is dummy, bits are ordered as RGBD */

#define DISP_BLACK 0x00
#define DISP_WHITE 0x0E
/* Primary colors */
#define DISP_RED 0x08
#define DISP_GREEN 0x04
#define DISP_BLUE 0x02

/* Other combinations */
#define DISP_CYAN 0x06
#define DISP_MAGENTA 0x0A
#define DISP_YELLOW 0x0C

#define DISP_CMD_ALL_CLEAR 0x20        /* Set all display memory to black */
#define DISP_CMD_UPDATE_4BIT_MODE 0x90 /* Send one or more lines to the display */
#define DISP_CMD_NO_UPDATE \
    0x00 /* Used for SPI VCOM update when not using external vcom toggling and no recent command was sent */
#define DISP_CMD_VCOM 0x40 /* used as a flag (M1) to invert vcom */

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Prototype
 ******************************************************************************/

/* Software related display defines */
#define RVDISP_WIDTH 240
#define RVDISP_HEIGHT 320

// void displayOn(int turnOn);
int RVDisp_Init(uint16_t *pDispBuffer);
int RVDisp_UnInit(uint16_t *pDispBuffer);
int RVDisp_PowerCycleDisplay(bool up);
void RVDisp_CopyFrameToDisplayBuffer(uint16_t *pFrameAddr, uint16_t *pDispBuffer);
void RVDisp_SendFrame(uint16_t *pFrame);

#if defined(__cplusplus)
}
#endif

#endif
#endif /* __RVDISPLAY_H__ */
