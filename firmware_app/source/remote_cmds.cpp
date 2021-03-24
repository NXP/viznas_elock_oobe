/*
 * Copyright 2019-2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */
#include <string.h>
#include "remote_cmds.h"
#include "database.h"
#include "oasis.h"
#include "sln_shell.h"
#include "sln_api.h"
#include "ble_connection.h"
#include "cJSON.h"

/* frame header region */
#define REMOTE_CTRL_FRAME_HEADER "FH"
#define REMOTE_CTRL_PREFIX       "PRE"
#define REMOTE_CTRL_TIME_STAMP   "TS"
#define FH_VERSION               "VER"
#define FH_FLAG                  "CIP"
#define REMOTE_CTRL_LENGTH       "LEN"

#define REMOTE_CTRL_COMMAND "CMD"
#define REMOTE_CTRL_PAYLOAD "PL"
#define REMOTE_CTRL_CRC     "CRC"

/* remote control payload json */
#define RC_PL_USERNAME      "username"
#define RC_PL_EMAIL         "email"
#define RC_PL_FEATURE       "feature"
#define RC_PL_OP            "op"
#define RC_PL_ID            "id"
#define RC_PL_SUCCESS       "success"
#define RC_PL_FEATURE_TABLE "featureTable"

#define RC_PL_UPDATE_RECORD_ADD    "add"
#define RC_PL_UPDATE_RECORD_DELETE "delete"

static void Remote_convertAscii2int(unsigned char *ascii, int len, void *value)
{
    unsigned char nibble[8];
    unsigned char *value_char = (unsigned char *)value;

    assert(len == 4 || len == 8);

    for (int j = 0; j < len; j++)
    {
        if (ascii[j] <= 'f' && ascii[j] >= 'a')
        {
            nibble[j] = ascii[j] - 'a' + 10;
        }
        else if (ascii[j] <= '9' && ascii[j] >= '0')
        {
            nibble[j] = ascii[j] - '0';
        }
        else
        {
            nibble[j] = 0;
        }
    }

    value_char[0] = nibble[0] << 4 | nibble[1];
    value_char[1] = nibble[2] << 4 | nibble[3];
    if (len == 8)
    {
        value_char[2] = nibble[4] << 4 | nibble[5];
        value_char[3] = nibble[6] << 4 | nibble[7];
    }
}

static void Remote_convertInt2ascii(void *value, int bytes, unsigned char *ascii)
{
    unsigned char nibble;
    unsigned char hex_table[] = "0123456789abcdef";
    unsigned char *p_value    = (unsigned char *)value;
    unsigned char *p_ascii    = ascii;

    for (int j = 0; j < bytes; j++)
    {
        nibble     = *p_value++;
        int low    = nibble & 0x0f;
        int high   = (nibble >> 4) & 0x0f;
        *p_ascii++ = hex_table[high];
        *p_ascii++ = hex_table[low];
    }
}

static int compareString(const char *str1, const char *str2, int count)
{
    while (count--)
    {
        if (*str1++ != *str2++)
        {
            return *(unsigned char *)(str1 - 1) - *(unsigned char *)(str2 - 1);
        }
    }
    return 0;
}

static int wiced_send_frame_head(int next_packet_length)
{
    cJSON *head_obj_json = cJSON_CreateObject();
    char *string         = NULL;

    if (head_obj_json == NULL)
    {
        goto end;
    }

    if (cJSON_AddStringToObject(head_obj_json, DEVICENAME_STRING, "RT-Vision") == NULL)
    {
        goto end;
    }

    if (cJSON_AddNumberToObject(head_obj_json, LENGTH_STRING, (double)next_packet_length) == NULL)
    {
        goto end;
    }

    string = cJSON_PrintUnformatted(head_obj_json);

    BleUartSendNotification((uint8_t *)string, strlen(string));

    cJSON_free(string);

end:
    cJSON_Delete(head_obj_json);

    return 0;
}

static int wiced_send_frame_packet(uint8_t *string, int len)
{
    BleUartSendNotification(string, len);

    return 0;
}

