package com.nxp.facemanager.model;

import android.bluetooth.BluetoothDevice;

public class BleScanBackgroundServiceStartScanEvent {
    private boolean isStartScan;

    public BluetoothDevice getBluetoothDevice() {
        return bluetoothDevice;
    }

    public void setBluetoothDevice(BluetoothDevice bluetoothDevice) {
        this.bluetoothDevice = bluetoothDevice;
    }

    private BluetoothDevice bluetoothDevice;

    public BleScanBackgroundServiceStartScanEvent(boolean isStartScan) {
        this.isStartScan = isStartScan;
    }

    public boolean isStartScan() {
        return isStartScan;
    }

    public void setStartScan(boolean startScan) {
        isStartScan = startScan;
    }
}
