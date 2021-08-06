package com.nxp.facemanager.ble;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.Context;

import com.nxp.facemanager.utility.AppLogger;

import java.util.UUID;

import androidx.annotation.NonNull;
import no.nordicsemi.android.ble.BleManager;
import no.nordicsemi.android.ble.WriteRequest;
import no.nordicsemi.android.log.LogContract;

/**
 * This class is responsible for sending and receiving byte array to connected ble device through RX and TX characteristics
 */
public class UARTManager extends BleManager<UARTManagerCallbacks> {
    /**
     * Ble device UART Service UUID
     */
    public final static UUID UART_SERVICE_UUID = UUID.fromString("01FF0100-BA5E-F4EE-5CA1-EB1E5E4B1CE0");
    /**
     * RX characteristic UUID for user training data.
     */
    private final static UUID UART_RX_CHARACTERISTIC_UUID = UUID.fromString("01FF0101-BA5E-F4EE-5CA1-EB1E5E4B1CE0");
    /**
     * TX characteristic UUID
     */
    private final static UUID UART_TX_CHARACTERISTIC_UUID = UUID.fromString("01FF0102-BA5E-F4EE-5CA1-EB1E5E4B1CE0");

    private BluetoothGattCharacteristic mRXCharacteristic, mTXCharacteristic;
    /**
     * BluetoothGatt callbacks for connection/disconnection, service discovery,
     * receiving indication, etc.
     */
    private final BleManagerGattCallback mGattCallback = new BleManagerGattCallback() {

        @Override
        protected void initialize() {
            setNotificationCallback(mTXCharacteristic)
                    .with((device, data) -> {
                        final String text = data.getStringValue(0);

//						log(LogContract.AppLogger.Level.APPLICATION, "\"" + text + "\" received");
                        mCallbacks.onDataReceived(device, data);
                    });
            requestMtu(256).enqueue();// including android 3 byte.
            enableNotifications(mTXCharacteristic).enqueue();
        }

        @Override
        public boolean isRequiredServiceSupported(@NonNull final BluetoothGatt gatt) {
            final BluetoothGattService service = gatt.getService(UART_SERVICE_UUID);
            if (service != null) {
                mRXCharacteristic = service.getCharacteristic(UART_RX_CHARACTERISTIC_UUID);
                mTXCharacteristic = service.getCharacteristic(UART_TX_CHARACTERISTIC_UUID);

            }

            boolean writeRequest = false;
            boolean writeCommand = false;
            if (mRXCharacteristic != null) {
                final int rxProperties = mRXCharacteristic.getProperties();
                writeRequest = (rxProperties & BluetoothGattCharacteristic.PROPERTY_WRITE) > 0;
                writeCommand = (rxProperties & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) > 0;

                // Set the WRITE REQUEST type when the characteristic supports it.
                // This will allow to send long write (also if the characteristic support it).
                // In case there is no WRITE REQUEST property, this manager will divide texts
                // longer then MTU-3 bytes into up to MTU-3 bytes chunks.
                if (writeRequest)
                    mRXCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);

            }


            return mRXCharacteristic != null && mTXCharacteristic != null && (writeRequest || writeCommand);
//            return true;
        }

        @Override
        protected void onDeviceDisconnected() {
            overrideMtu(23);
            mRXCharacteristic = null;
        }
    };

    UARTManager(final Context context) {
        super(context);
    }

    @NonNull
    @Override
    protected BleManagerGattCallback getGattCallback() {
        return mGattCallback;
    }

    @Override
    protected boolean shouldAutoConnect() {
        // We want the connection to be kept
        return false;
    }


    /**
     * Sends the given bytes to RX characteristic.
     *
     * @param bleData the ble data to be sent in bytes
     */
    void sendBytes(byte[] bleData) {
        // Are we connected?
        if (mRXCharacteristic == null)
            return;

        if (bleData != null) {
            WriteRequest request = writeCharacteristic(mRXCharacteristic, bleData).with((device, data) -> {
                mCallbacks.onDataSent(device, data);
                AppLogger.e("sendBytes", "onDataSent:" + data.getStringValue(0));

            });
            request.enqueue();

        }
    }


}
