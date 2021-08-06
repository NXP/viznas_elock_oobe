package com.nxp.facemanager.model;

public class BelStartTrainingDataSendEvent {
    private boolean isSendDataEnable;

    public BelStartTrainingDataSendEvent(boolean isDataSent) {
        this.isSendDataEnable = isDataSent;
    }

    public boolean isSendDataEnable() {
        return isSendDataEnable;
    }

    public void setSendDataEnable(boolean sendDataEnable) {
        isSendDataEnable = sendDataEnable;
    }
}
