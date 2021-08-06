package com.nxp.facemanager.model;

import java.util.ArrayList;

public class BleSnapshotEvent {

    private ArrayList<String> snapShotList;
    private int cmd;
    private int maxPayloadLength;
    private int auth;


    public BleSnapshotEvent(ArrayList<String> snapData) {

        this.snapShotList = snapData;

    }


    public ArrayList<String> getSnapShotList() {
        return snapShotList;
    }

    public void setSnapShotList(ArrayList<String> snapShotList) {
        this.snapShotList = snapShotList;
    }
}
