package com.nxp.facemanager.viewModels;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Handler;
import android.text.TextUtils;
import android.util.Log;
import android.util.Patterns;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.firebase.firestore.DocumentReference;
import com.google.firebase.firestore.FirebaseFirestore;
import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.CreateUserActivity;
import com.nxp.facemanager.activity.HomeActivity;
import com.nxp.facemanager.activity.TrainingActivity;
import com.nxp.facemanager.ble.BleReceiveDataModel;
import com.nxp.facemanager.ble.BleReceiveListener;
import com.nxp.facemanager.ble.BleSendDataModel;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.DatabaseOperations;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.model.CommonResponseModel;
import com.nxp.facemanager.model.CreateUserRequestModel;
import com.nxp.facemanager.model.UpdateUserRequestModel;
import com.nxp.facemanager.model.UpdateUserResponseModel;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.webservice.ApiError;
import com.nxp.facemanager.webservice.ApiInterface;
import com.nxp.facemanager.webservice.ErrorUtils;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONObject;

import java.time.Instant;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import androidx.annotation.NonNull;
import androidx.databinding.Bindable;
import androidx.databinding.ObservableField;
import androidx.lifecycle.LifecycleOwner;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModel;
import io.opencensus.tags.Tag;
import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;

import static com.nxp.facemanager.activity.CreateUserActivity.REQUEST_CODE_CAPTURE_TRAINING;
import static com.nxp.facemanager.utility.AppConstants.FACEREC_FILE_NAME;
import static com.nxp.facemanager.utility.AppConstants.LOGIN_USER_EMAIL;
import static com.nxp.facemanager.utility.CommonUtils.showLoadingDialog;

public class CreateUserViewModel extends ViewModel {


    public static final String IS_EDIT_USER = "isEdit";
    private String strProfilePic = "";
    /**
     * User name string.
     */
    public ObservableField<String> strName = new ObservableField<>("");
    /**
     * User email string.
     */
    public ObservableField<String> strEmail = new ObservableField<>("");
    public ObservableField<String> strPhone = new ObservableField<>();
    public ObservableField<Integer> intSelectedAccess = new ObservableField<>(0);
    private Boolean isEditUser;
    private String userID;
    private DatabaseOperations dbOperation;
    private int local_user_id;
    public ObservableField<String> strProfile = new ObservableField<>();
    public ObservableField<String> strProfile2 = new ObservableField<>();
    public ObservableField<String> strProfile3 = new ObservableField<>();
    public ObservableField<Integer> intId = new ObservableField<>();
    public ObservableField<String> strFeature = new ObservableField<>();
    public ObservableField<String> strFeature2 = new ObservableField<>();
    public ObservableField<String> strFeature3 = new ObservableField<>();
    public ObservableField<Boolean> bothFeatures = new ObservableField<>(Boolean.FALSE);
    public final int glassesHnadlingVersion = AppConstants.GLASSES_HANDLING_VERSION;

    public ObservableField<Boolean> getBothFeatures() {
        return bothFeatures;
    }

    public void setBothFeatures(ObservableField<Boolean> bothFeatures) {
        this.bothFeatures = bothFeatures;
    }

    public String getStrProfilePic() {
        return strProfilePic;
    }

    /**
     * {@link LiveData} object reference of {@link UserInformation}
     */
    private LiveData<List<UserInformation>> userLiveData = new LiveData<List<UserInformation>>() {
        @Override
        public void observe(@NonNull LifecycleOwner owner, @NonNull Observer<? super List<UserInformation>> observer) {
            super.observe(owner, observer);
        }
    };

    CreateUserViewModel(FaceDatabase faceDatabase, ApiInterface api, MySharedPreferences sharedPreferences) {
        this.apiInterface = api;
        dbOperation = new DatabaseOperations(faceDatabase);
        userLiveData = dbOperation.getAllUsers();
        this.mySharedPreferences = sharedPreferences;

        EventBus.getDefault().register(this);
    }

    /**
     * Live Data message for email error
     */
    private MutableLiveData<String> emailErrorMessage = new MutableLiveData<>();

    /**
     * * Error message for no internet connectivity
     */
    private MutableLiveData<String> internetErrorMessage = new MutableLiveData<>();


    public MutableLiveData<String> getInternetErrorMessage() {
        return internetErrorMessage;
    }


