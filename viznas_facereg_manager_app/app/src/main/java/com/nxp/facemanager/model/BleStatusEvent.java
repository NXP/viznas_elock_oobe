package com.nxp.facemanager.model;

import android.bluetooth.BluetoothDevice;

public class BleStatusEvent {
    private BluetoothDevice bluetoothDevice;
    private int status;

    public BleStatusEvent(BluetoothDevice bluetoothDevice, int status) {
        this.bluetoothDevice = bluetoothDevice;
        this.status = status;
    }

    public BluetoothDevice getBluetoothDevice() {
        return bluetoothDevice;
    }

    public void setBluetoothDevice(BluetoothDevice bluetoothDevice) {
        this.bluetoothDevice = bluetoothDevice;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }
}
