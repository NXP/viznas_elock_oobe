package com.nxp.facemanager.model;

public class BleSendingProgress {
    private int maxProgress;
    private int runningProgress;
    private String dialogMessage;

    public String getDialogMessage() {
        return dialogMessage;
    }

    public void setDialogMessage(String dialogMessage) {
        this.dialogMessage = dialogMessage;
    }

    public BleSendingProgress(int maxProgress, int runningProgress, String dialogMessage) {
        this.maxProgress = maxProgress;
        this.runningProgress = runningProgress;
        this.dialogMessage = dialogMessage;

    }

    public int getMaxProgress() {
        return maxProgress;
    }

    public void setMaxProgress(int maxProgress) {
        this.maxProgress = maxProgress;
    }

    public int getProgress() {
        return runningProgress;
    }

    public void setProgress(int progress) {
        this.runningProgress = progress;
    }
}
