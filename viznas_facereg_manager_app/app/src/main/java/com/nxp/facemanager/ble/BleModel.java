package com.nxp.facemanager.ble;

import android.bluetooth.BluetoothDevice;
import android.os.Parcel;
import android.os.Parcelable;

/**
 * Model class to store the Bluetooth device information when scanning the device.
 */
public class BleModel implements Parcelable {

    public static final Parcelable.Creator<BleModel> CREATOR = new Parcelable.Creator<BleModel>() {
        @Override
        public BleModel createFromParcel(Parcel source) {
            return new BleModel(source);
        }

        @Override
        public BleModel[] newArray(int size) {
            return new BleModel[size];
        }
    };
    /**
     * BLE mac address.
     */
    private String macAddress;
    /**
     * BLE name
     */
    private String devicename;
    /**
     * check for device paired or not.
     */
    private boolean isPaired;
    /**
     * Rssi value ble device
     */
    private int rssi;
    /**
     * BLE device object
     */
    private BluetoothDevice bluetoothDevice;
    /**
     * status of ble device.
     */
    private String status;

    public BleModel(String macAddress, String devicename, boolean isPaired, int rssi, BluetoothDevice bluetoothDevice, String status) {
        this.macAddress = macAddress;
        this.devicename = devicename;
        this.isPaired = isPaired;
        this.rssi = rssi;
        this.bluetoothDevice = bluetoothDevice;
        this.status = status;
    }

    private BleModel(Parcel in) {
        this.macAddress = in.readString();
        this.devicename = in.readString();
        this.isPaired = in.readByte() != 0;
        this.rssi = in.readInt();
        this.bluetoothDevice = in.readParcelable(BluetoothDevice.class.getClassLoader());
    }

    public String getMacAddress() {
        return macAddress;
    }

    public void setMacAddress(String macAddress) {
        this.macAddress = macAddress;
    }

    public String getDevicename() {
        return devicename;
    }

    public void setDevicename(String devicename) {
        this.devicename = devicename;
    }

    public boolean isPaired() {
        return isPaired;
    }

    public void setPaired(boolean paired) {
        isPaired = paired;
    }

    public BluetoothDevice getBluetoothDevice() {
        return bluetoothDevice;
    }

    public void setBluetoothDevice(BluetoothDevice bluetoothDevice) {
        this.bluetoothDevice = bluetoothDevice;
    }

    public int getRssi() {
        return rssi;
    }

    void setRssi(int rssi) {
        this.rssi = rssi;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.macAddress);
        dest.writeString(this.devicename);
        dest.writeByte(this.isPaired ? (byte) 1 : (byte) 0);
        dest.writeInt(this.rssi);
        dest.writeParcelable(this.bluetoothDevice, flags);
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof BleModel) {
            BleModel newModel = (BleModel) obj;
            return getMacAddress().equalsIgnoreCase(newModel.getMacAddress());
        }
        return false;
    }
}
