package com.nxp.facemanager.ble;

import com.nxp.facemanager.utility.AppConstants;

import org.json.JSONObject;

public class BleConfigModel {
    // Use enum for categories?
    private String version = "";
    private int logVerbose = -1;
    private String displaySelection = "";
    private String lpmMode = "";
    private String detectionResolution;
    private int far;
    private boolean liveness;
    private int emotionType;
    private boolean irFilter;
    private int irPWM;
    private int whitePWM;
    private String regMode;

    public BleConfigModel(){}

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }

    public int getLogVerbose() {
        return logVerbose;
    }

    public void setLogVerbose(int logVerbose) {
        this.logVerbose = logVerbose;
    }

    public String getDisplaySelection() {
        return displaySelection;
    }

    public void setDisplaySelection(String displaySelection) {
        this.displaySelection = displaySelection;
    }

    public String getLpmMode() {
        return lpmMode;
    }

    public void setLpmMode(String lpmMode) {
        this.lpmMode = lpmMode;
    }

    public String getDetectionResolution() {
        return detectionResolution;
    }

    public void setDetectionResolution(String detectionResolution) {
        this.detectionResolution = detectionResolution;
    }

    public int getFar() {
        return far;
    }

    public void setFar(int far) {
        this.far = far;
    }

    public boolean isLiveness() {
        return liveness;
    }

    public void setLiveness(boolean liveness) {
        this.liveness = liveness;
    }

    public int getEmotionType() {
        return emotionType;
    }

    public void setEmotionType(int emotionType) {
        this.emotionType = emotionType;
    }

    public boolean isIrFilter() {
        return irFilter;
    }

    public void setIrFilter(boolean irFilter) {
        this.irFilter = irFilter;
    }

    public int getIrPWM() {
        return irPWM;
    }

    public void setIrPWM(int irPWM) {
        this.irPWM = irPWM;
    }

    public int getWhitePWM() {
        return whitePWM;
    }

    public void setWhitePWM(int whitePWM) {
        this.whitePWM = whitePWM;
    }

    public String getRegMode() {
        return regMode;
    }

    public void setRegMode(String regMode) {
        this.regMode = regMode;
    }

    @Override
    public String toString(){
        JSONObject config = new JSONObject();
        try {
            config.put(AppConstants.JSON_KEY_CONFIG_VERSION, this.version);
        } catch(Exception e){
            e.printStackTrace();
        }
        return config.toString();
    }
}
