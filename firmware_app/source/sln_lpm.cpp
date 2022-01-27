/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_gpc.h"
#include "fsl_dcdc.h"
#include "fsl_gpio.h"
#include "fsl_log.h"
#include "fsl_common.h"
#include "specific.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "event_groups.h"

#include "commondef.h"
#include "sln_RT10xx_RGB_LED_driver.h"
#include "camera.h"
#include "display.h"
#include "pxp.h"
#include "platform_disp.h"
#include "sln_wdog.h"
#include "sln_lpm.h"
#include "sln_timers.h"
#include "sln_api_internal.h"
#include "sln_system_state.h"
#include "sln_usb_composite.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// #define USE_PIR_WAKEUP
#if RTVISION_BOARD
#define USE_SW_WAKEUP
#endif

#define LPM_PIR_INT_PORT GPIO2
#define LPM_PIR_INT_PIN  17U
#define LPM_PIR_INT_IRQ  GPIO2_Combined_16_31_IRQn

#define LPM_GPC_IMR_NUM (sizeof(GPC->IMR) / sizeof(GPC->IMR[0]))
/* Definition for delay API in clock driver, users can redefine it to the real application. */
#ifndef SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY
#define SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY (600000000UL)
#endif

/* Constants required to manipulate the core.  Registers first... */
#define NVIC_SYSTICK_CTRL_REG   (*((volatile uint32_t *)0xe000e010))
#define NVIC_SYSTICK_ENABLE_BIT (1UL << 0UL)

#define LMP_MSG_Q_COUNT 4

#define TIMER_LPM_CHECK 1
#define MS_LPM_CHECK    7000

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern EventGroupHandle_t g_SyncVideoEvents;
static uint32_t g_savedPrimask;

static QueueHandle_t s_LpmMsgQ = NULL;

static TimerHandle_t s_LpmCheckTimer = NULL;
static int s_TimerLpmCheckID         = TIMER_LPM_CHECK;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
DTC_BSS static StackType_t s_LpmTaskStack[LPMTASK_STACKSIZE];
DTC_BSS static StaticTask_t s_LpmTaskTCB;
#endif

extern EventGroupHandle_t g_SyncVideoEvents;

/*******************************************************************************
 * Code
 ******************************************************************************/

void EnableRegularLDO(void)
{
    /*  Enable Regular LDO 2P5 and 1P1 */
    PMU->REG_2P5_SET = PMU_REG_2P5_ENABLE_LINREG_MASK;
    PMU->REG_1P1_SET = PMU_REG_1P1_ENABLE_LINREG_MASK;
}

void DisableRegularLDO(void)
{
    /* Disable Regular LDO 2P5 and 1P1 */
    PMU->REG_2P5_CLR = PMU_REG_2P5_ENABLE_LINREG_MASK;
    PMU->REG_1P1_CLR = PMU_REG_1P1_ENABLE_LINREG_MASK;
}

