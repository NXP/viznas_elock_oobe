
package com.nxp.facemanager.model;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class CommonResponseModel {

    @SerializedName("message")
    @Expose
    private String message;
    @SerializedName("Success")
    @Expose
    private Boolean success;

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

}
