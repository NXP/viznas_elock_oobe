package com.nxp.facemanager.ble;

import android.os.Parcel;
import android.os.Parcelable;
import com.google.gson.annotations.SerializedName;
import com.nxp.facemanager.databinding.FragmentRemoteControlBinding;

import org.json.JSONObject;

import java.util.zip.CRC32;

/**
 * This model is used for creating json for send user name and pass code to the
 * ble device.
 */
public class BleSendDataModel implements Parcelable {
    public static final Creator<BleSendDataModel> CREATOR = new Creator<BleSendDataModel>() {
        @Override
        public BleSendDataModel createFromParcel(Parcel in) {
            return new BleSendDataModel();
        }

        @Override
        public BleSendDataModel[] newArray(int size) {
            return new BleSendDataModel[size];
        }
    };

    static class FrameHead {
        @SerializedName("PRE")
        private String prefix;

        @SerializedName("TS")
        private int timestamp;

        @SerializedName("VER")
        private int version;

        @SerializedName("CIP")
        private int cipher;

        @SerializedName("LEN")
        private long length;

        public FrameHead() {

        }

        public FrameHead(String prefix, int timestamp, int version, int cipher, long length) {
            this.prefix = prefix;
            this.timestamp = timestamp;
            this.version = version;
            this.cipher = cipher;
            this.length = length;
        }

        public void setPrefix(String prefix) {
            this.prefix = prefix;
        }

        public String getPrefix() {
            return prefix;
        }

        public void setTimestamp(int timestamp) {
            this.timestamp = timestamp;
        }

        public int getTimestamp() {
            return timestamp;
        }

        public void setVersion(int version) {
            this.version = version;
        }

        public int getVersion() {
            return version;
        }

        public void setCipher(int cipher) {
            this.cipher = cipher;
        }

        public int getCipher() {
            return cipher;
        }

        public void setLength(long length) {
            this.length = length;
        }

        public long getLength() {
            return length;
        }
    }

    // payload packet
    @SerializedName("FH")
    FrameHead frameHead = new FrameHead();

    @SerializedName("CMD")
    private int command;

    @SerializedName("PL")
    private String payload;

    @SerializedName("CRC")
    private long crc;

    public BleSendDataModel() {
    }

    // payload packet.
    public BleSendDataModel(FrameHead frameHead, int command, String payload, int crc) {
        this.frameHead = frameHead;
        this.command = command;
        this.payload = payload;
        this.crc = crc;
    }

    public BleSendDataModel(String prefix, int timestamp, int version, int cipher, long length, int command,
            String payload, int crc) {

        this.frameHead.setPrefix(prefix);
        this.frameHead.setTimestamp(timestamp);
        this.frameHead.setVersion(version);
        this.frameHead.setCipher(cipher);
        this.frameHead.setLength(length);
        this.command = command;
        this.payload = payload;
        this.crc = crc;
    }

    public BleSendDataModel(int timestamp, long length, int command, String payload, int crc) {
        this.frameHead.setPrefix("fac");
        this.frameHead.setTimestamp(timestamp);
        this.frameHead.setVersion(0x01);
        this.frameHead.setCipher(0);
        this.frameHead.setLength(length);

        this.command = command;
        this.payload = payload;
        this.crc = crc;
    }

    public String getPayload() {
        return payload;
    }

    public void setPayload(String payload) {
        this.payload = payload;
    }

    public BleSendDataModel(int command, long length, String payload) {
        this.frameHead.setPrefix("fac");
        this.frameHead.setTimestamp(0);
        this.frameHead.setVersion(0x01);
        this.frameHead.setCipher(0);
        this.frameHead.setLength(length);

        this.command = command;
        this.payload = payload;
        this.crc = 0;
    }

    public BleSendDataModel(int command, String payload) {
        this.frameHead.setPrefix("fac");
        this.frameHead.setTimestamp(0);
        this.frameHead.setVersion(0x01);
        this.frameHead.setCipher(0);
        this.frameHead.setLength(payload.length());

        this.command = command;
        this.payload = payload;
        this.crc = 0;
    }

    public BleSendDataModel.FrameHead getFrameHead() {
        return this.frameHead;
    }

    public void setFrameHead(BleSendDataModel.FrameHead frameHead) {
        this.frameHead = frameHead;
    }

    int getCommand() {
        return command;
    }

    public void setCommand(int command) {
        this.command = command;
    }

    void setLength(long length) {
        this.frameHead.length = length;
    }

    void setCrc() {
        CRC32 crc32 = new CRC32();
        crc32.update(this.payload.getBytes());
        this.crc = crc32.getValue();
    }

    public long getCrc() {
        return crc;
    }

    public void setCrc(long crc) {
        this.crc = crc;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) { }

    @Override
    public String toString() {
        return "BleSendDataModel{" + "command='" + command + '\''+'}';
    }

//    public void writePayload() {
//        JSONObject pl = new JSONObject();
//        try {
//            switch (this.cmd_code) {
//                case AppConstants.CMD_AUTH_REQ:
//
//                    pl.put(AppConstants.JSON_KEY_USERNAME, this.username);
//                    pl.put(AppConstants.JSON_KEY_PSW, this.psw);
//                    break;
//
//                case AppConstants.CMD_CONFIG_REQ:
//                    this.bleConfigModel.toString();
//                    break;
//                case AppConstants.CMD_REMOTE_CONTROL_REQ:
//                    break;
//                case AppConstants.CMD_UPDATE_RECORD_REQ:
//                    break;
//            }
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
//
//        payload = pl.toString();
//    }

    private BleReceiveListener bleReceiveListener = null;
    public void setBleReceiveListener(BleReceiveListener listener){
        this.bleReceiveListener = listener;
    }
}
