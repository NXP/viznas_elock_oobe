package com.nxp.facemanager.ble;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.nxp.facemanager.model.BleOnOffEvent;

import org.greenrobot.eventbus.EventBus;

/**
 * Receiver for state change in ble which will post event for the same and other classes who
 * have subscribed for BleOnOffEvent will receive it.
 */
public class BleOnBroadcastReceiverClass extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
            if (intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1)
                    == BluetoothAdapter.STATE_OFF) {
                EventBus.getDefault().post(new BleOnOffEvent(false));
            }
        }
    }
}
