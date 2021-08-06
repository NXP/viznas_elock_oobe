package com.nxp.facemanager.ble;

import com.google.gson.annotations.SerializedName;
import com.nxp.facemanager.utility.AppConstants;

public class BleSendDataPre {

    @SerializedName(AppConstants.PRE_DEVICE_NAME)
    private String deviceName;

    @SerializedName(AppConstants.PRE_LENGTH)
    private long length;

    public BleSendDataPre() {
        this.deviceName = null;
        this.length = 0;
    }

    public BleSendDataPre(String deviceName, long length) {
        this.deviceName = deviceName;
        this.length = length;
    }

    public String getDeviceName() {
        return deviceName;
    }

    public long getLength() {
        return length;
    }

    public void setDeviceName(String deviceName) {
        this.deviceName = deviceName;
    }

    public void setLength(long length) {
        this.length = length;
    }

}
