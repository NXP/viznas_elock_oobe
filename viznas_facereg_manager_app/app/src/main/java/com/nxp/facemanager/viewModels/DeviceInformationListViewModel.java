package com.nxp.facemanager.viewModels;

import android.app.ProgressDialog;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;

import com.nxp.facemanager.R;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.DatabaseOperations;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.model.BleScanBackgroundServiceStartScanEvent;
import com.nxp.facemanager.model.CommonResponseModel;
import com.nxp.facemanager.model.DeviceListResponseModel;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.webservice.ApiError;
import com.nxp.facemanager.webservice.ApiInterface;
import com.nxp.facemanager.webservice.ErrorUtils;

import org.greenrobot.eventbus.EventBus;

import java.util.HashMap;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.databinding.ObservableBoolean;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;

/**
 * This class used in {@link UserInformation} list and bind with {@link com.nxp.facemanager.activity.DashboardActivity}
 */
public class DeviceInformationListViewModel extends ViewModel {
    public ObservableBoolean isLoading = new ObservableBoolean();
    /**
     * Object reference for {@link DatabaseOperations}
     */
    private DatabaseOperations dbOperation;
    /**
     * {@link LiveData} object reference of {@link DeviceInfo}
     */
    private LiveData<List<DeviceInfo>> deviceLiveData;

    /**
     * Interface of retrofit object.
     */
    private ApiInterface apiInterface;
    /**
     * Object of {@link android.content.SharedPreferences}
     */
    private MySharedPreferences mySharedPreferences;
    /**
     * Context
     */
    private Context context;
    /**
     * Live data field for showing error message from server
     */
    private MutableLiveData<String> errorMessage = new MutableLiveData<>();
    /**
     * * Error message for no internet connectivity
     */
    private MutableLiveData<String> internetErrorMessage = new MutableLiveData<>();


    public MutableLiveData<String> getInternetErrorMessage() {
        return internetErrorMessage;
    }

    /**
     * Constructor to access model
     *
     * @param faceDatabase {@link FaceDatabase}
     */
    DeviceInformationListViewModel(FaceDatabase faceDatabase, ApiInterface apiInterface, MySharedPreferences mySharedPreferences) {
        this.apiInterface = apiInterface;
        this.mySharedPreferences = mySharedPreferences;
        dbOperation = new DatabaseOperations(faceDatabase);
        deviceLiveData = dbOperation.getAllDeviceInfo();
    }

    public Context getContext() {
        return context;
    }

    public void setContext(Context context) {
        this.context = context;
    }

    /**
     * List of {@link DeviceInfo}
     *
     * @return LiveData device info
     */
    public LiveData<List<DeviceInfo>> getDeviceList() {
        return deviceLiveData;
    }

    /**
     * Insert {@link DeviceInfo} in Room database with using {@link DatabaseOperations}
     *
     * @param deviceInfo {@link DeviceInfo}
     */
    public void insert(DeviceInfo deviceInfo) {
        //dbOperation.insert(deviceInfo);
    }

    /**
     * Delete {@link DeviceInfo} in Room database with using {@link DatabaseOperations}
     *
     * @param deviceInfo {@link DeviceInfo}
     */
    public void delete(DeviceInfo deviceInfo) {
        dbOperation.delete(deviceInfo);
    }

    /**
     * Fetch device list
     */
    public void getDeviceListFromCloud() {
        if (CommonUtils.isNetworkConnected(context)) {
            ProgressDialog progressDialog = CommonUtils.progressDialog(getContext(), "Getting devices...");
            Call<DeviceListResponseModel> call = apiInterface.getDevices(mySharedPreferences.getStringData(AppConstants.TOKEN), mySharedPreferences.getStringData(AppConstants.LOGIN_USER_ID));
            call.enqueue(new Callback<DeviceListResponseModel>() {
                @SuppressWarnings("NullableProblems")
                @Override
                public void onResponse(Call<DeviceListResponseModel> call, Response<DeviceListResponseModel> response) {
                    if (response.isSuccessful()) {
                        CommonUtils.dismissProgress(progressDialog);
                        if (response.body().getSuccess()) {
                            dbOperation.resetDeviceInfoDatabase();
                            if (response.body() != null && response.body().getDeviceArrayList() != null && response.body().getDeviceArrayList().size() > 0) {
                                for (DeviceInfo device : response.body().getDeviceArrayList()) {
                                    if (mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).equals(device.get_id())) {
                                        device.setConnected(true);
                                    } else {
                                        device.setConnected(false);
                                    }
                                    device.setMac_address(device.get_id());
                                    dbOperation.insert(device);
                                }
                            }

                        } else {
                            errorMessage.setValue(response.body().getMessage());
                        }
                        isLoading.set(false);
                    } else {
                        isLoading.set(false);
                        CommonUtils.dismissProgress(progressDialog);
                        ApiError apiError = ErrorUtils.parseError(response);
                        errorMessage.setValue(apiError.message());
                    }
                }

                @SuppressWarnings("NullableProblems")
                @Override
                public void onFailure(Call<DeviceListResponseModel> call, Throwable t) {
                    CommonUtils.dismissProgress(progressDialog);
                    errorMessage.setValue(context.getString(R.string.server_error));
                }
            });
        } else {
            isLoading.set(false);
            internetErrorMessage.setValue("");

        }
    }

    /**
     * Swipe to refresh view.
     */
    public void onRefresh() {
        isLoading.set(true);
        getDeviceListFromCloud();
    }

    /**
     * Delete device from server.
     */
    public void deleteDevice(DeviceInfo deviceInfo) {
        if (CommonUtils.isNetworkConnected(context)) {
            ProgressDialog progressDialog = CommonUtils.progressDialog(getContext(), "Deleting " + deviceInfo.get_id());
            HashMap<String, String> map = new HashMap<>();
            map.put("deviceId", deviceInfo.get_id());
            Call<CommonResponseModel> call = apiInterface.deleteDevices(mySharedPreferences.getStringData(AppConstants.TOKEN), map);
            call.enqueue(new Callback<CommonResponseModel>() {
                @Override
                public void onResponse(@NonNull Call<CommonResponseModel> call, @NonNull Response<CommonResponseModel> response) {

                    if (response.body() != null) {
                        if (response.isSuccessful() && response.body().getSuccess()) {
                            errorMessage.setValue(response.body().getMessage());
                            if (mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).equals(deviceInfo.get_id())) {
                                EventBus.getDefault().post(new BleScanBackgroundServiceStartScanEvent(false));
                            }
                            new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
                                @Override
                                public void run() {
                                    dbOperation.delete(deviceInfo);
                                    CommonUtils.dismissProgress(progressDialog);
                                }
                            }, 2000);

                        } else if (response.isSuccessful() && !response.body().getSuccess()) {
                            CommonUtils.dismissProgress(progressDialog);
                            errorMessage.setValue(response.body().getMessage());
                        } else {
                            ApiError apiError = ErrorUtils.parseError(response);
                            CommonUtils.dismissProgress(progressDialog);
                            errorMessage.setValue(apiError.message());
                        }
                    }
                }

                @SuppressWarnings("NullableProblems")
                @Override
                public void onFailure(Call<CommonResponseModel> call, Throwable t) {
                    CommonUtils.dismissProgress(progressDialog);
                }
            });
        } else {
            internetErrorMessage.setValue("");

        }
    }


    public void update(String mac_address) {
        dbOperation.updateConnected(mac_address);
    }

    public void edit(DeviceInfo deviceInfo){
    }
}
