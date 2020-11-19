/*
 * Copyright (c) 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_dcp.h"
#include "app.h"
#include "board.h"
#include "pin_mux.h"
#include "FreeRTOS.h"
#include "task.h"

#include "camera.h"
#include "database_api.h"
#include "display.h"
#include "oasis.h"
#include "switch.h"
#include "pxp.h"

#if WIFI_ENABLE
#include "sln_connection.h"
#include "sln_tcp_connection.h"
#endif
#include "sln_lpm.h"
#include "sln_api_init.h"
#include "sln_pcal.h"
#include "sln_shell.h"
#include "sln_flash.h"
#include "sln_flash_mgmt.h"
#include "sln_file_table.h"
#include "sln_system_state.h"
#include "sln_dev_cfg.h"
#include "sln_usb_composite.h"
#include "sln_wdog.h"
#include "sln_audio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void FileSystem_Init();

/*******************************************************************************
 * Code
 ******************************************************************************/
static void FileSystem_Init()
{
    /* Initialize DCP to enable CRC generation */
    dcp_config_t dcpConfig;
    DCP_GetDefaultConfig(&dcpConfig);
    DCP_Init(DCP, &dcpConfig);

    /* Initialize Flash to allow writing */
    SLN_Flash_Init();
    /* Initialize flash management */
    SLN_FLASH_MGMT_Init((sln_flash_entry_t *)g_fileTable, false);

    /* Initialize the user application config data */
    Cfg_AppDataInit();
}

/*!
 * @brief Main function
 */
int main(void)
{
#if defined(RELOCATE_VECTOR_TABLE) && RELOCATE_VECTOR_TABLE
    BOARD_RelocateVectorTableToRam();
#endif

    /* Init board hardware. */
    BOARD_InitHardware();

    FileSystem_Init();
    SysState_Init();

    uart_shell_init();
    USB_CompositeInit();

#if WIFI_ENABLE
    SLN_Connections_Init();
    TCP_Connection_Start();
#endif
    DB_Init();
    Camera_Start();
    APP_PXP_Start();
    Display_Start();
    Oasis_Start();
    Switch_Start();
#if VOICE_PROMPT
    Audio_Start();
#endif
    LPM_Start();
    vizn_api_init();
#if defined(WDOG_ENABLE) && WDOG_ENABLE
    SLN_WatchdogEnable(SLN_WDOG_TIMEOUT_SEC, SLN_WDOG_REFRESH_SEC);
#endif

    /* Run RTOS */
    vTaskStartScheduler();

    /* Should not reach this statement */
    while (1)
    {
    }
}
