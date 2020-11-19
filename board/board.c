/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"

#if defined(SDK_I2C_BASED_COMPONENT_USED) && SDK_I2C_BASED_COMPONENT_USED
#include "i2c.h"
#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
#include "fsl_lpi2c_freertos.h"
#else
#include "fsl_lpi2c.h"
#endif /* SDK_I2C_FREERTOS */
#endif /* SDK_I2C_BASED_COMPONENT_USED */

#if defined(SDK_SAI_BASED_COMPONENT_USED) && SDK_SAI_BASED_COMPONENT_USED
#include "sln_dev_cfg.h"
#include "pdm_pcm_definitions.h"
#include "sln_cfg_file.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_tfa9xxx.h"
#if defined(USE_TFA9894_PUI) && USE_TFA9894_PUI
#include "tfa_config_TFA9894N2A1_48kHz_PUI.h"
#else
#include "tfa_config_TFA9894N2.h"
#endif
#endif

#include "fsl_iomuxc.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern void __base_SRAM_DTC(void);
extern void __top_SRAM_DTC(void);
extern void __base_SRAM_ITC(void);
extern void __top_SRAM_ITC(void);
extern void __base_SRAM_OC_NON_CACHEABLE(void);
extern void __top_SRAM_OC_NON_CACHEABLE(void);
extern void __base_SRAM_OC_CACHEABLE(void);
extern void __top_SRAM_OC_CACHEABLE(void);

__attribute__((section(".vectorTableRam"), aligned(0x400))) uint32_t g_vectorTable[256] = {0};

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t __Vectors[];
extern uint32_t Image$$ARM_LIB_STACK$$ZI$$Limit;
#define __VECTOR_TABLE __Vectors
#define __StackTop     Image$$ARM_LIB_STACK$$ZI$$Limit
#elif defined(__MCUXPRESSO)
extern uint32_t __Vectors[];
extern void _vStackTop(void);
#define __VECTOR_TABLE __Vectors
#define __StackTop     _vStackTop
#elif defined(__ICCARM__)
extern uint32_t __vector_table[];
extern uint32_t CSTACK$$Limit;
#define __VECTOR_TABLE __vector_table
#define __StackTop     CSTACK$$Limit
#elif defined(__GNUC__)
extern uint32_t __StackTop;
extern uint32_t __Vectors[];
#define __VECTOR_TABLE __Vectors
#endif

#if defined(SDK_I2C_BASED_COMPONENT_USED) && SDK_I2C_BASED_COMPONENT_USED
/* Numbers if I2C instances */
uint8_t s_i2cHandle[5][((HAL_I2C_MASTER_HANDLE_SIZE + sizeof(uint32_t) - 1U) / sizeof(uint32_t))];
#endif

#if defined(SDK_SAI_BASED_COMPONENT_USED) && SDK_SAI_BASED_COMPONENT_USED
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t dummy_txbuffer[32], 32);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t dummy_rxbuffer[32], 32);

tfa9xxx_config_t tfa9xxxConfig = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = 10000000U},
    .slaveAddress = TFA9XXX_I2C_ADDR_0,
    .protocol     = kTFA9XXX_BusI2S,
    .format       = {.sampleRate = kTFA9XXX_AudioSampleRate48KHz, .bitWidth = kTFA9XXX_AudioBitWidth16bit},
    .tfaContainer = tfa_container_bin,
    .deviceIndex  = 0,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_TFA9XXX, .codecDevConfig = &tfa9xxxConfig};

void *BOARD_GetBoardCodecConfig(void)
{
    return (void *)&boardCodecConfig;
}
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

static uint8_t get_mpu_size(uint32_t ramSize)
{
    uint8_t mpuRegSize = 0U;

    if (ramSize == 0U)
    {
        mpuRegSize = 0U;
    }
    else if (ramSize <= (32 * 1024))
    {
        mpuRegSize = ARM_MPU_REGION_SIZE_32KB;
    }
    else if (ramSize <= (64 * 1024))
    {
        mpuRegSize = ARM_MPU_REGION_SIZE_64KB;
    }
    else if (ramSize <= (128 * 1024))
    {
        mpuRegSize = ARM_MPU_REGION_SIZE_128KB;
    }
    else if (ramSize <= (256 * 1024))
    {
        mpuRegSize = ARM_MPU_REGION_SIZE_256KB;
    }
    else
    {
        mpuRegSize = ARM_MPU_REGION_SIZE_512KB;
    }

    return mpuRegSize;
}