static remote_frame_cmd_t parse_remote_packet(const char *packet_str, remote_frame_struct_t *frame_packet)
{
    cJSON *frame_packet_json = cJSON_Parse(packet_str);
    {
        if (frame_packet_json == NULL)
        {
            goto end;
        }

        cJSON *frame_header_json = cJSON_GetObjectItemCaseSensitive(frame_packet_json, REMOTE_CTRL_FRAME_HEADER);
        if (frame_header_json == NULL)
        {
            goto end;
        }

        cJSON *frame_header_prefix_json = cJSON_GetObjectItemCaseSensitive(frame_header_json, REMOTE_CTRL_PREFIX);
        if (frame_header_prefix_json == NULL)
        {
            goto end;
        }
        memcpy(frame_packet->frame_header.prefix, frame_header_prefix_json->valuestring,
               strlen(frame_header_prefix_json->valuestring));

        cJSON *frame_header_timestamp_json =
            cJSON_GetObjectItemCaseSensitive(frame_header_json, REMOTE_CTRL_TIME_STAMP);
        if (frame_header_timestamp_json == NULL)
        {
            goto end;
        }
        frame_packet->frame_header.timestamp = frame_header_timestamp_json->valuedouble;

        cJSON *frame_header_version_json = cJSON_GetObjectItemCaseSensitive(frame_header_json, FH_VERSION);
        if (frame_header_version_json == NULL)
        {
            goto end;
        }
        frame_packet->frame_header.version = frame_header_version_json->valuedouble;

        cJSON *frame_header_cipher_json = cJSON_GetObjectItemCaseSensitive(frame_header_json, FH_FLAG);
        if (frame_header_cipher_json == NULL)
        {
            goto end;
        }
        frame_packet->frame_header.cipher = frame_header_cipher_json->valuedouble;

        cJSON *frame_header_length_json = cJSON_GetObjectItemCaseSensitive(frame_header_json, REMOTE_CTRL_LENGTH);
        if (frame_header_length_json == NULL)
        {
            goto end;
        }
        frame_packet->frame_header.length = frame_header_length_json->valuedouble;

        cJSON *frame_cmd_json = cJSON_GetObjectItemCaseSensitive(frame_packet_json, REMOTE_CTRL_COMMAND);
        if (frame_cmd_json == NULL)
        {
            goto end;
        }
        frame_packet->command = (remote_frame_cmd_t)frame_cmd_json->valuedouble;

        cJSON *frame_payload_json = cJSON_GetObjectItemCaseSensitive(frame_packet_json, REMOTE_CTRL_PAYLOAD);
        if (frame_payload_json == NULL)
        {
            goto end;
        }
        char *p_payload = (char *)pvPortMalloc(strlen(frame_payload_json->valuestring) + 1);
        if (p_payload == NULL)
        {
            goto end;
        }
        memcpy(p_payload, frame_payload_json->valuestring, strlen(frame_payload_json->valuestring) + 1);
        frame_packet->payload = p_payload;

        cJSON *frame_crc_json = cJSON_GetObjectItemCaseSensitive(frame_packet_json, REMOTE_CTRL_CRC);
        if (frame_crc_json == NULL)
        {
            goto end;
        }
        frame_packet->crc = frame_crc_json->valuedouble;

        cJSON_Delete(frame_packet_json);
        return frame_packet->command;
    }

end:
    cJSON_Delete(frame_packet_json);
    return UNKNOW_COMMAND;
}

static char *transform_remote_pakeck(const remote_frame_struct_t frame_packet)
{
    char *string = NULL;

    cJSON *frame_packet_json = cJSON_CreateObject();
    {
        if (frame_packet_json == NULL)
        {
            goto end;
        }

        cJSON *frame_header_json = cJSON_AddObjectToObject(frame_packet_json, REMOTE_CTRL_FRAME_HEADER);
        if (frame_header_json == NULL)
        {
            goto end;
        }

        if (cJSON_AddStringToObject(frame_header_json, REMOTE_CTRL_PREFIX, frame_packet.frame_header.prefix) == NULL)
        {
            goto end;
        }

        if (cJSON_AddNumberToObject(frame_header_json, REMOTE_CTRL_TIME_STAMP, frame_packet.frame_header.timestamp) ==
            NULL)
        {
            goto end;
        }

        if (cJSON_AddNumberToObject(frame_header_json, FH_VERSION, frame_packet.frame_header.version) == NULL)
        {
            goto end;
        }

        if (cJSON_AddNumberToObject(frame_header_json, FH_FLAG, frame_packet.frame_header.cipher) == NULL)
        {
            goto end;
        }

        if (cJSON_AddNumberToObject(frame_header_json, REMOTE_CTRL_LENGTH, frame_packet.frame_header.length) == NULL)
        {
            goto end;
        }

        if (cJSON_AddNumberToObject(frame_packet_json, REMOTE_CTRL_COMMAND, frame_packet.command) == NULL)
        {
            goto end;
        }

        if (cJSON_AddStringToObject(frame_packet_json, REMOTE_CTRL_PAYLOAD, (char *)frame_packet.payload) == NULL)
        {
            goto end;
        }

        if (cJSON_AddNumberToObject(frame_packet_json, REMOTE_CTRL_CRC, frame_packet.crc) == NULL)
        {
            goto end;
        }

        string = cJSON_PrintUnformatted(frame_packet_json);
    }

end:
    if (string == NULL)
    {
        configPRINTF("JSON packet error!.\r\n");
    }
    cJSON_Delete(frame_packet_json);

    return string;
}

