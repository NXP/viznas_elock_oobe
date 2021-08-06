package com.nxp.facemanager.viewModels;

import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.webservice.ApiInterface;

import androidx.annotation.NonNull;
import androidx.lifecycle.ViewModel;
import androidx.lifecycle.ViewModelProvider;

/**
 * Custom factory class to pass parameter in viewModel constructor in each activity where we have used
 * ViewModelProvide.
 */
public class ViewFactoryCommon extends ViewModelProvider.NewInstanceFactory {
    /**
     * Database reference.
     */
    private FaceDatabase faceDatabase;
    /**
     * Reference of shared preference.
     */
    private MySharedPreferences mySharedPreferences;
    /**
     * Reference of api interface.
     */
    private ApiInterface apiInterface;

    /**
     * Constructor
     *
     * @param dbOperation         {@link FaceDatabase}
     * @param api                 {@link ApiInterface}
     * @param mySharedPreferences {@link android.content.SharedPreferences}
     */
    public ViewFactoryCommon(@NonNull FaceDatabase dbOperation, ApiInterface api, MySharedPreferences mySharedPreferences) {
        this.faceDatabase = dbOperation;
        this.apiInterface = api;
        this.mySharedPreferences = mySharedPreferences;
    }

    @NonNull
    @SuppressWarnings("unchecked")
    @Override
    public <T extends ViewModel> T create(@NonNull Class<T> modelClass) {
        if (modelClass == HomeViewModel.class) {
            return (T) new HomeViewModel(faceDatabase, apiInterface, mySharedPreferences);
        } else if (modelClass == UserInformationListViewModel.class) {
            return (T) new UserInformationListViewModel(faceDatabase,apiInterface,mySharedPreferences);
        } else if (modelClass == DeviceInformationListViewModel.class) {
            return (T) new DeviceInformationListViewModel(faceDatabase, apiInterface, mySharedPreferences);
        } else if (modelClass == DeviceScanModule.class) {
            return (T) new DeviceScanModule(faceDatabase, apiInterface, mySharedPreferences);
        } else if (modelClass == RegistrationViewModel.class) {
            return (T) new RegistrationViewModel(apiInterface);
        } else if (modelClass == LoginViewModel.class) {
            return (T) new LoginViewModel(faceDatabase, apiInterface, mySharedPreferences);
        } else if (modelClass == ChangePasswordViewModel.class) {
            return (T) new ChangePasswordViewModel(faceDatabase, apiInterface, mySharedPreferences);
        } else if (modelClass == CreateUserViewModel.class) {
            return (T) new CreateUserViewModel(faceDatabase, apiInterface, mySharedPreferences);
        } else {
            return super.create(modelClass);
        }
    }
}
