package com.nxp.facemanager.viewModels;

import android.app.ProgressDialog;
import android.content.Context;
import android.widget.Toast;

import com.nxp.facemanager.activity.AddDeviceListener;
import com.nxp.facemanager.ble.BleSendDataModel;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.DatabaseOperations;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.model.AddDeviceRequestModel;
import com.nxp.facemanager.model.AddDeviceResponseModel;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.webservice.ApiError;
import com.nxp.facemanager.webservice.ApiInterface;
import com.nxp.facemanager.webservice.ErrorUtils;

import java.util.ArrayList;
import java.util.List;

import androidx.databinding.ObservableBoolean;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;

/***
 * {@link ViewModel} class for device scanning screen.
 * {@link com.nxp.facemanager.activity.DeviceScanActivity}
 */
public class DeviceScanModule extends ViewModel {
    /**
     * {@link LiveData} object reference of {@link UserInformation}
     */
    private final LiveData<List<UserInformation>> userInformationList;
    /**
     * {@link Boolean} for scanning running or not.
     */
    private ObservableBoolean mScanning = new ObservableBoolean(false);
    /**
     * Store {@link UserInformation } object of {@link UserInformation}.
     */
    private UserInformation userInformation;
    /**
     * Store {@link DeviceInfo } object of selected device.
     */
    private DeviceInfo deviceInformation;
    /**
     * Object reference for {@link DatabaseOperations}
     */
    private DatabaseOperations dbOperation;
    /**
     * List of {@link DeviceInfo}
     */
    private List<DeviceInfo> deviceInfoList;

    /**
     * Reference of shared preference.
     */
    private MySharedPreferences mySharedPreferences;
    /**
     * activity context
     */
    private Context activityContext;
    /**
     * apiInterface object
     */
    private ApiInterface apiInterface;

    /**
     ** Error message for no internet connectivity
     */
    private MutableLiveData<String> internetErrorMessage = new MutableLiveData<>();
    private ProgressDialog progressDialog;


    public MutableLiveData<String> getInternetErrorMessage() {
        return internetErrorMessage;
    }


    /**
     * Constructor to access model
     *
     * @param faceDatabase {@link FaceDatabase}
     */
    DeviceScanModule(FaceDatabase faceDatabase, ApiInterface apiInterface, MySharedPreferences mySharedPreferences) {
        this.apiInterface = apiInterface;
        this.mySharedPreferences = mySharedPreferences;
        dbOperation = new DatabaseOperations(faceDatabase);
        userInformationList = dbOperation.getAllUsers();
    }

    public Context getActivityContext() {
        return activityContext;
    }

    public void setActivityContext(Context activityContext) {
        this.activityContext = activityContext;
    }

    public DatabaseOperations getDbOperation() {
        return dbOperation;
    }

    /**
     * List of {@link UserInformation}
     *
     * @return LiveData userInformation
     */
    public LiveData<List<UserInformation>> getUserList() {
        return userInformationList;
    }

    /**
     * List of {@link DeviceInfo}
     *
     * @return LiveData deviceInfo
     */
    public List<DeviceInfo> getDeviceInfoList() {
        return deviceInfoList;
    }

    /**
     * Set device info list.
     *
     * @param deviceInfoList List of {@link DeviceInfo}
     */
    public void setDeviceInfoList(List<DeviceInfo> deviceInfoList) {
        this.deviceInfoList = deviceInfoList;
    }

