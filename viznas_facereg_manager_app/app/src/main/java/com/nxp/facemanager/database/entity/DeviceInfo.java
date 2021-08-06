package com.nxp.facemanager.database.entity;

import android.os.Parcel;
import android.os.Parcelable;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

import java.util.HashSet;
import java.util.Set;

import androidx.annotation.NonNull;
import androidx.room.ColumnInfo;
import androidx.room.Entity;
import androidx.room.Ignore;
import androidx.room.PrimaryKey;
import androidx.room.TypeConverters;

/**
 * Device Information Entity/Table store in Room Database.
 */
@Entity(tableName = "device_info")
public class DeviceInfo implements Parcelable {

    public static final Creator<DeviceInfo> CREATOR = new Creator<DeviceInfo>() {
        @Override
        public DeviceInfo createFromParcel(Parcel in) {
            return new DeviceInfo(in);
        }

        @Override
        public DeviceInfo[] newArray(int size) {
            return new DeviceInfo[size];
        }
    };

    /**
     * Server device id.
     */
    @SerializedName("_id")
    @Expose
    @NonNull
    @ColumnInfo(name = "_id")
    private String _id = "";
    /**
     * Mac address of device.
     */
    @SuppressWarnings("NullableProblems")
    @PrimaryKey
    @ColumnInfo(name = "mac_address")
    @NonNull
    private String mac_address = "";
    /**
     * User given device name.
     */
    @SerializedName("deviceName")
    @ColumnInfo(name = "deviceName")
    @NonNull
    @Expose
    private String device_name = "";
    /**
     * User given pass code.
     */
    @SerializedName("passcode")
    @ColumnInfo(name = "passcode")
    @NonNull
    @Expose
    private int pass_code = 0;

    /**
     * User given pass code.
     */
    @SerializedName("adminId")
    @ColumnInfo(name = "adminId")
    @NonNull
    @Expose
    private String adminId = "";
    /**
     * List of users which can access the lock.
     */
    @SerializedName("userIds")
    @Expose
    @ColumnInfo(name = "userIds")
    @TypeConverters(SetConverter.class)
    private Set<String> userIdList = new HashSet<>();

    @ColumnInfo(name = "isConnected")
    private boolean isConnected = false;

    public DeviceInfo(String _id, String mac_address, String device_name, int pass_code, String adminId, Set<String> userIdList) {
        this._id = _id;
        this.mac_address = mac_address;
        this.device_name = device_name;
        this.pass_code = pass_code;
        this.adminId = adminId;
        this.userIdList = userIdList;
    }

    protected DeviceInfo(Parcel in) {
        _id = in.readString();
        mac_address = in.readString();
        device_name = in.readString();
        pass_code = in.readInt();
    }

    @Ignore
    public DeviceInfo() {
    }

    public boolean isConnected() {
        return isConnected;
    }

    public void setConnected(boolean connected) {
        isConnected = connected;
    }

    public String getAdminId() {
        return adminId;
    }

    public void setAdminId(String adminId) {
        this.adminId = adminId;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(_id);
        dest.writeString(mac_address);
        dest.writeString(device_name);
        dest.writeInt(pass_code);
    }

    public String get_id() {
        return _id;
    }

    public void set_id(String _id) {
        this._id = _id;
    }

    public String getMac_address() {
        return mac_address;
    }

    public void setMac_address(String mac_address) {
        this.mac_address = mac_address;
    }

    public String getDevice_name() {
        return device_name;
    }

    public void setDevice_name(String device_name) {
        this.device_name = device_name;
    }

    public int getPass_code() {
        return pass_code;
    }

    public void setPass_code(int pass_code) {
        this.pass_code = pass_code;
    }

    public Set<String> getUserIdList() {
        if (userIdList == null) userIdList = new HashSet<>();
        return userIdList;
    }

    public void setUserIdList(Set<String> userIdList) {
        this.userIdList = userIdList;
    }
}
