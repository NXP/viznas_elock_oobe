/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "oasis.h"
#include "database.h"
#include "sln_cli.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "sln_shell.h"
#include "queue.h"
#include "sln_dev_cfg.h"
#include "sln_api.h"
#include "sln_connection.h"
#include "face_rec_rt_info.h"
/*******************************************************************************
 * Prototypes
 *******************************************************************************/

static shell_status_t FFI_CLI_ListCommand(shell_handle_t shellContextHandle,
                                          int32_t argc,
                                          char **argv); /*!< list command */

static shell_status_t FFI_CLI_AddCommand(shell_handle_t shellContextHandle,
                                         int32_t argc,
                                         char **argv); /*!< add command */

static shell_status_t FFI_CLI_DelCommand(shell_handle_t shellContextHandle,
                                         int32_t argc,
                                         char **argv); /*!< del command */

static shell_status_t FFI_CLI_RenCommand(shell_handle_t shellContextHandle,
                                         int32_t argc,
                                         char **argv); /*!< ren command */

static shell_status_t FFI_CLI_Verbose(shell_handle_t shellContextHandle,
                                      int32_t argc,
                                      char **argv); /*!< verbose command */

static shell_status_t FFI_CLI_SaveCommand(shell_handle_t shellContextHandle,
                                          int32_t argc,
                                          char **argv); /*!< save command */

static shell_status_t FFI_CLI_ResetCommand(shell_handle_t shellContextHandle,
                                           int32_t argc,
                                           char **argv); /*!< reset command */

static shell_status_t FFI_CLI_CameraCommand(shell_handle_t shellContextHandle,
                                            int32_t argc,
                                            char **argv); /*!< camera command */

static shell_status_t FFI_CLI_VersionCommand(shell_handle_t shellContextHandle,
                                             int32_t argc,
                                             char **argv); /*!< version command */

static shell_status_t FFI_CLI_FwUpdateOTW(shell_handle_t shellContextHandle,
                                          int32_t argc,
                                          char **argv); /*!< fwupdate-otw command */

static shell_status_t FFI_CLI_EmotionTypes(shell_handle_t shellContextHandle,
                                           int32_t argc,
                                           char **argv); /*!< emotion types command */

static shell_status_t FFI_CLI_LivenessCommand(shell_handle_t shellContextHandle,
                                              int32_t argc,
                                              char **argv); /*!< liveness command */

static shell_status_t FFI_CLI_DetResolutionCommand(shell_handle_t shellContextHandle,
                                                   int32_t argc,
                                                   char **argv); /*!< det resolution command */

static shell_status_t FFI_CLI_DisplayCommand(shell_handle_t shellContextHandle,
                                             int32_t argc,
                                             char **argv); /*!< display command */

static shell_status_t FFI_CLI_WiFiCommand(shell_handle_t shellContextHandle,
                                          int32_t argc,
                                          char **argv); /*!< wi-fi command */

static shell_status_t FFI_CLI_AppCommand(shell_handle_t shellContextHandle,
                                         int32_t argc,
                                         char **argv); /*!< App command */

static shell_status_t FFI_CLI_LowPowerCommand(shell_handle_t shellContextHandle,
                                              int32_t argc,
                                              char **argv); /*!< Low Power command */

static shell_status_t FFI_CLI_AlgoStartCommand(shell_handle_t shellContextHandle,
                                              int32_t argc,
                                              char **argv); /*!< Algo Start command */
static shell_status_t _RtInfoCommand(shell_handle_t shellContextHandle,
                                              int32_t argc,
                                              char **argv); /*!< RtInfo command */

extern "C" {

shell_status_t SHELL_HelpCommand(shell_handle_t shellContextHandle,

                                 int32_t argc,
                                 char **argv); /*!< help command */

shell_status_t SHELL_ExitCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv); /*!< exit command */
}

/*******************************************************************************
 * Variables
 *******************************************************************************/
SHELL_COMMAND_DEFINE(list,
                     (char *)"\r\n\"list\": List all registered users\r\n",
                     FFI_CLI_ListCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(add, (char *)"\r\n\"add username\": Add user\r\n"
                                  "\r\n\"add -s\": Stop the former add command issued\r\n", FFI_CLI_AddCommand, SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(del, (char *)"\r\n\"del username\": Delete user \r\n"
                                  "\r\n\"del -a\": Delete all users \r\n", FFI_CLI_DelCommand, SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(rename,
                     (char *)"\r\n\"rename oldusername newusername\": Rename user\r\n",
                     FFI_CLI_RenCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(verbose,
                     (char *)"\r\n\"verbose <0|1|2|3> (none-all)\": Debug message detail level setting\r\n",
                     FFI_CLI_Verbose,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(save,
                     (char *)"\r\n\"save n\": Save database into flash memory\r\n",
                     FFI_CLI_SaveCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(reset,
                     (char *)"\r\n\"reset\": Reset the MCU\r\n",
                     FFI_CLI_ResetCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(detection,
                     (char *)"\r\n\"detection resolution <qvga|vga>\": Set detection resolution and reset\r\n",
                     FFI_CLI_DetResolutionCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(camera,
                     (char *)"\r\n\"camera ir_pwm <value>\": PWM pulse width for IR led, value should be between 0 (inactive) to 100\r\n"
                             "\r\n\"camera white_pwm <value>\": PWM pulse width for white led, value should be between 0 (inactive) to 100\r\n",
                     FFI_CLI_CameraCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(version,
                     (char *)"\r\n\"version\": The version information\r\n",
                     FFI_CLI_VersionCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(updateotw,
                     (char *)"\r\n\"updateotw\": Reboot the board and start the OTW firmware update\r\n",
                     FFI_CLI_FwUpdateOTW,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(emotion,
                     (char *)"\r\n\"emotion <0|2|4|7>\"\r\n",
                     FFI_CLI_EmotionTypes,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(liveness,
                     (char *)"\r\n\"liveness <on|off>\"\r\n",
                     FFI_CLI_LivenessCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(display, (char *)"\r\n\"display output_mode <rgb|ir>\"\r\n"
                                      "\r\n\"display output_device <usb|lcd>\"\r\n"
                                      "\r\n\"display interface <loopback|infobar>\"\r\n", FFI_CLI_DisplayCommand, SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(wifi, (char *)"\r\n\"wifi <on|off>\":  Turn the Wi-Fi connection on|off\r\n"
									"\r\n\"wifi reset\": Reset wifi to re-link. \r\n"
        							  "\r\n\"wifi credentials\": Get the credentials. \r\n"
        							  "\r\n\"wifi credentials SSID [PASSWORD]\": Set Wi-Fi credentials.\r\n"
                                      "\r\n\"wifi ip\": Get the ip address. \r\n"
                                      "\r\n\"wifi erase\": Erase the Wi-Fi credentials from flash\r\n", FFI_CLI_WiFiCommand, SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(app_type, (char *)"\r\n\"app_type <0|1|2|3|4|5|6|7|8>\":\r\n "
                                        "0 - Elock(light)dual\r\n"
                                        "1 - Elock(heavy)dual\r\n"
                                        "2 - Door access(light)dual\r\n"
                                        "3 - Door access(heavy)dual\r\n"
                                        "4 - Elock(light)single\r\n"
                                        "5 - Elock(heavy)single\r\n"
                                        "6 - Door access(light)single\r\n"
                                        "7 - Door access(heavy)single\r\n"
                                        "8 - Userid \r\n", FFI_CLI_AppCommand, SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(low_power, (char *)"\r\n\"low_power <on|off>\":  Turn low power mode on|off\r\n", FFI_CLI_LowPowerCommand, SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(algo_start, (char *)"\r\n\"algo_start <auto|manual>\":  Set algo start mode auto|manual\r\n", FFI_CLI_AlgoStartCommand, SHELL_IGNORE_PARAMETER_COUNT);

SHELL_COMMAND_DEFINE(rtinfo, (char *)"\r\n\"rtinfo\": runtime information filter\r\n", _RtInfoCommand, SHELL_IGNORE_PARAMETER_COUNT);

extern QueueHandle_t g_UsbShellQueue;
extern VIZN_api_handle_t gApiHandle;
extern VIZN_api_client_t VIZN_API_CLIENT(Shell);
//extern std::string g_AddNewFaceName;

/*******************************************************************************
 * Code
 *******************************************************************************/

shell_status_t RegisterFFICmds(shell_handle_t shellContextHandle)
{
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(list));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(add));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(del));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(rename));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(verbose));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(camera));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(version));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(save));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(updateotw));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(reset));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(emotion));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(liveness));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(detection));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(display));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(wifi));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(app_type));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(low_power));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(algo_start));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(rtinfo));
    return kStatus_SHELL_Success;
}

