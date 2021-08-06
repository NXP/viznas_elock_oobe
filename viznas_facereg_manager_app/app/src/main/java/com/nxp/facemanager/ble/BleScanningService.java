package com.nxp.facemanager.ble;

import android.Manifest;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.HomeActivity;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.DatabaseOperations;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.model.BelStartTrainingDataSendEvent;
import com.nxp.facemanager.model.BleNewDeviceEvent;
import com.nxp.facemanager.model.BleOnOffEvent;
import com.nxp.facemanager.model.BleSendingProgress;
import com.nxp.facemanager.model.BleServiceDisableEvent;
import com.nxp.facemanager.model.BleSnapshotEvent;
import com.nxp.facemanager.model.BleStatusEvent;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.AppLogger;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import javax.inject.Inject;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat;
import androidx.core.content.ContextCompat;
import androidx.lifecycle.Observer;
import dagger.android.DaggerService;
import no.nordicsemi.android.ble.data.Data;

import static androidx.core.app.ActivityCompat.requestPermissions;
import static androidx.core.app.ActivityCompat.shouldShowRequestPermissionRationale;
import static androidx.core.app.NotificationCompat.PRIORITY_LOW;
import static com.nxp.facemanager.ble.BleService.BROADCAST_CONNECTION_STATE;
import static com.nxp.facemanager.ble.BleService.BROADCAST_DEVICE_READY;
import static com.nxp.facemanager.utility.AppConstants.ACTION_DISABLE_SERVICE;
import static com.nxp.facemanager.utility.AppConstants.FOREGROUND_SERVICE_NOTIFICATION;
import static com.nxp.facemanager.utility.AppConstants.KEY_SNAPSHOT_INFORMATION;
import static com.nxp.facemanager.utility.AppConstants.PERMISSION_NOTIFICATION;

/**
 * Main entry and core service class for ble events which includes: 1. Scan
 * Callback to filter out facemanager device 2. BLE read/write operations 3.
 * Connect/Disconnect logic based on app lifecycle 4. Notification posting for
 * the foreground service on/off 5. Broadcast receiver that listens to data sent
 * and data received broadcasts from uart service and calls corresponding
 * activity methods
 */
public class BleScanningService extends DaggerService {
    private static final String TAG = "BleScanningService ";
    /**
     * Object of {@link DeviceInfo}
     */
    public DeviceInfo deviceInfo;
    /**
     * FaceREc database object.
     */
    @Inject
    FaceDatabase faceDatabase;
    public ScanCallback bleScanCallback=new ScanCallback(){@Override public void onScanResult(int callbackType,ScanResult result){super.onScanResult(callbackType,result);if(null!=result){BluetoothDevice device=result.getDevice();boolean isPaired=mBluetoothAdapter.getBondedDevices().contains(device);DeviceInfo deviceInformation=checkDeviceExitsInDb(device.getAddress());

    BleModel bleModel=new BleModel(device.getAddress(),(deviceInformation!=null&&deviceInformation.getDevice_name()!=null)?deviceInformation.getDevice_name():device.getName(),isPaired,result.getRssi(),device,"");EventBus.getDefault().post(new BleNewDeviceEvent(bleModel,deviceInformation));if(deviceInformation!=null){AppLogger.d("Test Device",deviceInformation);deviceInfo=deviceInformation;Log.e("TAG","onScanResult: "+deviceInfo.getDevice_name()+" "+deviceInfo.getMac_address());
    /*
     * if (isPaired && isRunning && !isConnectionDisable) { isConnectionDisable =
     * true; startConnection(deviceInfo.getMac_address(),
     * deviceInfo.getDevice_name()); }
     */
    if(isRunning&&!isConnectionDisable){isConnectionDisable=true;startConnection(deviceInfo.getMac_address(),deviceInfo.getDevice_name());}}}}

    @Override public void onBatchScanResults(List<ScanResult>results){super.onBatchScanResults(results);}

    @Override public void onScanFailed(int errorCode){super.onScanFailed(errorCode);}};
    /**
     * Binder object of UART service.
     */
    UARTService.UARTBinder mServiceBinder;
    /**
     * Binding connection class.
     */
    ServiceConnectionBle serviceConnectionBle;
    /**
     * Is service bounded boolean.
     */
    boolean mIsBound = false;
    /**
     * Observer object for {@link UserInformation}
     */
    UserInfoObserver userInfoObserver;

    /**
     * Observer object for {@link UserInformation}
     */
    DeviceInfoObserver deviceInfoObserver;
    /**
     * List of {@link UserInformation}
     */
    List<UserInformation> userInformationList;
    /**
     * {@link BroadcastReceiver} for ble send and receive event.
     */
    BroadcastReceiverBleStatus broadcastReceiverBleStatus;
    /**
     * {@link BroadcastReceiver} for ble status event.
     */
    BroadcastReceiverBle broadcastReceiverBle;

