/*
 * Copyright (c) NXP 2019
 * Copyright (c) Riverdi Sp. z o.o. sp. k. <riverdi@riverdi.com>
 * Copyright (c) Skalski Embedded Technologies <contact@lukasz-skalski.com>
 */

#if RTVISION_BOARD

#include "platform_disp.h"

#include "board.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_debug_console.h"
#include "fsl_qtmr.h"
#include "fsl_lpspi.h"
#ifdef USE_RTOS_SPI
#include "fsl_lpspi_freertos.h"
#endif
#include "fsl_pxp.h"
#include "sln_pcal.h"
#include "sln_rvdisp.h"

#include "fsl_video_common.h"
#include "fsl_time.h"
#include "fsl_log.h"

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


#ifdef	RTBOARD_REVC
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

#ifdef  RTBOARD_REVC
#define DISP_LPSPI_TX_DMA_REQUEST kDmaRequestMuxLPSPI4Tx
#else
#define DISP_LPSPI_TX_DMA_REQUEST kDmaRequestMuxLPSPI3Tx
#endif

#define DISP_LPSPI_DMA_TRANSFER_COMPLETED   ( 1 << 0 )

static lpspi_master_edma_handle_t g_lpspiDispMasterEdmaHandle;
static edma_handle_t g_lpspiDispTxEdmaHandle;
static EventGroupHandle_t g_lpspiDispTransferCompleted;

/* LPSPI EDMA callback */
static void DISP_LPSPI_CallbackEDMA(LPSPI_Type *base, lpspi_master_edma_handle_t *handle, status_t status, void *userData)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(g_lpspiDispTransferCompleted, DISP_LPSPI_DMA_TRANSFER_COMPLETED, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#endif

static void initDisplayLPSPI(uint32_t maxclk)
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
    NVIC_SetPriority(s_lpspiIRQ[LPSPI_GetInstance(DISP_LPSPI_MASTER_BASEADDR)],2);
#endif

#ifdef DISP_USE_SPI_DMA
    edma_config_t config;

    /* Create Event Group for transfer completed */
    g_lpspiDispTransferCompleted = xEventGroupCreate();

    /* Init DMAMUX */
    /* Initialize only the LPSPI_EDMA_TX(ISR) in order to have a faster SPI transmission */
    DMAMUX_Init(DISP_LPSPI_DMAMUX_BASEADDR);
    DMAMUX_SetSource(DISP_LPSPI_DMAMUX_BASEADDR, DISP_LPSPI_TX_DMA_CHANNEL, DISP_LPSPI_TX_DMA_REQUEST);
    DMAMUX_EnableChannel(DISP_LPSPI_DMAMUX_BASEADDR, DISP_LPSPI_TX_DMA_CHANNEL);

    /* Init the EDMA module */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(DISP_LPSPI_DMA_BASEADDR, &config);
    EDMA_CreateHandle(&g_lpspiDispTxEdmaHandle, DISP_LPSPI_DMA_BASEADDR, DISP_LPSPI_TX_DMA_CHANNEL);

    /* Create LPSPI DMA handle. */
    LPSPI_MasterTransferCreateHandleEDMA(DISP_LPSPI_MASTER_BASEADDR, &g_lpspiDispMasterEdmaHandle,
                                        DISP_LPSPI_CallbackEDMA, NULL, &g_lpspiDispTxEdmaHandle, &g_lpspiDispTxEdmaHandle);

    NVIC_SetPriority(DISP_LPSPI_TX_IRQn, 2);
#endif
}


/*
 * platform_init()
 */
bool_t
platform_init (Gpu_HalInit_t *halinit)
{
   /* Init already done up to this point */
   return TRUE;
}


/*
 * platform_sleep_ms()
 */
void
platform_sleep_ms (uint32_t ms)
{
	vTaskDelay(pdMS_TO_TICKS(ms));
}


/*
 * platform_spi_init()
 */
bool_t
platform_spi_init (Gpu_Hal_Context_t *host, uint32_t maxclk)
{
#ifdef	RTBOARD_REVC
	BOARD_InitLPSPI4Pins();
#else
	BOARD_InitLPSPI3Pins();
#endif
	initDisplayLPSPI(maxclk);
  return TRUE;
}