/* Get debug console frequency. */
uint32_t BOARD_DebugConsoleSrcFreq(void)
{
    uint32_t freq;

    /* To make it simple, we assume default PLL and divider settings, and the only variable
       from application is use PLL3 source or OSC source */
    if (CLOCK_GetMux(kCLOCK_UartMux) == 0) /* PLL3 div6 80M */
    {
        freq = (CLOCK_GetPllFreq(kCLOCK_PllUsb1) / 6U) / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    }
    else
    {
        freq = CLOCK_GetOscFreq() / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    }

    return freq;
}

/* Initialize debug console. */
void BOARD_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq = BOARD_DebugConsoleSrcFreq();

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

/* Deinitialize debug console. */
void BOARD_DeInitDebugConsole(void)
{
    DbgConsole_Deinit();

    /* Set g_serialHandle to NULL so the DbgConsole_Printf will not try
     * to write data to the serial when the DbgConsole is not initialized.
     */
    g_serialHandle = NULL;
}



#if defined(SDK_I2C_BASED_COMPONENT_USED) && SDK_I2C_BASED_COMPONENT_USED
void BOARD_LPI2C_Init(uint8_t i2cInstance, uint32_t clkSrc_Hz)
{
    /*
     * lpi2cConfig.debugEnable = false;
     * lpi2cConfig.ignoreAck = false;
     * lpi2cConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * lpi2cConfig.baudRate_Hz = 100000U;
     * lpi2cConfig.busIdleTimeout_ns = 0;
     * lpi2cConfig.pinLowTimeout_ns = 0;
     * lpi2cConfig.sdaGlitchFilterWidth_ns = 0;
     * lpi2cConfig.sclGlitchFilterWidth_ns = 0;
     */
    hal_i2c_master_config_t masterConfig;
    masterConfig.enableMaster = true;
    masterConfig.baudRate_Bps = 400000U;
    masterConfig.srcClock_Hz  = clkSrc_Hz;
    masterConfig.instance     = i2cInstance;

#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
    HAL_I2cMasterInit_FreeRTOS((hal_i2c_master_handle_t *)s_i2cHandle[i2cInstance], &masterConfig);
#else
    HAL_I2cMasterInit((hal_i2c_master_handle_t *)s_i2cHandle[i2cInstance], &masterConfig);
#endif /* SDK_I2C_FREERTOS */


}

status_t BOARD_LPI2C_Send(uint8_t i2cInstance,
                          uint8_t deviceAddress,
                          uint32_t subAddress,
                          uint8_t subAddressSize,
                          uint8_t *txBuff,
                          uint8_t txBuffSize)
{
    status_t reVal;
    hal_i2c_master_transfer_t transfer;
    transfer.flags          = kHAL_I2cTransferDefaultFlag;;
    transfer.slaveAddress   = deviceAddress;
    transfer.direction      = kHAL_I2cWrite;
    transfer.data           = txBuff;
    transfer.dataSize       = txBuffSize;
    transfer.subaddressSize = subAddressSize;
    transfer.subaddress 	= subAddress;

#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
    /* Send master blocking data to slave */
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
    {
        reVal = HAL_I2cMasterTransferBlocking((hal_i2c_master_handle_t *)s_i2cHandle[i2cInstance], &transfer);
    }
    else
    {
        reVal = HAL_I2cMasterTransfer_FreeRTOS((hal_i2c_master_handle_t *)s_i2cHandle[i2cInstance], &transfer);
    }
#else
    reVal = HAL_I2cMasterTransferBlocking((hal_i2c_master_handle_t *)s_i2cHandle[i2cInstance], &transfer);
#endif
    return reVal;
}

