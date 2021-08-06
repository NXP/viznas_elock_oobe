package com.nxp.facemanager.model;

import com.nxp.facemanager.ble.BleModel;
import com.nxp.facemanager.database.entity.DeviceInfo;

public class BleNewDeviceEvent {
    private DeviceInfo deviceInformation;
    private BleModel bleModel;

    public BleNewDeviceEvent(BleModel bleModel, DeviceInfo deviceInformation) {
        this.bleModel = bleModel;
        this.deviceInformation = deviceInformation;
    }

    public BleModel getBleModel() {
        return bleModel;
    }

    public void setBleModel(BleModel bleModel) {
        this.bleModel = bleModel;
    }

    public DeviceInfo getDeviceInformation() {
        return deviceInformation;
    }

    public void setDeviceInformation(DeviceInfo deviceInformation) {
        this.deviceInformation = deviceInformation;
    }
}