static shell_status_t UsbShell_QueueSendFromISR(shell_handle_t shellContextHandle,
                                                int32_t argc,
                                                char **argv,
                                                char shellCommand)
{
    UsbShellCmdQueue_t queueMsg;

    if (argc > USB_SHELL_PARAMS_COUNT)
    {
        SHELL_Printf(shellContextHandle, "Parameters count overflow\r\n");
        return kStatus_SHELL_Error;
    }

    for (int i = 0; i < argc; i++)
    {
        if (strlen(argv[i]) < USB_SHELL_PARAMS_SIZE)
        {
            strcpy(queueMsg.argv[i], argv[i]);
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Parameter '%s' overflow\r\n", argv[i]);
            return kStatus_SHELL_Error;
        }
    }
    queueMsg.argc               = argc;
    queueMsg.shellContextHandle = shellContextHandle;
    queueMsg.shellCommand       = shellCommand;

    if (pdTRUE != xQueueSendFromISR(g_UsbShellQueue, (void *)&queueMsg, NULL))
    {
        SHELL_Printf(shellContextHandle, "ERROR: Cannot send command to shell processing queue\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t FFI_CLI_ListCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc > 1)
    {
        SHELL_Printf(shellContextHandle, "Wrong parameters\r\n");
        return kStatus_SHELL_Error;
    }

    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_LIST);
}

static shell_status_t FFI_CLI_AddCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc != 2)
    {
        SHELL_Printf(shellContextHandle, "Wrong parameters\r\n");
        return kStatus_SHELL_Error;
    }

    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_ADD);
}

static shell_status_t FFI_CLI_DelCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc != 2)
    {
        SHELL_Printf(shellContextHandle, "Wrong parameters\r\n");
        return kStatus_SHELL_Error;
    }

    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_DEL);
}

static shell_status_t FFI_CLI_RenCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc != 3)
    {
        SHELL_Printf(shellContextHandle, "Wrong parameters\r\n");
        return kStatus_SHELL_Error;
    }

    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_RENAME);
}

static shell_status_t FFI_CLI_Verbose(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Wrong parameters\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc == 1)
    {
        // List current mode
        SHELL_Printf(shellContextHandle, "verbose level: %d\r\n", Cfg_AppDataGetVerbosity());
        return kStatus_SHELL_Success;
    }
    else
    {
        return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_VERBOSE);
    }
}

static shell_status_t FFI_CLI_SaveCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Wrong parameters\r\n");
        return kStatus_SHELL_Error;
    }

    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_SAVE);
}

static shell_status_t FFI_CLI_ResetCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_RESET);
}

static shell_status_t FFI_CLI_DetResolutionCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if ((argc > 3) || (argc < 2))
    {
        SHELL_Printf(shellContextHandle, "Wrong parameters\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc == 2)
    {
        SHELL_Printf(shellContextHandle, "Current detection resolution mode: %s\r\n",
                     (DETECT_RESOLUTION_QVGA == Cfg_AppDataGetDetectResolutionMode()) ? "qvga" : "vga");
        return kStatus_SHELL_Success;
    }
    else
    {
        return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_DET_RESOLUTION);
    }
}

static shell_status_t FFI_CLI_CameraCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_CAMERA);
}

static shell_status_t FFI_CLI_VersionCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc != 1)
    {
        SHELL_Printf(shellContextHandle, "Version command has no arguments\r\n");
        return kStatus_SHELL_Error;
    }
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_VERSION);
}

static shell_status_t FFI_CLI_FwUpdateOTW(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_FWUPDATE_OTW);
}

static shell_status_t FFI_CLI_EmotionTypes(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_EMOTION_TYPES);
}

static shell_status_t FFI_CLI_LivenessCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_LIVENESS);
}

static shell_status_t FFI_CLI_DisplayCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc == 1)
    {
        SHELL_Printf(shellContextHandle, "Incomplete command for display\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc == 2)
    {
        if (!strcmp((char *)argv[1], "output_device"))
        {
            SHELL_Printf(shellContextHandle, "Current output device is: %s\r\n",
                         (DISPLAY_USB == Cfg_AppDataGetOutputMode()) ? "USB" : "LCD");
            return kStatus_SHELL_Success;
        }
        else if (!strcmp((char *)argv[1], "interface"))
        {
            SHELL_Printf(shellContextHandle, "Current interface  is: %s\r\n",
                         (DISPLAY_INTERFACE_INFOBAR == Cfg_AppDataGetInterfaceMode()) ? "Infobar" : "Loopback");
            return kStatus_SHELL_Success;
        }
        else if (!strcmp((char *)argv[1], "output_mode"))
        {
            SHELL_Printf(shellContextHandle, "Currently displaying output from %s camera\r\n",
                         (DISPLAY_MODE_RGB == Cfg_AppDataGetDisplayMode()) ? "RGB" : "IR");
            return kStatus_SHELL_Success;
        }
    }
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_DISPLAY);
}