    /**
     * Live Data message for email error
     */
    private MutableLiveData<String> nameErrorMessage = new MutableLiveData<>();
    /**
     * Live Data message for password error
     */
    private MutableLiveData<String> passwordErrorMessage = new MutableLiveData<>();
    /**
     * Live Data message for servor message/error
     */
    private MutableLiveData<String> errorMessage = new MutableLiveData<>();
    /**
     * activity context
     */
    private Context activityContext;
    /**
     * apiInterface object
     */
    private ApiInterface apiInterface;

    private int getLocal_user_id() {
        return local_user_id;
    }

    /**
     * apiInterface object
     */
    private MySharedPreferences mySharedPreferences;
    private ProgressDialog progressDialog;

    private boolean featureChanged = false;

    public void setLocal_user_id(int local_user_id) {
        this.local_user_id = local_user_id;
    }

    public void startTraining(int frame) {
        Intent intent = new Intent(activityContext, TrainingActivity.class);
        intent.putExtra(IS_EDIT_USER, isEditUser);
        mySharedPreferences.putData(AppConstants.FRAME_NUM, frame);
        ((CreateUserActivity) activityContext).startActivityForResult(intent, REQUEST_CODE_CAPTURE_TRAINING);
    }


    BleReceiveListener bleReceiveListener = new BleReceiveListener() {
        @Override
        public void onDataReceive(BleReceiveDataModel bleReceiveDataModel) {
            switch (bleReceiveDataModel.getOp()){
                case AppConstants.OP_UPDATE_RECORD_ADD:
                    if(bothFeatures != null && bothFeatures.get()) {
                        JSONObject pl2 = new JSONObject();
                        try {
                            pl2.put(AppConstants.JSON_KEY_USERNAME, "_" + strName.get());
//                    pl.put(AppConstants.JSON_KEY_EMAIL, strEmail.get());
                            pl2.put(AppConstants.JSON_KEY_FEATURE, strFeature2.get());
                            pl2.put(AppConstants.JSON_KEY_UPDATE_OP, AppConstants.OP_UPDATE_RECORD_ADD);
                            pl2.put(AppConstants.JSON_KEY_ID, intId.get() + 1);
//                    pl.put(AppConstants.JSON_KEY_PROFILE_PIC, strProfile.get());
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                        BleSendDataModel bleSendDataModel2 = new BleSendDataModel(AppConstants.FACE_RECORD_UPDATE_REQUEST, pl2.toString());
                        bleSendDataModel2.setPayload(pl2.toString());
                        bleSendDataModel2.setBleReceiveListener(bleReceiveListener);
                        dataReceived = false;
                        EventBus.getDefault().post(bleSendDataModel2);
                    }
                    List<UserInformation> userInformationList = bleReceiveDataModel.getUserInformation();
                    for(UserInformation userInformation: userInformationList) {
                        dbOperation.insert(userInformation);
                    }
                    break;
                case AppConstants.OP_UPDATE_RECORD_DELETE:
                    break;
                case AppConstants.OP_UPDATE_RECORD_NAME:
                case AppConstants.OP_UPDATE_RECORD_FEATURE:
                    UserInformation updateUser = new UserInformation();
                    updateUser.setName(strName.get());
                    updateUser.setAdmin(false);
                    updateUser.set_id(intId.get().toString());
                    updateUser.setProfilePic(strProfile.get());
                    updateUser.setLocal_user_id(intId.get());
                    dbOperation.updateUser(updateUser);
                    break;
            }
            // save to local db
//            UserInformation curUser = new UserInformation();
//            curUser.setEmail(strEmail.get());
//            curUser.setName(strName.get());
//            curUser.setAdmin(false);
//            curUser.set_id(intId.get().toString());
//
//            curUser.setProfilePic(strProfile.get());
////                curUser.setTrainingData(strFeature.get());
//            curUser.setPhoneNo(strPhone.get());
//            if (isEditUser){
//                curUser.setLocal_user_id(intId.get());
//                dbOperation.updateUser(curUser);
//            } else{
//
//                dbOperation.insert(curUser);
//            }

//            Activity contex = (Activity) getActivityContext();
//            contex.finish();

        }

        @Override
        public void onFailure() {
            CharSequence text = "Could not add user...";
            int duration = Toast.LENGTH_SHORT;
            Toast toast = Toast.makeText(getActivityContext(), text, duration);
            toast.show();
            if(submitProgress != null){
                submitProgress.dismiss();
            }
        }
    };

    private boolean dataReceived;
    /**
     * On click for login
     */
    public void performSubmit() {
        if (TextUtils.isEmpty(strName.get())) {
            nameErrorMessage.postValue(activityContext.getString(R.string.name_validation));
        } else if (TextUtils.isEmpty(strEmail.get()) || !Patterns.EMAIL_ADDRESS.matcher(Objects.requireNonNull(strEmail.get())).matches()) {
            emailErrorMessage.postValue(activityContext.getString(R.string.email_validation));
        } else if(strFeature.get() == ""){
            Log.e("CUVM", "No valid feature");
        } else {
            //----------------------------------------------------
            String connection_type = mySharedPreferences.getStringData(AppConstants.CONNECTION_TYPE);
            if (connection_type.compareTo(AppConstants.CONNECTION_BLE)==0){
                // Create new user data model
                String op = "";
                if (isEditUser){
                    if (nameChanged){
                        op = AppConstants.OP_UPDATE_RECORD_NAME;
                        Log.e("Create User VM", "username changed");
                    }

//                            op = AppConstants.OP_UPDATE_RECORD_UPDATE;
                } else {
                    op = AppConstants.OP_UPDATE_RECORD_ADD;
                }
                JSONObject pl = new JSONObject();
                try{
                    pl.put(AppConstants.JSON_KEY_USERNAME, strName.get());
//                    pl.put(AppConstants.JSON_KEY_EMAIL, strEmail.get());
                    pl.put(AppConstants.JSON_KEY_FEATURE, strFeature.get());
                    pl.put(AppConstants.JSON_KEY_UPDATE_OP, op);
                    pl.put(AppConstants.JSON_KEY_ID, intId.get());
//                    pl.put(AppConstants.JSON_KEY_PROFILE_PIC, strProfile.get());
                }catch(Exception e){
                    e.printStackTrace();
                }
                BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.FACE_RECORD_UPDATE_REQUEST, pl.toString());
                bleSendDataModel.setPayload(pl.toString());
                bleSendDataModel.setBleReceiveListener(bleReceiveListener);
                dataReceived = false;
                EventBus.getDefault().post(bleSendDataModel);
//                final Handler mHandler = new Handler();
//                mHandler.postDelayed(new Runnable() {
//                    @Override
//                    public void run() {
//                        if (! dataReceived){
//                            bleReceiveListener.onFailure();
//                        }
//                        bleSendDataModel.setBleReceiveListener(null);
//                    }
//                }, AppConstants.BLE_CB_WAIT_INTERVAL);



            } else {
                // TODO: WIFI send
            }
//-----------------------------------------------------------------
            /*
            if (CommonUtils.isNetworkConnected(activityContext)) {
                //Api call for login
                progressDialog = showLoadingDialog(activityContext, activityContext.getString(R.string.please_wait));
//                progressDialog.show();
                CommonUtils.hideKeyboard(((CreateUserActivity) activityContext));

                if (isEditUser) {
                    UpdateUserRequestModel updateUserRequestModel = new UpdateUserRequestModel();
                    updateUserRequestModel.setEmail(strEmail.get());
                    updateUserRequestModel.setName(strName.get());
                    updateUserRequestModel.setId(userID);

                    if (!TextUtils.isEmpty(strProfilePic)) {
                        updateUserRequestModel.setProfilePic(strProfilePic);
                    }
                    String trainingData = CommonUtils.readAssetFile(activityContext, FACEREC_FILE_NAME);
                    updateUserRequestModel.setTrainingData(trainingData);
                    try {
                        updateUserRequestModel.setPhoneNo(Long.valueOf(Objects.requireNonNull(strPhone.get())));
                    } catch (Exception e) {
                        e.printStackTrace();
                    }


                    Call<UpdateUserResponseModel> call = apiInterface.updateUser(mySharedPreferences.getStringData(AppConstants.TOKEN), updateUserRequestModel);
                    call.enqueue(new Callback<UpdateUserResponseModel>() {
                        @SuppressWarnings("NullableProblems")
                        @Override
                        public void onResponse(Call<UpdateUserResponseModel> call, Response<UpdateUserResponseModel> response) {
                            if (response.body() != null) {
                                if (response.isSuccessful() && response.body().getSuccess()) {
                                    progressDialog.dismiss();
                                    response.body().getUserInformation().setLocal_user_id(getLocal_user_id());
                                    dbOperation.updateUser(response.body().getUserInformation());
                                    showSuccessDialog();
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
                        public void onFailure(Call<UpdateUserResponseModel> call, Throwable t) {
                            progressDialog.dismiss();
                            errorMessage.setValue(activityContext.getString(R.string.server_error));
                        }
                    });
                } else {
                    CreateUserRequestModel createUserRequestModel = new CreateUserRequestModel();
                    createUserRequestModel.setEmail(strEmail.get());
                    createUserRequestModel.setName(strName.get());
//                    String trainingData = CommonUtils.readAssetFile(activityContext, FACEREC_FILE_NAME);

                    if (!TextUtils.isEmpty(strProfilePic)) {
                        createUserRequestModel.setProfilePic(strProfilePic);
                    }
//                    createUserRequestModel.setTrainingData(trainingData);
                    createUserRequestModel.setTrainingData(userFeature);
                    try {
                        createUserRequestModel.setPhoneNo(Long.valueOf(Objects.requireNonNull(strPhone.get())));
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    createUserRequestModel.setAdminId(mySharedPreferences.getStringData(LOGIN_USER_EMAIL));


                    Call<CommonResponseModel> call = apiInterface.createUser(mySharedPreferences.getStringData(AppConstants.TOKEN), createUserRequestModel);
                    call.enqueue(new Callback<CommonResponseModel>() {
                        @SuppressWarnings("NullableProblems")
                        @Override
                        public void onResponse(Call<CommonResponseModel> call, Response<CommonResponseModel> response) {
                            try {
                                progressDialog.dismiss();
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                            if (response.body() != null) {
                                if (response.isSuccessful() && response.body().getSuccess()) {

                                    showSuccessDialog();
                                } else if (response.isSuccessful() && !response.body().getSuccess()) {

                                    errorMessage.setValue(response.body().getMessage());
                                }
                            } else {

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

                }
            } else {
                internetErrorMessage.setValue("");

            }
            */


        }

    }
    private ProgressDialog submitProgress;
    public boolean submitDone = false;
    private void showProgress(){
        submitProgress = new ProgressDialog(getActivityContext(), R.style.ProgressDialogStyle);
        submitProgress.setMessage("submitting your record...");
        submitDone = false;
        submitProgress.show();
    }