/*
 * platform_spi_deinit()
 */
void
platform_spi_deinit (Gpu_Hal_Context_t *host)
{

#ifndef USE_RTOS_SPI
  LPSPI_Deinit(DISP_LPSPI_MASTER_BASEADDR);
#else
  LPSPI_RTOS_Deinit(&g_rv_disp_spi_handle);
#endif

}


/*
 * platform_spi_send_recv_byte();
 */
uchar8_t
platform_spi_send_recv_byte (Gpu_Hal_Context_t  *host,
                             uchar8_t            data,
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
 * platform_spi_send_data()
 */
uint16_t
platform_spi_send_data (Gpu_Hal_Context_t  *host,
                        uchar8_t           *data,
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
 * platform_spi_recv_data()
 */
void
platform_spi_recv_data (Gpu_Hal_Context_t  *host,
                        uchar8_t           *data,
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

#ifdef	RTBOARD_REVC
/*
 * platform_gpio_init()
 */
bool_t
platform_gpio_init (Gpu_Hal_Context_t *host,
                    gpio_name          ngpio)
{
#if 0
	__HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef gpio;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pin = ngpio;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(GPIOA, &gpio);
#endif
  gpio_pin_config_t pin_config = {.direction = kGPIO_DigitalOutput, .outputLogic = 1};
switch(ngpio)
{
case   GPIO_CS:
	  {
		  CLOCK_EnableClock(kCLOCK_Iomuxc);

		  IOMUXC_SetPinMux(
				  IOMUXC_GPIO_B0_00_GPIO2_IO00,
				  0U);
		  IOMUXC_SetPinConfig(
				  IOMUXC_GPIO_B0_00_GPIO2_IO00,
		  		  0x10B0u);

		  GPIO_PinInit(GPIO2, 0, &pin_config);
		  break;
	  }
case   GPIO_CLK:
	  {
		  CLOCK_EnableClock(kCLOCK_Iomuxc);

		  IOMUXC_SetPinMux(
				  IOMUXC_GPIO_B0_03_GPIO2_IO03,
				  0U);
		  IOMUXC_SetPinConfig(
				  IOMUXC_GPIO_B0_03_GPIO2_IO03,
		  		  0x10B0u);

		  GPIO_PinInit(GPIO2, 3, &pin_config);
		  break;
	  }
case   GPIO_SDO:
	  {
		  CLOCK_EnableClock(kCLOCK_Iomuxc);

		  IOMUXC_SetPinMux(
				  IOMUXC_GPIO_B0_02_GPIO2_IO02,
				  0U);
		  IOMUXC_SetPinConfig(
				  IOMUXC_GPIO_B0_02_GPIO2_IO02,
		  		  0x10B0u);
		  GPIO_PinInit(GPIO2, 2, &pin_config);
		  break;
	  }
case   GPIO_PD:
	  {
#if BOARD_HAS_PCAL
		  set_iox_pin_dir(13, PCAL_PIN_DIR_OUTPUT);
#endif
		  break;
	  }
case   GPIO_INT:
	  {
		  // Unused for now
		   gpio_pin_config_t int_config = {
		        kGPIO_DigitalInput, 0,
				kGPIO_IntLowLevel,
		    };

		  CLOCK_EnableClock(kCLOCK_Iomuxc);

  		  IOMUXC_SetPinMux(
  				IOMUXC_GPIO_B0_05_GPIO2_IO05,
  				  0U);
  		  IOMUXC_SetPinConfig(
  				IOMUXC_GPIO_B0_05_GPIO2_IO05,
  		  		  0x10B0u);

		    /* Init int GPIO. */
		    GPIO_PinInit(GPIO2, 5, &int_config);
		   break;
	  }
default:
	  break;
}

  return TRUE;
}


/*
 * platform_gpio_value()
 */
bool_t
platform_gpio_value (Gpu_Hal_Context_t  *host,
                     gpio_name           ngpio,
                     gpio_val            vgpio)
{
  switch(ngpio)
  {
  case   GPIO_CS:
  	  {
 		  GPIO_PinWrite(GPIO2, 0, vgpio);
  		  break;
  	  }
  case   GPIO_CLK:
  	  {
		  GPIO_PinWrite(GPIO2, 3, vgpio);
  		  break;
  	  }
  case   GPIO_SDO:
	  {
		  GPIO_PinWrite(GPIO2, 2, vgpio);
		  break;
	  }
  case   GPIO_PD:
	  {
#if BOARD_HAS_PCAL
		  set_iox_port_pin(OUTPUT_PORT1, 5, vgpio);
#endif
		  break;
	  }
  case   GPIO_INT:
  	  {
  		  // Not used yet
  		  // set_iox_port_pin(OUTPUT_PORT0, 0, vgpio);
  		  break;
  	  }
  default:
	  break;
  }
  return TRUE;
}

#else
/*
 * platform_gpio_init()
 */
bool_t
platform_gpio_init (Gpu_Hal_Context_t *host,
                    gpio_name          ngpio)
{
#if 0
	__HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef gpio;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pin = ngpio;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(GPIOA, &gpio);
#endif
  gpio_pin_config_t pin_config = {.direction = kGPIO_DigitalOutput, .outputLogic = 1};
switch(ngpio)
{
case   GPIO_CS:
	  {
		  CLOCK_EnableClock(kCLOCK_Iomuxc);

		  IOMUXC_SetPinMux(
				  IOMUXC_GPIO_AD_B0_03_GPIO1_IO03,
				  0U);
		  IOMUXC_SetPinConfig(
		  		  IOMUXC_GPIO_AD_B0_03_GPIO1_IO03,
		  		  0x10B0u);

		  GPIO_PinInit(GPIO1,3, &pin_config);
		  break;
	  }
case   GPIO_CLK:
	  {
		  CLOCK_EnableClock(kCLOCK_Iomuxc);

		  IOMUXC_SetPinMux(
				  IOMUXC_GPIO_AD_B0_00_GPIO1_IO00,
				  0U);
		  IOMUXC_SetPinConfig(
				  IOMUXC_GPIO_AD_B0_00_GPIO1_IO00,
		  		  0x10B0u);

		  GPIO_PinInit(GPIO1,0, &pin_config);
		  break;
	  }
case   GPIO_SDO:
	  {
		  CLOCK_EnableClock(kCLOCK_Iomuxc);

		  IOMUXC_SetPinMux(
				  IOMUXC_GPIO_AD_B0_01_GPIO1_IO01,
				  0U);
		  IOMUXC_SetPinConfig(
				  IOMUXC_GPIO_AD_B0_01_GPIO1_IO01,
		  		  0x10B0u);
		  GPIO_PinInit(GPIO1,1, &pin_config);
		  break;
	  }
case   GPIO_PD:
	  {
		  set_iox_pin_dir(1, PCAL_PIN_DIR_OUTPUT);
		  break;
	  }
case   GPIO_INT:
	  {
		  // Unused for now
		  set_iox_pin_dir(0, PCAL_PIN_DIR_INPUT); // This does not seem right to set it as output
		  break;
	  }
default:
	  break;
}

  return TRUE;
}


/*
 * platform_gpio_value()
 */
bool_t
platform_gpio_value (Gpu_Hal_Context_t  *host,
                     gpio_name           ngpio,
                     gpio_val            vgpio)
{
  switch(ngpio)
  {
  case   GPIO_CS:
  	  {
 		  GPIO_PinWrite(GPIO1, 3, vgpio);
  		  break;
  	  }
  case   GPIO_CLK:
  	  {
		  GPIO_PinWrite(GPIO1, 0, vgpio);
  		  break;
  	  }
  case   GPIO_SDO:
	  {
		  GPIO_PinWrite(GPIO1, 1, vgpio);
		  break;
	  }
  case   GPIO_PD:
	  {
		  set_iox_port_pin(OUTPUT_PORT0, 1, vgpio);
		  break;
	  }
  case   GPIO_INT:
  	  {
  		  // Not used yet
  		  // set_iox_port_pin(OUTPUT_PORT0, 0, vgpio);
  		  break;
  	  }
  default:
	  break;
  }
  return TRUE;
}
#endif
#endif
