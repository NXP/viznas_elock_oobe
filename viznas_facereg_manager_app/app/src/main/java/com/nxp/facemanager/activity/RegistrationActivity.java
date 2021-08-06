package com.nxp.facemanager.activity;

import android.os.Bundle;
import android.view.inputmethod.EditorInfo;

import com.nxp.facemanager.R;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.databinding.ActivityRegistrationBinding;
import com.nxp.facemanager.viewModels.RegistrationViewModel;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;

import javax.inject.Inject;

import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.ViewModelProviders;

/**
 * Login view
 */
public class RegistrationActivity extends BaseActivity {

    /**
     * FaceDB inject
     */
    @Inject
    FaceDatabase faceDatabase;
    /**
     * Activity Binding object
     */
    private ActivityRegistrationBinding activityRegistrationBinding;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        activityRegistrationBinding = DataBindingUtil.setContentView(this, R.layout.activity_registration);
        RegistrationViewModel registrationViewModel = ViewModelProviders.of(this, new ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(RegistrationViewModel.class);
        activityRegistrationBinding.setRegister(registrationViewModel);
        registrationViewModel.setContext(this);
        setUpEmailValidation();
        setUpPasswordValidation();
        setUpConfirmPasswordValidation();
        setUpServerError();
        setUpNetworkError();
        setEditActionDoneListener();
    }

    private void setUpNetworkError() {
        activityRegistrationBinding.getRegister().getInternetErrorMessage().observe(this, s -> showNoNetworkSnackbar());
    }

    private void setEditActionDoneListener() {
        activityRegistrationBinding.edtPhone.setOnEditorActionListener((textView, i, keyEvent) -> {
            if (i == EditorInfo.IME_ACTION_GO) {
                activityRegistrationBinding.getRegister().performRegister();
            }
            return false;
        });
    }

    /**
     * Observe live data for server message
     */
    private void setUpServerError() {
        activityRegistrationBinding.getRegister().getErrorMessage().observe(this, this::showSnackbar);
    }

    /**
     * Set observer on error message of empty confirm password
     */
    private void setUpConfirmPasswordValidation() {
        activityRegistrationBinding.getRegister().getConfirmPasswordError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                activityRegistrationBinding.confirmPassword.setError(null);
            } else {
                activityRegistrationBinding.confirmPassword.setError(s);
                activityRegistrationBinding.edtConfirmPassword.requestFocus();
            }
        });
    }

    /**
     * Set observer on error message of empty password
     */
    private void setUpPasswordValidation() {
        activityRegistrationBinding.getRegister().getPasswordError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                activityRegistrationBinding.password.setError(null);
            } else {
                activityRegistrationBinding.password.setError(s);
                activityRegistrationBinding.edtPassword.requestFocus();
            }
        });
    }

    /**
     * Set observer on error message of invalid email
     */
    private void setUpEmailValidation() {
        activityRegistrationBinding.getRegister().getEmailError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                activityRegistrationBinding.email.setError(null);
            } else {
                activityRegistrationBinding.email.setError(s);
                activityRegistrationBinding.edtEmail.requestFocus();
            }
        });
    }


}