void EnableWeakLDO(void)
{
    /*  Enable Weak LDO 2P5 and 1P1 */
    PMU->REG_2P5_SET = PMU_REG_2P5_ENABLE_WEAK_LINREG_MASK;
    PMU->REG_1P1_SET = PMU_REG_1P1_ENABLE_WEAK_LINREG_MASK;

    SDK_DelayAtLeastUs(40, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

void DisableWeakLDO(void)
{
    /* Disable Weak LDO 2P5 and 1P1 */
    PMU->REG_2P5_CLR = PMU_REG_2P5_ENABLE_WEAK_LINREG_MASK;
    PMU->REG_1P1_CLR = PMU_REG_1P1_ENABLE_WEAK_LINREG_MASK;
}

void BandgapOn(void)
{
    /* Turn on regular bandgap and wait for stable */
    PMU->MISC0_CLR = PMU_MISC0_REFTOP_PWD_MASK;
    while ((PMU->MISC0 & PMU_MISC0_REFTOP_VBGUP_MASK) == 0)
    {
    }
    /* Low power band gap disable */
    XTALOSC24M->LOWPWR_CTRL_CLR = XTALOSC24M_LOWPWR_CTRL_LPBG_SEL_MASK;
}

void BandgapOff(void)
{
    XTALOSC24M->LOWPWR_CTRL_SET = XTALOSC24M_LOWPWR_CTRL_LPBG_SEL_MASK;
    PMU->MISC0_SET              = PMU_MISC0_REFTOP_PWD_MASK;
}

/*!
 * @brief Set CCM MUX node to certain value.
 *
 * @param mux   Which mux node to set, see \ref clock_mux_t.
 * @param value Clock mux value to set, different mux has different value range.
 */
void CLOCK_SET_MUX(clock_mux_t mux, uint32_t value)
{
    uint32_t busyShift;

    busyShift               = CCM_TUPLE_BUSY_SHIFT(mux);
    CCM_TUPLE_REG(CCM, mux) = (CCM_TUPLE_REG(CCM, mux) & (~CCM_TUPLE_MASK(mux))) |
                              (((uint32_t)((value) << CCM_TUPLE_SHIFT(mux))) & CCM_TUPLE_MASK(mux));

    assert(busyShift <= CCM_NO_BUSY_WAIT);

    /* Clock switch need Handshake? */
    if (CCM_NO_BUSY_WAIT != busyShift)
    {
        /* Wait until CCM internal handshake finish. */
        while (CCM->CDHIPR & (1U << busyShift))
        {
        }
    }
}

/*!
 * @brief Set CCM DIV node to certain value.
 *
 * @param divider Which div node to set, see \ref clock_div_t.
 * @param value   Clock div value to set, different divider has different value range.
 */
void CLOCK_SET_DIV(clock_div_t divider, uint32_t value)
{
    uint32_t busyShift;

    busyShift                   = CCM_TUPLE_BUSY_SHIFT(divider);
    CCM_TUPLE_REG(CCM, divider) = (CCM_TUPLE_REG(CCM, divider) & (~CCM_TUPLE_MASK(divider))) |
                                  (((uint32_t)((value) << CCM_TUPLE_SHIFT(divider))) & CCM_TUPLE_MASK(divider));

    assert(busyShift <= CCM_NO_BUSY_WAIT);

    /* Clock switch need Handshake? */
    if (CCM_NO_BUSY_WAIT != busyShift)
    {
        /* Wait until CCM internal handshake finish. */
        while (CCM->CDHIPR & (1U << busyShift))
        {
        }
    }
}

void ClockSelectXtalOsc(void)
{
    /* Enable XTAL 24MHz clock source. */
    CLOCK_InitExternalClk(0);
    /* Wait CCM operation finishes */
    CLOCK_CCM_HANDSHAKE_WAIT();
    /* Take some delay */
    SDK_DelayAtLeastUs(40, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    /* Switch clock source to external OSC. */
    CLOCK_SwitchOsc(kCLOCK_XtalOsc);
    /* Turn off XTAL-OSC detector */
    CCM_ANALOG->MISC0_CLR = CCM_ANALOG_MISC0_OSC_XTALOK_EN_MASK;
    /* Power Down internal RC. */
    CLOCK_DeinitRcOsc24M();
}

void ClockSelectRcOsc(void)
{
    /* Enable internal RC. */
    XTALOSC24M->LOWPWR_CTRL |= XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN_MASK;
    /* Wait CCM operation finishes */
    CLOCK_CCM_HANDSHAKE_WAIT();
    /* Take some delay */
    SDK_DelayAtLeastUs(4000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    /* Switch clock source to internal RC. */
    XTALOSC24M->LOWPWR_CTRL_SET = XTALOSC24M_LOWPWR_CTRL_SET_OSC_SEL_MASK;
    /* Disable XTAL 24MHz clock source. */
    CCM_ANALOG->MISC0_SET = CCM_ANALOG_MISC0_XTAL_24M_PWD_MASK;
}

void LPM_Init(void)
{
    uint32_t i;
    uint32_t tmp_reg = 0;

    CLOCK_SetMode(kCLOCK_ModeRun);

    CCM->CGPR |= CCM_CGPR_INT_MEM_CLK_LPM_MASK;

    /* Enable RC OSC. It needs at least 4ms to be stable, so self tuning need to be enabled. */
    XTALOSC24M->LOWPWR_CTRL |= XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN_MASK;
    /* Configure RC OSC */
    XTALOSC24M->OSC_CONFIG0 = XTALOSC24M_OSC_CONFIG0_RC_OSC_PROG_CUR(0x4) | XTALOSC24M_OSC_CONFIG0_SET_HYST_MINUS(0x2) |
                              XTALOSC24M_OSC_CONFIG0_RC_OSC_PROG(0xA7) | XTALOSC24M_OSC_CONFIG0_START_MASK |
                              XTALOSC24M_OSC_CONFIG0_ENABLE_MASK;
    XTALOSC24M->OSC_CONFIG1 = XTALOSC24M_OSC_CONFIG1_COUNT_RC_CUR(0x40) | XTALOSC24M_OSC_CONFIG1_COUNT_RC_TRG(0x2DC);
    /* Take some delay */
    SDK_DelayAtLeastUs(4000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    /* Add some hysteresis */
    tmp_reg = XTALOSC24M->OSC_CONFIG0;
    tmp_reg &= ~(XTALOSC24M_OSC_CONFIG0_HYST_PLUS_MASK | XTALOSC24M_OSC_CONFIG0_HYST_MINUS_MASK);
    tmp_reg |= XTALOSC24M_OSC_CONFIG0_HYST_PLUS(3) | XTALOSC24M_OSC_CONFIG0_HYST_MINUS(3);
    XTALOSC24M->OSC_CONFIG0 = tmp_reg;
    /* Set COUNT_1M_TRG */
    tmp_reg = XTALOSC24M->OSC_CONFIG2;
    tmp_reg &= ~XTALOSC24M_OSC_CONFIG2_COUNT_1M_TRG_MASK;
    tmp_reg |= XTALOSC24M_OSC_CONFIG2_COUNT_1M_TRG(0x2d7);
    XTALOSC24M->OSC_CONFIG2 = tmp_reg;
    /* Hardware requires to read OSC_CONFIG0 or OSC_CONFIG1 to make OSC_CONFIG2 write work */
    tmp_reg                 = XTALOSC24M->OSC_CONFIG1;
    XTALOSC24M->OSC_CONFIG1 = tmp_reg;

    /* ERR007265 */
    IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_GINT_MASK;

    /* Initialize GPC to mask all IRQs */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = 0xFFFFFFFFU;
    }
}

void LPM_EnableWakeupSource(uint32_t irq)
{
    GPC_EnableIRQ(GPC, irq);
}

void LPM_DisableWakeupSource(uint32_t irq)
{
    GPC_DisableIRQ(GPC, irq);
}

void LPM_PreEnterWaitMode(void)
{
    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();
}

void LPM_PostExitWaitMode(void)
{
    EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
}

void LPM_PreEnterStopMode(void)
{
    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();
}

void LPM_PostExitStopMode(void)
{
    EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
}

void LPM_SetRunModeConfig(void)
{
    CCM->CLPCR &= ~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK);
}

void LPM_SetWaitModeConfig(void)
{
    uint32_t clpcr;

    /*
     * ERR007265: CCM: When improper low-power sequence is used,
     * the SoC enters low power mode before the ARM core executes WFI.
     *
     * Software workaround:
     * 1) Software should trigger IRQ #41 (GPR_IRQ) to be always pending
     *      by setting IOMUXC_GPR_GPR1_GINT.
     * 2) Software should then unmask IRQ #41 in GPC before setting CCM
     *      Low-Power mode.
     * 3) Software should mask IRQ #41 right after CCM Low-Power mode
     *      is set (set bits 0-1 of CCM_CLPCR).
     */
    GPC_EnableIRQ(GPC, GPR_IRQ_IRQn);
    clpcr      = CCM->CLPCR & (~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK));
    CCM->CLPCR = clpcr | CCM_CLPCR_LPM(kCLOCK_ModeWait) | CCM_CLPCR_MASK_SCU_IDLE_MASK | CCM_CLPCR_MASK_L2CC_IDLE_MASK |
                 CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK | CCM_CLPCR_STBY_COUNT_MASK | CCM_CLPCR_BYPASS_LPM_HS0_MASK |
                 CCM_CLPCR_BYPASS_LPM_HS1_MASK;
    GPC_DisableIRQ(GPC, GPR_IRQ_IRQn);
}

void LPM_SetStopModeConfig(void)
{
    uint32_t clpcr;

    /*
     * ERR007265: CCM: When improper low-power sequence is used,
     * the SoC enters low power mode before the ARM core executes WFI.
     *
     * Software workaround:
     * 1) Software should trigger IRQ #41 (GPR_IRQ) to be always pending
     *      by setting IOMUXC_GPR_GPR1_GINT.
     * 2) Software should then unmask IRQ #41 in GPC before setting CCM
     *      Low-Power mode.
     * 3) Software should mask IRQ #41 right after CCM Low-Power mode
     *      is set (set bits 0-1 of CCM_CLPCR).
     */
    GPC_EnableIRQ(GPC, GPR_IRQ_IRQn);
    clpcr      = CCM->CLPCR & (~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK));
    CCM->CLPCR = clpcr | CCM_CLPCR_LPM(kCLOCK_ModeStop) | CCM_CLPCR_MASK_L2CC_IDLE_MASK | CCM_CLPCR_MASK_SCU_IDLE_MASK |
                 CCM_CLPCR_VSTBY_MASK | CCM_CLPCR_STBY_COUNT_MASK | CCM_CLPCR_SBYOS_MASK |
                 CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK | CCM_CLPCR_BYPASS_LPM_HS0_MASK | CCM_CLPCR_BYPASS_LPM_HS1_MASK;
    GPC_DisableIRQ(GPC, GPR_IRQ_IRQn);
}

void LPM_OverDriveRun(void)
{
    /* CCM Mode */
    DCDC_BootIntoCCM(DCDC);
    /* Connect internal the load resistor */
    DCDC->REG1 |= DCDC_REG1_REG_RLOAD_SW_MASK;
    /* Adjust SOC voltage to 1.275V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);

    /* Enable FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* Connect vdd_high_in and connect vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    BandgapOn();
    EnableRegularLDO();
    DisableWeakLDO();

    ClockSetToOverDriveRun();
}

void LPM_FullSpeedRun(void)
{
    /* CCM Mode */
    DCDC_BootIntoCCM(DCDC);
    /* Connect internal the load resistor */
    DCDC->REG1 |= DCDC_REG1_REG_RLOAD_SW_MASK;
    /* Adjust SOC voltage to 1.275V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);

    /* Enable FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* Connect vdd_high_in and connect vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    BandgapOn();
    EnableRegularLDO();
    DisableWeakLDO();

    ClockSetToFullSpeedRun();

    /* Adjust SOC voltage to 1.15V */
    DCDC_AdjustTargetVoltage(DCDC, 0xe, 0x1);
}

void LPM_LowSpeedRun(void)
{
    /* CCM Mode */
    DCDC_BootIntoCCM(DCDC);
    /* Connect internal the load resistor */
    DCDC->REG1 |= DCDC_REG1_REG_RLOAD_SW_MASK;
    /* Adjust SOC voltage to 1.275V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);

    /* Enable FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* Connect vdd_high_in and connect vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    BandgapOn();
    EnableRegularLDO();
    DisableWeakLDO();

    ClockSetToLowSpeedRun();

    /* Adjust SOC voltage to 1.15V */
    DCDC_AdjustTargetVoltage(DCDC, 0xe, 0x1);
}

void LPM_LowPowerRun(void)
{
    ClockSetToLowPowerRun();

    /* Power down USBPHY */
    PowerDownUSBPHY();

    /* Adjust SOC voltage to 0.95V */
    DCDC_AdjustTargetVoltage(DCDC, 0x6, 0x1);
    /* DCM Mode */
    DCDC_BootIntoDCM(DCDC);
    /* Disconnect internal the load resistor */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;
    /* Power Down output range comparator */
    DCDC->REG0 |= DCDC_REG0_PWD_CMP_OFFSET_MASK;

    /* Enable FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* Connect vdd_high_in and connect vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    EnableWeakLDO();
    DisableRegularLDO();
    BandgapOff();
}

void LPM_EnterSystemIdle(void)
{
    LPM_SetWaitModeConfig();
    SetLowPowerClockGate();

    ClockSetToSystemIdle();

    /* Power down USBPHY */
    PowerDownUSBPHY();

    /* DCDC to 1.15V */
    DCDC_AdjustTargetVoltage(DCDC, 0xe, 0x1);
    /* DCM Mode */
    DCDC_BootIntoDCM(DCDC);
    /* Disconnect internal the load resistor */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;
    /* Power Down output range comparator */
    DCDC->REG0 |= DCDC_REG0_PWD_CMP_OFFSET_MASK;

    /* Enable FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* Connect vdd_high_in and connect vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    EnableRegularLDO();
    DisableWeakLDO();
    BandgapOn();

    PeripheralEnterDozeMode();

    __DSB();
    __WFI();
    __ISB();
}

void LPM_ExitSystemIdle(void)
{
    PeripheralExitDozeMode();
    LPM_SetRunModeConfig();
}

void LPM_EnterLowPowerIdle(void)
{
    LPM_SetWaitModeConfig();
    SetLowPowerClockGate();

    ClockSetToLowPowerIdle();

    /* Power down USBPHY */
    PowerDownUSBPHY();

    /* Adjust SOC voltage to 0.95V */
    DCDC_AdjustTargetVoltage(DCDC, 0x6, 0x1);
    /* DCM Mode */
    DCDC_BootIntoDCM(DCDC);
    /* Disconnect internal the load resistor */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;
    /* Power Down output range comparator */
    DCDC->REG0 |= DCDC_REG0_PWD_CMP_OFFSET_MASK;

    /* Enable FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* Connect vdd_high_in and connect vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    EnableWeakLDO();
    DisableRegularLDO();
    BandgapOff();

    PeripheralEnterDozeMode();

    __DSB();
    __WFI();
    __ISB();
}

void LPM_ExitLowPowerIdle(void)
{
    PeripheralExitDozeMode();
    LPM_SetRunModeConfig();
}

void LPM_EnterSuspend(void)
{
    uint32_t i;
    uint32_t gpcIMR[LPM_GPC_IMR_NUM];

    /* Clean and disable data cache to make sure context is saved into RAM */
    SCB_CleanDCache();
    SCB_DisableDCache();

    LPM_SetStopModeConfig();
    SetLowPowerClockGate();

    /* Disconnect internal the load resistor */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;

    /* Turn off FlexRAM0 */
    GPC->CNTR |= GPC_CNTR_PDRAM0_PGE_MASK;
    /* Turn off FlexRAM1 */
    PGC->MEGA_CTRL |= PGC_MEGA_CTRL_PCR_MASK;

    /* Adjust LP voltage to 0.925V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);
    /* Switch DCDC to use DCDC internal OSC */
    DCDC_SetClockSource(DCDC, kDCDC_ClockInternalOsc);

    /* Power down USBPHY */
    PowerDownUSBPHY();

    /* Power down CPU when requested */
    PGC->CPU_CTRL = PGC_CPU_CTRL_PCR_MASK;

    /* Enable FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* Connect vdd_high_in and connect vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;
    /* STOP_MODE config, turn off all analog except RTC in stop mode */
    PMU->MISC0_CLR = PMU_MISC0_STOP_MODE_CONFIG_MASK;

    /* Mask all GPC interrupts before enabling the RBC counters to
     * avoid the counter starting too early if an interupt is already
     * pending.
     */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        gpcIMR[i]   = GPC->IMR[i];
        GPC->IMR[i] = 0xFFFFFFFFU;
    }

    /*
     * ERR006223: CCM: Failure to resuem from wait/stop mode with power gating
     *   Configure REG_BYPASS_COUNTER to 2
     *   Enable the RBC bypass counter here to hold off the interrupts. RBC counter
     *  needs to be no less than 2.
     */
    CCM->CCR = (CCM->CCR & ~CCM_CCR_REG_BYPASS_COUNT_MASK) | CCM_CCR_REG_BYPASS_COUNT(2);
    CCM->CCR |= (CCM_CCR_OSCNT(0xAF) | CCM_CCR_COSC_EN_MASK | CCM_CCR_RBC_EN_MASK);

    /* Now delay for a short while (3usec) at this point
     * so a short loop should be enough. This delay is required to ensure that
     * the RBC counter can start counting in case an interrupt is already pending
     * or in case an interrupt arrives just as ARM is about to assert DSM_request.
     */
    SDK_DelayAtLeastUs(3, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    /* Recover all the GPC interrupts. */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = gpcIMR[i];
    }

    PeripheralEnterStopMode();
}

