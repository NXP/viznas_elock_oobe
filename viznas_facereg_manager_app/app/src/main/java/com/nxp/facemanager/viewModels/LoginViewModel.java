package com.nxp.facemanager.viewModels;

import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.util.Patterns;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.google.android.material.textfield.TextInputLayout;
import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.HomeActivity;
import com.nxp.facemanager.activity.LoginActivity;
import com.nxp.facemanager.activity.RegistrationActivity;
import com.nxp.facemanager.ble.BleConfigModel;
import com.nxp.facemanager.ble.BleReceiveDataModel;
import com.nxp.facemanager.ble.BleSendDataModel;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.DatabaseOperations;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.model.CommonResponseModel;
import com.nxp.facemanager.model.ForgotPasswordRequestModel;
import com.nxp.facemanager.model.LoginRequestModel;
import com.nxp.facemanager.model.LoginResponseModel;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.webservice.ApiError;
import com.nxp.facemanager.webservice.ApiInterface;
import com.nxp.facemanager.webservice.ErrorUtils;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.time.Instant;
import java.util.Objects;

import androidx.databinding.ObservableField;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;

import static com.nxp.facemanager.utility.CommonUtils.showLoadingDialog;

public class LoginViewModel extends ViewModel {

    /**
     * User email string.
     */
    public ObservableField<String> strEmail = new ObservableField<>("");

    /**
     * User password string.
     */
    public ObservableField<String> strPassword = new ObservableField<>("");
    /**
     * Progress Visibility
     */
    public ObservableField<Integer> progressVisibility = new ObservableField<>(View.INVISIBLE);

    /**
     * Live Data message for email error
     */
    private MutableLiveData<String> emailErrorMessage = new MutableLiveData<>();
    /**
     * Live Data message for password error
     */
    private MutableLiveData<String> passwordErrorMessage = new MutableLiveData<>();
    /**
     * Live Data message for servor message/error
     */
    private MutableLiveData<String> errorMessage = new MutableLiveData<>();

    public MutableLiveData<String> getInternetErrorMessage() {
        return internetErrorMessage;
    }

    private MutableLiveData<String> internetErrorMessage = new MutableLiveData<>();
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

    LoginViewModel(FaceDatabase faceDatabase, ApiInterface api, MySharedPreferences sharedPreferences) {
        this.apiInterface = api;
        this.faceDatabase = faceDatabase;
        dbOperation = new DatabaseOperations(faceDatabase);
        this.mySharedPreferences = sharedPreferences;
        EventBus.getDefault().register(this);
    }

