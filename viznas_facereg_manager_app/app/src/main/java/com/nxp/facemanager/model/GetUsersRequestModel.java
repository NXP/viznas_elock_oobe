
package com.nxp.facemanager.model;

import com.google.gson.annotations.Expose;

@SuppressWarnings("unused")
public class GetUsersRequestModel {

    @Expose
    private String adminId;

    public String getAdminId() {
        return adminId;
    }

    public void setAdminId(String adminId) {
        this.adminId = adminId;
    }

}
