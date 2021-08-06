package com.nxp.facemanager.ble;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import no.nordicsemi.android.ble.BleManager;
import no.nordicsemi.android.ble.BleManagerCallbacks;


/**
 * This service is responsible for performing ble connection and communication with the ble device
 * This service must be started only after successful bonding the device or with already bonded device
 */
public abstract class BleService extends Service implements BleManagerCallbacks {
    /**
     * The parameter passed when creating the service. Must contain the address of the sensor that we want to connect to
     */
    public static final String EXTRA_DEVICE_ADDRESS = "com.nxp.facemanager.EXTRA_DEVICE_ADDRESS";
    /**
     * The parameter passed when sending the status using broadcast.
     */
    public static final String BROADCAST_CONNECTION_STATE = "com.nxp.facemanager.BROADCAST_CONNECTION_STATE";
    /**
     * The parameter passed when ble service discovered.
     */
    public static final String BROADCAST_SERVICES_DISCOVERED = "com.nxp.facemanager.BROADCAST_SERVICES_DISCOVERED";
    /**
     * The parameter passed when ble ready to send data.
     */
    public static final String BROADCAST_DEVICE_READY = "com.nxp.facemanager.DEVICE_READY";
    /**
     * The parameter used for check the bonding state of ble.
     */
    public static final String BROADCAST_BOND_STATE = "com.nxp.facemanager.BROADCAST_BOND_STATE";
    /**
     * The parameter passed when any error occurred.
     */
    public static final String BROADCAST_ERROR = "com.nxp.facemanager.BROADCAST_ERROR";
    /**
     * The parameter used for passing device name.
     */
    public static final String EXTRA_DEVICE_NAME = "com.nxp.facemanager.EXTRA_DEVICE_NAME";
    /**
     * The parameter used for device object.
     */
    public static final String EXTRA_DEVICE = "com.nxp.facemanager.EXTRA_DEVICE";
    /**
     * The parameter used for device connection state string object.
     */
    public static final String EXTRA_CONNECTION_STATE = "com.nxp.facemanager.EXTRA_CONNECTION_STATE";
    /**
     * The parameter used for device bonding state string object.
     */
    public static final String EXTRA_BOND_STATE = "com.nxp.facemanager.EXTRA_BOND_STATE";
    /**
     * The parameter used to check service type
     */
    public static final String EXTRA_SERVICE_PRIMARY = "com.nxp.facemanager.EXTRA_SERVICE_PRIMARY";
    /**
     * The parameter used to check service type
     */
    public static final String EXTRA_SERVICE_SECONDARY = "com.nxp.facemanager.EXTRA_SERVICE_SECONDARY";
    /**
     * The parameter used to passed error message.
     */
    public static final String EXTRA_ERROR_MESSAGE = "com.nxp.facemanager.EXTRA_ERROR_MESSAGE";
    public static final String EXTRA_LOG_URI = "com.nxp.facemanager.EXTRA_LOG_URI";
    /**
     * The parameter used to passed error code.
     */
    public static final String EXTRA_ERROR_CODE = "com.nxp.facemanager.EXTRA_ERROR_CODE";
    /**
     * Constant for ble device link loss.
     */
    public static final int STATE_LINK_LOSS = -1;
    /**
     * Constant for ble device disconnected.
     */
    public static final int STATE_DISCONNECTED = 0;
    /**
     * Constant for ble device connected.
     */
    public static final int STATE_CONNECTED = 1;
    /**
     * Constant for ble device connecting.
     */
    public static final int STATE_CONNECTING = 2;
    /**
     * Constant for ble device disconnecting.
     */
    public static final int STATE_DISCONNECTING = 3;

    /**
     * Constant for ble device ready to send data.
     */
    public static final int STATE_DEVICE_READY = 4;

    /**
     * Connected ble device object
     */
    private BluetoothDevice mBluetoothDevice;
    /**
     * Handler to notify.
     */
    private Handler mHandler;
    /**
     * BleManager to find the ble adapter.
     */
    private BleManager mBleManager;
    /**
     * Ble device name.
     */
    private String mDeviceName;

    /**
     * Initializes the Ble Manager responsible for connecting to a single device.
     *
     * @return a new BleManager object
     */
    @SuppressWarnings("rawtypes")
    protected abstract BleManager initializeManager();