status_t BOARD_LPI2C_Receive(uint8_t i2cInstance,
                             uint8_t deviceAddress,
                             uint32_t subAddress,
                             uint8_t subAddressSize,
                             uint8_t *rxBuff,
                             uint8_t rxBuffSize)
{
    status_t reVal;
    hal_i2c_master_transfer_t transfer;
    transfer.flags          = kHAL_I2cTransferDefaultFlag;;
    transfer.slaveAddress   = deviceAddress;
    transfer.direction      = kHAL_I2cRead;
    transfer.data           = rxBuff;
    transfer.dataSize       = rxBuffSize;
    transfer.subaddressSize = subAddressSize;
    transfer.subaddress 	= subAddress;

#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
    /* Send master blocking data to slave */
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
    {
        reVal = HAL_I2cMasterTransferBlocking((hal_i2c_master_handle_t *)s_i2cHandle[i2cInstance], &transfer);
    }
    else
    {
        reVal = HAL_I2cMasterTransfer_FreeRTOS((hal_i2c_master_handle_t *)s_i2cHandle[i2cInstance], &transfer);
    }
#else
    reVal = HAL_I2cMasterTransferBlocking((hal_i2c_master_handle_t *)s_i2cHandle[i2cInstance], &transfer);
#endif 
    return reVal;
}

void BOARD_Accel_I2C_Init(void)
{
    BOARD_LPI2C_Init(BOARD_ACCEL_I2C_INSTANCE, BOARD_ACCEL_I2C_CLOCK_FREQ);
}

status_t BOARD_Accel_I2C_Send(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff)
{
    uint8_t data = (uint8_t)txBuff;

    return BOARD_LPI2C_Send(BOARD_ACCEL_I2C_INSTANCE, deviceAddress, subAddress, subaddressSize, &data, 1);
}

status_t BOARD_Accel_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_ACCEL_I2C_INSTANCE, deviceAddress, subAddress, subaddressSize, rxBuff, rxBuffSize);
}

void BOARD_Codec_I2C_Init(void)
{
    //    BOARD_LPI2C_Init(BOARD_CODEC_I2C_BASEADDR, BOARD_CODEC_I2C_CLOCK_FREQ);
}

status_t BOARD_Codec_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_LPI2C_Send(BOARD_CODEC_I2C_INSTANCE, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                            txBuffSize);
}

status_t BOARD_Codec_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_CODEC_I2C_INSTANCE, deviceAddress, subAddress, subAddressSize, rxBuff, rxBuffSize);
}

void BOARD_Camera_I2C_Init(void)
{
    CLOCK_SetMux(kCLOCK_Lpi2cMux, BOARD_CAMERA_I2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, BOARD_CAMERA_I2C_CLOCK_SOURCE_DIVIDER);
    BOARD_LPI2C_Init(BOARD_CAMERA_I2C_INSTANCE, BOARD_CAMERA_I2C_CLOCK_FREQ);
}

status_t BOARD_Camera_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_LPI2C_Send(BOARD_CAMERA_I2C_INSTANCE, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                            txBuffSize);
}

status_t BOARD_Camera_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_CAMERA_I2C_INSTANCE, deviceAddress, subAddress, subAddressSize, rxBuff,
                               rxBuffSize);
}

void BOARD_PCAL_I2C_Init(void)
{
    BOARD_LPI2C_Init(BOARD_PCAL_I2C_INSTANCE, BOARD_PCAL_I2C_CLOCK_FREQ);
}

status_t BOARD_PCAL_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_LPI2C_Send(BOARD_PCAL_I2C_INSTANCE, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                            txBuffSize);
}

status_t BOARD_PCAL_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_PCAL_I2C_INSTANCE, deviceAddress, subAddress, subAddressSize, rxBuff, rxBuffSize);
}

status_t BOARD_Camera_I2C_SendSCCB(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_LPI2C_SendSCCB(BOARD_CAMERA_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                                txBuffSize);
}

status_t BOARD_Camera_I2C_ReceiveSCCB(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_ReceiveSCCB(BOARD_CAMERA_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, rxBuff,
                                   rxBuffSize);
}

#endif /* SDK_I2C_BASED_COMPONENT_USED */

