package com.nxp.facemanager.ble;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

import com.google.firebase.firestore.auth.User;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;
import com.google.gson.reflect.TypeToken;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.utility.AppConstants;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;

public class BleReceiveDataModel implements Parcelable {
    @Expose @SerializedName("CMD")
    private int cmd;
    @Expose @SerializedName("PL")
    private String payload;

    private transient int maxPayloadLength;
    private transient boolean success;
    private transient String username;
    private transient String feature;
    private transient String email;
    private transient BleConfigModel bleConfigModel;
    private transient ArrayList<UserInformation> userInformationList = new ArrayList<>();
    private transient int id;
//    private transient String profilePic;
    private transient String op;


    public static final Creator<BleReceiveDataModel> CREATOR = new Creator<BleReceiveDataModel>() {
        @Override
        public BleReceiveDataModel createFromParcel(Parcel in) {
            return new BleReceiveDataModel(in);
        }

        @Override
        public BleReceiveDataModel[] newArray(int size) {
            return new BleReceiveDataModel[size];
        }
    };

    public BleConfigModel getBleConfigModel() {
        return bleConfigModel;
    }

    public void setBleConfigModel(BleConfigModel bleConfigModel) {
        this.bleConfigModel = bleConfigModel;
    }

    private BleReceiveDataModel(Parcel in) {

    }

    public BleReceiveDataModel(){

    }

    public BleReceiveDataModel(int cmd, String payload){
        this.cmd = cmd;
        this.payload = payload;
        parsePayload();
    }

    // constructor for ready indication
    public BleReceiveDataModel(int cmd, int mpl){
        this.cmd = cmd;
        this.maxPayloadLength = mpl;
    }

    // constructor for auth response
    public BleReceiveDataModel(int cmd, boolean success){
        this.cmd = cmd;
        this.success = success;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {

    }

    public int getCmd() {
        return cmd;
    }
    public void setCmd(int cmd){
        this.cmd = cmd;
    }

    public boolean isSuccess() {
        return success;
    }

    public String getFeature() {
        return feature;
    }

    public void setFeature(String feature) {
        this.feature = feature;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public String getUsername() {
        return username;
    }

    public String getPayload() {
        return payload;
    }

    public void setPayload(String payload) {
        this.payload = payload;
        parsePayload();
    }

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    public ArrayList<UserInformation> getUserInformation() {
        return userInformationList;
    }

    public void setUserInformation(UserInformation userInformation) {
        this.userInformationList = userInformationList;
    }

    public String getOp() {
        return op;
    }

    public void setOp(String op) {
        this.op = op;
    }

    /**
     * Parse payload and set variables
     */
    public void parsePayload(){

        JsonParser parser = new JsonParser();
        try{
            JsonElement jsonElement = parser.parse(this.payload);
            if (jsonElement.isJsonObject()) {
               JsonObject jsonObject = jsonElement.getAsJsonObject();
                switch (this.cmd) {
                    case AppConstants.READY_INDICATION:
                        this.maxPayloadLength = jsonObject.get(AppConstants.JSON_KEY_MPL).getAsInt();
                        break;
                    case AppConstants.AUTHENTICATION_RESPONSE:
                        this.success = jsonObject.get(AppConstants.JSON_KEY_RES_SUCCESS).getAsBoolean();
                        this.username = jsonObject.get(AppConstants.JSON_KEY_USERNAME).getAsString();
                        this.email = jsonObject.get(AppConstants.JSON_KEY_EMAIL).getAsString();
                        this.feature = jsonObject.get(AppConstants.JSON_KEY_FEATURE).getAsString();
                        break;

                    case AppConstants.OPEN_DOOR_RESPONSE:
                    case AppConstants.FACE_RECORD_UPDATE_RESPONSE:
                        this.success = jsonObject.get(AppConstants.JSON_KEY_RES_SUCCESS).getAsBoolean();
                        this.id = jsonObject.get(AppConstants.JSON_KEY_ID).getAsInt();
                        this.username = jsonObject.get(AppConstants.JSON_KEY_USERNAME).getAsString();
                        this.feature = jsonObject.get(AppConstants.JSON_KEY_FEATURE).getAsString();
                        this.op = jsonObject.get(AppConstants.JSON_KEY_UPDATE_OP).getAsString();
                        UserInformation userInformationUpdate = new UserInformation();
                        userInformationUpdate.set_id(Integer.toString(id));
                        userInformationUpdate.setName(username);
                        userInformationUpdate.setTrainingData(feature);
                        userInformationList.add(userInformationUpdate);
                        break;

                    case AppConstants.FACE_RECORD_GET_RESPONSE:
                        this.success = jsonObject.get(AppConstants.JSON_KEY_RES_SUCCESS).getAsBoolean();
                        JsonArray jsonArray = jsonObject.getAsJsonArray(AppConstants.JSON_KEY_FEATURE_TABLE);
                        for(JsonElement user: jsonArray)
                        {
                            UserInformation userInformation = new UserInformation();
                            id = user.getAsJsonObject().get(AppConstants.JSON_KEY_ID).getAsInt();
                            username = user.getAsJsonObject().get(AppConstants.JSON_KEY_USERNAME).getAsString();
                            feature = user.getAsJsonObject().get(AppConstants.JSON_KEY_FEATURE).getAsString();
                            userInformation.set_id(Integer.toString(id));
                            userInformation.setName(username);
                            userInformation.setTrainingData(feature);
                            userInformationList.add(userInformation);
                        }
                        break;

                    default:
                        throw new IllegalStateException("Unexpected value: " + this.cmd);
                }
            }
        }catch(Exception e){
            e.printStackTrace();
        }


    }
}
