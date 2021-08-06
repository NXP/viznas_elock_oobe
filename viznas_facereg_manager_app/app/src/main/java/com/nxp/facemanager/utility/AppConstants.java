package com.nxp.facemanager.utility;

import com.nxp.facemanager.ble.BleConfigModel;

import java.sql.Timestamp;
import java.time.Instant;

/**
 * This is common constant file for accessing anywhere from the application.
 */
public class AppConstants {
    /**
     * Constant for Database name.
     */
    public static final String DB_NAME = "face-db.db";
    /**
     * Constant for theme which will be stored in {@link android.content.SharedPreferences}.
     */
    public static final String THEME = "theme";
    /**
     * Constant for Blue and Black theme.
     */
    public static final String THEME_BLUE_BLACK = "theme_blue_black";
    /**
     * Constant for Gray and White theme.
     */
    public static final String THEME_GRAY_WHITE = "theme_gray_white";
    public static final String INTENT_NOTIFICATION = "intent_notification";
    public static final String IS_APP_FORGROUND = "is_app_forground";
    /**
     * Constant to represent intent action for startings service.
     */
    public static final String STARTFOREGROUND_ACTION = "com.nxp.facemanager.foregroundservice.action.startforeground";
    /**
     * Constant used inthe action of disabling the foreground service.
     */
    public static final String ACTION_DISABLE_SERVICE = "ACTION_DISABLE";
    /**
     * Constant to request location permission.
     */
    public static final int LOCATION_PERMISSION_REQUEST = 123;
    /**
     * Constant is Notification id for permission notification.
     */
    public static final int PERMISSION_NOTIFICATION = 100;
    /**
     * Forground service notification Id
     */
    public static final int FOREGROUND_SERVICE_NOTIFICATION = 101;
    /**
     * This constant is used for Command Type when sending data to ble.
     */
    public static final String USER_INFORMATION = "user_information";
    /**
     * This constant is used for Command Type when sending data to ble.
     */
    public static final String TRAINING_INFORMATION = "training_information";

    public static final String FACEREC_FILE_NAME = "facerec1.txt";
    /**
     * This constant is used for Command Type when sending data to ble.
     */
    public static final String SNAPSHOT_INFORMATION = "snapshot_information";
    /**
     * Constant for BLE DATA intent.
     */
    public static final String BLE_DATA = "ble_data";
    /**
     * Constant for time stamp.
     */
    static final String TIMESTAMP_FORMAT = "yyyyMMdd_HHmmss";
    /**
     * Snapshot command key
     */
    public static final String KEY_SNAPSHOT_INFORMATION = "snapshot_information";
    /**
     * Web service URl
     */
    public static final String BASE_URL = "http://18.222.241.32:3000/";
    /**
     * Web service secure URl
     */
    public static final String BASE_URL_SECURE = "https://18.222.241.32:443/";
    /**
     * Login user server id.
     */
    public static final String LOGIN_USER_ID = "login_user_id";
    /**
     * Login user email id.
     */
    public static final String LOGIN_USER_EMAIL = "login_user_email";
    /**
     * Login user role.
     */
    public static final String IS_ADMIN = "is_admin";
    /**
     * Login user name
     */
    public static final String LOGIN_USER_NAME = "login_user_name";
    /**
     * Token for fetch api details.
     */
    public static final String TOKEN = "token";

    public static final String CONNECTED_MAC_ADDRESS = "connected_mac_address";


    public static final String IS_EDIT_USER = "isEditUser";
    public static final String USER_DATA = "user_data";

    public static final String CONNECTION_TYPE = "connection_type";
    public static final String SMARTLOCK_IP = "smartlock_ip";
    public static final String SMARTLOCK_IP_DEFAULT = "192.168.1.102";
    public static final int SMARTLOCK_PORT_DEFAULT = 7;
    public static final String SMARTLOCK_NAME_DEFAULT = "nxp_smartlock";
    public static final int CONNECTION_NETWORK_FLAG = 1;
    public static final int CONNECTION_BLE_FLAG = 2;
    public static final String CONNECTION_NETWORK = "connection_network";
    public static final String CONNECTION_BLE = "connection_ble";