    /**BluetoothStatusReceiver
     * {@link BroadcastReceiver} for bluetooth status event.
     */
    BluetoothStatusReceiver bluetoothStatusReceiver;
    /**
     * Reference object of NotificationCompat.Builder
     */
    NotificationCompat.Builder notificationBuilder;
    /**
     * Reference object of {@link NotificationManager}
     */
    NotificationManager mNotifyManager;
    /**
     * Local object of user training data.
     */
    String remoteControlDataPacket;
    /**
     * Object of {@link UserInformation}
     */
    private UserInformation userInformation;
    /**
     * Ble Service running or not.
     */
    private boolean isRunning = false;
    /**
     * Database operation object.
     */
    private DatabaseOperations databaseOperations;
    /**
     * {@link BluetoothAdapter} class
     */
    private BluetoothAdapter mBluetoothAdapter;
    /**
     * Object of scanner.
     */
    private Object mLeScanner = null;
    /**
     * Object of connection
     */
    private boolean isConnectionDisable = false;
    /**
     * List of {@link DeviceInfo}
     */
    private List<DeviceInfo> deviceInfoList = new ArrayList<>();
    /**
     * Ble scanner call back.
     */
    private boolean isReceivingRemoteDataPacket = false;
    private long lengthOfRemoteFrameRemain = 0;
    private String remoteFrame = "";
    /**
     * Inject dagger shared preference object.
     */
    @Inject
    MySharedPreferences mySharedPreferences;

    private DeviceInfo checkDeviceExitsInDb(String macAdd) {
        if (deviceInfoList != null && deviceInfoList.size() > 0) {
            for (DeviceInfo deviceInformation : deviceInfoList) {
                if (deviceInformation.getMac_address().equalsIgnoreCase(macAdd)) {
                    return deviceInformation;
                }
            }
        }
        return null;
    }

    /**
     * List of byte object which used for divide the object.
     */
    private ArrayList<byte[]> dataChunks = new ArrayList<>();
    /**
     * Model class of sending ble data.
     */
    private BleSendDataModel bleSendDataModel = null;
    private boolean isSendingRemoteDataPacket = false;

    /**
     * String for the start command which is used to start command for ble.
     */
    private String startCommand = "";
    /**
     * Counter object which is incremented after send to ble tag.
     */
    private int sendingCounter = 0;
    /**
     * Command Type for the request;
     */
    private String commandType;

    /**
     * Send data to ble in chunk of data.
     *
     * @param data data which you want to send ble
     * @return array list of byte.
     */
    public static ArrayList<byte[]> convertStringToByteArrayChunks(String data) {
        ArrayList<byte[]> chunks = new ArrayList<>();

        byte[] wholeData = data.getBytes();
        int PAYLOAD_BYTES_LENGTH = AppConstants.MAX_PAYLOAD_LENGTH; // Enter Max allowed payload size here

        if (wholeData.length > PAYLOAD_BYTES_LENGTH) {

            int totalLength = wholeData.length / PAYLOAD_BYTES_LENGTH;

            for (int i = 0; i < totalLength; i++) {
                int from = i * PAYLOAD_BYTES_LENGTH;
                int to = from + PAYLOAD_BYTES_LENGTH;

                byte[] chunk = Arrays.copyOfRange(wholeData, from, to);

                chunks.add(chunk);

                // AppLogger.e("chunk", "size : " + chunk.length);
            }

            int remainingChunkSize = wholeData.length % PAYLOAD_BYTES_LENGTH;

            if (remainingChunkSize > 0) {
                int from = wholeData.length - remainingChunkSize;
                int to = wholeData.length;

                byte[] chunk = Arrays.copyOfRange(wholeData, from, to);
                // AppLogger.e("lastchunk", "size : " + chunk.length);
                chunks.add(chunk);
            }

        } else {
            // less than equal to max bytes payload
            chunks.add(wholeData);
        }

        return chunks;
    }


    /**
     * Native service life cycle method called while service created
     */
    @Override
    public void onCreate() {
        super.onCreate();
        EventBus.getDefault().register(this);
        isRunning = true;
        databaseOperations = new DatabaseOperations(faceDatabase);
        userInfoObserver = new UserInfoObserver();
        deviceInfoObserver = new DeviceInfoObserver();
        databaseOperations.getAllUsers().observeForever(userInfoObserver);
        databaseOperations.getAllDeviceInfo().observeForever(deviceInfoObserver);
        bindBleFunctions();
        setUpBle();
    }