    /**
     * Returns the binder implementation. This must return class implementing the additional manager interface that may be used in the bound activity.
     *
     * @return the service binder
     */
    protected LocalBinder getBinder() {
        // default implementation returns the basic binder. You can overwrite the LocalBinder with your own, wider implementation
        return new LocalBinder();
    }

    /**
     * Called when start service with service connection from view.
     *
     * @param intent {@link Intent}
     * @return IBinder
     */
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return getBinder();
    }

    @Override
    public final void onRebind(final Intent intent) {
    }

    @Override
    public final boolean onUnbind(final Intent intent) {
        // We want the onRebind method be called if anything else binds to it again
        return true;
    }

    /**
     * Called when service start first time and initialize.
     */
    @SuppressWarnings("unchecked")
    @Override
    public void onCreate() {
        super.onCreate();
        mHandler = new Handler();
        // Initialize the manager
        mBleManager = initializeManager();
        mBleManager.setGattCallbacks(this);

    }

    /**
     * Called when start service called and contains the extra data with intent and start id.
     *
     * @param intent  {@link Intent}
     * @param flags   flags
     * @param startId startId
     * @return int
     */
    @Override
    public int onStartCommand(final Intent intent, final int flags, final int startId) {
//        if (intent == null || !intent.hasExtra(EXTRA_DEVICE_ADDRESS))
//            throw new UnsupportedOperationException("No device address at EXTRA_DEVICE_ADDRESS key");
//        mDeviceName = intent.getStringExtra(EXTRA_DEVICE_NAME);
//        final BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
//        final String deviceAddress = intent.getStringExtra(EXTRA_DEVICE_ADDRESS);
//        mBluetoothDevice = adapter.getRemoteDevice(deviceAddress);
//        mBleManager.connect(mBluetoothDevice).enqueue();


        if (intent == null || !intent.hasExtra(EXTRA_DEVICE_ADDRESS))
            throw new UnsupportedOperationException("No device address at EXTRA_DEVICE_ADDRESS key");


        mDeviceName = intent.getStringExtra(EXTRA_DEVICE_NAME);

        final BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        final String deviceAddress = intent.getStringExtra(EXTRA_DEVICE_ADDRESS);
        mBluetoothDevice = adapter.getRemoteDevice(deviceAddress);

        mBleManager.connect(mBluetoothDevice).enqueue();

        return START_REDELIVER_INTENT;
    }

    /**
     * BleManagerCallbacks and send the broadcast for device connecting mode.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onDeviceConnecting(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_CONNECTION_STATE);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_CONNECTION_STATE, STATE_CONNECTING);
        sendBroadcast(broadcast);
    }

    /**
     * This method should return false if the service needs to do some asynchronous work after if has disconnected from the device.
     * In that case the {@link #stopService()} method must be called when done.
     *
     * @return true (default) to automatically stop the service when device is disconnected. False otherwise.
     */
    protected boolean stopWhenDisconnected() {
        return true;
    }

    @Override
    public void onTaskRemoved(final Intent rootIntent) {
        super.onTaskRemoved(rootIntent);
        // This method is called when user removed the app from Recents.
        // By default, the service will be killed and recreated immediately after that.
        // However, all managed devices will be lost and devices will be disconnected.
        stopSelf();
    }

    /**
     * This method called when service stop or destroy by system.
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        // shutdown the manager
        mBleManager.close();
        mBleManager = null;
        mBluetoothDevice = null;
        mDeviceName = null;
        mHandler = null;
    }

    /**
     * BleManagerCallbacks and send the broadcast for device connected.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onDeviceConnected(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_CONNECTION_STATE);
        broadcast.putExtra(EXTRA_CONNECTION_STATE, STATE_CONNECTED);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_DEVICE_NAME, mDeviceName);
        sendBroadcast(broadcast);
    }

    /**
     * BleManagerCallbacks and send the broadcast for device disconnecting.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onDeviceDisconnecting(final BluetoothDevice device) {
        // Notify user about changing the state to DISCONNECTING
        final Intent broadcast = new Intent(BROADCAST_CONNECTION_STATE);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_CONNECTION_STATE, STATE_DISCONNECTING);
        sendBroadcast(broadcast);
    }

    /**
     * BleManagerCallbacks and send the broadcast for device disconnected.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onDeviceDisconnected(final BluetoothDevice device) {
        // Note 1: Do not use the device argument here unless you change calling onDeviceDisconnected from the binder above

        // Note 2: if BleManager#shouldAutoConnect() for this device returned true, this callback will be
        // invoked ONLY when user requested disconnection (using Disconnect button). If the device
        // disconnects due to a link loss, the onLinkLossOccurred(BluetoothDevice) method will be called instead.

        final Intent broadcast = new Intent(BROADCAST_CONNECTION_STATE);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_CONNECTION_STATE, STATE_DISCONNECTED);
        sendBroadcast(broadcast);

        if (stopWhenDisconnected())
            stopService();
    }

    /**
     * Shows a message as a Toast notification. This method is thread safe, you can call it from any thread
     *
     * @param messageResId an resource id of the message to be shown
     */
    protected void showToast(final int messageResId) {
        mHandler.post(() -> Toast.makeText(BleService.this, messageResId, Toast.LENGTH_SHORT).show());
    }

    /**
     * BleManagerCallbacks and send the broadcast for LinkLoss.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onLinkLossOccurred(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_CONNECTION_STATE);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_CONNECTION_STATE, STATE_LINK_LOSS);
        sendBroadcast(broadcast);
    }

    /**
     * For stopping service
     */
    protected void stopService() {
        // user requested disconnection. We must stop the service
        stopSelf();
    }

    /**
     * BleManagerCallbacks and send the broadcast for ServicesDiscovered.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onServicesDiscovered(final BluetoothDevice device, final boolean optionalServicesFound) {
        final Intent broadcast = new Intent(BROADCAST_SERVICES_DISCOVERED);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_SERVICE_PRIMARY, true);
        broadcast.putExtra(EXTRA_SERVICE_SECONDARY, optionalServicesFound);
        sendBroadcast(broadcast);
    }

    /**
     * BleManagerCallbacks and send the broadcast for DeviceReady.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onDeviceReady(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_DEVICE_READY);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        sendBroadcast(broadcast);
    }

    /**
     * BleManagerCallbacks and send the broadcast for DeviceNotSupported.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onDeviceNotSupported(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_SERVICES_DISCOVERED);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_SERVICE_PRIMARY, false);
        broadcast.putExtra(EXTRA_SERVICE_SECONDARY, false);
        LocalBroadcastManager.getInstance(this).sendBroadcast(broadcast);
    }

    /**
     * BleManagerCallbacks and send the broadcast for BondingRequired.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onBondingRequired(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_BOND_STATE);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_BOND_STATE, BluetoothDevice.BOND_BONDING);
        LocalBroadcastManager.getInstance(this).sendBroadcast(broadcast);
    }

    /**
     * BleManagerCallbacks and send the broadcast for onBonded.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onBonded(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_BOND_STATE);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_BOND_STATE, BluetoothDevice.BOND_BONDED);
        LocalBroadcastManager.getInstance(this).sendBroadcast(broadcast);
    }

    /**
     * BleManagerCallbacks and send the broadcast for onBondingFailed.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onBondingFailed(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_BOND_STATE);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_BOND_STATE, BluetoothDevice.BOND_NONE);
        LocalBroadcastManager.getInstance(this).sendBroadcast(broadcast);
    }

    /**
     * BleManagerCallbacks and send the broadcast for onError.
     *
     * @param device {@link BluetoothDevice}
     */
    @Override
    public void onError(final BluetoothDevice device, final String message, final int errorCode) {
        final Intent broadcast = new Intent(BROADCAST_ERROR);
        broadcast.putExtra(EXTRA_DEVICE, mBluetoothDevice);
        broadcast.putExtra(EXTRA_ERROR_MESSAGE, message);
        broadcast.putExtra(EXTRA_ERROR_CODE, errorCode);
        LocalBroadcastManager.getInstance(this).sendBroadcast(broadcast);
    }

    /**
     * Shows a message as a Toast notification. This method is thread safe, you can call it from any thread
     *
     * @param message a message to be shown
     */
    protected void showToast(final String message) {
        mHandler.post(() -> Toast.makeText(BleService.this, message, Toast.LENGTH_SHORT).show());
    }

    /**
     * Returns the device address
     *
     * @return device address
     */
    protected String getDeviceAddress() {
        return mBluetoothDevice.getAddress();
    }