#if defined(SDK_SAI_BASED_COMPONENT_USED) && SDK_SAI_BASED_COMPONENT_USED
void BOARD_SAI_Enable_Mclk_Output(I2S_Type *base, bool enable)
{
    uint32_t mclk_dir_mask = 0;

    if (SAI1 == base)
        mclk_dir_mask = IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK;
    else if (SAI2 == base)
        mclk_dir_mask = IOMUXC_GPR_GPR1_SAI2_MCLK_DIR_MASK;
    else if (SAI3 == base)
        mclk_dir_mask = IOMUXC_GPR_GPR1_SAI3_MCLK_DIR_MASK;
    else
        return;

    if (enable)
    {
        IOMUXC_GPR->GPR1 |= mclk_dir_mask;
    }
    else
    {
        IOMUXC_GPR->GPR1 &= (~mclk_dir_mask);
    }
}

void BOARD_SAI_Init(sai_init_handle_t saiInitHandle)
{
    sai_config_t saiConfig = {0};
    sai_transfer_t txfer;
    sai_transfer_t rxfer;

    sai_transfer_format_t saiAmpFormat = {
        /* Configure the audio format */
        .bitWidth           = kSAI_WordWidth16bits,
        .sampleRate_Hz      = kSAI_SampleRate48KHz,
        .channel            = 1U,
        .protocol           = kSAI_BusLeftJustified,
        .isFrameSyncCompact = true,
        .stereo             = kSAI_MonoLeft,
        .watermark          = FSL_FEATURE_SAI_FIFO_COUNT / 2U,
    };

    /*Clock setting for SAI1*/
    CLOCK_SetMux(kCLOCK_Sai1Mux, BOARD_AMP_SAI_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Sai1PreDiv, BOARD_AMP_SAI_CLOCK_SOURCE_PRE_DIVIDER);
    CLOCK_SetDiv(kCLOCK_Sai1Div, BOARD_AMP_SAI_CLOCK_SOURCE_DIVIDER);

    BOARD_SAI_Enable_Mclk_Output(BOARD_AMP_SAI, true);

    EDMA_CreateHandle(saiInitHandle.amp_dma_tx_handle, DMA0, BOARD_AMP_SAI_EDMA_TX_CH);
    //    EDMA_CreateHandle(saiInitHandle.amp_dma_tx_handle, DMA0, BOARD_AMP_SAI_EDMA_RX_CH);
    DMAMUX_SetSource(DMAMUX, BOARD_AMP_SAI_EDMA_TX_CH, (uint8_t)BOARD_AMP_SAI_EDMA_TX_REQ);
    //    DMAMUX_SetSource(DMAMUX, BOARD_AMP_SAI_EDMA_RX_CH, (uint8_t)BOARD_AMP_SAI_EDMA_RX_REQ);
    DMAMUX_EnableChannel(DMAMUX, BOARD_AMP_SAI_EDMA_TX_CH);
    //    DMAMUX_EnableChannel(DMAMUX, BOARD_AMP_SAI_EDMA_RX_CH);

    /* Initialize SAI Tx */
    SAI_TxGetDefaultConfig(&saiConfig);
    SAI_TxInit(BOARD_AMP_SAI, &saiConfig);

    //    /* Initialize SAI Rx */
    //    SAI_RxGetDefaultConfig(&saiConfig);
    //    SAI_RxInit(BOARD_AMP_SAI, &saiConfig);

    SAI_TransferTxCreateHandleEDMA(BOARD_AMP_SAI, saiInitHandle.amp_sai_tx_handle, saiInitHandle.sai_tx_callback, NULL,
                                   saiInitHandle.amp_dma_tx_handle);
    //    SAI_TransferRxCreateHandleEDMA(BOARD_AMP_SAI, saiInitHandle.amp_sai_rx_handle,
    //            saiInitHandle.sai_rx_callback, NULL, saiInitHandle.amp_dma_rx_handle);

    SAI_TransferTxSetFormatEDMA(BOARD_AMP_SAI, saiInitHandle.amp_sai_tx_handle, &saiAmpFormat, BOARD_AMP_SAI_CLK_FREQ,
                                BOARD_AMP_SAI_CLK_FREQ);
    //    SAI_TransferRxSetFormatEDMA(BOARD_AMP_SAI, saiInitHandle.amp_sai_rx_handle, &saiAmpFormat,
    //    BOARD_AMP_SAI_CLK_FREQ, BOARD_AMP_SAI_CLK_FREQ);

    /* Force bit clock to override standard enablement */
    SAI_TxSetBitClockRate(BOARD_AMP_SAI, BOARD_AMP_SAI_CLK_FREQ, saiAmpFormat.sampleRate_Hz, saiAmpFormat.bitWidth, 2U);
    //    SAI_RxSetBitClockRate(BOARD_AMP_SAI, BOARD_AMP_SAI_CLK_FREQ, saiAmpFormat.sampleRate_Hz,
    //    saiAmpFormat.bitWidth, 2U);

    /* Enable interrupt to handle FIFO error */
    SAI_TxEnableInterrupts(BOARD_AMP_SAI, kSAI_FIFOErrorInterruptEnable);
    //    SAI_RxEnableInterrupts(BOARD_AMP_SAI, kSAI_FIFOErrorInterruptEnable);

    NVIC_SetPriority(BOARD_AMP_SAI_EDMA_TX_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);
    //    NVIC_SetPriority(BOARD_AMP_SAI_EDMA_RX_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);

    EnableIRQ(BOARD_AMP_SAI_IRQ);

    memset(dummy_txbuffer, 0, 32);
    txfer.dataSize = 32;
    txfer.data     = dummy_txbuffer;
    SAI_TransferSendEDMA(BOARD_AMP_SAI, saiInitHandle.amp_sai_tx_handle, &txfer);

    //    rxfer.dataSize = 32;
    //    rxfer.data = dummy_rxbuffer;
    //    SAI_TransferReceiveEDMA(BOARD_AMP_SAI, saiInitHandle.amp_sai_rx_handle, &rxfer);
}

