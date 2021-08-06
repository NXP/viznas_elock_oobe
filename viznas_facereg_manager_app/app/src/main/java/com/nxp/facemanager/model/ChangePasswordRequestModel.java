
package com.nxp.facemanager.model;

import com.google.gson.annotations.Expose;

@SuppressWarnings("unused")
public class ChangePasswordRequestModel {

    @Expose
    private String email;
    @Expose
    private String password;
    @Expose
    private String resetPasswordToken;

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public String getResetPasswordToken() {
        return resetPasswordToken;
    }

    public void setResetPasswordToken(String resetPasswordToken) {
        this.resetPasswordToken = resetPasswordToken;
    }

}
