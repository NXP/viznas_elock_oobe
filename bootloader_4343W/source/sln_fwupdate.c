/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "sln_fwupdate.h"
#include "comms_message_handler.h"
#include "fsl_debug_console.h"

#if COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTW_UART
void otwAppStart()
{
    PRINTF("Starting the OTW server\r\n");
    /* Flush the console before we deinitialize it */
    DbgConsole_Flush();
    BOARD_DeInitDebugConsole();

    SLN_COMMS_MESSAGE_Init(kCommonInterfaceUart);
}
#endif

#if COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTA_TCP
void otaAppStart()
{
    SLN_COMMS_MESSAGE_Init(kCommonInterfaceTcp);
}
#endif
