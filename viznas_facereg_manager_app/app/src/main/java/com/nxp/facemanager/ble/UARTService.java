package com.nxp.facemanager.ble;

import android.bluetooth.BluetoothDevice;
import android.content.Intent;

import no.nordicsemi.android.ble.BleManager;
import no.nordicsemi.android.ble.data.Data;


/**
 * This service interacts with Uart manager to send
 * and receive data from ble device and broadcast it within the application
 */
public class UARTService extends BleService implements UARTManagerCallbacks {
    public static final String BROADCAST_UART_TX = "com.nxp.facemanager.uart.BROADCAST_UART_TX";
    public static final String BROADCAST_UART_RX = "com.nxp.facemanager.uart.BROADCAST_UART_RX";
    public static final String EXTRA_DATA = "com.nxp.facemanager.uart.EXTRA_DATA";
    private static final String TAG = "UARTService";
    private final LocalBinder mBinder = new UARTBinder();
    private UARTManager mManager;

    @Override
    protected LocalBinder getBinder() {
        return mBinder;
    }

    @Override
    protected BleManager<UARTManagerCallbacks> initializeManager() {
        return mManager = new UARTManager(this);
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onDeviceConnected(final BluetoothDevice device) {
        super.onDeviceConnected(device);
    }

    @Override
    protected boolean stopWhenDisconnected() {
        return false;
    }

    @Override
    public void onDeviceDisconnected(final BluetoothDevice device) {
        super.onDeviceDisconnected(device);
    }

    @Override
    public void onLinkLossOccurred(final BluetoothDevice device) {
        super.onLinkLossOccurred(device);
    }

    @Override
    public void onDataReceived(final BluetoothDevice device, final Data data) {
        final Intent broadcast = new Intent(BROADCAST_UART_RX);
        broadcast.putExtra(EXTRA_DEVICE, getBluetoothDevice());
        broadcast.putExtra(EXTRA_DATA, data);
        sendBroadcast(broadcast);
    }

    @Override
    public void onDataSent(final BluetoothDevice device, final Data data) {
        final Intent broadcast = new Intent(BROADCAST_UART_TX);
        broadcast.putExtra(EXTRA_DEVICE, getBluetoothDevice());
        broadcast.putExtra(EXTRA_DATA, data);
        sendBroadcast(broadcast);
    }

    @Override
    public void onBondingRequired(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_BOND_STATE);
        broadcast.putExtra(EXTRA_DEVICE, device);
        broadcast.putExtra(EXTRA_BOND_STATE, BluetoothDevice.BOND_BONDING);
        sendBroadcast(broadcast);
    }

    @Override
    public void onBonded(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_BOND_STATE);
        broadcast.putExtra(EXTRA_DEVICE, device);
        broadcast.putExtra(EXTRA_BOND_STATE, BluetoothDevice.BOND_BONDED);
        sendBroadcast(broadcast);
    }

    @Override
    public void onBondingFailed(final BluetoothDevice device) {
        final Intent broadcast = new Intent(BROADCAST_BOND_STATE);
        broadcast.putExtra(EXTRA_DEVICE, device);
        broadcast.putExtra(EXTRA_BOND_STATE, BluetoothDevice.BOND_NONE);
        sendBroadcast(broadcast);
    }

    public class UARTBinder extends LocalBinder implements UARTInterface {
        @Override
        public void sendBytes(byte[] data) {
            mManager.sendBytes(data);
        }
    }

}
