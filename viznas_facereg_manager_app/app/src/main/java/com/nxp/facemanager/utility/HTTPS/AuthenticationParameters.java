package com.nxp.facemanager.utility.HTTPS;

/**
 * Authentication parameters, including client cert, server cert, user name, and password.
 */
public class AuthenticationParameters {
    private String caCertificate = null;

    public String getCaCertificate() {
        return caCertificate;
    }

    public void setCaCertificate(String caCertificate) {
        this.caCertificate = caCertificate;
    }
}
