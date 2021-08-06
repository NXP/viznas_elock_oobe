package com.nxp.facemanager;

import com.nxp.facemanager.database.entity.UserInformation;

import java.util.List;

public class Oasis {
    static {
        System.loadLibrary("JniLib");
    }
    public native String faceRecognize(byte[] data, int width , int height, int[] box, int isEditUser, int[] id, byte[] feature, int[] di_db, byte[][] feature_db, int[] result);
    public native int faceAdd(String name, byte[] feature);
    public native int faceDel(String name);
    public native int faceDelAll();
    public native int Init(int useHeavyModel, int isElock);
    public native int Exit();
    public native int getOasisHeight();
    public native int getOasisWidth();
    public native int getFaceItemSize();
    public native int cancelOp();
}
