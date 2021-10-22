/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if RT106F_ELOCK_BOARD
#include "board.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"

#if defined(SDK_I2C_BASED_COMPONENT_USED) && SDK_I2C_BASED_COMPONENT_USED
#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
#include "fsl_lpi2c_freertos.h"
#else
#include "fsl_lpi2c.h"
#endif /* SDK_I2C_FREERTOS */
#endif /* SDK_I2C_BASED_COMPONENT_USED */
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
#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
static lpi2c_rtos_handle_t g_i2c_handle[5];
#endif
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
    //added by Raymond to solve UART crash issue because of priority is not set (0 by default)
    NVIC_SetPriority(BOARD_UART_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);
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
void BOARD_LPI2C_Init(LPI2C_Type *base, uint32_t clkSrc_Hz)
{
    uint32_t i2cInstance = LPI2C_GetInstance(base);
    lpi2c_master_config_t lpi2cConfig = {0};
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
    LPI2C_MasterGetDefaultConfig(&lpi2cConfig);
    
    //Some HW may can not support 400Kbps speed
    lpi2cConfig.baudRate_Hz = 200000U;

#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
    LPI2C_RTOS_Init(&g_i2c_handle[i2cInstance], base, &lpi2cConfig, clkSrc_Hz);
#else
    LPI2C_MasterInit(base, &lpi2cConfig, clkSrc_Hz);
#endif
    NVIC_SetPriority(LPI2C_GetIRQn(i2cInstance), configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);
}

status_t BOARD_LPI2C_Send(LPI2C_Type *base,
                          uint8_t deviceAddress,
                          uint32_t subAddress,
                          uint8_t subAddressSize,
                          uint8_t *txBuff,
                          uint8_t txBuffSize)
{
    status_t reVal;
    uint32_t i2cInstance =  LPI2C_GetInstance(base);
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = deviceAddress;
    transfer.direction = kLPI2C_Write;
    transfer.data = txBuff;
    transfer.dataSize = txBuffSize;
    transfer.subaddressSize = subAddressSize;
    transfer.subaddress     = subAddress;

#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
    /* Send master blocking data to slave */
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
    {
        reVal = LPI2C_MasterTransferBlocking(base, &transfer);
    }else
    {
        reVal = LPI2C_RTOS_Transfer(&g_i2c_handle[i2cInstance], &transfer);
    }
#else
	reVal = LPI2C_MasterTransferBlocking(base, &transfer);
#endif
    return reVal;
}

status_t BOARD_LPI2C_Receive(LPI2C_Type *base,
                             uint8_t deviceAddress,
                             uint32_t subAddress,
                             uint8_t subAddressSize,
                             uint8_t *rxBuff,
                             uint8_t rxBuffSize)
{
    status_t reVal;
    uint32_t i2cInstance =  LPI2C_GetInstance(base);
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = deviceAddress;
    transfer.direction = kLPI2C_Read;
    transfer.data = rxBuff;
    transfer.dataSize = rxBuffSize;
    transfer.subaddressSize = subAddressSize;
    transfer.subaddress     = subAddress;

#if defined(SDK_I2C_FREERTOS) && SDK_I2C_FREERTOS
    /* Send master blocking data to slave */
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
    {
        reVal = LPI2C_MasterTransferBlocking(base, &transfer);
    }else
    {
        reVal = LPI2C_RTOS_Transfer(&g_i2c_handle[i2cInstance], &transfer);
    }
#else
	reVal = LPI2C_MasterTransferBlocking(base, &transfer);
#endif
    return reVal;
}

void BOARD_Accel_I2C_Init(void)
{
    BOARD_LPI2C_Init(BOARD_ACCEL_I2C_BASEADDR, BOARD_ACCEL_I2C_CLOCK_FREQ);
}

status_t BOARD_Accel_I2C_Send(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff)
{
    uint8_t data = (uint8_t)txBuff;

    return BOARD_LPI2C_Send(BOARD_ACCEL_I2C_BASEADDR, deviceAddress, subAddress, subaddressSize, &data, 1);
}

status_t BOARD_Accel_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_ACCEL_I2C_BASEADDR, deviceAddress, subAddress, subaddressSize, rxBuff, rxBuffSize);
}

void BOARD_Codec_I2C_Init(void)
{
    //    BOARD_LPI2C_Init(BOARD_CODEC_I2C_BASEADDR, BOARD_CODEC_I2C_CLOCK_FREQ);
}

status_t BOARD_Codec_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_LPI2C_Send(BOARD_CODEC_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                            txBuffSize);
}

status_t BOARD_Codec_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_CODEC_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, rxBuff, rxBuffSize);
}

void BOARD_Camera_I2C_Init(void)
{
    CLOCK_SetMux(kCLOCK_Lpi2cMux, BOARD_CAMERA_I2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, BOARD_CAMERA_I2C_CLOCK_SOURCE_DIVIDER);
    BOARD_LPI2C_Init(BOARD_CAMERA_I2C_BASEADDR, BOARD_CAMERA_I2C_CLOCK_FREQ);
#if CAMERA_DIFF_I2C_BUS
    BOARD_LPI2C_Init(BOARD_CAMERA_IR_I2C_BASEADDR, BOARD_CAMERA_I2C_CLOCK_FREQ);
#endif
}

status_t BOARD_Camera_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_LPI2C_Send(BOARD_CAMERA_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                            txBuffSize);
}

status_t BOARD_Camera_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_CAMERA_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, rxBuff,
                               rxBuffSize);
}

status_t BOARD_Camera_IR_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_LPI2C_Send(BOARD_CAMERA_IR_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                            txBuffSize);
}

status_t BOARD_Camera_IR_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_LPI2C_Receive(BOARD_CAMERA_IR_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, rxBuff,
                               rxBuffSize);
}

#endif /* SDK_I2C_BASED_COMPONENT_USED */


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
    return 0;
    //return GPIO_PinReadPadStatus(SW1_GPIO, SW1_GPIO_PIN);
}

uint8_t BUTTON_OTWPressed(void)
{
    /* Check if OTW Mode button (SW2) is pushed */
    return 0;
    //return get_iox_port_pin(SW_PORT_PCAL, SW2_PIN_PCAL);
}
#endif
