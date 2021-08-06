package com.nxp.facemanager.model;

import android.bluetooth.BluetoothDevice;

/**
 * Event bus model to detect the bond state of device.
 */
public class PairingEventModel {
    private BluetoothDevice bluetoothDevice;

    public PairingEventModel(BluetoothDevice bluetoothDevice) {
        this.bluetoothDevice = bluetoothDevice;
    }

    public BluetoothDevice getBluetoothDevice() {
        return bluetoothDevice;
    }

}
