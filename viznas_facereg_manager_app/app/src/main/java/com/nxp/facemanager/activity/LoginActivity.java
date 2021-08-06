package com.nxp.facemanager.activity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.Button;

import com.nxp.facemanager.R;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.databinding.ActivityLoginBinding;
import com.nxp.facemanager.viewModels.LoginViewModel;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;

import javax.inject.Inject;

import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.ViewModelProviders;

/**
 * Login view
 */
public class LoginActivity extends BaseActivity {

    /**
     * Inject FaceDb
     */
    @Inject
    FaceDatabase faceDatabase;
    /**
     * Activity Binding Object
     */
    private ActivityLoginBinding activityLoginBinding;
    /**
     * View model for login
     */
    private LoginViewModel loginViewModel;

    /**
     * Initialize of activity context.
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        activityLoginBinding = DataBindingUtil.setContentView(this, R.layout.activity_login);
        loginViewModel = ViewModelProviders.of(this, new ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(LoginViewModel.class);
        activityLoginBinding.setLogin(loginViewModel);
        loginViewModel.setActivityContext(this);
        setUpEmailError();
        setUpPasswordError();
        setServerError();
        setInternetError();
        setEditActionDoneListener();

    }

    private void setInternetError() {
        loginViewModel.getInternetErrorMessage().observe(this, s -> showNoNetworkSnackbar());
    }

    private void setEditActionDoneListener() {
        activityLoginBinding.edtPassword.setOnEditorActionListener((textView, i, keyEvent) -> {
            if (i == EditorInfo.IME_ACTION_GO) {
                activityLoginBinding.getLogin().performSignIn();
            }
            return false;
        });
    }

    private void startHomeActivity() {
        Intent intent = new Intent(LoginActivity.this, HomeActivity.class);
        startActivity(intent);
        //finish();
    }
    /**
     * Server error Live Data observer
     */
    private void setServerError() {
        loginViewModel.getServerError().observe(this, this::showSnackbar);
    }

    /**
     * Set observer on error message of invalid email
     */
    private void setUpEmailError() {
        loginViewModel.getEmailError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                activityLoginBinding.email.setError(null);
            } else {
                activityLoginBinding.email.setError(s);
                activityLoginBinding.edtEmail.requestFocus();
            }
        });
    }

    /**
     * Set observer on error message when password is empty
     */
    private void setUpPasswordError() {
        loginViewModel.getPasswordError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                activityLoginBinding.password.setError(null);
            } else {
                activityLoginBinding.password.setError(s);
                activityLoginBinding.edtPassword.requestFocus();
            }
        });

    }

    @Override
    protected void onStop() {
        super.onStop();

    }
}