    /**
     * On click for login
     */
    public void performSignIn() {

        if (TextUtils.isEmpty(strEmail.get())
                || !Patterns.EMAIL_ADDRESS.matcher(Objects.requireNonNull(strEmail.get())).matches()) {
            emailErrorMessage.postValue(activityContext.getString(R.string.email_validation));
        } else if (TextUtils.isEmpty(strPassword.get())) {
            passwordErrorMessage.postValue(activityContext.getString(R.string.password_validation));
        } else {

            if (CommonUtils.isNetworkConnected(activityContext)) {
                // Api call for login
                progressDialog = showLoadingDialog(activityContext, activityContext.getString(R.string.please_wait));
                progressDialog.show();
                CommonUtils.hideKeyboard((((LoginActivity) activityContext)));
                LoginRequestModel loginRequestModel = new LoginRequestModel();
                loginRequestModel.setEmail(strEmail.get());
                loginRequestModel.setPassword(strPassword.get());
                // TODO: Send authentication msg and wait for response
                // BleSendDataModel bleSendDataModel = new BleSendDataModel(BleRemoteCommand.AUTHENTICATION_REQUEST,
                //         strEmail.get() + strPassword.get());
                // EventBus.getDefault().post(bleSendDataModel);

                // Commented out api operations below
                /*
                 * Call<LoginResponseModel> call = apiInterface.loginUser(loginRequestModel);
                 * call.enqueue(new Callback<LoginResponseModel>() {
                 * 
                 * @SuppressWarnings("NullableProblems")
                 * 
                 * @Override public void onResponse(Call<LoginResponseModel> call,
                 * Response<LoginResponseModel> response) { if (response.body() != null) { if
                 * (response.isSuccessful() && response.body().getSuccess()) {
                 * progressDialog.dismiss(); UserInformation user = response.body().getUser();
                 * if (user != null) { mySharedPreferences.putData(AppConstants.LOGIN_USER_ID,
                 * user.get_id()); mySharedPreferences.putData(AppConstants.LOGIN_USER_EMAIL,
                 * user.getEmail()); mySharedPreferences.putData(AppConstants.IS_ADMIN,
                 * user.isAdmin()); mySharedPreferences.putData(AppConstants.LOGIN_USER_NAME,
                 * user.getName()); mySharedPreferences.putData(AppConstants.TOKEN,
                 * response.body().getToken()); dbOperation.insert(user);
       
                 * Intent intent = new Intent(activityContext, HomeActivity.class);
                 * activityContext.startActivity(intent); ((LoginActivity)
                 * activityContext).finish(); }
                 * 
                 * } else if (response.isSuccessful() && !response.body().getSuccess()) {
                 * progressDialog.dismiss();
                 * errorMessage.setValue(response.body().getMessage()); }
                 * 
                 * } else { progressDialog.dismiss(); ApiError apiError =
                 * ErrorUtils.parseError(response); errorMessage.setValue(apiError.message()); }
                 * }
                 * 
                 * @SuppressWarnings("NullableProblems")
                 * 
                 * @Override public void onFailure(Call<LoginResponseModel> call, Throwable t) {
                 * progressDialog.dismiss();
                 * errorMessage.setValue(activityContext.getString(R.string.server_error)); }
                 * });
                 */
            } else {
                internetErrorMessage.setValue("");
            }
        }

    }

    /**
     * This model called from the {@link com.nxp.facemanager.ble.BleScanningService}
     * Login success handling
     *
     * @param bleReceiveDataModel {@link BleSendDataModel}
     */
    @Subscribe
    public void bleReceiveDataModel(BleReceiveDataModel bleReceiveDataModel){
        switch (bleReceiveDataModel.getCmd()){
            case AppConstants.AUTHENTICATION_RESPONSE:
                if(bleReceiveDataModel.isSuccess()){
                    // TODO: set variables and switch activity; received data model should include username and profile picture data(in feature field)
                    try{
                        AppConstants.CUR_USERNAME = bleReceiveDataModel.getUsername();
                        AppConstants.CUR_USER_PROFILE = bleReceiveDataModel.getFeature();
                        AppConstants.USER_LOGGED_IN = true;

                        Intent intent = new Intent(activityContext, HomeActivity.class);
                        activityContext.startActivity(intent);
                    } catch(Exception e){
                        e.printStackTrace();
                    }
                } else{
                    CharSequence text = "Login failed...";
                    int duration = Toast.LENGTH_SHORT;
                    Toast toast = Toast.makeText(activityContext, text, duration);
                    toast.show();
                }
        }
    }

    /**
     * Open registration screen
     */
    public void openRegistration() {
        strEmail.set("");
        strPassword.set("");
        Intent intent = new Intent(activityContext, RegistrationActivity.class);
        activityContext.startActivity(intent);

    }

    /**
     * Open forgot password screen
     */
    public void forgotPassword() {

        showDialogForForgotPassword();

    }

