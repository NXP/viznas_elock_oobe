
package com.nxp.facemanager.model;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;
import com.nxp.facemanager.database.entity.UserInformation;

public class LoginResponseModel {

    @Expose
    private String message;
    @SerializedName("Success")
    @Expose
    private Boolean success;
    @Expose
    private String token;
    @Expose
    @SerializedName("user")
    private UserInformation user;

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public Boolean getSuccess() {
        return success;
    }

    public void setSuccess(Boolean success) {
        this.success = success;
    }

    public String getToken() {
        return token;
    }

    public void setToken(String token) {
        this.token = token;
    }

    public UserInformation getUser() {
        return user;
    }

    public void setUser(UserInformation user) {
        this.user = user;
    }




}

