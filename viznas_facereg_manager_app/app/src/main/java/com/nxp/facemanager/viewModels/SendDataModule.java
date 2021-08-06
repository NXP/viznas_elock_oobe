package com.nxp.facemanager.viewModels;

import androidx.databinding.ObservableBoolean;
import androidx.databinding.ObservableField;
import androidx.lifecycle.ViewModel;

/**
 * This class file used in {@link com.nxp.facemanager.activity.TrainingDataSendActivity}.
 */
public class SendDataModule extends ViewModel {
    /**
     * User image string.
     */
    private ObservableField<String> userImageString = new ObservableField<>("");
    /**
     * Device name string.
     */
    private ObservableField<String> devicename = new ObservableField<>("");
    /**
     * Device Mac address string.
     */
    private ObservableField<String> macaddress = new ObservableField<>("");
    /**
     * Status of device.
     */
    private ObservableField<String> status = new ObservableField<>("");
    /**
     * Status of ble.
     * e.g connected,disconnected...
     */
    private ObservableBoolean isRunning = new ObservableBoolean(false);
    /**
     * To show progress view when start training.
     */
    private ObservableBoolean isVisible = new ObservableBoolean(true);

    /**
     * User image string
     *
     * @return ObservableField
     */
    public ObservableField<String> getUserImageString() {
        return userImageString;
    }

    public void setUserImageString(ObservableField<String> userImageString) {
        this.userImageString = userImageString;
    }

    /**
     * Return device name and bind to the view.
     *
     * @return ObservableField
     */
    public ObservableField<String> getDevicename() {
        return devicename;
    }

    public void setDevicename(ObservableField<String> devicename) {
        this.devicename = devicename;
    }

    /**
     * Return ble mac address and bind to the view.
     *
     * @return ObservableField
     */
    public ObservableField<String> getMacaddress() {
        return macaddress;
    }

    public void setMacaddress(ObservableField<String> macaddress) {
        this.macaddress = macaddress;
    }

    /**
     * Return ble connection status and bind to the view.
     *
     * @return ObservableField
     */
    public ObservableField<String> getStatus() {
        return status;
    }

    public void setStatus(ObservableField<String> status) {
        this.status = status;
    }

    /**
     * To show and hide progress bar.
     *
     * @return ObservableField
     */
    public ObservableBoolean getIsVisible() {
        return isVisible;
    }

    public void setIsVisible(ObservableBoolean isVisible) {
        this.isVisible = isVisible;
    }

    /**
     * To show and hide progress bar.
     *
     * @return ObservableField
     */
    public ObservableBoolean getIsRunning() {
        return isRunning;
    }

    public void setIsRunning(ObservableBoolean isRunning) {
        this.isRunning = isRunning;
    }
}
