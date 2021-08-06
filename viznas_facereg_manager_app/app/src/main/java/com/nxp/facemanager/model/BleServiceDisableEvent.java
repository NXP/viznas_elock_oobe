package com.nxp.facemanager.model;

public class BleServiceDisableEvent {
    private boolean isServiceDisable;

    public BleServiceDisableEvent(boolean serviceDesable) {
        isServiceDisable = serviceDesable;
    }

    public boolean isServiceDisable() {
        return isServiceDisable;
    }

    public void setServiceDisable(boolean serviceDisable) {
        isServiceDisable = serviceDisable;
    }
}
