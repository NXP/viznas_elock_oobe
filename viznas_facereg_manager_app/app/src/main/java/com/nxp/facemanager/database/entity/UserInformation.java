package com.nxp.facemanager.database.entity;

import android.os.Parcel;
import android.os.Parcelable;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

import java.util.Set;

import androidx.annotation.NonNull;
import androidx.room.ColumnInfo;
import androidx.room.Entity;
import androidx.room.Ignore;
import androidx.room.PrimaryKey;
import androidx.room.TypeConverters;


/**
 * User Information Entity/Table store in Room Database.
 */
@Entity(tableName = "user_information")
public class UserInformation implements Parcelable {

    @PrimaryKey(autoGenerate = true)
    @ColumnInfo(name = "local_user_id")
    private int local_user_id;

    @SerializedName("_id")
    @Expose
    //@NonNull
    @ColumnInfo(name = "_id")
    private String _id = "";

    //@NonNull
    @SerializedName("email")
    @Expose
    @ColumnInfo(name = "email")
    private String email;

    @SerializedName("name")
    @Expose
    @ColumnInfo(name = "name")
    private String name;

    @SerializedName("phoneNo")
    @Expose
    @ColumnInfo(name = "phoneNo")
    private String phoneNo;

    @SerializedName("trainingData")
    @Expose
    @ColumnInfo(name = "trainingData")
    private String trainingData;

    @SerializedName("profilePic")
    @Expose
    @ColumnInfo(name = "profilePic")
    private String profilePic;

    //    @NonNull
    @SerializedName("role")
    @Expose
    @ColumnInfo(name = "role")
    private int role;// 0 = admin & 1 = guest

    //    @NonNull
    @SerializedName("status")
    @Expose
    @ColumnInfo(name = "status")
    private int status;

    @SerializedName("createdAt")
    @Expose
//    @NonNull
    @ColumnInfo(name = "createdAt")
    private long createdAt;

    //    @NonNull
    @SerializedName("updatedAt")
    @Expose
    @ColumnInfo(name = "updatedAt")
    private long updatedAt;

    //    @NonNull
    @SerializedName("deviceIds")
    @Expose
    @ColumnInfo(name = "deviceIds")
    @TypeConverters(SetConverter.class)
    private Set<String> deviceIds;

    @SerializedName("adminId")
    @Expose
    @ColumnInfo(name = "adminId")
    private String adminId;

    @Expose
    @SerializedName("isAdmin")
    @ColumnInfo(name = "isAdmin")
    private boolean isAdmin;

    @ColumnInfo(name = "cookie")
    private String cookie;

    @Ignore
    public UserInformation() {
    }


    public UserInformation(int local_user_id, String _id, String email, String name, String phoneNo, String trainingData, String profilePic, int role, int status, long createdAt, long updatedAt, Set<String> deviceIds, String adminId, boolean isAdmin, String cookie) {
        this.local_user_id = local_user_id;
        this._id = _id;
        this.email = email;
        this.name = name;
        this.phoneNo = phoneNo;
        this.trainingData = trainingData;
        this.profilePic = profilePic;
        this.role = role;
        this.status = status;
        this.createdAt = createdAt;
        this.updatedAt = updatedAt;
        this.deviceIds = deviceIds;
        this.adminId = adminId;
        this.isAdmin = isAdmin;
        this.cookie = cookie;
    }

    protected UserInformation(Parcel in) {
        local_user_id = in.readInt();
        _id = in.readString();
        email = in.readString();
        name = in.readString();
        phoneNo = in.readString();
        trainingData = in.readString();
        profilePic = in.readString();
        role = in.readInt();
        status = in.readInt();
        createdAt = in.readLong();
        updatedAt = in.readLong();
        adminId = in.readString();
        isAdmin = in.readByte() != 0;
        cookie = in.readString();
    }

    public static final Creator<UserInformation> CREATOR = new Creator<UserInformation>() {
        @Override
        public UserInformation createFromParcel(Parcel in) {
            return new UserInformation(in);
        }

        @Override
        public UserInformation[] newArray(int size) {
            return new UserInformation[size];
        }
    };

    @NonNull
    public String getAdminId() {
        return adminId;
    }

    public void setAdminId(@NonNull String adminId) {
        this.adminId = adminId;
    }

    public int getLocal_user_id() {
        return local_user_id;
    }

    public void setLocal_user_id(int local_user_id) {
        this.local_user_id = local_user_id;
    }

    public String get_id() {
        return _id;
    }

    public void set_id(String _id) {
        this._id = _id;
    }

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getPhoneNo() {
        return phoneNo;
    }

    public void setPhoneNo(String phoneNo) {
        this.phoneNo = phoneNo;
    }

    public String getCookie() {
        return cookie;
    }

    public void setCookie(String cookie) {
        this.cookie = cookie;
    }

    public String getTrainingData() {
        return trainingData;
    }

    public void setTrainingData(String trainingData) {
        this.trainingData = trainingData;
    }

    public String getProfilePic() {
        return profilePic;
    }

    public void setProfilePic(String profilePic) {
        this.profilePic = profilePic;
    }

    public int getRole() {
        return role;
    }

    public void setRole(int role) {
        this.role = role;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    public long getCreatedAt() {
        return createdAt;
    }

    public void setCreatedAt(long createdAt) {
        this.createdAt = createdAt;
    }

    public long getUpdatedAt() {
        return updatedAt;
    }

    public void setUpdatedAt(long updatedAt) {
        this.updatedAt = updatedAt;
    }

    public Set<String> getDeviceIds() {
        return deviceIds;
    }

    public void setDeviceIds(Set<String> deviceIds) {
        this.deviceIds = deviceIds;
    }

    public boolean isAdmin() {
        return isAdmin;
    }

    public void setAdmin(boolean admin) {
        isAdmin = admin;
    }


    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(local_user_id);
        dest.writeString(_id);
        dest.writeString(email);
        dest.writeString(name);
        dest.writeString(phoneNo);
        dest.writeString(trainingData);
        dest.writeString(profilePic);
        dest.writeInt(role);
        dest.writeInt(status);
        dest.writeLong(createdAt);
        dest.writeLong(updatedAt);
        dest.writeString(adminId);
        dest.writeByte((byte) (isAdmin ? 1 : 0));
        dest.writeString(cookie);
    }
}
