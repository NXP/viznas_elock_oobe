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

#include "board.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_debug_console.h"
#include "fsl_qtmr.h"
#include "fsl_lpspi.h"
#include "pin_mux.h"
#ifdef USE_RTOS_SPI
#include "fsl_lpspi_freertos.h"
#endif
#include "fsl_pxp.h"

#include "fsl_video_common.h"
#include "fsl_time.h"
#include "fsl_log.h"
#include "cr_section_macros.h"

#ifdef DISP_USE_SPI_DMA
#include <FreeRTOS.h>
#include <event_groups.h>
#include "fsl_edma.h"
#include "fsl_lpspi_edma.h"
#include "fsl_dmamux.h"
#endif

#define FUNC_AT_ITC __attribute__((section(".ramfunc.$SRAM_ITC")))

/*******************************************************************************
 * Prototype
 ******************************************************************************/
/*!
* @brief LPSPI initialization
*/
#if  RT106F_ELOCK_BOARD
#define DISP_LPSPI_MASTER_BASEADDR (LPSPI4)
#define DISP_LPSPI_MASTER_IRQN LPSPI4_IRQn
#define DISP_LPSPI_MASTER_IRQHandler LPSPI4_IRQHandler
#else
/* Master related */
#define DISP_LPSPI_MASTER_BASEADDR (LPSPI3)
#define DISP_LPSPI_MASTER_IRQN LPSPI3_IRQn
#define DISP_LPSPI_MASTER_IRQHandler LPSPI3_IRQHandler
#endif

/* Select SYS PLL2 PFD2 (328 MHz) as lpspi clock source */
#define DISP_LPSPI_CLOCK_SOURCE_SELECT (3U)
/* Clock divider for master lpspi clock source */
#define DISP_LPSPI_CLOCK_SOURCE_DIVIDER (2U)

#define DISP_LPSPI_MASTER_CLK_FREQ (CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk) / (DISP_LPSPI_CLOCK_SOURCE_DIVIDER + 1U))
#define DISP_TRANSFER_BAUDRATE 28000000U /*! Transfer baudrate - 20M */

#define DISP_LPSPI_MASTER_PCS_FOR_INIT (kLPSPI_Pcs0)
#define DISP_LPSPI_MASTER_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)

#ifdef USE_RTOS_SPI
uint32_t LPSPI_GetInstance(LPSPI_Type *base);
static lpspi_rtos_handle_t g_rv_disp_spi_handle;
static const IRQn_Type s_lpspiIRQ[] = LPSPI_IRQS;
#endif

#ifdef DISP_USE_SPI_DMA
#define DISP_LPSPI_DMA_BASEADDR     DMA0
#define DISP_LPSPI_DMAMUX_BASEADDR  DMAMUX
#define DISP_LPSPI_TX_DMA_CHANNEL   0U
#define DISP_LPSPI_RX_DMA_CHANNEL   1U
#define DISP_LPSPI_TX_IRQn          DMA0_DMA16_IRQn

#if RT106F_ELOCK_BOARD
#define DISP_LPSPI_TX_DMA_REQUEST kDmaRequestMuxLPSPI4Tx
#else
#define DISP_LPSPI_TX_DMA_REQUEST kDmaRequestMuxLPSPI3Tx
#endif

#define DISP_LPSPI_DMA_TRANSFER_COMPLETED   ( 1 << 0 )

static lpspi_master_edma_handle_t g_lpspiDispMasterEdmaHandle;
static edma_handle_t g_lpspiDispTxEdmaHandle;
static EventGroupHandle_t g_lpspiDispTransferCompleted;