    //Path constants for URL
    /**
     * Login PATH
     */
    public static final String PATH_LOGIN = "user/login";
    /**
     * Registration PATH
     */
    public static final String PATH_REGISTER = "user/";
    /**
     * Create user PATH
     */
    public static final String PATH_CREATE_USER = "user/addUser";
    /**
     * Update User Path
     */
    public static final String PATH_UPDATE_USER = "user/";
    /**
     * Get User List
     */
    public static final String PATH_GET_USERS = "user/getUsersById";
    /**
     * Path fro delete user
     */
    public static final String PATH_DELETE_USER = "user/";
    /**
     * Path for reset password
     */
    public static final String PATH_RESET_PASSWORD = "user/resetPassword";
    /**
     * Path to forgot password
     */
    public static final String PATH_FORGOT_PASSWORD = "user/forgotPassword";
    /**
     * Delete device
     */
    public static final String PATH_DEVICE_DELETE = "device/";
    /**
     *  Get devices by user id
     */
    public static final String PATH_GET_DEVICES_USER_ID = "device/{user_id}";
    /**
     * Add device Path
     */
    public static final String PATH_ADD_DEVICE = "device/";
    /**
     * Path get users
     */
    public static final String PATH_GET_USERS_USER = "users/{user}";

    public static Instant CONNECTION_START;
    public static String VERSION = "1.1";

     /**
     * Commands for communication with eLock
     */
    public final static int READY_INDICATION              =  (0x00);
    public final static int AUTHENTICATION_REQUEST        =  (0x02);
    public final static int AUTHENTICATION_RESPONSE       =  (0x03);
    public final static int PASSWORD_REQUEST              =  (0x04);
    public final static int PASSWORD_RESPONSE             =  (0x05);
    public final static int CONFIGURATION_GET_REQUEST     =  (0x06);
    public final static int CONFIGURATION_GET_RESPONSE    =  (0x07);
    public final static int CONFIGURATION_UPDATE_REQUEST  =  (0x08);
    public final static int CONFIGURATION_UPDATE_RESPONSE =  (0x09);
    public final static int FACE_RECORD_GET_REQUEST       =  (0x0A);
    public final static int FACE_RECORD_GET_RESPONSE      =  (0x0B);
    public final static int FACE_RECORD_UPDATE_REQUEST    =  (0x0C);
    public final static int FACE_RECORD_UPDATE_RESPONSE   =  (0x0D);
    public final static int OPEN_DOOR_RESPONSE            =  (0x5A);
    public final static int OPEN_DOOR_REQUEST             =  (0xA5);
    public final static int UNKNOW_COMMAND                =  (0xFF);

    /**
     * This constant is used for Command Type when sending data to ble.
     */
    public static final String PRE_DEVICE_NAME = "dev";
    public static final String PRE_LENGTH= "len";

    public static final String OP_UPDATE_RECORD_ADD = "add";
    public static final String OP_UPDATE_RECORD_DELETE = "delete";
    public static final String OP_UPDATE_RECORD_NAME = "update_name";
    public static final String OP_UPDATE_RECORD_FEATURE = "update_feature";

    public static final int MAX_PAYLOAD_LENGTH = 244;

    public static boolean ELOCK_READY_RECEIVED = false;
    public static boolean USER_LOGGED_IN = false;
    public static String CUR_USERNAME;
    public static String CUR_USER_PROFILE;
    public static String CUR_USER_EMAIL;

    /**
     * Configurations response from eLock
     */
    public static BleConfigModel CONFIG;

    /**
     * JSON keys for payload encoding
     */
    public static String JSON_KEY_USERNAME = "username";
    public static String JSON_KEY_MPL = "max_payload";
    public static String JSON_KEY_RES_SUCCESS = "success";
    public static String JSON_KEY_PSW = "password";
    public static String JSON_KEY_EMAIL = "email";
    public static String JSON_KEY_CONFIG_VERSION = "version";
    public static String JSON_KEY_FEATURE = "feature";
    public static String JSON_KEY_UPDATE_OP = "op";
    public static String JSON_KEY_ID = "id";
    public static String JSON_KEY_PSW_NEW = "new_psw";
    public static String JSON_KEY_PROFILE_PIC = "profilePic";
    public static String JSON_KEY_USER_INFO = "userInformation";
    public static String JSON_KEY_FEATURE_TABLE = "featureTable";

    public static int BLE_CB_WAIT_INTERVAL = 3000;

    public static String FRAME_NUM = "frameNum";

    public static int GLASSES_HANDLING_VERSION = 1;
    public static int GLASSES_HANDLING_USER_SELECT = 1;
    public static int GLASSES_HANDLING_NO_GLASSES = 2;

}
