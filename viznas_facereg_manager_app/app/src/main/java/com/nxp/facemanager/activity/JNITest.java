package com.nxp.facemanager.activity;

public class JNITest {
    static {
        System.loadLibrary("JniLib");
    }
    public native  String getString();
    public native int[] incArray(int[] array);
    public native int[] getArray(int size);
    public native int getInt();
}