void LPM_EnterSNVS(void)
{
    SNVS->LPCR |= SNVS_LPCR_TOP_MASK;
    while (1) /* Shutdown */
    {
    }
}

static void LPM_SendDeinitCameraMsg(void)
{
    QMsg *pQMsgCamera;
    /* Camera */
    pQMsgCamera             = (QMsg *)pvPortMalloc(sizeof(QMsg));
    pQMsgCamera->id         = QMSG_CMD;
    pQMsgCamera->msg.cmd.id = QCMD_DEINIT_CAMERA;
    Camera_SendQMsg((void *)&pQMsgCamera);
}

int LPM_PreEnterSuspend(void)
{
    LPM_SendDeinitCameraMsg();

    xEventGroupWaitBits(g_SyncVideoEvents, 1 << SYNC_VIDEO_CAMERA_DEINIT_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    /* Wait for the pipeline to clean  after camera is closed*/
    vTaskDelay(500);
    /* Display */
    Display_Deinit();

#if VOICE_PROMPT

#if RT_VISION_BOARD
    /* Audio */
    BOARD_SAI_Deinit();
#endif
#endif
#if RT_VISION_BOARD
    DMAMUX_Deinit(DMAMUX);
    EDMA_Deinit(DMA0);
#endif
    //    /* Pxp */
    //    APP_PXP_Deinit();

    /* Filesystem */
    //    DCP_Deinit();
    //    DB_Exit();

    //    /* LED */
    //    PIT_Deinit(PIT);

    /* Deinit Composite device */
    USB_CompositeDeInit();

    /* Debug Console*/
    BOARD_DeInitDebugConsole();

    /* Deinit led control */
    SysState_DeInit();

    return 0;
}

void PIR_EnterSuspend(void)
{
#ifdef USE_PIR_WAKEUP
    gpio_pin_config_t swConfig = {
        kGPIO_DigitalInput,
        0,
        kGPIO_IntRisingEdge,
    };

    /* Init input switch GPIO. */
    GPIO_PinInit(LPM_PIR_INT_PORT, LPM_PIR_INT_PIN, &swConfig);
#endif

#ifdef USE_SW_WAKEUP
    /* Mask pacl's other switch interrupt, only enable SW3. Interrupt mask: 0 = enabled */
    set_iox_reg(INT_MASK_REG_PORT2, ~(1 << SW3_PIN_PCAL));
#endif

    LPM_PreEnterSuspend();
    LPM_Init();

#ifdef USE_PIR_WAKEUP
    GPIO_ClearPinsInterruptFlags(LPM_PIR_INT_PORT, 1U << LPM_PIR_INT_PIN);
    /* Enable GPIO pin interrupt */
    GPIO_EnableInterrupts(LPM_PIR_INT_PORT, 1U << LPM_PIR_INT_PIN);
    NVIC_SetPriority(LPM_PIR_INT_IRQ, LIB_LPM_PIR_INT_PRI);
    /* Enable the Interrupt */
    EnableIRQ(LPM_PIR_INT_IRQ);
    /* Enable GPC interrupt */
    LPM_EnableWakeupSource(LPM_PIR_INT_IRQ);
#endif

#ifdef USE_SW_WAKEUP
    GPIO_ClearPinsInterruptFlags(PCAL_INT_GPIO, 1U << PCAL_INT_GPIO_PIN);
    /* Disable SW1 pin interupt */
    GPIO_DisableInterrupts(SW1_GPIO, 1 << SW1_GPIO_PIN);
    /* Enable PACL INT pin interrupt */
    GPIO_EnableInterrupts(PCAL_INT_GPIO, 1U << PCAL_INT_GPIO_PIN);
    NVIC_SetPriority(BOARD_USER_BUTTON_IRQ, LIB_BOARD_USER_BUTTON_WKUP_PRI);
    /* Enable the Interrupt */
    EnableIRQ(BOARD_USER_BUTTON_IRQ);
    /* Enable GPC interrupt */
    LPM_EnableWakeupSource(BOARD_USER_BUTTON_IRQ);
#endif

    /* Stop the SysTick momentarily.  The time the SysTick is stopped for
    is accounted for as best it can be, but using the tickless mode will
    inevitably result in some tiny drift of the time maintained by the
    kernel with respect to calendar time. */
    NVIC_SYSTICK_CTRL_REG &= ~NVIC_SYSTICK_ENABLE_BIT;

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
    method as that will mask interrupts that should exit sleep mode. */
    __asm volatile("cpsid i" ::: "memory");
    __asm volatile("dsb");
    __asm volatile("isb");

    LPM_EnterSuspend();

    __asm volatile("dsb" ::: "memory");
    __asm volatile("wfi");
    __asm volatile("isb");
}

static void LPM_TimerCallback(TimerHandle_t xTimer)
{
    switch (*(int *)pvTimerGetTimerID(xTimer))
    {
        case TIMER_LPM_CHECK:
            LPM_SendControlStatus(LPM_EnterSuspendReady, 0);
            break;
    }
}

static void LPM_StartCheckTimer(void)
{
    if (s_LpmCheckTimer != NULL)
    {
        xTimerStart(s_LpmCheckTimer, 0);
    }
}

static void LPM_ResetCheckTimer(void)
{
    if (s_LpmCheckTimer != NULL && xTimerIsTimerActive(s_LpmCheckTimer))
    {
        xTimerReset(s_LpmCheckTimer, 0);
    }
}

static void LPM_StopCheckTimer(void)
{
    if (s_LpmCheckTimer != NULL && xTimerIsTimerActive(s_LpmCheckTimer))
    {
        xTimerStop(s_LpmCheckTimer, 0);
    }
}

static void LPM_Task(void *param)
{
    BaseType_t ret;
    QMsg *pQMsg;
    bool isTriggered = false;
    uint8_t lowPower = Cfg_AppDataGetLowPowerMode();
#if 0
    xEventGroupWaitBits(g_SyncVideoEvents,
                        1 << SYNC_VIDEO_DISPLAY_INIT_BIT | 1 << SYNC_VIDEO_CAMERA_INIT_BIT |
                            1 << SYNC_VIDEO_BLUETOOTH_SERVER_START_INIT,
                        pdFALSE, pdTRUE, portMAX_DELAY);
#endif
    LOGD("[Lpm]:running\r\n");

    while (1)
    {
        ret = xQueueReceive(s_LpmMsgQ, (void *)&pQMsg, portMAX_DELAY);

        if (ret == pdTRUE)
        {
            if (pQMsg->id == QMSG_LPM)
            {
                switch (pQMsg->msg.cmd.id)
                {
                    case LPM_DetNoFaceTimeout:
                    case LPM_RecNoFaceTimeout:
                    {
                        if (lowPower == LOW_POWER_MODE_ON)
                        {
                            isTriggered = true;
                            LPM_StartCheckTimer();
                        }
                    }
                    break;

                    case LPM_LockState:
                    {
                        lowPower = pQMsg->msg.cmd.data.control_status;
                        if (lowPower == LOW_POWER_MODE_OFF)
                        {
                            isTriggered = false;
                            LPM_StopCheckTimer();
                        }
                    }
                    break;

                    case LPM_EnterSuspendReady:
                    {
                        PIR_EnterSuspend();
                    }
                    break;

                    case LPM_ButtonOperation:
                        if (isTriggered && pQMsg->msg.cmd.data.control_status)
                        {
                            isTriggered = false;
                            LPM_StopCheckTimer();
                            VIZN_StartRecognition(NULL);
                        }
                        break;

                    case LPM_BleRemoteTransmit:
                    case LPM_WifiRemoteTransmit:
                    {
                        if (isTriggered && pQMsg->msg.cmd.data.control_status)
                        {
                            LPM_ResetCheckTimer();
                        }
                    }
                    break;

                    default:
                        break;
                }
            }
            else
            {
                LOGD("[Lpm]: QMsg type not support.\r\n");
            }
        }

        vPortFree(pQMsg);
    }
}

static int LPM_SendQMsg(void *msg)
{
    BaseType_t ret;

    ret = xQueueSend(s_LpmMsgQ, msg, (TickType_t)0);

    if (ret != pdPASS)
    {
        LOGE("[ERROR]:Lpm_SendQMsg failed\r\n");
        return -1;
    }

    return 0;
}

int LPM_SendControlStatus(lpm_control_type_t id, uint16_t status)
{
    QMsg *pQMsg                        = (QMsg *)pvPortMalloc(sizeof(QMsg));

    if (NULL == pQMsg)
    {
        LOGE("[ERROR]: lpm pQMsg pvPortMalloc failed\r\n");
        return -1;
    }
    pQMsg->id                          = QMSG_LPM;
    pQMsg->msg.cmd.id                  = id;
    pQMsg->msg.cmd.data.control_status = status;
    return LPM_SendQMsg((void *)&pQMsg);
}

int LPM_Start(void)
{
    s_LpmMsgQ = xQueueCreate(LMP_MSG_Q_COUNT, sizeof(QMsg *));

    if (s_LpmMsgQ == NULL)
    {
        LOGE("[ERROR]:xQueueCreate lpm queue\r\n");
        return -1;
    }
    s_LpmCheckTimer = xTimerCreate("LpmCheckTimer", pdMS_TO_TICKS(MS_LPM_CHECK), pdFALSE, (void *)&s_TimerLpmCheckID,
                                   LPM_TimerCallback);

    if (s_LpmCheckTimer == NULL)
    {
        LOGE("[ERROR]:xTimerCreate lpm check timer\r\n");
        return -1;
    }

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (NULL == xTaskCreateStatic(LPM_Task, "Lpm Task", LPMTASK_STACKSIZE, NULL, LPMTASK_PRIORITY, s_LpmTaskStack,
                                  &s_LpmTaskTCB))
#else
    if (xTaskCreate(LPM_Task, "Lpm Task", LPMTASK_STACKSIZE, NULL, LPMTASK_PRIORITY, NULL) != pdPASS)
#endif
    {
        LOGE("[ERROR]:Lpm Task created failed\r\n");

        while (1)
            ;
    }
    LOGD("[Lpm]:started\r\n");

    return 0;
}