static int Remote_send_ready(void)
{
    char *str_pre = NULL;

    remote_frame_header_t frame_header = {
        .prefix    = "fac",
        .timestamp = 0,
        .version   = 0,
        .cipher    = 0,
        .length    = 0,
    };

    remote_frame_struct_t frame_packet = {
        .frame_header = frame_header,
        .command      = READY_INDICATION,
        .payload      = 0,
        .crc          = 0,
    };

    cJSON *ready_json = cJSON_CreateObject();
    {
        if (ready_json == NULL)
        {
            goto end;
        }

        cJSON_AddNumberToObject(ready_json, "max_payload", MAX_BLE_MTU);
        str_pre = cJSON_PrintUnformatted(ready_json);

        frame_packet.frame_header.length = strlen(str_pre) + 1;
        frame_packet.payload             = str_pre;

        char *str_data = transform_remote_pakeck(frame_packet);
        cJSON_free(str_pre);

        wiced_send_frame_head(strlen(str_data));

        wiced_send_frame_packet((uint8_t *)str_data, strlen(str_data));
        cJSON_free(str_data);
    }

end:
    cJSON_Delete(ready_json);
    return 0;
}

static int Remote_send_cmd_packet(remote_frame_cmd_t cmd, uint8_t *payload, int len)
{
    remote_frame_header_t frame_header = {
        .prefix    = "fac",
        .timestamp = 0,
        .version   = 0,
        .cipher    = 0,
        .length    = (uint16_t)len,
    };

    remote_frame_struct_t frame_packet = {
        .frame_header = frame_header,
        .command      = cmd,
        .payload      = payload,
        .crc          = 0,
    };

    char *str_data = transform_remote_pakeck(frame_packet);

    wiced_send_frame_head(strlen(str_data));

    uint8_t *p_str_data = (uint8_t *)str_data;
    for (size_t i = 0; i < strlen(str_data); i += RECV_DATA_QUEUE_ITEM_SIZE)
    {
        if (i + RECV_DATA_QUEUE_ITEM_SIZE < strlen(str_data))
            wiced_send_frame_packet(p_str_data + i, RECV_DATA_QUEUE_ITEM_SIZE);
        else
            wiced_send_frame_packet(p_str_data + i, strlen((char *)p_str_data + i));
    }

    configPRINTF(str_data);

    cJSON_free(str_data);

    return 0;
}

int Remote_command_configuration_get_request(const char *payload_str)
{
    return 0;
}

int Remote_command_face_record_get_request(const char *payload_str)
{
    //    cJSON *payload_json = cJSON_Parse((const char*)payload_str);
    //    if (payload_json == NULL) {
    //        configPRINTF("payload json parse error.\r\n");
    //        goto end;
    //    }

    // get ids
    std::vector<uint16_t> allIDs;
    DB_GetIDs(allIDs);

    float *face                            = (float *)pvPortMalloc(OASISLT_getFaceItemSize());
    uint8_t *hex_str                       = (uint8_t *)pvPortMalloc(OASISLT_getFaceItemSize() * 2 + 1);
    hex_str[OASISLT_getFaceItemSize() * 2] = '\0';

    cJSON *features_json = cJSON_CreateObject();
    {
        if (features_json == NULL)
        {
            goto end;
        }
        cJSON *array_json = cJSON_CreateArray();
        if (array_json == NULL)
        {
            goto end;
        }

        cJSON_AddItemToObject(features_json, RC_PL_FEATURE_TABLE, array_json);

        for (uint32_t i = 0; i < allIDs.size(); i++)
        {
            cJSON *object_json = cJSON_CreateObject();
            if (object_json == NULL)
            {
                goto end;
            }

            if (cJSON_AddNumberToObject(object_json, RC_PL_ID, allIDs[i]) == NULL)
            {
                cJSON_Delete(object_json);
                goto end;
            }

            string name;
            DB_GetName(allIDs[i], name);
            if (cJSON_AddStringToObject(object_json, RC_PL_USERNAME, name.c_str()) == NULL)
            {
                cJSON_Delete(object_json);
                goto end;
            }

            DB_GetFeature(allIDs[i], face);
            Remote_convertInt2ascii(face, OASISLT_getFaceItemSize(), hex_str);
            if (cJSON_AddStringToObject(object_json, RC_PL_FEATURE, (const char *)hex_str) == NULL)
            {
                cJSON_Delete(object_json);
                goto end;
            }

            cJSON_AddItemToArray(array_json, object_json);
        }

        if (cJSON_AddBoolToObject(features_json, RC_PL_SUCCESS, true) == NULL)
        {
            goto end;
        }

        char *json_str = cJSON_PrintUnformatted(features_json);
        //        configPRINTF((json_str));
        Remote_send_cmd_packet(FACE_RECORD_GET_RESPONSE, (uint8_t *)json_str, strlen(json_str));

        cJSON_free(json_str);
    }

end:
    cJSON_Delete(features_json);
    vPortFree(face);
    vPortFree(hex_str);
    return 0;
}