void BOARD_SAI_Deinit(void)
{
    SAI_Deinit(BOARD_AMP_SAI);

    DisableIRQ(BOARD_AMP_SAI_IRQ);
}

// read bit in flash to know TFA has been calibrated
void BOARD_Get_Calibration_State(uint8_t *cur_state)
{
    status_t ret;
    uint32_t len       = 0;
    sln_cfg_data_t cfg = DEFAULT_CFG_VALUES;

    ret = SLN_FLASH_MGMT_Read(DEVICE_CONFIG_FILE_NAME, (uint8_t *)&cfg, &len);
    if (kStatus_Success != ret)
    {
        configPRINTF(("Warning, unknown calibration state! %d\r\n", ret));
        *cur_state = 0; // FIXME: calibrate or do not calibrate? What's the risk?
    }
    else
    {
        *cur_state = cfg.audio_amp_calibration_state;
    }
}

// write bit in flash to set TFA calibration state
void BOARD_Set_Calibration_State(uint8_t new_state)
{
    status_t ret;
    uint32_t len       = 0;
    sln_cfg_data_t cfg = DEFAULT_CFG_VALUES;

    ret = SLN_FLASH_MGMT_Read(DEVICE_CONFIG_FILE_NAME, (uint8_t *)&cfg, &len);

    /* If this is a new file, then carry on and save the file */
    if (SLN_FLASH_MGMT_ENOENTRY2 == ret)
    {
        len = sizeof(sln_cfg_data_t);
    }
    else if (SLN_FLASH_MGMT_OK != ret)
    {
        configPRINTF(("Warning, unknown calibration state! %d\r\n", ret));
        return;
    }

    /* If we got here, then we are ready to set the calibrated bit */
    if (cfg.audio_amp_calibration_state != new_state)
    {
        /* update bit in flash */
        cfg.audio_amp_calibration_state = new_state;
        ret                             = SLN_FLASH_MGMT_Save(DEVICE_CONFIG_FILE_NAME, (uint8_t *)&cfg, len);
        if (kStatus_Success != ret)
        {
            configPRINTF(("Warning, unknown new calibration state! %d\r\n", ret));
            return;
        }
    }
}

