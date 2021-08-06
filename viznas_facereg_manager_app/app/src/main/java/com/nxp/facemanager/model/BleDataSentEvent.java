package com.nxp.facemanager.model;

public class BleDataSentEvent {
    private boolean isDataSent;

    public BleDataSentEvent(boolean isDataSent) {
        this.isDataSent = isDataSent;
    }

    public boolean isDataSent() {
        return isDataSent;
    }

    public void setDataSent(boolean dataSent) {
        isDataSent = dataSent;
    }
}
