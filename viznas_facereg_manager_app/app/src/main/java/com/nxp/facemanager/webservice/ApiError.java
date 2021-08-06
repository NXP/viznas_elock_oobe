package com.nxp.facemanager.webservice;

public class ApiError {

    private boolean Success;
    private String message;
    public ApiError() {
    }

    public boolean status() {
        return Success;
    }

    public String message() {
        return message;
    }
}