    /**
     * Text change listener for email
     */
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        if (!Patterns.EMAIL_ADDRESS.matcher(s).matches() && s.length() > 0) {
            emailErrorMessage.postValue(activityContext.getString(R.string.email_validation));
        } else {
            emailErrorMessage.postValue("");
        }
    }

    /**
     * Text change listener for password
     */
    public void onPasswordChanged(CharSequence s, int start, int before, int count) {
        if (s.length() > 0) {
            passwordErrorMessage.postValue("");
        }
    }

    /**
     * @return error message for email editText
     */
    public LiveData<String> getEmailError() {
        return emailErrorMessage;
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

    /**
     * Login info mismatch dialog
     */
    private void showDialogMismatch() {

    }

    /**
     * Forgot password Dialog.
     */
    private void showDialogForForgotPassword() {
        final Dialog dialog = new Dialog(activityContext);
        dialog.setContentView(R.layout.dialog_forgot_passcode);
        WindowManager.LayoutParams lp = new WindowManager.LayoutParams();
        lp.copyFrom(Objects.requireNonNull(dialog.getWindow()).getAttributes());
        lp.width = WindowManager.LayoutParams.MATCH_PARENT;
        lp.height = WindowManager.LayoutParams.WRAP_CONTENT;

        dialog.setCancelable(false);
        final EditText edtEmail = dialog.findViewById(R.id.edtEmail);
        final TextInputLayout inputEmail = dialog.findViewById(R.id.inputEmail);
        edtEmail.clearFocus();
        edtEmail.addTextChangedListener(new TextWatcher() {

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                String email = edtEmail.getText().toString();
                if (TextUtils.isEmpty(email)) {
                    inputEmail.setError(activityContext.getString(R.string.email_empty_validation));
                } else if (!CommonUtils.isEmailValid(email)) {
                    inputEmail.setError(activityContext.getString(R.string.email_validation));
                } else {
                    inputEmail.setError("");
                }
            }
        });
        dialog.setOnShowListener(dialogInterface -> {

            Button btnResetPassword = dialog.findViewById(R.id.btnResetPassword);
            btnResetPassword.setOnClickListener(view -> {
                String email = edtEmail.getText().toString();
                if (TextUtils.isEmpty(email)) {
                    inputEmail.setError(activityContext.getString(R.string.email_empty_validation));
                } else if (!CommonUtils.isEmailValid(email)) {
                    inputEmail.setError(activityContext.getString(R.string.email_validation));
                } else {

                    if (CommonUtils.isNetworkConnected(activityContext)) {
                        ForgotPasswordRequestModel forgotPasswordRequestModel = new ForgotPasswordRequestModel();
                        forgotPasswordRequestModel.setEmail(edtEmail.getText().toString());
                        Call<CommonResponseModel> call = apiInterface.forgotPassword(forgotPasswordRequestModel);
                        progressDialog = showLoadingDialog(activityContext,
                                activityContext.getString(R.string.please_wait));
                        progressDialog.show();
                        call.enqueue(new Callback<CommonResponseModel>() {
                            @SuppressWarnings("NullableProblems")
                            @Override
                            public void onResponse(Call<CommonResponseModel> call,
                                    Response<CommonResponseModel> response) {
                                progressDialog.dismiss();
                                if (response.body() != null) {
                                    if (response.isSuccessful() && response.body().getSuccess()) {
                                        errorMessage.setValue(response.body().getMessage());
                                        progressDialog.dismiss();
                                        dialog.dismiss();

                                    } else if (response.isSuccessful() && !response.body().getSuccess()) {
                                        progressDialog.dismiss();
                                        errorMessage.setValue(response.body().getMessage());
                                    } else {
                                        ApiError apiError = ErrorUtils.parseError(response);
                                        progressDialog.dismiss();
                                        errorMessage.setValue(apiError.message());
                                    }
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
            });
            Button btnCancel = dialog.findViewById(R.id.btnCancel);
            btnCancel.setOnClickListener(view -> dialog.dismiss());

        });
        dialog.show();
        dialog.getWindow().setAttributes(lp);
    }

    @Override
    protected void onCleared(){
        super.onCleared();
        EventBus.getDefault().unregister(this);
    }

}
