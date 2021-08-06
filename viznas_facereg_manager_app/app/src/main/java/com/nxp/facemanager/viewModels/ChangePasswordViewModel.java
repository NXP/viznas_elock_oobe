package com.nxp.facemanager.viewModels;

import android.app.ProgressDialog;
import android.content.Context;
import android.text.TextUtils;
import android.util.Patterns;
import android.view.View;

import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.HomeActivity;
import com.nxp.facemanager.ble.BleReceiveDataModel;
import com.nxp.facemanager.ble.BleSendDataModel;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.DatabaseOperations;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.model.ChangePasswordRequestModel;
import com.nxp.facemanager.model.CommonResponseModel;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.webservice.ApiError;
import com.nxp.facemanager.webservice.ApiInterface;
import com.nxp.facemanager.webservice.ErrorUtils;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONObject;

import java.util.Objects;

import androidx.databinding.ObservableField;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;

import static com.nxp.facemanager.utility.CommonUtils.showLoadingDialog;

public class ChangePasswordViewModel extends ViewModel {


    /**
     * User password string.
     */
    public ObservableField<String> strOldPassword = new ObservableField<>("");
    public ObservableField<String> strNewPassword = new ObservableField<>("");
    public ObservableField<String> strConfirmPassword = new ObservableField<>("");
    /**
     * Progress Visibility
     */
    public ObservableField<Integer> progressVisibility = new ObservableField<>(View.INVISIBLE);

    /**
     * Live Data message for email error
     */
    private MutableLiveData<String> oldPasswordErrorMessage = new MutableLiveData<>();
    private MutableLiveData<String> newPasswordErrorMessage = new MutableLiveData<>();
    private MutableLiveData<String> confirmPasswordErrorMessage = new MutableLiveData<>();
    /**
     * Live Data message for password error
     */
    private MutableLiveData<String> passwordErrorMessage = new MutableLiveData<>();

    public MutableLiveData<String> getErrorMessage() {
        return errorMessage;
    }

    /**
     * Live Data message for server message/error
     */
    private MutableLiveData<String> errorMessage = new MutableLiveData<>();

    private MutableLiveData<String> internetErrorMessage = new MutableLiveData<>();


    public MutableLiveData<String> getInternetErrorMessage() {
        return internetErrorMessage;
    }

    /**
     * activity context
     */
    private Context activityContext;
    /**
     * apiInterface object
     */
    private ApiInterface apiInterface;
    private FaceDatabase faceDatabase;
    /**
     * Object reference for {@link DatabaseOperations}
     */
    private DatabaseOperations dbOperation;

    /**
     * shared preferences object
     */
    private MySharedPreferences mySharedPreferences;
    private ProgressDialog progressDialog;

    ChangePasswordViewModel(FaceDatabase faceDatabase, ApiInterface api, MySharedPreferences sharedPreferences) {
        this.apiInterface = api;
        this.faceDatabase = faceDatabase;
        this.mySharedPreferences = sharedPreferences;
        EventBus.getDefault().register(this);
    }

