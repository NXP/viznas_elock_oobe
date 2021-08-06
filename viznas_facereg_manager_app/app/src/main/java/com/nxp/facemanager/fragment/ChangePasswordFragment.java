package com.nxp.facemanager.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;

import com.nxp.facemanager.R;
import com.nxp.facemanager.databinding.FragmentChangePasswordBinding;
import com.nxp.facemanager.viewModels.ChangePasswordViewModel;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;

import androidx.databinding.DataBindingUtil;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProviders;

/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * to handle interaction events.
 * Use the {@link ChangePasswordFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class ChangePasswordFragment extends BaseFragment {


    private ChangePasswordViewModel changePasswordViewModel;
    private FragmentChangePasswordBinding binding;

    public ChangePasswordFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment SmartLocksFragment.
     */
    public static ChangePasswordFragment newInstance() {
        return new ChangePasswordFragment();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        binding = DataBindingUtil.inflate(
                inflater, R.layout.fragment_change_password, container, false);
        changePasswordViewModel = ViewModelProviders.of(this, new ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(ChangePasswordViewModel.class);
        binding.setChangePassword(changePasswordViewModel);
        changePasswordViewModel.setActivityContext(context);
        setUpConfirmPasswordValidation();
        setUpPasswordValidation();
        setUpNewPasswordValidation();
        setUpServerError();
        setUpNetworkError();
        setEditActionDoneListener();
        return binding.getRoot();
    }

    private void setUpServerError() {
        changePasswordViewModel.getErrorMessage().observe(this, this::showSnackbar);
    }


    private void setEditActionDoneListener() {
        binding.edtConfirmNewPassword.setOnEditorActionListener((textView, i, keyEvent) -> {
            if (i == EditorInfo.IME_ACTION_GO) {
                binding.getChangePassword().performSignIn();
            }
            return false;
        });
    }

    /**
     * Set observer on error message of empty confirm password
     */
    private void setUpConfirmPasswordValidation() {
        changePasswordViewModel.getConfirmPasswordError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                binding.confirmNewPassword.setError(null);
            } else {
                binding.confirmNewPassword.setError(s);
                binding.edtConfirmNewPassword.requestFocus();
            }
        });
    }

    /**
     * Set observer on error message of empty password
     */
    private void setUpPasswordValidation() {
        changePasswordViewModel.getOldPasswordError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                binding.password.setError(null);
            } else {
                binding.password.setError(s);
                binding.edtPassword.requestFocus();
            }
        });
    }

    /**
     * Set observer on error message of invalid email
     */
    private void setUpNewPasswordValidation() {

        changePasswordViewModel.getNewPasswordError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                binding.newPassword.setError(null);
            } else {
                binding.newPassword.setError(s);
                binding.edtNewPassword.requestFocus();
            }
        });
    }

    private void setUpNetworkError() {
        binding.getChangePassword().getInternetErrorMessage().observe(this, s -> showNoNetworkSnackbar());
    }


}