    int rcvCount = 0;
    @Subscribe
    public void bleReceiveDataModel(BleReceiveDataModel bleReceiveDataModel){
        if (bleReceiveDataModel.getCmd() == AppConstants.FACE_RECORD_UPDATE_RESPONSE){
            if (bleReceiveDataModel.isSuccess()){
                dataReceived = true;
                rcvCount ++;
                if(!bothFeatures.get()){
                    bleReceiveListener.onDataReceive(bleReceiveDataModel);
                }


                Log.d("CUVM", "received data model: "+rcvCount);
                if(bothFeatures.get() && rcvCount!=2){
                    bleReceiveListener.onDataReceive(bleReceiveDataModel);
                    dataReceived = false;
                    return;
                }
                if(submitProgress!=null){
                    submitProgress.dismiss();
                }
                showSuccessDialog();
                Intent intent = new Intent(getActivityContext(), HomeActivity.class);
                getActivityContext().startActivity(intent);
            } else{
                bleReceiveListener.onFailure();
            }
        }

    }

    private void showSuccessDialog() {

        final Dialog dialog = new Dialog(activityContext);
        dialog.setContentView(R.layout.dialog_user_create_success);
        TextView txtMessage = dialog.findViewById(R.id.txtMessage);
        if (isEditUser) {
            txtMessage.setText(activityContext.getString(R.string.user_edited));
        } else{
            txtMessage.setText("User added.");
        }


        dialog.setCancelable(false);
        dialog.show();

        Handler handler = new Handler();

        final Runnable r = () -> {

            dialog.dismiss();
            ((CreateUserActivity) activityContext).finish();
        };

        handler.postDelayed(r, 2000);


    }

private boolean emailChanged = false;
    private boolean nameChanged = false;

    /**
     * Text change listener for email
     */
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        if (!Patterns.EMAIL_ADDRESS.matcher(s).matches() && s.length() > 0) {
            emailErrorMessage.postValue(activityContext.getString(R.string.email_validation));
        } else {
            emailErrorMessage.postValue("");
        }
        emailChanged = true;

    }

    /**
     * Text change listener for password
     */
    public void onNameChanged(CharSequence s, int start, int before, int count) {
        nameChanged = true;
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
     * @return error message for email editText
     */
    public LiveData<String> getNameError() {
        return nameErrorMessage;
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

    public Boolean getEditUser() {
        return isEditUser;
    }

    public void setEditUser(Boolean editUser) {
        isEditUser = editUser;
    }

    public String getUserID() {
        return userID;
    }

    public void setUserID(String userID) {
        this.userID = userID;
    }

    @Override
    protected void onCleared(){
        super.onCleared();
        EventBus.getDefault().unregister(this);
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
        return userLiveData;
    }

}