/* LPSPI EDMA callback */
static void Disp_Lpspi_CallbackEdma(LPSPI_Type *base, lpspi_master_edma_handle_t *handle, status_t status, void *userData)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(g_lpspiDispTransferCompleted, DISP_LPSPI_DMA_TRANSFER_COMPLETED, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#endif

static void LPSPI_Init(uint32_t maxclk)
{
    uint32_t srcClock_Hz;
    lpspi_master_config_t masterConfig;

    /*Set clock source for LPSPI*/
    CLOCK_SetMux(kCLOCK_LpspiMux, DISP_LPSPI_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, DISP_LPSPI_CLOCK_SOURCE_DIVIDER);

    /* Initialize SPI master */
    /*Master config*/
    masterConfig.baudRate = maxclk;
    masterConfig.bitsPerFrame = 8;
    masterConfig.cpol = kLPSPI_ClockPolarityActiveLow;
    masterConfig.cpha = kLPSPI_ClockPhaseFirstEdge;
    masterConfig.direction = kLPSPI_MsbFirst;

    masterConfig.pcsToSckDelayInNanoSec = 1000000000 / masterConfig.baudRate;
    masterConfig.lastSckToPcsDelayInNanoSec = 1000000000 / masterConfig.baudRate;
    masterConfig.betweenTransferDelayInNanoSec = 1000000000 / masterConfig.baudRate;

    masterConfig.whichPcs = DISP_LPSPI_MASTER_PCS_FOR_INIT;
    masterConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;

    masterConfig.pinCfg = kLPSPI_SdiInSdoOut;
    masterConfig.dataOutConfig = kLpspiDataOutRetained;

    srcClock_Hz = DISP_LPSPI_MASTER_CLK_FREQ;
    PRINTF("[SPI Clock] %d\r\n", srcClock_Hz);

#ifndef USE_RTOS_SPI
    LPSPI_MasterInit(DISP_LPSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);
#else
    LPSPI_RTOS_Init(&g_rv_disp_spi_handle, DISP_LPSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);
    NVIC_SetPriority(s_lpspiIRQ[LPSPI_GetInstance(DISP_LPSPI_MASTER_BASEADDR)],LIB_DISP_LPSPI_MASTER_PRI);
#endif

#ifdef DISP_USE_SPI_DMA
    edma_config_t config;

    /* Create Event Group for transfer completed */
    g_lpspiDispTransferCompleted = xEventGroupCreate();

    /* Init DMAMUX */
    DMAMUX_Init(DISP_LPSPI_DMAMUX_BASEADDR);
    DMAMUX_SetSource(DISP_LPSPI_DMAMUX_BASEADDR, DISP_LPSPI_TX_DMA_CHANNEL, DISP_LPSPI_TX_DMA_REQUEST);
    DMAMUX_EnableChannel(DISP_LPSPI_DMAMUX_BASEADDR, DISP_LPSPI_TX_DMA_CHANNEL);

    /* Init the EDMA module */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(DISP_LPSPI_DMA_BASEADDR, &config);
    EDMA_CreateHandle(&g_lpspiDispTxEdmaHandle, DISP_LPSPI_DMA_BASEADDR, DISP_LPSPI_TX_DMA_CHANNEL);

    /* Create LPSPI DMA handle. */
    LPSPI_MasterTransferCreateHandleEDMA(DISP_LPSPI_MASTER_BASEADDR, &g_lpspiDispMasterEdmaHandle,
                                        Disp_Lpspi_CallbackEdma, NULL, &g_lpspiDispTxEdmaHandle, &g_lpspiDispTxEdmaHandle);

    NVIC_SetPriority(DISP_LPSPI_TX_IRQn, LIB_DISP_LPSPI_TX_PRI);
#endif
}
/*
 * Disp_spi_init()
 */
bool_t
Disp_Spi_Init(uint32_t maxclk)
{
    LPSPI4_InitPins();

    LPSPI_Init(maxclk);

    return TRUE;
}


/*
 * Disp_spi_deinit()
 */
void
Disp_Spi_Deinit()
{

#ifndef USE_RTOS_SPI
  LPSPI_Deinit(DISP_LPSPI_MASTER_BASEADDR);
#else
  LPSPI_RTOS_Deinit(&g_rv_disp_spi_handle);
#endif

}


/*
 * Disp_spi_send_recv_byte();
 */
uchar8_t
Disp_Spi_Send_Recv_Byte (uchar8_t            data,
                             uint32_t            opt)
{
  uint8_t answer;

  lpspi_transfer_t masterXfer;
  masterXfer.dataSize = 1;
  masterXfer.txData = &data;
  masterXfer.rxData = &answer;
  masterXfer.configFlags = DISP_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous;

#ifndef USE_RTOS_SPI
   LPSPI_MasterTransferBlocking(DISP_LPSPI_MASTER_BASEADDR, &masterXfer);
#else
   LPSPI_RTOS_Transfer(&g_rv_disp_spi_handle, &masterXfer);
#endif
  return answer;
}

/*
 * Disp_spi_send_data()
 */
uint16_t
Disp_Spi_Send_Data (uchar8_t           *data,
                        uint16_t            size,
                        uint32_t            opt)
{

   lpspi_transfer_t masterXfer;

   masterXfer.dataSize = size;
   masterXfer.txData = data;
   masterXfer.rxData = NULL;
   masterXfer.configFlags = DISP_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous;

#ifdef DISP_USE_SPI_DMA
   if (RIVERDI_TRANSFER_STEP_B == size)
   {
       LPSPI_MasterTransferEDMA(DISP_LPSPI_MASTER_BASEADDR, &g_lpspiDispMasterEdmaHandle, &masterXfer);
       EDMA_EnableChannelInterrupts(g_lpspiDispMasterEdmaHandle.edmaTxDataToTxRegHandle->base,
                                    g_lpspiDispMasterEdmaHandle.edmaTxDataToTxRegHandle->channel,
                                    kEDMA_MajorInterruptEnable);
       xEventGroupWaitBits(g_lpspiDispTransferCompleted, DISP_LPSPI_DMA_TRANSFER_COMPLETED, pdTRUE, pdFALSE, portMAX_DELAY);
   }
   else
   {
#endif
#ifndef USE_RTOS_SPI
   LPSPI_MasterTransferBlocking(DISP_LPSPI_MASTER_BASEADDR, &masterXfer);
#else
   LPSPI_RTOS_Transfer(&g_rv_disp_spi_handle, &masterXfer);
#endif
#ifdef DISP_USE_SPI_DMA
   }
#endif
  return size;
}


/*
 * Disp_spi_recv_data()
 */
void
Disp_Spi_Recv_Data (uchar8_t           *data,
                        uint16_t            size,
                        uint32_t            opt)
{
   lpspi_transfer_t masterXfer;

   masterXfer.dataSize = size;
   masterXfer.txData = NULL;
   masterXfer.rxData = data;
   masterXfer.configFlags = DISP_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous;

#ifndef USE_RTOS_SPI
   LPSPI_MasterTransferBlocking(DISP_LPSPI_MASTER_BASEADDR, &masterXfer);
#else
   LPSPI_RTOS_Transfer(&g_rv_disp_spi_handle, &masterXfer);
#endif
    return;
}