static shell_status_t FFI_CLI_WiFiCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc == 1)
    {
        SHELL_Printf(shellContextHandle, "Incomplete command for wi-fi\r\n");
        return kStatus_SHELL_Error;
    }
    if (argc == 2)
    {
        if (!strcmp((char *)argv[1], "credentials"))
            return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_WIFI_CREDENTIALS);

        if (!strcmp((char *)argv[1], "ip"))
        {
            uint32_t ip_address;
            ip_address = SLN_Connection_WIFI_getIPAddress();

            SHELL_Printf(shellContextHandle, "IPv4 Address: %u.%u.%u.%u\r\n", ((u8_t *)&ip_address)[0],
                         ((u8_t *)&ip_address)[1], ((u8_t *)&ip_address)[2], ((u8_t *)&ip_address)[3]);
            return kStatus_SHELL_Error;
        }
        if (!strcmp((char *)argv[1], "erase"))
            return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_WIFI_ERASE);

        if (!strcmp((char *)argv[1], "on") || !strcmp((char *)argv[1], "off") || !strcmp((char *)argv[1], "reset"))
            return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_WIFI);
    }

    if (!strcmp((char *)argv[1], "credentials") && argc != 4)
    {
        SHELL_Printf(shellContextHandle, "Wrong command for wi-fi credentials\r\n");
        return kStatus_SHELL_Error;
    }
    else if (!strcmp((char *)argv[1], "credentials"))
    {
        return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_WIFI_CREDENTIALS);
    }

    SHELL_Printf(shellContextHandle, "Wrong command for wi-fi\r\n");
    return kStatus_SHELL_Error;
}

static shell_status_t FFI_CLI_AppCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_APP);
}

static shell_status_t FFI_CLI_LowPowerCommand(shell_handle_t shellContextHandle,
                                        int32_t argc,
                                        char **argv)
{
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_LOW_POWER);
}

static shell_status_t FFI_CLI_AlgoStartCommand(shell_handle_t shellContextHandle,
                                        int32_t argc,
                                        char **argv)
{
    return UsbShell_QueueSendFromISR(shellContextHandle, argc, argv, SHELL_EV_FFI_CLI_ALGO_START);
}


