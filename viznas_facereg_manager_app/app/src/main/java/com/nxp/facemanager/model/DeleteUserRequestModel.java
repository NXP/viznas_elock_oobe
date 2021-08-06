
package com.nxp.facemanager.model;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

@SuppressWarnings("unused")
public class DeleteUserRequestModel {

    @Expose
    @SerializedName("userId")
    private String mUserId;

    public String getUserId() {
        return mUserId;
    }

    public void setUserId(String userId) {
        mUserId = userId;
    }

}