int Remote_command_face_record_update_get_op(const char *op_str)
{
    if (compareString(op_str, RC_PL_UPDATE_RECORD_ADD, sizeof(RC_PL_UPDATE_RECORD_ADD)) == 0)
        return UPDATE_RECORD_ADD;
    else if (compareString(op_str, RC_PL_UPDATE_RECORD_DELETE, sizeof(RC_PL_UPDATE_RECORD_DELETE)) == 0)
        return UPDATE_RECORD_DELETE;
    else
        return UPDATE_RECORD_UNKNOWN;
}

int Remote_command_face_record_update_request(const char *payload_str)
{
    int ret = -1;
    char *json_str;

    cJSON *payload_json = cJSON_Parse((const char *)payload_str);
    {
        if (payload_json == NULL)
        {
            goto end;
        }

        cJSON *op_json = cJSON_GetObjectItemCaseSensitive(payload_json, RC_PL_OP);
        if (op_json == NULL)
        {
            goto end;
        }

        switch (Remote_command_face_record_update_get_op(op_json->valuestring))
        {
            case UPDATE_RECORD_ADD:
            {
                cJSON *id_json = cJSON_GetObjectItemCaseSensitive(payload_json, RC_PL_ID);
                if (id_json == NULL)
                {
                    goto end;
                }

                cJSON *username_json = cJSON_GetObjectItemCaseSensitive(payload_json, RC_PL_USERNAME);
                if (username_json == NULL)
                {
                    goto end;
                }

                //                cJSON *email_json = cJSON_GetObjectItemCaseSensitive(payload_json, RC_PL_EMAIL);
                //                if (email_json == NULL) {
                //                    goto end;
                //                }

                cJSON *feature_json = cJSON_GetObjectItemCaseSensitive(payload_json, RC_PL_FEATURE);
                if (feature_json == NULL)
                {
                    goto end;
                }

                unsigned char *pFeature = (unsigned char *)feature_json->valuestring;
                float *feature          = (float *)pvPortMalloc(OASISLT_getFaceItemSize());
                if (feature == NULL)
                {
                    goto end;
                }

                float fValue;
                for (unsigned int i = 0; i < FEATUREDATA_FEATURE_SIZE; i++)
                {
                    Remote_convertAscii2int(pFeature, sizeof(fValue) * 2, &fValue);
                    pFeature += sizeof(fValue) * 2;
                    feature[i] = fValue;
                }

                // get the id
                uint16_t id_remote, id_local;

                id_remote = id_json->valuedouble;
                ret       = DB_GenID(&id_local);
                if (ret < 0)
                {
                    vPortFree(feature);
                    goto db_end;
                }

                // cmp ids between remote and local.
                if (id_local != id_remote)
                {
                    UsbShell_Printf("remote id different with local, [r]%d [l]%d\r\n", id_remote, id_local);
                    vPortFree(feature);
                    goto db_end;
                }

                string name = username_json->valuestring;
                ret         = DB_Add(id_local, name, feature);
                if (ret == DB_MGMT_FAILED)
                {
                    UsbShell_Printf("remote add user into local fail, [%d]: [%s]\r\n", id_remote, name.c_str());
                    vPortFree(feature);
                    goto db_end;
                }

                UsbShell_Printf("remote register [%d][%s] success.\r\n", id_local, name.c_str());
                vPortFree(feature);
                break;
            }

            case UPDATE_RECORD_DELETE:
            {
                string name;

                cJSON *id_json = cJSON_GetObjectItemCaseSensitive(payload_json, RC_PL_ID);
                if (id_json == NULL)
                {
                    goto end;
                }

                cJSON *username_json = cJSON_GetObjectItemCaseSensitive(payload_json, RC_PL_USERNAME);
                if (username_json == NULL)
                {
                    goto end;
                }

                ret = DB_GetName(id_json->valuedouble, name);
                if (ret == DB_MGMT_FAILED)
                {
                    configPRINTF("remote delete user failed.\r\n");
                    goto db_end;
                }

                if (compareString(name.c_str(), username_json->valuestring, strlen(username_json->valuestring)) != 0)
                {
                    UsbShell_Printf("remote database different with local [%d]:[r]%s [l]%s\r\n", id_json->valueint,
                                    username_json->valuestring, name.c_str());
                    goto db_end;
                }

                name    = username_json->valuestring;
                int ret = DB_Del(name);
                if (ret == DB_MGMT_FAILED)
                {
                    configPRINTF("remote delete user failed.\r\n");
                    goto db_end;
                }
                UsbShell_Printf("remote delete user[%s] success.\r\n", name.c_str());
                break;
            }

            default:
                break;
        }
    }

    if (cJSON_AddBoolToObject(payload_json, RC_PL_SUCCESS, true) == NULL)
    {
        goto end;
    }

    json_str = cJSON_PrintUnformatted(payload_json);
    if (json_str == NULL)
    {
        goto end;
    }
    Remote_send_cmd_packet(FACE_RECORD_UPDATE_RESPONSE, (uint8_t *)json_str, strlen(json_str));

    cJSON_free(json_str);

    cJSON_Delete(payload_json);

    return 0;

db_end:
    if (cJSON_AddBoolToObject(payload_json, RC_PL_SUCCESS, false) == NULL)
    {
        goto end;
    }

    json_str = cJSON_PrintUnformatted(payload_json);
    if (json_str == NULL)
    {
        goto end;
    }

    Remote_send_cmd_packet(FACE_RECORD_UPDATE_RESPONSE, (uint8_t *)json_str, strlen(json_str));

    cJSON_free(json_str);

    cJSON_Delete(payload_json);

    return -2;

end:
    cJSON_Delete(payload_json);
    configPRINTF("remote json parse error.\r\n");
    return -1;
}