    /**
     * On click for login
     */
    public void performSignIn() {

        if (TextUtils.isEmpty(strOldPassword.get())) {
            oldPasswordErrorMessage.postValue(activityContext.getString(R.string.enter_old));
        } else if (!CommonUtils.isValidPassword(strNewPassword.get())) {
            newPasswordErrorMessage.postValue(activityContext.getString(R.string.password_validation_regex));
        } else if (TextUtils.isEmpty(strConfirmPassword.get())) {
            confirmPasswordErrorMessage.postValue(activityContext.getString(R.string.enter_new_confirm));

        } else if ((!TextUtils.isEmpty(strNewPassword.get()) && !TextUtils.isEmpty(strConfirmPassword.get())) && !Objects.equals(strNewPassword.get(), strConfirmPassword.get())) {
            confirmPasswordErrorMessage.postValue(activityContext.getString(R.string.password_confirm_validation));
        } else {

            //Api call for change password
            if (CommonUtils.isNetworkConnected(activityContext)) {
                progressDialog = showLoadingDialog(activityContext, activityContext.getString(R.string.please_wait));
                progressDialog.show();
                CommonUtils.hideKeyboard((((HomeActivity) activityContext)));
                ChangePasswordRequestModel changePasswordRequestModel = new ChangePasswordRequestModel();
                changePasswordRequestModel.setEmail(mySharedPreferences.getStringData(AppConstants.LOGIN_USER_EMAIL));
                changePasswordRequestModel.setResetPasswordToken(strOldPassword.get());
                changePasswordRequestModel.setPassword(strNewPassword.get());

                String connection_type = mySharedPreferences.getStringData(AppConstants.CONNECTION_TYPE);

                if (connection_type.compareTo(AppConstants.CONNECTION_BLE)==0){
                    // TODO: build and send change password send data model
                    JSONObject pl = new JSONObject();
                    try{
                        // user id or user email?
                        pl.put(AppConstants.JSON_KEY_EMAIL, AppConstants.LOGIN_USER_EMAIL);
                        pl.put(AppConstants.JSON_KEY_PSW, strOldPassword.get());
                        pl.put(AppConstants.JSON_KEY_PSW_NEW, strNewPassword.get());
                    }catch(Exception e){
                        e.printStackTrace();
                    }
                    BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.PASSWORD_REQUEST, pl.toString());

                    EventBus.getDefault().post(bleSendDataModel);
                }
                Call<CommonResponseModel> call = apiInterface.resetPassword(mySharedPreferences.getStringData(AppConstants.TOKEN), changePasswordRequestModel);
                call.enqueue(new Callback<CommonResponseModel>() {
                    @SuppressWarnings("NullableProblems")
                    @Override
                    public void onResponse(Call<CommonResponseModel> call, Response<CommonResponseModel> response) {
                        if (response.body() != null) {
                            if (response.isSuccessful() && response.body().getSuccess()) {
                                progressDialog.dismiss();
                                errorMessage.setValue(response.body().getMessage());
                                ((HomeActivity) activityContext).setNavigationMenu(0);

                            } else if (response.isSuccessful() && !response.body().getSuccess()) {
                                progressDialog.dismiss();
                                errorMessage.setValue(response.body().getMessage());
                            }
                        } else {
                            progressDialog.dismiss();
                            ApiError apiError = ErrorUtils.parseError(response);
                            errorMessage.setValue(apiError.message());
                        }
                    }

                    @SuppressWarnings("NullableProblems")
                    @Override
                    public void onFailure(Call<CommonResponseModel> call, Throwable t) {
                        progressDialog.dismiss();
                        errorMessage.setValue(activityContext.getString(R.string.server_error));
                    }
                });
            } else {
                internetErrorMessage.setValue("");

            }
        }


    }

    @Subscribe
    public void bleReceiveDataModel(BleReceiveDataModel bleReceiveDataModel) {
        if (bleReceiveDataModel.getCmd() == AppConstants.PASSWORD_RESPONSE){
            // TODO: back to home activity on success
        }
    }


    /**
     * Text change listener for email
     */
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        if (!Patterns.EMAIL_ADDRESS.matcher(s).matches() && s.length() > 0) {
            oldPasswordErrorMessage.postValue(activityContext.getString(R.string.email_validation));
        } else {
            oldPasswordErrorMessage.postValue("");
        }
    }

    public LiveData<String> getOldPasswordError() {
        return oldPasswordErrorMessage;
    }

    public LiveData<String> getNewPasswordError() {
        return newPasswordErrorMessage;
    }

    public LiveData<String> getConfirmPasswordError() {
        return confirmPasswordErrorMessage;
    }

    /**
     * @return error message from server
     */
    public LiveData<String> getServerError() {
        return errorMessage;
    }


    /**
     * @return error message for password editText
     */
    public LiveData<String> getPasswordError() {
        return passwordErrorMessage;
    }


    public Context getActivityContext() {
        return activityContext;
    }

    public void setActivityContext(Context activityContext) {
        this.activityContext = activityContext;
    }

    @Override
    protected void onCleared(){
        super.onCleared();
        EventBus.getDefault().unregister(this);
    }

}
