package com.nxp.facemanager.viewModels;

import android.app.ProgressDialog;
import android.content.Context;
import android.os.Handler;
import android.util.Log;
import android.widget.Toast;

import com.google.firebase.firestore.auth.User;
import com.nxp.facemanager.R;
import com.nxp.facemanager.ble.BleReceiveDataModel;
import com.nxp.facemanager.ble.BleReceiveListener;
import com.nxp.facemanager.ble.BleSendDataModel;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.DatabaseOperations;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.model.AddDeviceRequestModel;
import com.nxp.facemanager.model.AddDeviceResponseModel;
import com.nxp.facemanager.model.CommonResponseModel;
import com.nxp.facemanager.model.DeleteUserRequestModel;
import com.nxp.facemanager.model.GetUsersRequestModel;
import com.nxp.facemanager.model.GetUsersResponseModel;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.webservice.ApiError;
import com.nxp.facemanager.webservice.ApiInterface;
import com.nxp.facemanager.webservice.ErrorUtils;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.logging.LogRecord;

import androidx.annotation.NonNull;
import androidx.databinding.ObservableBoolean;
import androidx.lifecycle.LifecycleOwner;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModel;
import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;

import static com.nxp.facemanager.utility.AppConstants.LOGIN_USER_EMAIL;
import static com.nxp.facemanager.utility.CommonUtils.showLoadingDialog;

/**
 * This class used in {@link UserInformation} list and bind with {@link com.nxp.facemanager.activity.DashboardActivity}
 */
public class UserInformationListViewModel extends ViewModel {

    public ObservableBoolean isLoading = new ObservableBoolean();
    /**
     * Object reference for {@link DatabaseOperations}
     */
    private DatabaseOperations dbOperation;
    /**
     * Shared Preferences object
     */
    private MySharedPreferences mySharedPreferences;
    /**
     * ApiInterface Object
     */
    private ApiInterface apiInterface;
    /**
     * {@link LiveData} object reference of {@link UserInformation}
     */
    private LiveData<List<UserInformation>> userLiveData = new LiveData<List<UserInformation>>() {
        @Override
        public void observe(@NonNull LifecycleOwner owner, @NonNull Observer<? super List<UserInformation>> observer) {
            super.observe(owner, observer);
        }
    };

    /**
     * {@link LiveData} object reference of {@link UserInformation}
     */
    private MutableLiveData<List<UserInformation>> mutableUserLiveData = new MutableLiveData<>();
    /**
     * Live data field for showing error message from server
     */
    private MutableLiveData<String> errorMessage = new MutableLiveData<>();

    private ProgressDialog progressDialog;

    public Context getContext() {
        return context;
    }

    public void setContext(Context context) {
        this.context = context;
    }

    /**
     * * Error message for no internet connectivity
     */
    private MutableLiveData<String> internetErrorMessage = new MutableLiveData<>();


    public MutableLiveData<String> getInternetErrorMessage() {
        return internetErrorMessage;
    }

    private Context context;


    /**
     * List of {@link DeviceInfo}
     */
    private List<DeviceInfo> deviceInfoList = new ArrayList<>();

    /**
     * Constructor to access model
     *
     * @param faceDatabase      {@link FaceDatabase}
     * @param api               api object
     * @param sharedPreferences : preferences object
     */
    UserInformationListViewModel(FaceDatabase faceDatabase, ApiInterface api, MySharedPreferences sharedPreferences) {
        dbOperation = new DatabaseOperations(faceDatabase);
        apiInterface = api;
        mySharedPreferences = sharedPreferences;
        userLiveData = dbOperation.getAllUsers();
        EventBus.getDefault().register(this);
    }

    public MutableLiveData<List<UserInformation>> getMutableUserLiveData() {
        return mutableUserLiveData;
    }



    public void setMutableUserLiveData(MutableLiveData<List<UserInformation>> mutableUserLiveData) {
        this.mutableUserLiveData = mutableUserLiveData;
    }

