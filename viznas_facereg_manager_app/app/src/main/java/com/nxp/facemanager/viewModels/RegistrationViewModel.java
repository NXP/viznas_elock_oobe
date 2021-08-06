package com.nxp.facemanager.viewModels;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.text.TextUtils;
import android.util.Patterns;
import android.view.View;

import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.RegistrationActivity;
import com.nxp.facemanager.model.CommonResponseModel;
import com.nxp.facemanager.model.RegistrationRequestModel;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.webservice.ApiError;
import com.nxp.facemanager.webservice.ApiInterface;
import com.nxp.facemanager.webservice.ErrorUtils;

import java.util.Objects;

import androidx.core.content.ContextCompat;
import androidx.databinding.ObservableField;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;

import static com.nxp.facemanager.utility.CommonUtils.showLoadingDialog;

public class RegistrationViewModel extends ViewModel {


    /**
     * User email string.
     */
    public ObservableField<String> strEmail = new ObservableField<>("");

    /**
     * User password string.
     */
    public ObservableField<String> strPassword = new ObservableField<>("");
    /**
     * User confirm password string.
     */
    public ObservableField<String> strConfirmPassword = new ObservableField<>("");
    /**
     * User phone number string.
     */
    public ObservableField<String> strPhoneNumber = new ObservableField<>("");
    /**
     * Progress Visibility
     */
    public ObservableField<Integer> progressVisibility = new ObservableField<>(View.INVISIBLE);
    /**
     * APi interface object
     */
    private ApiInterface apiInterface;

    public Context getContext() {
        return context;
    }

    public void setContext(Context context) {
        this.context = context;
    }

    /**
     * Context variable
     */
    private Context context;


    /**
     * Live data field for email validation
     */
    private MutableLiveData<String> emailErrorMessage = new MutableLiveData<>();
    /**
     * Live data field for password validation
     */
    private MutableLiveData<String> passwordErrorMessage = new MutableLiveData<>();
    /**
     * Live data field for confirm password validation
     */
    private MutableLiveData<String> confirmPasswordErrorMessage = new MutableLiveData<>();
    /**
     * Live data field for showing error message from server
     */
    private MutableLiveData<String> errorMessage = new MutableLiveData<>();
    private MutableLiveData<String> internetErrorMessage = new MutableLiveData<>();

    public MutableLiveData<String> getInternetErrorMessage() {
        return internetErrorMessage;
    }

    RegistrationViewModel(ApiInterface api) {
        this.apiInterface = api;
    }

    /**
     * Sign in action click
     */
    public void performRegister() {

        if (TextUtils.isEmpty(strEmail.get()) || !Patterns.EMAIL_ADDRESS.matcher(Objects.requireNonNull(strEmail.get())).matches()) {
            emailErrorMessage.postValue(context.getString(R.string.email_validation));
        } else if (TextUtils.isEmpty(strPassword.get())) {
            passwordErrorMessage.postValue(context.getString(R.string.password_validation));

        } else if (!CommonUtils.isValidPassword(strPassword.get())) {
            passwordErrorMessage.postValue(context.getString(R.string.password_validation_regex));
        } else if (!TextUtils.isEmpty(strPassword.get()) && Objects.requireNonNull(strPassword.get()).length() < 6) {
            passwordErrorMessage.postValue(context.getString(R.string.password_length_validation));
        } else if (TextUtils.isEmpty(strConfirmPassword.get())) {
            confirmPasswordErrorMessage.postValue(context.getString(R.string.confirm_password_validation));

        } else if (!Objects.requireNonNull(strPassword.get()).equals(strConfirmPassword.get())) {
            confirmPasswordErrorMessage.postValue(context.getString(R.string.password_confirm_validation));
        } else {
            if (CommonUtils.isNetworkConnected(context)) {
                //Api call for registration
                ProgressDialog progressDialog = showLoadingDialog(context, context.getString(R.string.please_wait));
                progressDialog.show();
                CommonUtils.hideKeyboard((((RegistrationActivity) context)));
                RegistrationRequestModel registrationRequestModel = new RegistrationRequestModel();
                registrationRequestModel.setEmail(strEmail.get());
                registrationRequestModel.setPassword(strPassword.get());
                if (!TextUtils.isEmpty(strPhoneNumber.get())) {
                    try {
                        registrationRequestModel.setPhoneNo(Long.valueOf(Objects.requireNonNull(strPhoneNumber.get())));
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                Call<CommonResponseModel> call = apiInterface.registerAdmin(registrationRequestModel);
                call.enqueue(new Callback<CommonResponseModel>() {
                    @SuppressWarnings("NullableProblems")
                    @Override
                    public void onResponse(Call<CommonResponseModel> call, Response<CommonResponseModel> response) {
                        if (response.body() != null) {
                            if (response.isSuccessful() && response.body().getSuccess()) {
                                progressDialog.dismiss();
                                showActivationEmailDialog(response.body().getMessage());

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
                        errorMessage.setValue(context.getString(R.string.server_error));
                    }
                });
            } else {
                internetErrorMessage.setValue("");
            }


        }


    }

    /**
     * Show dialog for successful registration
     *
     * @param alertMessage: message from server
     */
    private void showActivationEmailDialog(String alertMessage) {

        AlertDialog alertDialog = new AlertDialog.Builder(context).create();
        alertDialog.setTitle(context.getString(R.string.alert_title));
        alertDialog.setMessage(alertMessage);
        alertDialog.setCancelable(false);
        alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, context.getString(R.string.ok_message),
                (dialog, which) ->
                {
                    ((RegistrationActivity) context).finish();
                    dialog.dismiss();
                });
        alertDialog.setOnShowListener(arg0 -> alertDialog.
                getButton(AlertDialog.BUTTON_POSITIVE).
                setTextColor(ContextCompat.getColor(context, R.color.icon_color)));
        alertDialog.show();

    }

    /**
     * on texchanged method for Email
     */
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        if (!Patterns.EMAIL_ADDRESS.matcher(s).matches() && s.length() > 0) {
            emailErrorMessage.postValue(context.getString(R.string.email_validation));
        } else {
            emailErrorMessage.postValue("");
        }
    }

    /**
     * on texchanged method for Password
     */
    public void onPasswordChanged(CharSequence s, int start, int before, int count) {
        if (s.length() > 0) {
            passwordErrorMessage.postValue("");
        }
    }

    /**
     * on texchanged method for Confirm Password
     */
    public void onConfirmPasswordChanged(CharSequence s, int start, int before, int count) {
        if (s.length() > 0) {
            confirmPasswordErrorMessage.postValue("");
        }
    }


    /**
     * @return error message for email editText
     */
    public LiveData<String> getEmailError() {
        return emailErrorMessage;
    }


    /**
     * @return error message for password editText
     */
    public LiveData<String> getPasswordError() {
        return passwordErrorMessage;
    }

    /**
     * @return error message for  confirm password editText
     */
    public LiveData<String> getConfirmPasswordError() {
        return confirmPasswordErrorMessage;
    }

    /**
     * @return error message for  confirm password editText
     */
    public LiveData<String> getErrorMessage() {
        return errorMessage;
    }


}