    /**
     * Native service method
     *
     * @param intent  provide intent with action and data
     * @param flags   Provide flag of connection
     * @param startId is Service id
     * @return type of service
     */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        isRunning = true;
        if (mBluetoothAdapter != null)
            mLeScanner = mBluetoothAdapter.getBluetoothLeScanner();
        checkIntent(intent);
        startForGroundService();
        checkPermission();
        startBleScanning();
        return START_STICKY;
    }

    /**
     * Start scanning to connect with ble device
     */
    private void startBleScanning() {
        if (null != mLeScanner) {
            List<ScanFilter> scanFilters = new ArrayList<>();
            // default setting.
            final ScanSettings settings = new ScanSettings.Builder().build();
            ScanFilter scanFilter = new ScanFilter.Builder()
                    .setServiceUuid(new ParcelUuid(UARTManager.UART_SERVICE_UUID)).build();
            scanFilters.add(scanFilter);
            ((BluetoothLeScanner) mLeScanner).startScan(scanFilters, settings, bleScanCallback);
        }
    }

    /**
     * set up bluetooth environment to connect with ble
     */
    private void setUpBle() {
        // Initializes a Bluetooth adapter. For API level 18 and above, get a reference
        // to
        // BluetoothAdapter through BluetoothManager.
        final BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            mBluetoothAdapter = bluetoothManager.getAdapter();
        }
        // Checks if Bluetooth is supported on the device.
        if (mBluetoothAdapter == null) {
            Toast.makeText(this, R.string.error_bluetooth_not_supported, Toast.LENGTH_SHORT).show();
            revokePermissionNotification();
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        throw new UnsupportedOperationException("Not yet implemented");
    }

    /**
     * Check and validate the permission based on android version and start the
     * preview.
     */
    private void checkPermission() {
        if (android.os.Build.VERSION.SDK_INT > 23) {
            if (ContextCompat.checkSelfPermission(this,
                    Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                String token = mySharedPreferences.getStringData(AppConstants.TOKEN);
                if (!TextUtils.isEmpty(token)) {
                    revokePermissionNotification();
                }
            } else {
                setUpBle();
            }
        }
    }

    /**
     * Check intent to perform appropriate action on the base of intent actuin and
     * data
     *
     * @param intent intent with actions and data
     * @return boolean value to start service or not
     */
    private boolean checkIntent(Intent intent) {
        if (intent != null) {
            String action = intent.getAction();
            if (action != null && action.equalsIgnoreCase(ACTION_DISABLE_SERVICE)) {
                AppLogger.d(TAG + "onStartCommand: StopForeground");
                stopForeground(true);
                if (mServiceBinder != null) {
                    mServiceBinder.disconnect();
                }
                if (serviceConnectionBle != null) {
                    unbindService(serviceConnectionBle);
                    serviceConnectionBle = null;
                }
                mIsBound = false;
                isRunning = false;
                isConnectionDisable = false;
                sendingCounter = 0;
                if (null != deviceInfoList) {
                    deviceInfoList.clear();
                }
                if (null != userInformationList) {
                    userInformationList.clear();
                }
                mySharedPreferences.putData(AppConstants.CONNECTED_MAC_ADDRESS, "");
                EventBus.getDefault().post(new BleServiceDisableEvent(true));
                stopSelf();
                return true;
            } else if (intent.hasExtra(BleService.EXTRA_DEVICE)) {
                BluetoothDevice device = intent.getParcelableExtra(BleService.EXTRA_DEVICE);
                if (device != null && isRunning) {
                    AppLogger.d(TAG + "onStartCommand: Device Not Null");
                    EventBus.getDefault().post(new BleStatusEvent(device, BleService.STATE_CONNECTING));
                    startConnection(device.getAddress(), device.getName());
                } else {
                    AppLogger.d(TAG + "onStartCommand: Device Null");
                }
                return false;
            }
        }
        return false;
    }

    /**
     * This model called from the
     * {@link com.nxp.facemanager.activity.StartFGServiceActivity} Send start
     * command to ble device.
     *
     * @param bleSendDataModel {@link BleSendDataModel}
     */
    @Subscribe
    public void bleSendDataModel(BleSendDataModel bleSendDataModel) {
        BleSendDataModel.FrameHead frameHead = new BleSendDataModel.FrameHead("fac", 0, 0, 0,
                bleSendDataModel.getPayload().length());
        bleSendDataModel.setFrameHead(frameHead);
        bleSendDataModel.setCrc();
        this.bleSendDataModel = bleSendDataModel;
        remoteControlDataPacket = new Gson().toJson(bleSendDataModel);
        String framePre = new Gson().toJson(
                new BleSendDataPre("RT-Vision", remoteControlDataPacket.length()));
        sendCommandInformation(framePre);
        Log.d("BleSendPre", framePre);
    }



    /**
     * This model called from the {@link com.nxp.facemanager.ble.BleScanningService}
     * Let the app know that the ble is ready
     *
     * @param bleReceiveDataModel {@link BleSendDataModel}
     */
    @Subscribe
    public void bleReceiveDataModel(BleReceiveDataModel bleReceiveDataModel) {
        switch (bleReceiveDataModel.getCmd()) {
            case AppConstants.READY_INDICATION:
                AppConstants.ELOCK_READY_RECEIVED = true;
                BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.READY_INDICATION,
                       "{\"max_payload\":244}");
                EventBus.getDefault().post(bleSendDataModel);
                break;
        }
    }

    /**
     * Called event from {@link BleOnOffEvent} Stop the sevice.
     *
     * @param bleOnOffEvent {@link BleOnOffEvent}
     */
    @Subscribe
    public void bleOnOffEvent(BleOnOffEvent bleOnOffEvent) {
        if (bleOnOffEvent.isOff()) {
            AppLogger.d(TAG + ":bleOnOffEvent:");
            EventBus.getDefault().post(new BleServiceDisableEvent(true));
            stopSelf();
            isConnectionDisable = false;
            sendingCounter = 0;
        }
    }

    /**
     * When application is killed from recents menu following method will perform
     * the clearing off tasks for the app
     *
     * @param rootIntent intent from the system
     */
    @Override
    public void onTaskRemoved(Intent rootIntent) {

        if (mServiceBinder != null) {
            mServiceBinder.disconnect();
        }
        sendingCounter = 0;
        if (null != deviceInfoList) {
            deviceInfoList.clear();
        }
        if (null != userInformationList) {
            userInformationList.clear();
        }
        if (null != mySharedPreferences) {
            mySharedPreferences.putData(AppConstants.CONNECTED_MAC_ADDRESS, "");
        }

        if (notificationBuilder != null && mNotifyManager != null) {
            notificationBuilder.setProgress(0, 0, false);
            notificationBuilder.setContentText(getString(R.string.forground_service_notification));
            mNotifyManager.notify(FOREGROUND_SERVICE_NOTIFICATION, notificationBuilder.build());
        }
    }

    /**
     * Release resource here.
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);
        isRunning = false;
        isConnectionDisable = false;
        mySharedPreferences.putData(AppConstants.CONNECTED_MAC_ADDRESS, "");
        if (null != deviceInfoList) {
            deviceInfoList.clear();
        }
        if (null != userInformationList) {
            userInformationList.clear();
        }
        sendingCounter = 0;
        if (userInfoObserver != null)
            databaseOperations.getAllUsers().removeObserver(userInfoObserver);
        if (deviceInfoObserver != null)
            databaseOperations.getAllDeviceInfo().removeObserver(deviceInfoObserver);
        if (serviceConnectionBle != null) {
            unbindService(serviceConnectionBle);
            serviceConnectionBle = null;
        }
        mIsBound = false;
        bleSendDataModel = null;
        unregisterReceiver(broadcastReceiverBle);
        unregisterReceiver(broadcastReceiverBleStatus);
        unregisterReceiver(bluetoothStatusReceiver);

    }

    /**
     * start connection with iBinder Service
     *
     * @param bleAddress mac address
     * @param bleName    name of device
     */
    private void startConnection(String bleAddress, String bleName) {
        final Intent service = new Intent(this, UARTService.class);
        service.putExtra(BleService.EXTRA_DEVICE_ADDRESS, bleAddress);
        service.putExtra(BleService.EXTRA_DEVICE_NAME, bleName);
        startService(service);

        if (isRunning && (serviceConnectionBle == null || mServiceBinder == null)) {
            serviceConnectionBle = new ServiceConnectionBle();
            bindService(service, serviceConnectionBle, 0);
        }
        if (mServiceBinder != null && isRunning) {
            mServiceBinder.connect();
        }
    }

    /**
     * shows notification while User permission revoked
     */
    private void revokePermissionNotification() {
        NotificationCompat.Builder builder;
        NotificationManager mNotificationManager;
        // Create notification default intent.
        Intent intent = new Intent(this, HomeActivity.class);
        intent.putExtra("isFromNotification", true);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, 0);

        // Create notification builder.
        builder = new NotificationCompat.Builder(this, "Channel2");

        // Make notification show big text.
        NotificationCompat.BigTextStyle bigTextStyle = new NotificationCompat.BigTextStyle();
        bigTextStyle.setBigContentTitle("Application Permission Revoked");
        bigTextStyle.bigText("Please allow Location Permission");
        // Set big text style.
        builder.setStyle(bigTextStyle);

        builder.setWhen(System.currentTimeMillis());
        builder.setSmallIcon(R.mipmap.ic_launcher);
        Bitmap largeIconBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.ic_bluetooth_white_conneted);
        builder.setLargeIcon(largeIconBitmap);
        // Make the notification max priority.
        builder.setPriority(Notification.PRIORITY_MAX);
        // Make head-up notification.
        builder.setFullScreenIntent(pendingIntent, true);
        builder.setAutoCancel(true);

        // Build the notification.
        Notification notification = builder.build();
        notification.flags = Notification.FLAG_AUTO_CANCEL;

        mNotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        /* notificationID allows you to update the notification later on. */
        if (mNotificationManager != null) {
            mNotificationManager.notify(PERMISSION_NOTIFICATION, notification);
        }

    }

    /**
     * show foreground service notification
     */
    private void startForGroundService() {
        String channel;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
            channel = createChannel();
        else {
            channel = "";
        }
        mNotifyManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        Intent notificationIntent = new Intent(this, HomeActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
        Bitmap icon = BitmapFactory.decodeResource(getResources(), R.mipmap.ic_launcher);
        notificationBuilder = new NotificationCompat.Builder(this, channel).setSmallIcon(R.drawable.ic_stat_nxp)
                .setColor(getResources().getColor(R.color.statusbar_color))
                .setLargeIcon(Bitmap.createScaledBitmap(icon, 128, 128, false))
                .setContentTitle(getString(R.string.app_name))
                .setContentText(getString(R.string.forground_service_notification)).setContentIntent(pendingIntent)
                .setStyle(new NotificationCompat.BigTextStyle()
                        .bigText(getString(R.string.forground_service_notification)));
        Intent disableIntent = new Intent(this, BleScanningService.class);
        disableIntent.setAction(ACTION_DISABLE_SERVICE);
        PendingIntent pendingDisableIntent = PendingIntent.getService(this, 0, disableIntent, 0);
        NotificationCompat.Action disableAction = new NotificationCompat.Action(
                android.R.drawable.ic_menu_close_clear_cancel, "Disable", pendingDisableIntent);
        notificationBuilder.addAction(disableAction);
        Notification notification = notificationBuilder.setPriority(PRIORITY_LOW)
                .setCategory(Notification.CATEGORY_SERVICE).build();
        startForeground(FOREGROUND_SERVICE_NOTIFICATION, notification);
    }

    /**
     * Create channel
     *
     * @return string for channel
     */
    @NonNull
    @TargetApi(26)
    private synchronized String createChannel() {
        NotificationManager mNotificationManager = (NotificationManager) this
                .getSystemService(Context.NOTIFICATION_SERVICE);

        int importance = NotificationManager.IMPORTANCE_LOW;
        NotificationChannel mChannel = new NotificationChannel("Channel1", getString(R.string.app_name), importance);

        mChannel.enableLights(true);
        mChannel.setLightColor(Color.BLUE);
        if (mNotificationManager != null) {
            mNotificationManager.createNotificationChannel(mChannel);
        } else {
            stopSelf();
        }
        return "Channel1";
    }

    /**
     * to bind broadcast receiver to receive Ble event
     */
    private void bindBleFunctions() {
        // Listen to data sent and data received events
        IntentFilter dataIntentFilter = new IntentFilter();
        dataIntentFilter.addAction(UARTService.BROADCAST_UART_RX);
        dataIntentFilter.addAction(UARTService.BROADCAST_UART_TX);
        broadcastReceiverBleStatus = new BroadcastReceiverBleStatus();
        registerReceiver(broadcastReceiverBleStatus, dataIntentFilter);

        // Listen to link loss, reconnected, bonding failed events
        IntentFilter connectionIntentFilter = new IntentFilter();
        connectionIntentFilter.addAction(BROADCAST_CONNECTION_STATE);
        connectionIntentFilter.addAction(BROADCAST_DEVICE_READY);
        broadcastReceiverBle = new BroadcastReceiverBle();
        registerReceiver(broadcastReceiverBle, connectionIntentFilter);

        IntentFilter filterBluetoothStatusReceiver = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
        bluetoothStatusReceiver = new BluetoothStatusReceiver();
        registerReceiver(bluetoothStatusReceiver, filterBluetoothStatusReceiver);

    }

    private void connectBle() {
        mServiceBinder.connect();
        mIsBound = true;
    }

    /**
     * Send start command to ble tag.
     *
     * @param startCommandData String
     */
    private void sendCommandInformation(String startCommandData) {
        if (mServiceBinder != null && serviceConnectionBle != null) {
            mServiceBinder.sendBytes(startCommandData.getBytes());
        }
    }

    /**
     * Take data from chunk list and start sending.
     *
     * @param sendingCounter take data from chunk list with index.
     */
    public void sendData(int sendingCounter) {
        if (mServiceBinder != null && serviceConnectionBle != null && bleSendDataModel != null) {
            mServiceBinder.sendBytes(dataChunks.get(sendingCounter));
        }
    }

    private void bleCallBackResult(Data data, BluetoothDevice device) {
        if (data != null) {
            String strSendData = data.getStringValue(0);
            if (strSendData != null && strSendData.contains(AppConstants.USER_INFORMATION)) {
                AppLogger.d(TAG + ":BroadcastReceiverBleStatus:USER_INFORMATION" + strSendData);
                if (mySharedPreferences.getBooleanData(AppConstants.IS_APP_FORGROUND)) {
                    EventBus.getDefault().post(new BelStartTrainingDataSendEvent(true));
                }
            } else if (strSendData != null && strSendData.contains(AppConstants.PRE_DEVICE_NAME) && !isSendingRemoteDataPacket) {
                AppLogger.d(TAG + ":BroadcastReceiverBleStatus:TRAINING_INFORMATION" + strSendData);
                if (remoteControlDataPacket != null) {
                    dataChunks = convertStringToByteArrayChunks(remoteControlDataPacket);
                    AppLogger.d(TAG + ":BroadcastReceiverBleStatus:DataChunks Total:" + dataChunks.size());
                    sendingCounter = 0;
                    sendData(sendingCounter);
                    isSendingRemoteDataPacket = true;
                    // AppLogger.d(TAG + ":BroadcastReceiverBleStatus:sendingCounter:" +
                    // sendingCounter);
                    EventBus.getDefault()
                            .post(new BleSendingProgress(dataChunks.size(), sendingCounter, "Sending User Data ..."));
                    try {
                        if (notificationBuilder != null && mNotifyManager != null) {
                            notificationBuilder.setProgress(dataChunks.size() - 1, sendingCounter, false);
//                            notificationBuilder.setContentText(
//                                    "Sending " + deviceInfo.getDevice_name() + ": " + deviceInfo.getMac_address());
                            mNotifyManager.notify(FOREGROUND_SERVICE_NOTIFICATION, notificationBuilder.build());
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            } else if (isSendingRemoteDataPacket) {
                sendingCounter++;
                try {
                    if (notificationBuilder != null && mNotifyManager != null) {
                        notificationBuilder.setProgress(dataChunks.size() - 1, sendingCounter, false);
                        notificationBuilder.setContentText(
                                mServiceBinder != null ? "Sending data to: " + mServiceBinder.getDeviceAddress() : "");
                        mNotifyManager.notify(FOREGROUND_SERVICE_NOTIFICATION, notificationBuilder.build());
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                // AppLogger.d(TAG + ":BroadcastReceiverBleStatus:sendingCounter:" +
                // sendingCounter);
                if (sendingCounter < dataChunks.size()) {
                    EventBus.getDefault()
                            .post(new BleSendingProgress(dataChunks.size(), sendingCounter, "Sending User Data ..."));
                    sendData(sendingCounter);
                } else {
                    AppLogger.d(TAG + ":BroadcastReceiverBleStatus:Complete Transfer");
                    isConnectionDisable = false;
                    isSendingRemoteDataPacket = false;
                    if (mServiceBinder != null && mServiceBinder.isConnected()) {
                        if (notificationBuilder != null && mNotifyManager != null) {
                            new Handler().postDelayed(() -> {

                                notificationBuilder.setContentText("Connected to: " + mServiceBinder.getDeviceAddress()
                                        + "\n " + getString(R.string.forground_service_notification));
                                notificationBuilder.setProgress(0, 0, false);
                                notificationBuilder.setStyle(new NotificationCompat.BigTextStyle()
                                        .bigText("Connected to: " + mServiceBinder.getDeviceAddress() + "\n "
                                                + getString(R.string.forground_service_notification)));
                                mNotifyManager.notify(FOREGROUND_SERVICE_NOTIFICATION, notificationBuilder.build());
                            }, 1000);

                        }
                    } else {
                        if (notificationBuilder != null && mNotifyManager != null) {
                            notificationBuilder.setProgress(0, 0, false);
                            notificationBuilder.setContentText(getString(R.string.forground_service_notification));
                            mNotifyManager.notify(FOREGROUND_SERVICE_NOTIFICATION, notificationBuilder.build());
                        }
                    }
                }
            }
        }
    }


    /**
     * class Observe the UserInformation data object
     */
    class UserInfoObserver implements Observer<List<UserInformation>> {

        @Override
        public void onChanged(@Nullable List<UserInformation> information) {
            if (information != null && information.size() > 0) {
                userInformationList = information;
            }
        }
    }

    /**
     * class Observe the UserInformation data object
     */
    class DeviceInfoObserver implements Observer<List<DeviceInfo>> {

        @Override
        public void onChanged(@Nullable List<DeviceInfo> deviceInfos) {
            if (deviceInfos != null && deviceInfos.size() > 0) {
                deviceInfoList = new ArrayList<>();
                deviceInfoList = deviceInfos;
            }
        }
    }

    class ServiceConnectionBle implements ServiceConnection {

        @Override
        public void onServiceConnected(final ComponentName name, final IBinder service) {
            mServiceBinder = (UARTService.UARTBinder) service;
            connectBle();
        }

        @Override
        public void onServiceDisconnected(final ComponentName name) {
            mServiceBinder = null;
        }
    }

    /**
     * Broadcast receiver that listens to data sent and data received broadcasts
     * from uart service and calls corresponding activity methods
     */
    class BroadcastReceiverBleStatus extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction() != null) {

                switch (intent.getAction()) {
                    case UARTService.BROADCAST_UART_RX:
                        // data received from ble
                        BluetoothDevice device1 = intent.getParcelableExtra(BleService.EXTRA_DEVICE);
                        Data dataReceived = intent.getParcelableExtra(UARTService.EXTRA_DATA);
                        String rec_str = dataReceived.getStringValue(0);
                        Log.d("Receive data", dataReceived.getStringValue(0));

                        if (!isReceivingRemoteDataPacket) {
                            try {
                                JSONObject jsonObject = new JSONObject(dataReceived.getStringValue(0));
                                Gson gsonPre = new Gson();
                                BleSendDataPre packetPre = gsonPre.fromJson(dataReceived.getStringValue(0), BleSendDataPre.class);
                                if(packetPre.getDeviceName() != null) {
                                    lengthOfRemoteFrameRemain = packetPre.getLength();
                                    isReceivingRemoteDataPacket = true;
                                }
                            } catch (JSONException e) {
                                e.printStackTrace();
                            }
                        } else {
                            remoteFrame = remoteFrame + dataReceived.getStringValue(0);
                            Log.d(TAG, "xcs>>>>>> " + (lengthOfRemoteFrameRemain - remoteFrame.length()) +"/");
                            if (lengthOfRemoteFrameRemain - remoteFrame.length() <= 0) {
                                try {
                                    JSONObject jsonObject = new JSONObject(remoteFrame);
                                    Gson gsonPacket = new GsonBuilder()
                                            .excludeFieldsWithoutExposeAnnotation()
                                            .create();

                                    BleReceiveDataModel bleReceiveDataModel = gsonPacket.fromJson(remoteFrame, BleReceiveDataModel.class);
                                    bleReceiveDataModel.parsePayload();
                                    Log.d("Receive remote command", ":" + bleReceiveDataModel.getCmd());
                                    if (bleReceiveDataModel.getCmd()==AppConstants.READY_INDICATION){
                                        AppConstants.ELOCK_READY_RECEIVED = true;
                                    }

                                    Log.d(TAG, "xcs>>>>>> " + (lengthOfRemoteFrameRemain - remoteFrame.length()) +"/" + remoteFrame);
                                    EventBus.getDefault().post(bleReceiveDataModel);

                                    remoteFrame = "";
                                    isReceivingRemoteDataPacket = false;
                                    lengthOfRemoteFrameRemain = 0;
                                } catch (JSONException e) {
                                    remoteFrame = "";
                                    isReceivingRemoteDataPacket = false;
                                    lengthOfRemoteFrameRemain = 0;
                                    e.printStackTrace();
                                }
                            }
//                            else if (lengthOfRemoteFrameRemain < 0) {
//                                Log.e("Parse Error","isReceivingRemoteDataPacket:" + isReceivingRemoteDataPacket);
//                                remoteFrame = "";
//                                isReceivingRemoteDataPacket = false;
//                                lengthOfRemoteFrameRemain = 0;
//                            }
                        }
                        break;

                    case UARTService.BROADCAST_UART_TX:
                        // data successfully sent to ble
                        BluetoothDevice device = intent.getParcelableExtra(BleService.EXTRA_DEVICE);
                        Data data = intent.getParcelableExtra(UARTService.EXTRA_DATA);
                        Log.d(TAG, "BLE Sent data: " + data.getStringValue(0));
                        bleCallBackResult(data, device);
                        
                        databaseOperations.getAllUsers().observeForever(userInfoObserver);
                        break;

                    default:
                        throw new IllegalStateException("Unexpected value: " + intent.getAction());
                }
            }
        }
    }

    /**
     * Broadcast receiver that listens to device connection status
     */
    class BroadcastReceiverBle extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction() != null && isRunning) {
                BluetoothDevice device = intent.getParcelableExtra(BleService.EXTRA_DEVICE);
                // if (device != null && device.getBondState() == BluetoothDevice.BOND_BONDED) {
                if (device != null) {
                    switch (intent.getAction()) {
                        case BROADCAST_CONNECTION_STATE:
                            int connectionState = intent.getIntExtra(BleService.EXTRA_CONNECTION_STATE, 4);
                            if (connectionState == BleService.STATE_LINK_LOSS) {
                                AppLogger.d(TAG + ":BroadcastReceiverBle:Link Loss" + device.getAddress());
                                sendingCounter = 0;
                                isConnectionDisable = false;
                                EventBus.getDefault().post(new BleStatusEvent(device, BleService.STATE_LINK_LOSS));
                            } else if (connectionState == BleService.STATE_DISCONNECTED) {
                                AppLogger.d(TAG + ":BroadcastReceiverBle:Disconnected" + device.getAddress());
                                mySharedPreferences.putData(AppConstants.CONNECTED_MAC_ADDRESS, "");
                                isConnectionDisable = false;
                                sendingCounter = 0;
                                if (notificationBuilder != null && mNotifyManager != null) {
                                    notificationBuilder.setProgress(0, 0, false);
                                    notificationBuilder
                                            .setContentText(getString(R.string.forground_service_notification));
                                    mNotifyManager.notify(FOREGROUND_SERVICE_NOTIFICATION,
                                            notificationBuilder.build());
                                }
                                EventBus.getDefault()
                                        .post(new BleStatusEvent(device, BleService.STATE_DISCONNECTED));
                            } else if (connectionState == BleService.STATE_CONNECTED) {
                                mySharedPreferences.putData(AppConstants.CONNECTED_MAC_ADDRESS,
                                        device.getAddress());
                                isConnectionDisable = true;
                                sendingCounter = 0;
                                AppLogger.d(TAG + ":BroadcastReceiverBle:Connected:" + device.getAddress());
                                if (notificationBuilder != null && mNotifyManager != null && deviceInfo != null) {
                                    notificationBuilder
                                            .setContentText("Connected to: " + deviceInfo.getMac_address());
                                    mNotifyManager.notify(FOREGROUND_SERVICE_NOTIFICATION,
                                            notificationBuilder.build());
                                }
                                EventBus.getDefault().post(new BleStatusEvent(device, BleService.STATE_CONNECTED));
                            } else if (connectionState == BleService.STATE_CONNECTING) {
                                AppLogger.d(TAG + ":BroadcastReceiverBle:Connecting" + device.getAddress());
                                EventBus.getDefault().post(new BleStatusEvent(device, BleService.STATE_CONNECTING));
                            } else if (connectionState == BleService.STATE_DISCONNECTING) {
                                AppLogger.d(TAG + ":BroadcastReceiverBle:Disconnecting" + device.getAddress());
                                EventBus.getDefault()
                                        .post(new BleStatusEvent(device, BleService.STATE_DISCONNECTING));
                            }
                            break;
                        case BROADCAST_DEVICE_READY:
                            if (mySharedPreferences.getBooleanData(AppConstants.IS_APP_FORGROUND)) {
                                AppLogger.d(TAG + ":BroadcastReceiverBle:Device Ready Foreground");
                                BluetoothDevice btDevice = intent.getParcelableExtra(BleService.EXTRA_DEVICE);
                                mySharedPreferences.putData(AppConstants.CONNECTED_MAC_ADDRESS,
                                        btDevice.getAddress());
                                EventBus.getDefault()
                                        .post(new BleStatusEvent(btDevice, BleService.STATE_DEVICE_READY));
                            } else {
                                //cannt get here???
                                if (deviceInfo != null && isRunning) {
                                    AppLogger.d(TAG + ":BroadcastReceiverBle:Device Ready Background");
                                    String cookie = "";
                                    if (null != userInformation && null != userInformation.getCookie()) {
                                        cookie = userInformation.getCookie();
                                    }
                                    // TODO: send ready msg.
                                    BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.READY_INDICATION,
                                            deviceInfo.getDevice_name());
                                    EventBus.getDefault().post(bleSendDataModel);
                                }
                            }
                            break;
                    }
                } else if (device != null && device.getBondState() == BluetoothDevice.BOND_NONE) {
                    switch (intent.getAction()) {
                        case BROADCAST_CONNECTION_STATE:
                            int connectionState = intent.getIntExtra(BleService.EXTRA_CONNECTION_STATE, 4);
                            if (connectionState == BleService.STATE_DISCONNECTED) {
                                AppLogger.d(TAG + ":BroadcastReceiverBle:Disconnected" + device.getAddress());
                                mySharedPreferences.putData(AppConstants.CONNECTED_MAC_ADDRESS, "");
                                isConnectionDisable = false;
                                sendingCounter = 0;
                                if (notificationBuilder != null && mNotifyManager != null) {
                                    notificationBuilder.setProgress(0, 0, false);
                                    notificationBuilder
                                            .setContentText(getString(R.string.forground_service_notification));
                                    mNotifyManager.notify(FOREGROUND_SERVICE_NOTIFICATION,
                                            notificationBuilder.build());
                                }
                                EventBus.getDefault()
                                        .post(new BleStatusEvent(device, BleService.STATE_DISCONNECTED));
                            }
                            break;

                    }
                }
            }
        }
    }

    /**
     * internal ble status receiver class which performs actions within the service
     * specifically stopping the service
     */
    class BluetoothStatusReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (action != null && action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                final int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
                switch (state) {
                    case BluetoothAdapter.STATE_OFF:
                        stopSelf();
                        break;
                    case BluetoothAdapter.STATE_TURNING_OFF:
                        break;
                    case BluetoothAdapter.STATE_ON:
                        break;
                    case BluetoothAdapter.STATE_TURNING_ON:
                        break;
                }

            }
        }
    }
}