    BleReceiveListener syncUserListener = new BleReceiveListener() {
        @Override
        public void onDataReceive(BleReceiveDataModel bleReceiveDataModel) {
            if (bleReceiveDataModel.getCmd()!=AppConstants.FACE_RECORD_GET_RESPONSE){
                return;
            }
            dbOperation.deleteSubUsers();
            for(UserInformation user: bleReceiveDataModel.getUserInformation()){
                try{
                    dbOperation.insert(user);
                } catch (Exception e){
                    e.printStackTrace();
                }
            }
            isLoading.set(false);
            progressDialog.dismiss();

            CharSequence text = "Synced with ble successfully!";
            int duration = Toast.LENGTH_SHORT;
            Toast toast = Toast.makeText(context, text, duration);
            toast.show();
        }

        @Override
        public void onFailure() {
            progressDialog.dismiss();
            isLoading.set(false);
            errorMessage.setValue(context.getString(R.string.server_error));
            CharSequence text = "Could not read user records from remote...";
            int duration = Toast.LENGTH_SHORT;
            Toast toast = Toast.makeText(context, text, duration);
            toast.show();
        }
    };


    private boolean dataReceived;
    public void getUsersListFromBle(){
        dataReceived = false;
        BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.FACE_RECORD_GET_REQUEST, "");
        EventBus.getDefault().post(bleSendDataModel);
        bleSendDataModel.setBleReceiveListener(syncUserListener);
        progressDialog = showLoadingDialog(context, context.getString(R.string.please_wait));
        progressDialog.show();
        // TODO: Remove handler latter - timeout for testing convenience
        final Handler mHandler = new Handler();
        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (! dataReceived){
                    syncUserListener.onFailure();
                }
                bleSendDataModel.setBleReceiveListener(null);
            }
        }, AppConstants.BLE_CB_WAIT_INTERVAL);

    }

    @Subscribe
    public void bleReceiveDataModel(BleReceiveDataModel bleReceiveDataModel){
        if (bleReceiveDataModel.getCmd() == AppConstants.FACE_RECORD_GET_RESPONSE){
            dataReceived = true;
            if (bleReceiveDataModel.isSuccess()){
                syncUserListener.onDataReceive(bleReceiveDataModel);
            }else{
                syncUserListener.onFailure();
            }
        } else if (bleReceiveDataModel.getCmd() == AppConstants.FACE_RECORD_UPDATE_RESPONSE && bleReceiveDataModel.getOp() == AppConstants.OP_UPDATE_RECORD_DELETE){
            dataReceived = true;
            if (bleReceiveDataModel.isSuccess()){
                deleteUserListener.onDataReceive(bleReceiveDataModel);
            }else{
                deleteUserListener.onFailure();
            }
        }
    }




    public void getUsersListFromCloud() {
        if (CommonUtils.isNetworkConnected(context)) {
            GetUsersRequestModel getUsersRequestModel = new GetUsersRequestModel();
            getUsersRequestModel.setAdminId(mySharedPreferences.getStringData(LOGIN_USER_EMAIL));
            ProgressDialog progressDialog = showLoadingDialog(context, context.getString(R.string.please_wait));
            progressDialog.show();
            Call<GetUsersResponseModel> call = apiInterface.getUsers(mySharedPreferences.getStringData(AppConstants.TOKEN), getUsersRequestModel);
            call.enqueue(new Callback<GetUsersResponseModel>() {
                @SuppressWarnings("NullableProblems")
                @Override
                public void onResponse(Call<GetUsersResponseModel> call, Response<GetUsersResponseModel> response) {
                    if (response.body() != null) {
                        if (response.isSuccessful() && response.body().getSuccess()) {
                            //                        userListAPI = response.body().getUsers();
                            dbOperation.deleteSubUsers();
                            for (UserInformation user : response.body().getUsers()) {
                                try {
                                    dbOperation.insert(user);
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            }
                            isLoading.set(false);
                            progressDialog.dismiss();
                        } else if (response.isSuccessful() && !response.body().getSuccess()) {
                            progressDialog.dismiss();
                            isLoading.set(false);
                            errorMessage.setValue(response.body().getMessage());
                        } else {
                            isLoading.set(false);
                            progressDialog.dismiss();
                            ApiError apiError = ErrorUtils.parseError(response);
                            errorMessage.setValue(apiError.message());
                        }
                    }
                }

                @SuppressWarnings("NullableProblems")
                @Override
                public void onFailure(Call<GetUsersResponseModel> call, Throwable t) {
                    progressDialog.dismiss();
                    isLoading.set(false);
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
//        getUsersListFromCloud();
        // TODO: check connection type and decide get user list from ble or cloud
        getUsersListFromBle();
    }


    /**
     * List of {@link UserInformation}
     *
     * @return LiveData userInformation
     */
    public LiveData<List<UserInformation>> getUserList() {
        return userLiveData;
    }

    /**
     * Insert {@link UserInformation} in Room database with using {@link DatabaseOperations}
     *
     * @param userInformation {@link UserInformation}
     */
    public void insert(UserInformation userInformation) {
        dbOperation.insert(userInformation);
    }

    /**
     * Delete {@link UserInformation} from Room database with using {@link DatabaseOperations}
     *
     * @param userInformation {@link UserInformation}
     */
    public void deleteUser(UserInformation userInformation) {
        dbOperation.delete(userInformation);
    }

    /**
     * check in database user exits or not with user name key.
     *
     * @param username String
     * @return UserInformation
     */
    public UserInformation userExits(String username) {
        if (userLiveData != null && userLiveData.getValue() != null) {
            for (UserInformation userInformation : userLiveData.getValue()) {
                if (userInformation.getName().equalsIgnoreCase(username)) {
                    return userInformation;
                }
            }
        }
        return null;
    }

    BleReceiveListener deleteUserListener;

    public void deleteUserFromBle(UserInformation userInformation){
        JSONObject pl = new JSONObject();
        String payload = "";
        try{
            // TODO: populate payload
            pl.put(AppConstants.JSON_KEY_ID, Integer.parseInt(userInformation.get_id()));
            pl.put(AppConstants.JSON_KEY_UPDATE_OP, AppConstants.OP_UPDATE_RECORD_DELETE);
            pl.put(AppConstants.JSON_KEY_USERNAME, userInformation.getName());
            payload = pl.toString();
        } catch (Exception e){
			e.printStackTrace();
        }
        BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.FACE_RECORD_UPDATE_REQUEST, payload);
        EventBus.getDefault().post(bleSendDataModel);
        deleteUserListener = new BleReceiveListener() {
            @Override
            public void onDataReceive(BleReceiveDataModel bleReceiveDataModel) {
                if(bleReceiveDataModel.getUsername().charAt(0) != '_'){
                    String payload="";
                    try{
                        JSONObject pl2 = new JSONObject();
                        pl2.put(AppConstants.JSON_KEY_ID, Integer.parseInt(userInformation.get_id())+1);
                        pl2.put(AppConstants.JSON_KEY_UPDATE_OP, AppConstants.OP_UPDATE_RECORD_DELETE);
                        pl2.put(AppConstants.JSON_KEY_USERNAME, "_"+userInformation.getName());
                        payload = pl2.toString();
                        Log.e("UINFO", "user2 "+payload);
                    } catch (Exception e){
                        e.printStackTrace();
                    }
                    BleSendDataModel bleSendDataModel2 = new BleSendDataModel(AppConstants.FACE_RECORD_UPDATE_REQUEST, payload);
                    EventBus.getDefault().post(bleSendDataModel2);
                }
            }

            @Override
            public void onFailure() {

            }
        };
        bleSendDataModel.setBleReceiveListener(deleteUserListener);
        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (!dataReceived){
                    deleteUserListener.onFailure();
                }
                bleSendDataModel.setBleReceiveListener(null);
            }
        }, AppConstants.BLE_CB_WAIT_INTERVAL);

    }

    public void deleteUserFromApi(UserInformation userInformation) {
        if (CommonUtils.isNetworkConnected(context)) {
            DeleteUserRequestModel deleteUserRequestModel = new DeleteUserRequestModel();
            deleteUserRequestModel.setUserId(userInformation.get_id());
            ProgressDialog progressDialog = showLoadingDialog(context, context.getString(R.string.please_wait));
            progressDialog.show();
            Call<CommonResponseModel> call = apiInterface.deleteUser(mySharedPreferences.getStringData(AppConstants.TOKEN), deleteUserRequestModel);
            call.enqueue(new Callback<CommonResponseModel>() {
                @SuppressWarnings("NullableProblems")
                @Override
                public void onResponse(Call<CommonResponseModel> call, Response<CommonResponseModel> response) {
                    CommonUtils.dismissProgress(progressDialog);
                    if (response.body() != null) {
                        if (response.isSuccessful() && response.body().getSuccess()) {
                            errorMessage.setValue(response.body().getMessage());
                            dbOperation.delete(userInformation);
                        } else if (response.isSuccessful() && !response.body().getSuccess()) {
                            errorMessage.setValue(response.body().getMessage());
                        }
                    } else {
                        ApiError apiError = ErrorUtils.parseError(response);
                        errorMessage.setValue(apiError.message());
                    }

                    progressDialog.dismiss();
                }

                @SuppressWarnings("NullableProblems")
                @Override
                public void onFailure(Call<CommonResponseModel> call, Throwable t) {
                    progressDialog.dismiss();
                    errorMessage.setValue(context.getString(R.string.server_error));
                }
            });
        } else {
            internetErrorMessage.setValue("");
        }

    }

    public List<DeviceInfo> getDeviceInfoList() {
        return deviceInfoList;
    }

    public void setDeviceInfoList(List<DeviceInfo> deviceInfoList) {
        this.deviceInfoList = deviceInfoList;
    }

    public DatabaseOperations getDbOperation() {
        return dbOperation;
    }

    public void updateDeviceListWithSelectedUsers(ArrayList<UserInformation> multiselect_list) {

        if (CommonUtils.isNetworkConnected(context)) {
            //Api call for update device list
            ProgressDialog progressDialog = CommonUtils.progressDialog(context, "Updating Users in Device...");
            AddDeviceRequestModel addDeviceRequestModel = new AddDeviceRequestModel();
            addDeviceRequestModel.setAdminId(mySharedPreferences.getStringData(AppConstants.LOGIN_USER_EMAIL));
            DeviceInfo deviceInfo = new DeviceInfo();
            if (deviceInfoList != null && deviceInfoList.size() > 0) {
                deviceInfo = deviceInfoList.get(0);
            }
            addDeviceRequestModel.setDeviceId(deviceInfo.getMac_address());
            addDeviceRequestModel.setDeviceName(deviceInfo.getDevice_name());
            addDeviceRequestModel.setPasscode(deviceInfo.getPass_code());
            ArrayList<String> list = new ArrayList<>();
            for (UserInformation userInformation : multiselect_list) {
                list.add(userInformation.get_id());
            }

            addDeviceRequestModel.setUserIds(list);

            Call<AddDeviceResponseModel> call = apiInterface.addDevice(mySharedPreferences.getStringData(AppConstants.TOKEN), addDeviceRequestModel);
            call.enqueue(new Callback<AddDeviceResponseModel>() {
                @SuppressWarnings("NullableProblems")
                @Override
                public void onResponse(Call<AddDeviceResponseModel> call, Response<AddDeviceResponseModel> response) {
                    if (response.isSuccessful()) {
                        CommonUtils.dismissProgress(progressDialog);
                        if (response.body() != null && response.body().getDevice() != null) {
                            Toast.makeText(context, response.body().getMessage(), Toast.LENGTH_LONG).show();
                        } else if (response.isSuccessful() && !Objects.requireNonNull(response.body()).getSuccess()) {
                            Toast.makeText(context, response.body().getMessage(), Toast.LENGTH_SHORT).show();
                        }
                    } else {
                        CommonUtils.dismissProgress(progressDialog);
                        ApiError apiError = ErrorUtils.parseError(response);
                        Toast.makeText(context, apiError.message(), Toast.LENGTH_SHORT).show();
                    }
                }

                @SuppressWarnings("NullableProblems")
                @Override
                public void onFailure(Call<AddDeviceResponseModel> call, Throwable t) {
                    CommonUtils.dismissProgress(progressDialog);
                    errorMessage.setValue(context.getString(R.string.server_error));
                }
            });
        } else {
            internetErrorMessage.setValue("");
        }


    }

    @Override
    protected void onCleared(){
        super.onCleared();
        EventBus.getDefault().unregister(this);
    }
}