extern void SAI_UserTxIRQHandler(void);
extern void SAI_UserRxIRQHandler(void);
void BOARD_AMP_SAI_IRQ_Handler(void)
{
    if (BOARD_AMP_SAI->TCSR & kSAI_FIFOErrorFlag)
    {
        SAI_UserTxIRQHandler();
    }

    if (BOARD_AMP_SAI->RCSR & kSAI_FIFOErrorFlag)
    {
        SAI_UserRxIRQHandler();
    }
}
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
/* The function sets the cacheable memory to shareable, this suggestion is referred from chapter 2.2.1 Memory regions,
 * types and attributes in Cortex-M7 Devices, Generic User Guide */
void BOARD_ConfigUSBMPU()
{
    /* Disable I cache and D cache */
    SCB_DisableICache();
    SCB_DisableDCache();

    /* Disable MPU */
    ARM_MPU_Disable();
    /* MPU configure:
     * Use ARM_MPU_RASR(DisableExec, AccessPermission, TypeExtField, IsShareable, IsCacheable, IsBufferable,
     * SubRegionDisable, Size)
     * API in core_cm7.h.
     * param DisableExec       Instruction access (XN) disable bit,0=instruction fetches enabled, 1=instruction fetches
     * disabled.
     * param AccessPermission  Data access permissions, allows you to configure read/write access for User and
     * Privileged mode.
     *      Use MACROS defined in core_cm7.h:
     * ARM_MPU_AP_NONE/ARM_MPU_AP_PRIV/ARM_MPU_AP_URO/ARM_MPU_AP_FULL/ARM_MPU_AP_PRO/ARM_MPU_AP_RO
     * Combine TypeExtField/IsShareable/IsCacheable/IsBufferable to configure MPU memory access attributes.
     *  TypeExtField  IsShareable  IsCacheable  IsBufferable   Memory Attribtue    Shareability        Cache
     *     0             x           0           0             Strongly Ordered    shareable
     *     0             x           0           1              Device             shareable
     *     0             0           1           0              Normal             not shareable   Outer and inner write
     * through no write allocate
     *     0             0           1           1              Normal             not shareable   Outer and inner write
     * back no write allocate
     *     0             1           1           0              Normal             shareable       Outer and inner write
     * through no write allocate
     *     0             1           1           1              Normal             shareable       Outer and inner write
     * back no write allocate
     *     1             0           0           0              Normal             not shareable   outer and inner
     * noncache
     *     1             1           0           0              Normal             shareable       outer and inner
     * noncache
     *     1             0           1           1              Normal             not shareable   outer and inner write
     * back write/read acllocate
     *     1             1           1           1              Normal             shareable       outer and inner write
     * back write/read acllocate
     *     2             x           0           0              Device              not shareable
     *  Above are normal use settings, if your want to see more details or want to config different inner/outter cache
     * policy.
     *  please refer to Table 4-55 /4-56 in arm cortex-M7 generic user guide <dui0646b_cortex_m7_dgug.pdf>
     * param SubRegionDisable  Sub-region disable field. 0=sub-region is enabled, 1=sub-region is disabled.
     * param Size              Region size of the region to be configured. use ARM_MPU_REGION_SIZE_xxx MACRO in
     * core_cm7.h.
     */
    MPU->RBAR = ARM_MPU_RBAR(7, 0x80000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 1, 1, 1, 0, ARM_MPU_REGION_SIZE_32MB);
    /* Enable MPU */
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

    /* Enable I cache and D cache */
    SCB_EnableDCache();
    SCB_EnableICache();
}

