package com.nxp.facemanager.model;

/**
 * Event bus model for enable ble.
 */
public class BleEnableEvent {
    private boolean isBleOn;
    private String fromToolbar = "";


    public BleEnableEvent(boolean isBleOn) {
        this.isBleOn = isBleOn;
    }

    public boolean isBleOn() {
        return isBleOn;
    }
}