int Remote_cmd_process(uint8_t *cmd_str)
{
    remote_frame_cmd_t cmd_state;
    remote_frame_struct_t frame_packet;

    // JSON parse
    cmd_state = parse_remote_packet((const char *)cmd_str, &frame_packet);

    switch (cmd_state)
    {
            // TODO: use to send ready to remote app.
        case READY_INDICATION:
            Remote_send_ready();
            configPRINTF("APK Ready.\r\n");
            break;

            // TODO:
        case AUTHENTICATION_REQUEST:
            break;

        case AUTHENTICATION_RESPONSE:
            break;

        case PASSWORD_REQUEST:
            break;

        case PASSWORD_RESPONSE:
            break;

        case CONFIGURATION_GET_REQUEST:
            // Remote_command_configuration_get_request(frame_packet.payload);
            // TODO: bridge CLI command with remote control
            break;

        case CONFIGURATION_GET_RESPONSE:
            break;

        case CONFIGURATION_UPDATE_REQUEST:
            break;

        case CONFIGURATION_UPDATE_RESPONSE:
            break;

        case FACE_RECORD_GET_REQUEST:
            Remote_command_face_record_get_request((char *)frame_packet.payload);
            // manager user list send to app.
            break;

        case FACE_RECORD_GET_RESPONSE:
            break;

        case FACE_RECORD_UPDATE_REQUEST:
            // manager user update face feature.
            // add, edit, delete
            Remote_command_face_record_update_request((char *)frame_packet.payload);
            break;

        case FACE_RECORD_UPDATE_RESPONSE:
            break;

        case OPEN_DOOR_RESPONSE:
            break;

        case OPEN_DOOR_REQUEST:
            // remote control
            break;

        default:
            configPRINTF(("Unknown Command\r\n"));
            break;
    }

    vPortFree(frame_packet.payload);

    return 0;
}