/* MPU configuration. */
void BOARD_ConfigMPU(void)
{
    unsigned int dtcSize = (unsigned int)__top_SRAM_DTC - (unsigned int)__base_SRAM_DTC;
    unsigned int itcSize = (unsigned int)__top_SRAM_ITC - (unsigned int)__base_SRAM_ITC;
    unsigned int ocSize_non_cacheable =
        (unsigned int)__top_SRAM_OC_NON_CACHEABLE - (unsigned int)__base_SRAM_OC_NON_CACHEABLE;
    unsigned int ocSize_cacheable = (unsigned int)__top_SRAM_OC_CACHEABLE - (unsigned int)__base_SRAM_OC_CACHEABLE;
#if defined(__MCUXPRESSO)
    /*! MCUX IDE changed how they check memory regions, we have to add special case for them. */
    if (4 == ocSize_cacheable)
    {
        ocSize_cacheable = 0;
    }
#endif
    unsigned int ocSize = ocSize_non_cacheable + ocSize_cacheable;

    /* Disable I cache and D cache */
    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
    {
        SCB_DisableICache();
    }
    if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR))
    {
        SCB_DisableDCache();
    }

    /* Disable MPU */
    ARM_MPU_Disable();

    /* MPU configure:
     * Use ARM_MPU_RASR(DisableExec, AccessPermission, TypeExtField, IsShareable, IsCacheable, IsBufferable,
     * SubRegionDisable, Size)
     * API in core_cm7.h.
     * param DisableExec       Instruction access (XN) disable bit,0=instruction fetches enabled, 1=instruction fetches
     * disabled.
     * param AccessPermission  Data access permissions, allows you to configure read/write access for User and
     * Privileged mode.
     *      Use MACROS defined in core_cm7.h:
     * ARM_MPU_AP_NONE/ARM_MPU_AP_PRIV/ARM_MPU_AP_URO/ARM_MPU_AP_FULL/ARM_MPU_AP_PRO/ARM_MPU_AP_RO
     * Combine TypeExtField/IsShareable/IsCacheable/IsBufferable to configure MPU memory access attributes.
     *  TypeExtField  IsShareable  IsCacheable  IsBufferable   Memory Attribtue    Shareability        Cache
     *     0             x           0           0             Strongly Ordered    shareable
     *     0             x           0           1              Device             shareable
     *     0             0           1           0              Normal             not shareable   Outer and inner write
     * through no write allocate 0             0           1           1              Normal             not shareable
     * Outer and inner write back no write allocate 0             1           1           0              Normal
     * shareable       Outer and inner write through no write allocate 0             1           1           1 Normal
     * shareable       Outer and inner write back no write allocate 1             0           0           0 Normal not
     * shareable   outer and inner noncache 1             1           0           0              Normal shareable outer
     * and inner noncache 1             0           1           1              Normal             not shareable   outer
     * and inner write back write/read acllocate 1             1           1           1              Normal shareable
     * outer and inner write back write/read acllocate 2             x           0           0              Device not
     * shareable Above are normal use settings, if your want to see more details or want to config different
     * inner/outter cache policy. please refer to Table 4-55 /4-56 in arm cortex-M7 generic user guide
     * <dui0646b_cortex_m7_dgug.pdf> param SubRegionDisable  Sub-region disable field. 0=sub-region is enabled,
     * 1=sub-region is disabled. param Size              Region size of the region to be configured. use
     * ARM_MPU_REGION_SIZE_xxx MACRO in core_cm7.h.
     */

    /* Region 0 setting: Memory with Device type, not shareable, non-cacheable. */
    MPU->RBAR = ARM_MPU_RBAR(0, 0xC0000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_512MB);

    /* Region 2 setting */
#if defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1)
    /* Setting Memory with Normal type, not shareable, outer/inner write back. */
    MPU->RBAR = ARM_MPU_RBAR(2, 0x60000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_512MB);
#else
    /* Setting Memory with Device type, not shareable, non-cacheable. */
    MPU->RBAR = ARM_MPU_RBAR(2, 0x60000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_512MB);