    /**
     * Check if the device exits in selected user.
     *
     * @param macAddress ble mac address.
     * @return true/false.
     */
    public boolean isDeviceExitsInUserInformation(String macAddress) {
        if (getDeviceInfoList() != null && getDeviceInfoList().size() > 0) {
            for (DeviceInfo deviceInfo : getDeviceInfoList()) {
                if (deviceInfo.getMac_address().equalsIgnoreCase(macAddress)) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Scanning
     *
     * @return ObservableBoolean
     */
    public ObservableBoolean getmScanning() {
        return mScanning;
    }

    /**
     * Set Scanning.
     *
     * @param mScanning isScanning is running
     */
    public void setmScanning(ObservableBoolean mScanning) {
        this.mScanning = mScanning;
    }

    /**
     * Return device information.
     *
     * @return deviceInfo
     */
    public DeviceInfo getDeviceInformation() {
        return deviceInformation;
    }

    /**
     * Set device information.
     *
     * @param deviceInformation {@link DeviceInfo}
     */
    public void setDeviceInformation(DeviceInfo deviceInformation) {
        this.deviceInformation = deviceInformation;
    }

    /**
     * On click for
     * @param newPage
     */
    public void performAddDevice(AddDeviceListener newPage) {

        newPage.callFinishMethod();
        return;
        /*
        if (CommonUtils.isNetworkConnected(activityContext)) {
            progressDialog = CommonUtils.progressDialog(activityContext, "Add Device...");
            AddDeviceRequestModel addDeviceRequestModel = new AddDeviceRequestModel();
            addDeviceRequestModel.setAdminId(mySharedPreferences.getStringData(AppConstants.LOGIN_USER_EMAIL));
            addDeviceRequestModel.setDeviceId(deviceInformation.getMac_address());
            addDeviceRequestModel.setDeviceName(deviceInformation.getDevice_name());
            addDeviceRequestModel.setPasscode(deviceInformation.getPass_code());
            ArrayList<String> list = new ArrayList<>();
            list.add(mySharedPreferences.getStringData(AppConstants.LOGIN_USER_ID));
            addDeviceRequestModel.setUserIds(list);

            Call<AddDeviceResponseModel> call = apiInterface.addDevice(mySharedPreferences.getStringData(AppConstants.TOKEN), addDeviceRequestModel);
            call.enqueue(new Callback<AddDeviceResponseModel>() {
                @SuppressWarnings("NullableProblems")
                @Override
                public void onResponse(Call<AddDeviceResponseModel> call, Response<AddDeviceResponseModel> response) {
                    if (response.isSuccessful()) {
                        if (response.body() != null && response.body().getDevice() != null) {
                            DeviceInfo deviceInfo = new DeviceInfo();
                            deviceInfo.setMac_address(response.body().getDevice().getDeviceId());
                            deviceInfo.setDevice_name(response.body().getDevice().getDeviceName());
                            deviceInfo.setPass_code(response.body().getDevice().getPasscode());
                            deviceInfo.set_id(response.body().getDevice().getDeviceId());
                            deviceInfo.getUserIdList().addAll(response.body().getDevice().getUserIds());
                            setDeviceInformation(deviceInfo);

                            BleSendDataModel bleSendDataModel = new BleSendDataModel();
                            bleSendDataModel.setCommand(AppConstants.USER_INFORMATION);
                            bleSendDataModel.setDeviceName(getDeviceInformation().getDevice_name());
                            bleSendDataModel.setPass_code("" + getDeviceInformation().getPass_code());
                            bleSendDataModel.setCookie("");
                            dbOperation.insert(getDeviceInformation());
                            Toast.makeText(activityContext, response.body().getMessage(), Toast.LENGTH_SHORT).show();
                            CommonUtils.dismissProgress(progressDialog);
                            newPage.callFinishMethod();
                        } else {
                            AppLogger.d("TAG","Device info from cloud is null");
                            CommonUtils.dismissProgress(progressDialog);
                            newPage.callFinishMethod();
                        }
                    } else {
                        BleSendDataModel bleSendDataModel = new BleSendDataModel();
                        bleSendDataModel.setCommand(AppConstants.USER_INFORMATION);
                        bleSendDataModel.setDeviceName(getDeviceInformation().getDevice_name());
                        bleSendDataModel.setPass_code("" + getDeviceInformation().getPass_code());
                        bleSendDataModel.setCookie("");
                        dbOperation.insert(getDeviceInformation());
                        CommonUtils.dismissProgress(progressDialog);
                        ApiError apiError = ErrorUtils.parseError(response);
                        Toast.makeText(activityContext, apiError.message(), Toast.LENGTH_SHORT).show();
                        newPage.callFinishMethod();
                    }
                }

                @SuppressWarnings("NullableProblems")
                @Override
                public void onFailure(Call<AddDeviceResponseModel> call, Throwable t) {
                    CommonUtils.dismissProgress(progressDialog);
                    newPage.callFinishMethod();
    //                errorMessage.setValue(activityContext.getString(R.string.server_error));
                }
            });
        } else {

            internetErrorMessage.setValue("");
            newPage.callFinishMethod();
        }
        */
    }


}


