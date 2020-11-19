/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "fsl_debug_console.h"
#include "serial_port_usb.h"
#include "board.h"
#include "usb_phy.h"
#include "clock_config.h"
#include "pin_mux.h"

void USB_DeviceClockInit(void);

#define CONTROLLER_ID kSerialManager_UsbControllerEhci0

void BOARD_InitHardware(void)
{
    /* Exit doze and stop mode after wakeup from suspend mode, otherwise peripherals(refer RM) can not work. */
    IOMUXC_GPR->GPR4  = 0x00000000;
    IOMUXC_GPR->GPR7  = 0x00000000;
    IOMUXC_GPR->GPR8  = 0x00000000;
    IOMUXC_GPR->GPR12 = 0x00000000;

    BOARD_ConfigMPU();
    // BOARD_ConfigUSBMPU();
    BOARD_BootClockRUN();
    BOARD_InitPins();
    BOARD_InitFlash();
    BOARD_InitBluetooth();
    BOARD_InitWifi();
    BOARD_InitAudio();
}

void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
#endif
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    if (CONTROLLER_ID == kSerialManager_UsbControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
#endif
}

void SysTick_DelayTicks(uint32_t n)
{
    vTaskDelay(n * 2);
}
