package com.nxp.facemanager.model;

public class BleOnOffEvent {
    private boolean isOff;

    public BleOnOffEvent(boolean isOff) {
        this.isOff = isOff;
    }

    public boolean isOff() {
        return isOff;
    }

    public void setOff(boolean off) {
        isOff = off;
    }
}