//    public class LocalBinder extends Binder {
//        /**
//         * Disconnects from the sensor.
//         */
//        public final void disconnect() {
//
//            if (isConnected()) {
//                final int state = mBleManager.getConnectionState();
//                if (state == BluetoothGatt.STATE_DISCONNECTED || state == BluetoothGatt.STATE_DISCONNECTING) {
//                    mBleManager.close();
//                    onDeviceDisconnected(mBluetoothDevice);
//                    return;
//                }
//
//                mBleManager.disconnect().enqueue();
//            } else {
//
//                stopSelf();
//            }
//        }
//
//        /**
//         * Connect the device with service binder object.
//         *
//         * @param macAddress device mac address.
//         * @param name       device name
//         */
//        public final void connect(String macAddress, String name) {
//            if (!isConnected()) {
//                setDeviceAddress(macAddress);
//                setmDeviceName(name);
//                final BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
//                if (deviceAddress != null) {
//                    mBluetoothDevice = adapter.getRemoteDevice(deviceAddress);
//                    mBleManager.connect(mBluetoothDevice).enqueue();
//                }
//            }
//
//        }
//
//        /**
//         * Returns the device address
//         *
//         * @return device address
//         */
//        String getDeviceAddress() {
//            return mBluetoothDevice.getAddress();
//        }
//
//        void setDeviceAddress(String deviceAddress1) {
//            deviceAddress = deviceAddress1;
//        }
//
//        /**
//         * Returns the device name
//         *
//         * @return the device name
//         */
//        String getDevice_name() {
//            return mDeviceName;
//        }
//
//        /**
//         * Returns the Bluetooth device
//         *
//         * @return the Bluetooth device
//         */
//        public BluetoothDevice getBluetoothDevice() {
//            return mBluetoothDevice;
//        }
//
//        /**
//         * Returns <code>true</code> if the device is connected to the sensor.
//         *
//         * @return <code>true</code> if device is connected to the sensor, <code>false</code> otherwise
//         */
//        public boolean isConnected() {
//            if (mBleManager != null) {
//                mBleManager.isConnected();
//            }
//            return false;
//        }
//
//        /**
//         * Returns the connection state of given device.
//         *
//         * @return the connection state, as in {@link BleManager#getConnectionState()}.
//         */
//        public int getConnectionState() {
//            return mBleManager.getConnectionState();
//        }
//
//        void setmDeviceName(String mDeviceName1) {
//            mDeviceName = mDeviceName1;
//        }
//
//    }

    /**
     * Returns the Bluetooth device object
     *
     * @return bluetooth device
     */
    protected BluetoothDevice getBluetoothDevice() {
        return mBluetoothDevice;
    }

    /**
     * Returns the device name
     *
     * @return the device name
     */
    protected String getDeviceName() {
        return mDeviceName;
    }

    /**
     * Returns <code>true</code> if the device is connected to the sensor.
     *
     * @return <code>true</code> if device is connected to the sensor, <code>false</code> otherwise
     */
    protected boolean isConnected() {
        return mBleManager != null && mBleManager.isConnected();
    }

    public class LocalBinder extends Binder {
        /**
         * Disconnects from the sensor.
         */
        public final void disconnect() {
            if (isConnected()) {
                final int state = mBleManager.getConnectionState();
                if (state == BluetoothGatt.STATE_DISCONNECTED || state == BluetoothGatt.STATE_DISCONNECTING) {
                    mBleManager.close();
                    onDeviceDisconnected(mBluetoothDevice);
                    return;
                }
                mBleManager.disconnect().enqueue();
            } else {
                stopSelf();
            }
        }

        /**
         * To connect the device
         */
        public final void connect() {
            if (!isConnected()) {
                final BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
                if (mBluetoothDevice != null) {
                    mBluetoothDevice = adapter.getRemoteDevice(mBluetoothDevice.getAddress());
                    mBleManager.connect(mBluetoothDevice).enqueue();
                }
            }
        }

        /**
         * Returns the device address
         *
         * @return device address
         */
        String getDeviceAddress() {
            return mBluetoothDevice.getAddress();
        }

        /**
         * Returns the device name
         *
         * @return the device name
         */
        public String getDeviceName() {
            return mDeviceName;
        }

        /**
         * Returns the Bluetooth device
         *
         * @return the Bluetooth device
         */
        public BluetoothDevice getBluetoothDevice() {
            return mBluetoothDevice;
        }

        /**
         * Returns <code>true</code> if the device is connected to the sensor.
         *
         * @return <code>true</code> if device is connected to the sensor, <code>false</code> otherwise
         */
        public boolean isConnected() {
            return mBleManager.isConnected();
        }


        /**
         * Returns the connection state of given device.
         *
         * @return the connection state, as in {@link BleManager#getConnectionState()}.
         */
        public int getConnectionState() {
            return mBleManager.getConnectionState();
        }


    }
}
