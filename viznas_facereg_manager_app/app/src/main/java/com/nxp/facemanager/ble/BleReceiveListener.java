package com.nxp.facemanager.ble;

import com.nxp.facemanager.database.entity.UserInformation;

import java.util.ArrayList;

public interface BleReceiveListener {
    public void onDataReceive(BleReceiveDataModel bleReceiveDataModel);
    public void onFailure();
}
