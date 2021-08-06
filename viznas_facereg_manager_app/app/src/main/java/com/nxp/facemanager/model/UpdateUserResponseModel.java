
package com.nxp.facemanager.model;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;
import com.nxp.facemanager.database.entity.UserInformation;

public class UpdateUserResponseModel {

    @SerializedName("message")
    @Expose
    private String message;
    @SerializedName("Success")
    @Expose
    private Boolean success;

    @SerializedName("user")
    @Expose
    private UserInformation userInformation;

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

    public UserInformation getUserInformation() {
        return userInformation;
    }

    public void setUserInformation(UserInformation userInformation) {
        this.userInformation = userInformation;
    }
}