#endif

    /* Region 3 setting: Memory with Device type, not shareable, non-cacheable. */
    MPU->RBAR = ARM_MPU_RBAR(3, 0x00000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_1GB);

    /* Region 4 setting: Memory with Normal type, not shareable, non-cacheable */
    uint8_t mpuRegSize = get_mpu_size(itcSize);
    MPU->RBAR          = ARM_MPU_RBAR(4, 0x00000000U);
    // MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, mpuRegSize);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, mpuRegSize);

    /* Region 5 setting: Memory with Normal type, not shareable, non-cacheable */
    mpuRegSize = get_mpu_size(dtcSize);
    MPU->RBAR  = ARM_MPU_RBAR(5, 0x20000000U);
    // MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, mpuRegSize);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, mpuRegSize);

    /* Region 6 setting: Memory with Normal type, not shareable, non-cacheable */
    if (ocSize_non_cacheable != 0)
    {
        mpuRegSize = get_mpu_size(ocSize_non_cacheable);
        MPU->RBAR  = ARM_MPU_RBAR(6, (unsigned int)__base_SRAM_OC_NON_CACHEABLE);
        // MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 0, 0, mpuRegSize);
        MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, mpuRegSize);
    }

    if (ocSize_cacheable != 0)
    {
        mpuRegSize = get_mpu_size(ocSize_cacheable);
        /* Region 7 setting: Memory with Normal type, not shareable, cacheable WB/WA */
        MPU->RBAR = ARM_MPU_RBAR(7, (unsigned int)__base_SRAM_OC_CACHEABLE);
        MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 1, 1, 0, mpuRegSize);
    }

    /* Region 8 setting: Memory with Normal type, not shareable, non-cacheable */
    mpuRegSize = get_mpu_size(((ocSize < (512 * 1024))) ? 0 : (ocSize - (512 * 1024)));
    MPU->RBAR  = ARM_MPU_RBAR(8, 0x20280000U);
    // MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 0, 0, mpuRegSize);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, mpuRegSize);

#if defined(ENABLE_BOARD_SDRAM)
    /* The define sets the cacheable memory to shareable,
     * this suggestion is referred from chapter 2.2.1 Memory regions,
     * types and attributes in Cortex-M7 Devices, Generic User Guide */
#if defined(SDRAM_IS_SHAREABLE)
    /* Region 9 setting: Memory with Normal type, not shareable, outer/inner write back */
    MPU->RBAR = ARM_MPU_RBAR(9, 0x80000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 1, 1, 1, 0, ARM_MPU_REGION_SIZE_32MB);
#else
    /* Region 9 setting: Memory with Normal type, not shareable, outer/inner write back */
    MPU->RBAR = ARM_MPU_RBAR(9, 0x80000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_32MB);
#endif /* SDRAM_IS_SHAREABLE */

    /* Region 10 setting, set last 2MB of SDRAM can't be accessed by cache, glocal variables which are not expected to
     * be accessed by cache can be put here */
    /* Memory with Normal type, not shareable, non-cacheable */
    MPU->RBAR = ARM_MPU_RBAR(10, 0x81E00000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_2MB);
#endif /* ENABLE_BOARD_SDRAM */

    /* Enable MPU */
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

    /* Enable I cache and D cache */
    SCB_EnableDCache();
    SCB_EnableICache();
}

void BOARD_MMC_Pin_Config(uint32_t speed, uint32_t strength)
{
}

void BOARD_RelocateVectorTableToRam(void)
{
    uint32_t n;
    uint32_t irqMaskValue;

    irqMaskValue = DisableGlobalIRQ();

    SCB_DisableDCache();
    SCB_DisableICache();

    /* Copy the vector table from ROM to RAM */
    for (n = 0; n < ((uint32_t)0x400) / sizeof(uint32_t); n++)
    {
        g_vectorTable[n] = __VECTOR_TABLE[n];
    }

    /* Set application defined stack pointer */
    volatile unsigned int vStackTop = (unsigned int)&__StackTop;
    g_vectorTable[0]                = vStackTop;

    /* Point the VTOR to the position of vector table */
    SCB->VTOR = (uint32_t)g_vectorTable;
    __DSB();

    SCB_EnableICache();
    SCB_EnableDCache();

    EnableGlobalIRQ(irqMaskValue);
}

uint8_t BUTTON_MSDPressed(void)
{
    /* Check if USB MSD Mode button (SW1) is pushed */
    return GPIO_PinReadPadStatus(SW1_GPIO, SW1_GPIO_PIN);
}

uint8_t BUTTON_OTWPressed(void)
{
    /* Check if OTW Mode button (SW2) is pushed */
    return get_iox_port_pin(SW_PORT_PCAL, SW2_PIN_PCAL);
}
