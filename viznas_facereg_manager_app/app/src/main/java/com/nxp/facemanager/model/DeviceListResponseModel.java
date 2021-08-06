
package com.nxp.facemanager.model;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;
import com.nxp.facemanager.database.entity.DeviceInfo;

import java.util.ArrayList;

public class DeviceListResponseModel {

    @SerializedName("Success")
    @Expose
    private boolean Success;

    @SerializedName("message")
    @Expose
    private String message;

    @SerializedName("devices")
    @Expose
    private ArrayList<DeviceInfo> deviceArrayList;


    public boolean getSuccess() {
        return Success;
    }

    public void setSuccess(boolean success) {
        Success = success;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public ArrayList<DeviceInfo> getDeviceArrayList() {
        return deviceArrayList;
    }

    public void setDeviceArrayList(ArrayList<DeviceInfo> deviceArrayList) {
        this.deviceArrayList = deviceArrayList;
    }

//    public class Device {
//
//        @SerializedName("_id")
//        @Expose
//        private String deviceId;
//
//        @SerializedName("deviceName")
//        @Expose
//        private String deviceName;
//
//        @SerializedName("adminId")
//        @Expose
//        private String adminId;
//
//        @SerializedName("userIds")
//        @Expose
//        private ArrayList<String> userIds = new ArrayList<>();
//
//        @SerializedName("passcode")
//        @Expose
//        private int passcode;
//
//
//        public String getDeviceId() {
//            return deviceId;
//        }
//
//        public void setDeviceId(String deviceId) {
//            this.deviceId = deviceId;
//        }
//
//        public String getDeviceName() {
//            return deviceName;
//        }
//
//        public void setDeviceName(String deviceName) {
//            this.deviceName = deviceName;
//        }
//
//        public String getAdminId() {
//            return adminId;
//        }
//
//        public void setAdminId(String adminId) {
//            this.adminId = adminId;
//        }
//
//        public ArrayList<String> getUserIds() {
//            return userIds;
//        }
//
//        public void setUserIds(ArrayList<String> userIds) {
//            this.userIds = userIds;
//        }
//
//        public int getPasscode() {
//            return passcode;
//        }
//
//        public void setPasscode(int passcode) {
//            this.passcode = passcode;
//        }
//    }
}
