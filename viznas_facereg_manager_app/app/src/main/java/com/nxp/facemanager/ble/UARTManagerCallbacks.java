package com.nxp.facemanager.ble;

import android.bluetooth.BluetoothDevice;

import no.nordicsemi.android.ble.BleManagerCallbacks;
import no.nordicsemi.android.ble.data.Data;

/**
 * Callbacks interface when data received or sent
 */
public interface UARTManagerCallbacks extends BleManagerCallbacks {

    void onDataReceived(final BluetoothDevice device, final Data data);

    void onDataSent(final BluetoothDevice device, final Data data);
}
