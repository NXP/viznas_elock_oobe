package com.nxp.facemanager.viewModels;

import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.DatabaseOperations;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.webservice.ApiInterface;

import java.util.ArrayList;
import java.util.List;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.ViewModel;

/**
 * This class used in {@link UserInformation} list and bind with {@link com.nxp.facemanager.activity.DashboardActivity}
 */
public class HomeViewModel extends ViewModel {
    /**
     * Interface of api object.
     */
    private ApiInterface apiInterface;
    /**
     * Shared Preference object.
     */
    private MySharedPreferences mySharedPreferences;
    /**
     * Object reference for {@link DatabaseOperations}
     */
    private DatabaseOperations dbOperation;

    private LiveData<UserInformation> userInformationLiveData;
    /**
     * List of {@link DeviceInfo}
     */
    private List<DeviceInfo> deviceInfoList = new ArrayList<>();

    /**
     * Constructor to access model
     *
     * @param faceDatabase {@link FaceDatabase}
     */
    HomeViewModel(FaceDatabase faceDatabase, ApiInterface apiInterface, MySharedPreferences mySharedPreferences) {
        this.apiInterface = apiInterface;
        this.mySharedPreferences = mySharedPreferences;
        dbOperation = new DatabaseOperations(faceDatabase);
        userInformationLiveData = dbOperation.getLoginUser(mySharedPreferences.getStringData(AppConstants.LOGIN_USER_ID));
    }

    public DatabaseOperations getDbOperation() {
        return dbOperation;
    }

    public LiveData<UserInformation> getUserInformationLiveData() {
        return userInformationLiveData;
    }

    public List<DeviceInfo> getDeviceInfoList() {
        return deviceInfoList;
    }

    public void setDeviceInfoList(List<DeviceInfo> deviceInfoList) {
        this.deviceInfoList = deviceInfoList;
    }

    public MySharedPreferences getMySharedPreferences() {
        return mySharedPreferences;
    }

    public void resetData() {
        mySharedPreferences.resetPreference();
        dbOperation.resetDatabase();
    }

    public DeviceInfo checkDeviceExitsInDb(String macAdd) {
        if (deviceInfoList != null && deviceInfoList.size() > 0) {
            for (DeviceInfo deviceInformation : deviceInfoList) {
                if (deviceInformation.getMac_address().equalsIgnoreCase(macAdd)) {
                    return deviceInformation;
                }
            }
        }
        return null;
    }


}