void UsbShell_CmdProcess_Task(void *arg)
{
    vTaskDelay(portTICK_PERIOD_MS * 1000);
    SHELL_Printf(usb_shellHandle[USB_SHELL_PROMPT_INDEX], "Type \"help\" to see what this shell can do!\r\n");
    SHELL_Printf(usb_shellHandle[USB_SHELL_PROMPT_INDEX], USB_SHELL_PROMPT);

    while (1)
    {
        vizn_api_status_t status;
        UsbShellCmdQueue_t queueMsg;
        xQueueReceive(g_UsbShellQueue, &queueMsg, portMAX_DELAY);

        shell_handle_t shellContextHandle = queueMsg.shellContextHandle;
        if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_LIST)
        {
            std::vector<std::string> names;
            uint32_t namescount;

            //get user name, maximum is 100
            names.reserve(100);
            VIZN_GetRegisteredUsers(&VIZN_API_CLIENT(Shell), names, names.capacity());
            namescount = names.size();

            SHELL_Printf(shellContextHandle, "Registered users count:%d\r\n", namescount);
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); it++)
            {
                SHELL_Printf(shellContextHandle, "%s\r\n", (*it).c_str());
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_ADD)
        {
            status = VIZN_AddUser(&VIZN_API_CLIENT(Shell), (char *)queueMsg.argv[1]);
            switch (status)
            {
                case kStatus_API_Layer_AddUser_NoAddCommand:
                    SHELL_Printf(shellContextHandle, "No add command registered\r\n");
                    break;
                case kStatus_API_Layer_AddUser_AddCommandStopped:
                    SHELL_Printf(shellContextHandle, "Stopped adding current user\r\n");
                    break;
                case kStatus_API_Layer_AddUser_InvalidUserName:
                    SHELL_Printf(shellContextHandle, "Invalid User Name\r\n");
                    break;
                case kStatus_API_Layer_AddUser_MultipleAddCommand:
                    SHELL_Printf(shellContextHandle,
                                 "Add command already in pending. Please wait for it complete or cancel it by add -s\r\n");
                    break;
                case kStatus_API_Layer_Success:
                    SHELL_Printf(shellContextHandle, "Start registering '%s'\r\n", queueMsg.argv[1]);
                    break;
                default:
                    SHELL_Printf(shellContextHandle, "ERROR API ENGINE\r\n");
                    break;
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_DEL)
        {
            if (0 == strcmp((char *)queueMsg.argv[1], "-a"))
            {
                status = VIZN_DelUser(&VIZN_API_CLIENT(Shell));
                if (status == kStatus_API_Layer_DelUser_FailedDeleteAll)
                {
                    SHELL_Printf(shellContextHandle, "Failed deleting all users\r\n");
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "All users successfuly deleted\r\n");
                }
            }
            else
            {
                status = VIZN_DelUser(&VIZN_API_CLIENT(Shell), (char *)queueMsg.argv[1]);
                switch (status)
                {
                    case kStatus_API_Layer_DelUser_InvalidUserName:
                        SHELL_Printf(shellContextHandle, "Invalid User Name\r\n");
                        break;
                    case kStatus_API_Layer_DelUser_FailedDelete:
                        SHELL_Printf(shellContextHandle, "Failed deleting user '%s'\r\n", queueMsg.argv[1]);
                        break;
                    case kStatus_API_Layer_Success:
                        SHELL_Printf(shellContextHandle, "User '%s' successfuly deleted\r\n", queueMsg.argv[1]);
                        break;
                    default:
                        SHELL_Printf(shellContextHandle, "ERROR API ENGINE\r\n");
                        break;
                }
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_RENAME)
        {
            status = VIZN_RenameUser(&VIZN_API_CLIENT(Shell), (char *)queueMsg.argv[1], (char *)queueMsg.argv[2]);
            switch (status)
            {
                case kStatus_API_Layer_RenameUser_InvalidUserName:
                    SHELL_Printf(shellContextHandle, "Invalid User Name\r\n");
                    break;
                case kStatus_API_Layer_RenameUser_AlreadyExists:
                    SHELL_Printf(shellContextHandle, "Failed renaming, user '%s' already exists\r\n", queueMsg.argv[2]);
                    break;
                case kStatus_API_Layer_RenameUser_FailedRename:
                    SHELL_Printf(shellContextHandle, "Failed renaming user '%s'\r\n", queueMsg.argv[1]);
                    break;
                case kStatus_API_Layer_Success:
                    SHELL_Printf(shellContextHandle, "User '%s' successfully renamed\r\n", queueMsg.argv[1]);
                    break;
                default:
                    SHELL_Printf(shellContextHandle, "ERROR API ENGINE\r\n");
                    break;
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_VERBOSE)
        {
            int verb_lvl = atoi(queueMsg.argv[1]);
            if (isdigit(queueMsg.argv[1][0]) && verb_lvl >= VERBOSE_MODE_OFF && verb_lvl <= VERBOSE_MODE_L3)
            {
                status = VIZN_SetVerboseMode(&VIZN_API_CLIENT(Shell), (cfg_verbose_t)verb_lvl);
                if (status == kStatus_API_Layer_Success)
                {
                    SHELL_Printf(shellContextHandle, "Verbose level set to %d\r\n", verb_lvl);
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Cannot save verbose level\r\n");
                }
            }
            else
            {
                SHELL_Printf(shellContextHandle, "Unsupported level\r\n");
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_SAVE)
        {
            if (queueMsg.argc == 1)
            {
                uint32_t time_tick = Time_Now();
                SHELL_Printf(shellContextHandle, "all sdram database will save into flash database\r\n");
                VIZN_SaveDB(&VIZN_API_CLIENT(Shell), 0);
                SHELL_Printf(shellContextHandle, "Database save success. Time taken in ms %d\r\n",
                             (time_tick - Time_Now()) * 1000 / PIT_TIMER_UNIT);
            }
            else if (queueMsg.argc == 2)
            {
                char *pEnd;
                int list_length = (int)strtol(queueMsg.argv[1], &pEnd, 10);

                if (pEnd == queueMsg.argv[1])
                {
                    SHELL_Printf(shellContextHandle, "\"%s\" not a valid number \r\n", queueMsg.argv[1]);
                }
                else
                {
                    uint32_t time_tick = Time_Now();
                    SHELL_Printf(shellContextHandle, "%d list will be save into flash database\r\n", list_length);
                    VIZN_SaveDB(&VIZN_API_CLIENT(Shell), list_length);
                    SHELL_Printf(shellContextHandle, "Database save success. Time taken in ms %d\r\n",
                                 (time_tick - Time_Now()) * 1000 / PIT_TIMER_UNIT);
                }
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_RESET)
        {
            SHELL_Printf(shellContextHandle, "System will reset\r\n");
            VIZN_SystemReset(&VIZN_API_CLIENT(Shell));
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_DET_RESOLUTION)
        {
            uint32_t status;
            if (strcmp((char *)queueMsg.argv[1], "resolution"))
            {
                SHELL_Printf(shellContextHandle, "wrong command\r\n");
            }
            else
            {
                if (!strcmp((char *)queueMsg.argv[2], "qvga"))
                {
                    status = VIZN_SetDetResolutionMode(&VIZN_API_CLIENT(Shell), DETECT_RESOLUTION_QVGA);
                    if (status == kStatus_API_Layer_Success)
                    {
                        SHELL_Printf(shellContextHandle,
                                     "Detection resolution QVGA mode selected, system will reset\r\n");
                        VIZN_SystemReset(&VIZN_API_CLIENT(Shell));
                    }
                    else if (status == kStatus_API_Layer_SetDetResolutionMode_Same)
                    {
                        SHELL_Printf(shellContextHandle, "Detection resolution is already QVGA mode\r\n");
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Cannot set detection resolution QVGA mode\r\n");
                    }
                }
                else if (!strcmp((char *)queueMsg.argv[2], "vga"))
                {
                    status = VIZN_SetDetResolutionMode(&VIZN_API_CLIENT(Shell), DETECT_RESOLUTION_VGA);
                    if (status == kStatus_API_Layer_Success)
                    {
                        SHELL_Printf(shellContextHandle,
                                     "Detection resolution VGA mode selected, system will reset\r\n");
                        VIZN_SystemReset(&VIZN_API_CLIENT(Shell));
                    }
                    else if (status == kStatus_API_Layer_SetDetResolutionMode_Same)
                    {
                        SHELL_Printf(shellContextHandle, "Detection resolution is already VGA mode\r\n");
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Cannot set detection resolution VGA mode\\r\n");
                    }
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Undefined mode\r\n");
                }
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_CAMERA)
        {
            if (!strcmp((char *)queueMsg.argv[1], "ir_pwm") || !strcmp((char *)queueMsg.argv[1], "white_pwm"))
            {
                uint8_t pulse_width;
                if (queueMsg.argc == 2)
                {
                    if (!strcmp((char *)queueMsg.argv[1], "ir_pwm"))
                    {
                        status = VIZN_GetPulseWidth(&VIZN_API_CLIENT(Shell), LED_IR, &pulse_width);
                        if (status == kStatus_API_Layer_Success)
                        {
                            SHELL_Printf(shellContextHandle, "Camera IR duty cycle %d%\r\n", pulse_width);
                        }
                        else
                        {
                            SHELL_Printf(shellContextHandle, "Camera IR PWM not supported\r\n");
                        }
                    }
                    else
                    {
                        status = VIZN_GetPulseWidth(&VIZN_API_CLIENT(Shell), LED_WHITE, &pulse_width);
                        if (status == kStatus_API_Layer_Success)
                        {
                            SHELL_Printf(shellContextHandle, "Camera white duty cycle %d%\r\n", pulse_width);
                        }
                        else
                        {
                            SHELL_Printf(shellContextHandle, "Camera white PWM not supported\r\n");
                        }
                    }
                }
                else
                {
                    char *pEnd;
                    pulse_width = (uint8_t)strtol(queueMsg.argv[2], &pEnd, 10);
                    if (pEnd == queueMsg.argv[2])
                    {
                        SHELL_Printf(shellContextHandle, "\"%s\" not a valid number \r\n", queueMsg.argv[2]);
                    }
                    else
                    {
                        if (!strcmp((char *)queueMsg.argv[1], "ir_pwm"))
                            status = VIZN_SetPulseWidth(&VIZN_API_CLIENT(Shell), LED_IR, (uint8_t)pulse_width);
                        else
                            status = VIZN_SetPulseWidth(&VIZN_API_CLIENT(Shell), LED_WHITE, (uint8_t)pulse_width);
                        switch (status)
                        {
                            case kStatus_API_Layer_Success:
                                SHELL_Printf(shellContextHandle, "Camera PWM duty cycle set to %d%\r\n", pulse_width);
                                break;
                            case kStatus_API_Layer_SetCameraPulseWidth_SaveFailed:
                                SHELL_Printf(shellContextHandle, "Cannot save PWM duty cycle\r\n");
                                break;
                            case kStatus_API_Layer_SetCameraPulseWidth_NotSupported:
                                SHELL_Printf(shellContextHandle, "Camera PWM duty cycle not supported\r\n");
                                break;
                            case kStatus_API_Layer_SetCameraPulseWidth_WrongValue:
                                SHELL_Printf(shellContextHandle, "Camera PWM duty cycle wrong value\r\n");
                                break;
                            default:
                                SHELL_Printf(shellContextHandle, "ERROR API ENGINE\r\n");
                                break;
                        }
                    }
                }
            }
            else
            {
                SHELL_Printf(shellContextHandle, "Undefined command for the camera\r\n");
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_VERSION)
        {
            SHELL_Printf(shellContextHandle, "Firmware Version:%d.%d.%d Bank:%s\r\n", APP_MAJ_VER, APP_MIN_VER, APP_BLD_VER, getAppBank());
            SHELL_Printf(shellContextHandle, "OASIS LITE:v%d.%d.%d\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_HOTFIX);
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_FWUPDATE_OTW)
        {
            /*
             * Clear the FWUPDATE bit indicator in flash memory and reset the board
             * in order to start the FWUPDATE OTW
             */
        	uint32_t status = VIZN_FWUpdate_Set(&VIZN_API_CLIENT(Shell), FWUPDATE_OTW);
        	if(status == kStatus_API_Layer_Success)
            {
                SHELL_Printf(shellContextHandle, "The OTW firmware update will be started\r\n");
            }
            else
            {
                SHELL_Printf(shellContextHandle, "Cannot start the OTW firmware update\r\n");
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_EMOTION_TYPES)
        {
        #if 0
        	unsigned char dummy_feature[] = {
        			0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x2,0xF2,0xFB,0x1,0x1,0xEC,0xC4,0xE4,0xDB,0x12,0x12,0x2F,0xEA,0x6,0x38,
        			0xE7,0xF1,0x23,0xE,0xCD,0xEF,0xF5,0x2,0xDE,0xF,0x29,0xE3,0x5,0xE3,0x11,0x15,0x10,0xE2,0xFB,0xEF,0x9,0x2D,0x1E,0xF0,0xE5,0xED,0x6,0xF8,0xF1,0xF3,0x13,0x21,
        			0x0,0xD1,0x2D,0xA,0xCB,0x1A,0x18,0xED,0x16,0xB,0xFD,0xD4,0x10,0xE6,0xD7,0xE,0xEA,0xF7,0xDC,0xC,0xC4,0xF4,0x18,0x7,0x2C,0x1D,0x10,0xDB,0x20,0x14,0xE7,0x14,
        			0xEB,0xF5,0x23,0xF0,0xFD,0xFD,0xF5,0xF2,0x2B,0x6,0x25,0x1,0xEA,0x2A,0xF1,0xFD,0xEA,0xF3,0xCF,0x9,0x24,0x16,0xE,0x24,0xF9,0xD6,0x1E,0xFA,0x2B,0x39,0x2B,0xE9,
        			0xF6,0x8,0xFD,0x7,0xD7,0x1E,0x10,0xEA,0xF,0xEF,0x1B,0xF0,0xA,0x3,0xDF,0xCB,0x1,0x2,0xF2,0xFB,0x1,0x1,0xEC,0xC4,0xE4,0xDB,0x12,0x12,0x2F,0xEA,0x6,0x38,
        			0xE7,0xF1,0x23,0xE,0xCD,0xEF,0xF5,0x2,0xDE,0xF,0x29,0xE3,0x5,0xE3,0x11,0x15,0x10,0xE2,0xFB,0xEF,0x9,0x2D,0x1E,0xF0,0xE5,0xED,0x6,0xF8,0xF1,0xF3,0x13,0x21,
        			0x0,0xD1,0x2D,0xA,0xCB,0x1A,0x18,0xED,0x16,0xB,0xFD,0xD4,0x10,0xE6,0xD7,0xE,0xEA,0xF7,0xDC,0xC,0xC4,0xF4,0x18,0x7,0x2C,0x1D,0x10,0xDB,0x20,0x14,0xE7,0x14,
        			0xEB,0xF5,0x23,0xF0,0xFD,0xFD,0xF5,0xF2,0x2B,0x6,0x25,0x1,0xEA,0x2A,0xF1,0xFD,0xEA,0xF3,0xCF,0x9,0x24,0x16,0xE,0x24,0xF9,0xD6,0x1E,0xFA,0x2B,0x39,0x2B,0xE9,
        			0xF6,0x8,0xFD,0x7,0xD7,0x1E,0x10,0xEA,0xF,0xEF,0x1B,0xF0,0xA,0x3,0xDF,0xCB,0x1,0x2,0xF2,0xFB,0x1,0x1,0xEC,0xC4,0xE4,0xDB,0x12,0x12,0x2F,0xEA,0x6,0x38,
        			0xE7,0xF1,0x23,0xE,0xCD,0xEF,0xF5,0x2,0xDE,0xF,0x29,0xE3,0x5,0xE3,0x11,0x15,0x10,0xE2,0xFB,0xEF,0x9,0x2D,0x1E,0xF0,0xE5,0xED,0x6,0xF8,0xF1,0xF3,0x13,0x21,
        			0x0,0xD1,0x2D,0xA,0xCB,0x1A,0x18,0xED,0x16,0xB,0xFD,0xD4,0x10,0xE6,0xD7,0xE,0xEA,0xF7,0xDC,0xC,0xC4,0xF4,0x18,0x7,0x2C,0x1D,0x10,0xDB,0x20,0x14,0xE7,0x14,
        			0xEB,0xF5,0x23,0xF0,0xFD,0xFD,0xF5,0xF2,0x2B,0x6,0x25,0x1,0xEA,0x2A,0xF1,0xFD,0xEA,0xF3,0xCF,0x9,0x24,0x16,0xE,0x24,0xF9,0xD6,0x1E,0xFA,0x2B,0x39,0x2B,0xE9,
        			0xF6,0x8,0xFD,0x7,0xD7,0x1E,0x10,0xEA,0xF,0xEF,0x1B,0xF0,0xA,0x3,0xDF,0xCB,

        	};

        	if (rand()&0x1)
        	{
        		SHELL_Printf(shellContextHandle,"Generate random feature\r\n");
				for (int i = 16;i<sizeof(dummy_feature);i++)
				{
					dummy_feature[i] += rand()%255;
				}
        	}else
        	{
        		SHELL_Printf(shellContextHandle,"Use default feature\r\n");
        	}

            QMsg *pQMsg = (QMsg*)pvPortMalloc(sizeof(QMsg));
            if (NULL == pQMsg)
            {
            	SHELL_Printf(shellContextHandle,"[ERROR]: AddFace pQMsg pvPortMalloc failed\r\n");
                return;
            }
            pQMsg->id = QMSG_FACEREC_ADDNEWFACEBY_FEA;
            pQMsg->msg.cmd.data.add_face.new_face_name[0] = rand()%26 + 'A';
            pQMsg->msg.cmd.data.add_face.new_face_name[1] = rand()%26 + 'A';
            pQMsg->msg.cmd.data.add_face.new_face_name[2] = rand()%26 + 'A';
            pQMsg->msg.cmd.data.add_face.new_face_name[3] = 0;
            pQMsg->msg.cmd.data.add_face.feature = pvPortMalloc(sizeof(dummy_feature));
            memcpy(pQMsg->msg.cmd.data.add_face.feature,dummy_feature,sizeof(dummy_feature));
            SHELL_Printf(shellContextHandle,"add face by feature name:%s\r\n",pQMsg->msg.cmd.data.add_face.new_face_name);
            Oasis_SendQMsg((void *)&pQMsg);
            #endif

        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_LIVENESS)
        {
            if (queueMsg.argc == 1)
            {
                cfg_liveness_t cur_mode;
                uint32_t status = VIZN_GetLivenessMode(&VIZN_API_CLIENT(Shell), &cur_mode);
                if (status == kStatus_API_Layer_Success)
                {
                    SHELL_Printf(shellContextHandle, "Liveness mode %s\r\n",
                                 (cur_mode == LIVENESS_MODE_ON) ? "on" : "off");
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Liveness detection not supported\r\r");
                }
            }
            else if (queueMsg.argc == 2)
            {
                if (!strcmp((char *)queueMsg.argv[1], "on"))
                {
                    status = VIZN_SetLivenessMode(&VIZN_API_CLIENT(Shell), LIVENESS_MODE_ON);
                    if (status == kStatus_API_Layer_Success)
                    {
                        SHELL_Printf(shellContextHandle, "Liveness mode enabled, system will reset.\r\n");
                        VIZN_SystemReset(&VIZN_API_CLIENT(Shell));
                    }
                    else if (status == kStatus_API_Layer_SetLivenessMode_Same)
                    {
                        SHELL_Printf(shellContextHandle, "Liveness mode already enabled.\r\n");
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Cannot save liveness mode\r\n");
                    }
                }
                else if (!strcmp((char *)queueMsg.argv[1], "off"))
                {
                    status = VIZN_SetLivenessMode(&VIZN_API_CLIENT(Shell), LIVENESS_MODE_OFF);
                    if (status == kStatus_API_Layer_Success)
                    {
                        SHELL_Printf(shellContextHandle, "Liveness mode disabled, system will reset.\r\n");
                        VIZN_SystemReset(&VIZN_API_CLIENT(Shell));
                    }
                    else if (status == kStatus_API_Layer_SetLivenessMode_Same)
                    {
                        SHELL_Printf(shellContextHandle, "Liveness mode already disabled.\r\n");
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Cannot save liveness mode\r\n");
                    }
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Undefined mode\r\n");
                }
            }
            else
            {
                SHELL_Printf(shellContextHandle, "Invalid command for liveness\r\n");
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_DISPLAY)
        {
            uint32_t status;
            if (queueMsg.argc == 3)
            {
                if (!strcmp((char *)queueMsg.argv[1], "output_mode"))
                {
                    if (!strcmp((char *)queueMsg.argv[2], "rgb"))
                    {
                        status = VIZN_SetDispMode(&VIZN_API_CLIENT(Shell), DISPLAY_MODE_RGB);
                        if (status == kStatus_API_Layer_Success)
                        {
                            SHELL_Printf(shellContextHandle, "display rgb frame\r\n");
                        }
                        else
                        {
                            SHELL_Printf(shellContextHandle, "display mode not supported\r\n");
                        }
                    }
                    else if (!strcmp((char *)queueMsg.argv[2], "ir"))
                    {
                        status = VIZN_SetDispMode(&VIZN_API_CLIENT(Shell), DISPLAY_MODE_IR);
                        if (status == kStatus_API_Layer_Success)
                        {
                            SHELL_Printf(shellContextHandle, "display ir frame\r\n");
                        }
                        else
                        {
                            SHELL_Printf(shellContextHandle, "display mode not supported\r\n");
                        }
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Undefined mode\r\n");
                    }
                }
                else if (!strcmp((char *)queueMsg.argv[1], "output_device"))
                {
                    if (!strcmp((char *)queueMsg.argv[2], "usb"))
                    {
                        status = VIZN_SetDispOutputDevice(&VIZN_API_CLIENT(Shell), DISPLAY_USB);
                    }
                    else if (!strcmp((char *)queueMsg.argv[2], "lcd"))
                    {
                        status = VIZN_SetDispOutputDevice(&VIZN_API_CLIENT(Shell), DISPLAY_LCD);
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Wrong output device\r\n");
                        SHELL_Printf(shellContextHandle, USB_SHELL_PROMPT);
                        continue;
                    }

                    switch (status)
                    {
                        case kStatus_API_Layer_SetDispOutputDevice_Same:
                            SHELL_Printf(shellContextHandle, "Display output mode is the same \r\n");
                            break;
                        case kStatus_API_Layer_SetDispOutputDevice_SaveFailed:
                            SHELL_Printf(shellContextHandle, "Failed to save the output mode \r\n");
                            break;
                        case kStatus_API_Layer_Success:
                            SHELL_Printf(shellContextHandle, "Display output mode set to %s, system will reset\r\n",
                                         (char *)queueMsg.argv[2]);
                            VIZN_SystemReset(&VIZN_API_CLIENT(Shell));
                            break;
                        case kStatus_API_Layer_SetDispOutputDevice_NotSupported:
                            SHELL_Printf(shellContextHandle,
                                         "The %s output_mode is not supported on this configuration\r\n",
                                         (char *)queueMsg.argv[2]);
                            break;
                        default:
                            SHELL_Printf(shellContextHandle, "ERROR API ENGINE\r\n");
                            break;
                    }
                }
                else if (!strcmp((char *)queueMsg.argv[1], "interface"))
                {
                    cfg_displayinterface_t command;
                    if (!strcmp((char *)queueMsg.argv[2], "loopback"))
                    {
                        command = DISPLAY_INTERFACE_LOOPBACK;
                    }
                    else if (!strcmp((char *)queueMsg.argv[2], "infobar"))
                    {
                        command = DISPLAY_INTERFACE_INFOBAR;
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Wrong display interface\r\n");
                        SHELL_Printf(shellContextHandle, USB_SHELL_PROMPT);
                        continue;
                    }
                    status = VIZN_SetDispOutputInterface(&VIZN_API_CLIENT(Shell), command);
                    switch (status)
                    {
                        case kStatus_API_Layer_SetDispOutputInterface_SaveFailed:
                            SHELL_Printf(shellContextHandle, "Failed to save the display interface\r\n");
                            break;
                        case kStatus_API_Layer_SetDispOutputInterface_Same:
                            SHELL_Printf(shellContextHandle,
                                         "The specified display interface is the same, no change required\r\n");
                            break;
                        case kStatus_API_Layer_SetDispOutputInterface_NotSupported:
                            SHELL_Printf(shellContextHandle, "The specified display interface is not supported\r\n");
                            break;
                        case kStatus_API_Layer_Success:
                            SHELL_Printf(shellContextHandle, "The display interface was successfully changed\r\n");
                            break;
                        default:
                            SHELL_Printf(shellContextHandle, "ERROR API ENGINE\r\n");
                            break;
                    }
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Wrong command for display\r\n");
                }
            }
            else
            {
                SHELL_Printf(shellContextHandle, "Incomplete command for display\r\n");
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_WIFI_ERASE)
        {
            status = VIZN_WiFiCredentials_Erase(&VIZN_API_CLIENT(Shell));
            if (status == kStatus_API_Layer_Success)
            {
                SHELL_Printf(shellContextHandle, "Wi-Fi credentials removed\r\n");
            }
            else
            {
                SHELL_Printf(shellContextHandle, "Failed to remove Wi-Fi credentials\r\n");
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_WIFI_CREDENTIALS)
        {
            if (queueMsg.argc == 2)
            {
                char SSID[SSID_NAME_SIZE], Password[WSEC_MAX_PSK_LEN];
                status = VIZN_WiFiCredentials_Get(&VIZN_API_CLIENT(Shell), SSID, Password);
                if (status == kStatus_API_Layer_Success)
                {
                    SHELL_Printf(shellContextHandle, "SSID is %s with password %s.\r\n", SSID, Password);
                }
                else if (status == kStatus_API_Layer_WIFI_Credentials_NotFound)
                {
                    SHELL_Printf(shellContextHandle, "Wi-Fi Credentials not found\r\n");
                }
                SHELL_Printf(shellContextHandle, USB_SHELL_PROMPT);
                continue;
            }
            status =
                VIZN_WiFiCredentials_Set(&VIZN_API_CLIENT(Shell), (char *)queueMsg.argv[2], (char *)queueMsg.argv[3]);
            if (status == kStatus_API_Layer_Success)
            {
                SHELL_Printf(shellContextHandle,
                             "Wi-Fi credentials set. New network will be available after Wi-Fi reset\r\n");
            }
            else
            {
                SHELL_Printf(shellContextHandle, "Failed to set Wi-Fi credentials\r\n");
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_WIFI)
        {
            uint32_t status;
            if (!strcmp((char *)queueMsg.argv[1], "on"))
            {
                status = SLN_Connection_Commands(COMMAND_WIFI_ON);
                if (status == CONNECTION_WIFI_SUCCESS)
                {
                    SHELL_Printf(shellContextHandle, "Wi-Fi will start\r\n");
                }
            }
            else if (!strcmp((char *)queueMsg.argv[1], "off"))
            {
                status = SLN_Connection_Commands(COMMAND_WIFI_OFF);
                if (status == CONNECTION_WIFI_SUCCESS)
                {
                    SHELL_Printf(shellContextHandle, "Wi-Fi will stop\r\n");
                }
            }
            else
            {
                status = SLN_Connection_Commands(COMMAND_WIFI_RESET);
                if (status == CONNECTION_WIFI_SUCCESS)
                {
                    SHELL_Printf(shellContextHandle, "Wi-Fi will reset\r\n");
                }
            }
            if (status == CONNECTION_WIFI_INIT)
            {
                SHELL_Printf(shellContextHandle, "Wi-Fi driver is on\r\n");
            }
            if (status == CONNECTION_WIFI_DISCONNECTED)
            {
                SHELL_Printf(shellContextHandle, "Wi-Fi driver is off\r\n");
            }
            else if (status == CONNECTION_WIFI_CONNECTING)
            {
                SHELL_Printf(shellContextHandle, "Wi-Fi driver is connecting, wait...\r\n");
            }
            else if (status == CONNECTION_WIFI_DISCONNECTING)
            {
                SHELL_Printf(shellContextHandle, "Wi-Fi driver is disconnecting, wait...\r\n");
            }
            else if (status == CONNECTION_WIFI_PROVISIONING_MODE)
            {
                SHELL_Printf(shellContextHandle, "Wi-Fi is in provisioning mode\r\n");
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_APP)
        {
            uint32_t status;
            uint8_t type;
            if (queueMsg.argc == 2)
            {
                type = atoi(queueMsg.argv[1]);
                if (type < APP_TYPE_SUPPORT_LAST)
                {
                    status = VIZN_SetAppTypeResetDatabase(&VIZN_API_CLIENT(Shell), (cfg_apptype_t)type);
                    switch (status)
                    {
                        case kStatus_API_Layer_SetAppType_NotSupported:
                            SHELL_Printf(shellContextHandle, "App type %d not supported.\r\n", type);
                            break;

                        case kStatus_API_Layer_SetAppType_Same:
                            SHELL_Printf(shellContextHandle, "App type %d is same with cuurent setting.\r\n", type);
                            break;

                        case kStatus_API_Layer_SetAppType_SaveFailed:
                            SHELL_Printf(shellContextHandle, "App type %d save fail into flash.\r\n", type);
                            break;

                        case kStatus_API_Layer_SetAppType_ResetDatabaseFailed:
                            SHELL_Printf(shellContextHandle,
                                         "Set app type %d success, but reset existing db failed.\r\n", type);
                            VIZN_SystemReset(&VIZN_API_CLIENT(Shell));
                            break;

                        case kStatus_API_Layer_Success:
                            SHELL_Printf(shellContextHandle,
                                         "Set app type %d, system will remove existing db & reset.\r\n", type);
                            VIZN_SystemReset(&VIZN_API_CLIENT(Shell));
                            break;

                        default:
                            SHELL_Printf(shellContextHandle, "ERROR API ENGINE\r\n");
                            break;
                    }
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Undefined app type\r\n");
                }
            }
            else
            {
                cfg_apptype_t type;
                status = VIZN_GetAppType(&VIZN_API_CLIENT(Shell), &type);
                if (status == kStatus_API_Layer_Success)
                {
                    SHELL_Printf(shellContextHandle, "App type:%d\r\n", type);
                }
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_LOW_POWER)
        {
            if (queueMsg.argc == 1)
            {
                cfg_lowpower_t cur_mode;
                uint32_t status = VIZN_GetLowPowerMode(&VIZN_API_CLIENT(Shell), &cur_mode);
                if (status == kStatus_API_Layer_Success)
                {
                    SHELL_Printf(shellContextHandle, "Low Power Mode is %s\r\n",
                                 (cur_mode == LOW_POWER_MODE_ON) ? "enabled" : "disabled");
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Low power mode not supported\r\r");
                }
            }
            else if (queueMsg.argc == 2)
            {
                if (!strcmp((char *)queueMsg.argv[1], "on"))
                {
                    status = VIZN_SetLowPowerMode(&VIZN_API_CLIENT(Shell), LOW_POWER_MODE_ON);
                    if (status == kStatus_API_Layer_Success)
                    {
                        SHELL_Printf(shellContextHandle, "Low power mode enabled.\r\n");
                    }
                    else if (status == kStatus_API_Layer_SetLowPowerMode_Same)
                    {
                        SHELL_Printf(shellContextHandle, "Low power mode already enabled.\r\n");
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Cannot save low power mode\r\n");
                    }
                }
                else if (!strcmp((char *)queueMsg.argv[1], "off"))
                {
                    status = VIZN_SetLowPowerMode(&VIZN_API_CLIENT(Shell), LOW_POWER_MODE_OFF);
                    if (status == kStatus_API_Layer_Success)
                    {
                        SHELL_Printf(shellContextHandle, "Low power mode disabled.\r\n");
                    }
                    else if (status == kStatus_API_Layer_SetLowPowerMode_Same)
                    {
                        SHELL_Printf(shellContextHandle, "Low power mode already disabled.\r\n");
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Cannot save low power mode\r\n");
                    }
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Undefined mode\r\n");
                }
            }
        }
        else if (queueMsg.shellCommand == SHELL_EV_FFI_CLI_ALGO_START)
        {
            if (queueMsg.argc == 1)
            {
                cfg_algo_start_mode_t cur_mode;
                uint32_t status = VIZN_GetAlgoStartMode(&VIZN_API_CLIENT(Shell), &cur_mode);
                if (status == kStatus_API_Layer_Success)
                {
                    SHELL_Printf(shellContextHandle, "Algo start mode is %s\r\n",
                                 (cur_mode == ALGO_START_MODE_AUTO) ? "auto" : "manual");
                    if (cur_mode == ALGO_START_MODE_MANUAL)
                    {
                        VIZN_StartRecognition(&VIZN_API_CLIENT(Shell));
                        SHELL_Printf(shellContextHandle, "And start Recognition in manual");
                    }
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Algo start mode not supported\r\r");
                }
            }
            else if (queueMsg.argc == 2)
            {
                if (!strcmp((char *)queueMsg.argv[1], "auto"))
                {
                    status = VIZN_SetAlgoStartMode(&VIZN_API_CLIENT(Shell), ALGO_START_MODE_AUTO);
                    if (status == kStatus_API_Layer_Success)
                    {
                        VIZN_StartRecognition(&VIZN_API_CLIENT(Shell));
                        SHELL_Printf(shellContextHandle, "Set algo start mode to auto.\r\n");
                    }
                    else if (status == kStatus_API_Layer_SetLowPowerMode_Same)
                    {
                        SHELL_Printf(shellContextHandle, "Algo start mode is already auto.\r\n");
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Cannot save algo start mode\r\n");
                    }
                }
                else if (!strcmp((char *)queueMsg.argv[1], "manual"))
                {
                    status = VIZN_SetAlgoStartMode(&VIZN_API_CLIENT(Shell), ALGO_START_MODE_MANUAL);
                    if (status == kStatus_API_Layer_Success)
                    {
                        SHELL_Printf(shellContextHandle, "Set algo start mode to manual.\r\n");
                    }
                    else if (status == kStatus_API_Layer_SetLowPowerMode_Same)
                    {
                        SHELL_Printf(shellContextHandle, "Algo start mode is already manual.\r\n");
                    }
                    else
                    {
                        SHELL_Printf(shellContextHandle, "Cannot save algo start mode\r\n");
                    }
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Undefined mode\r\n");
                }
            }
        }
        SHELL_Printf(shellContextHandle, USB_SHELL_PROMPT);
    }

    vTaskDelete(NULL);
}

static shell_status_t _RtInfoCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    char *pEnd;

    if (argc == 2)
    {
        if (!strcmp((char *)argv[1], "size"))
        {
            /* print the runtime information start address and size */
            unsigned char *pStart = NULL;
            unsigned int size     = 0;
            FaceRecRtInfo_Size(&pStart, &size);
            if (pStart && size)
            {
                SHELL_Printf(shellContextHandle, "\"%s\" start:0x%x size:0x%x\r\n", argv[1], pStart, size);
            }
            return kStatus_SHELL_Success;
        }
        else if (!strcmp((char *)argv[1], "clean"))
        {
            /* clean the captured runtime information */
            FaceRecRtInfo_Clean();
            return kStatus_SHELL_Success;
        }
        else if (!strcmp((char *)argv[1], "disable"))
        {
            /* disable all the filters */
            FaceRecRtInfo_Disable();
            return kStatus_SHELL_Success;
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
            return kStatus_SHELL_Error;
        }
    }
    else if ((argc == 3) || (argc == 4))
    {
        uint32_t tmp;
        face_rec_rt_info_id_t id = kFaceRecRtInfoId_Count;
        unsigned char enable     = 0;
        unsigned char filter     = 0;

        if (!strcmp((char *)argv[1], "global"))
        {
            id = kFaceRecRtInfoId_Global;
        }
        else if (!strcmp((char *)argv[1], "detect"))
        {
            id = kFaceRecRtInfoId_Detect;
        }
        else if (!strcmp((char *)argv[1], "fake"))
        {
            id = kFaceRecRtInfoId_Fake;
        }
        else if (!strcmp((char *)argv[1], "facerec"))
        {
            id = kFaceRecRtInfoId_FaceFecognize;
        }
        else
        {
            tmp = strtol(argv[1], &pEnd, 10);
            if (argv[1] == pEnd)
            {
                SHELL_Printf(shellContextHandle, "\"%s\" invalid item id.\r\n", argv[1]);
                return kStatus_SHELL_Error;
            }

            if ((tmp >= kFaceRecRtInfoId_Global) && (tmp < kFaceRecRtInfoId_Count))
            {
                id = (face_rec_rt_info_id_t)tmp;
            }
        }

        /* check the item id */
        if (id > kFaceRecRtInfoId_Count)
        {
            SHELL_Printf(shellContextHandle, "\"%s\" invalid item id.\r\n", argv[1]);
            return kStatus_SHELL_Error;
        }

        tmp = strtol(argv[2], &pEnd, 10);
        if ((argv[2] == pEnd) || ((tmp != 0) && (tmp != 1)))
        {
            SHELL_Printf(shellContextHandle, "\"%s\" invalid enable flag.\r\n", argv[2]);
            return kStatus_SHELL_Error;
        }
        enable = (unsigned char)tmp;

        if (argc == 4)
        {
            tmp = strtol(argv[3], &pEnd, 10);
            if ((argv[3] == pEnd) || ((tmp != 0) && (tmp != 1) && (tmp != 2)))
            {
                SHELL_Printf(shellContextHandle, "\"%s\" invalid enable flag.\r\n", argv[3]);
                return kStatus_SHELL_Error;
            }
            filter = tmp;
        }

        SHELL_Printf(shellContextHandle, "id:0x%02x enable:%d filter:%d\r\n", id, enable, filter);
        FaceRecRtInfo_Filter(id, enable, filter);
    }
    else
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}
