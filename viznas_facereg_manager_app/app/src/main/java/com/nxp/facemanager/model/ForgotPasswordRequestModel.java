
package com.nxp.facemanager.model;

import com.google.gson.annotations.Expose;

@SuppressWarnings("unused")
public class ForgotPasswordRequestModel {

    @Expose
    private String email;

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }


}
