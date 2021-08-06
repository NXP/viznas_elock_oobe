package com.nxp.facemanager.activity;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.widget.ImageView;
import android.widget.Button;
import android.widget.RadioGroup;
import android.widget.RadioButton;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.text.TextUtils;
import android.annotation.SuppressLint;

import com.nxp.facemanager.R;
import com.nxp.facemanager.utility.AppConstants;
import android.util.Log;

import java.time.Instant;

/**
 * Load when application start from initial state.
 */
public class SplashActivity extends BaseActivity {
    private static final String TAG = "SplashActivity";
    private Button mStart;
    private RadioGroup mRadioGroupConnection;
    private RadioButton mRadioButtonBLE,mRadioButtonNetwork;
    String mDevicename = AppConstants.SMARTLOCK_NAME_DEFAULT;
    String mDeviceIP = AppConstants.SMARTLOCK_IP_DEFAULT;

    /**
     * Initialize of activity context.
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "++onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);

        mRadioGroupConnection =(RadioGroup)findViewById(R.id.radioGroup_connection);
        mRadioButtonNetwork = (RadioButton)findViewById(R.id.connection_network);
        mRadioButtonBLE = (RadioButton)findViewById(R.id.connection_ble);

        mySharedPreferences.putData(AppConstants.LOGIN_USER_ID, "11");
        mySharedPreferences.putData(AppConstants.LOGIN_USER_EMAIL, "test@nxp.com");
        mySharedPreferences.putData(AppConstants.IS_ADMIN, true);
        mySharedPreferences.putData(AppConstants.LOGIN_USER_NAME, "test");
        mySharedPreferences.putData(AppConstants.TOKEN, "22");
        mySharedPreferences.putData(AppConstants.CONNECTION_TYPE, AppConstants.CONNECTION_BLE);
        mySharedPreferences.putData(AppConstants.SMARTLOCK_IP, AppConstants.SMARTLOCK_IP_DEFAULT);
        mRadioGroupConnection.clearCheck();
        // initView();
        //splashDelay();

        //Intent intent;
        //intent = new Intent(SplashActivity.this, HomeActivity.class);
        //startActivity(intent);
        mRadioGroupConnection.setOnCheckedChangeListener(new  RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                switch (checkedId) {
                    case R.id.connection_network:
                      Log.d(TAG, "++network");
                      mRadioButtonNetwork.setChecked(true);
                      mySharedPreferences.putData(AppConstants.CONNECTION_TYPE, AppConstants.CONNECTION_NETWORK);
                      showDialogForDeviceNameAndPasscode();
                      mRadioGroupConnection.clearCheck();
                      break;

                    case R.id.connection_ble:
                      Log.d(TAG, "++ble");
                      mRadioButtonBLE.setChecked(true);
                      mySharedPreferences.putData(AppConstants.CONNECTION_TYPE, AppConstants.CONNECTION_BLE);

                      startHomeActivity();
//                      startLoginActivity();
                      mRadioGroupConnection.clearCheck();
                      break;
                }
            }
        });

    }

    private void startLoginActivity(){
        Intent intent = new Intent(SplashActivity.this, LoginActivity.class);
        startActivity(intent);
    }
    private void startHomeActivity() {
        Intent intent = new Intent(SplashActivity.this, HomeActivity.class);
        startActivity(intent);
        //finish();
    }
    private void showDialogForDeviceNameAndPasscode() {
            LayoutInflater li = LayoutInflater.from(this);
            @SuppressLint("InflateParams")
            View dialogView = li.inflate(R.layout.dialog_devicenameip, null);
            AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
            alertDialogBuilder.setTitle(getString(R.string.smartlock_information));
            alertDialogBuilder.setView(dialogView);
            final EditText edtDeviceName = dialogView
                    .findViewById(R.id.edtDeviceName);
            final EditText edtIP = dialogView
                    .findViewById(R.id.edtIP);
            final Button btnOK = dialogView.findViewById(R.id.btnOK);

            edtDeviceName.setText(mDevicename);
            edtIP.setText(mDeviceIP);

            // set dialog message
            AlertDialog alertDialog = alertDialogBuilder.create();

            btnOK.setOnClickListener(new View.OnClickListener() {

                @Override
                public void onClick(View v) {
                    mDevicename = edtDeviceName.getText().toString();
                    mDeviceIP = edtIP.getText().toString();
                     if (TextUtils.isEmpty(mDevicename)) {
                        showToast(getString(R.string.please_enter_device_name));
                    } else if (TextUtils.isEmpty(mDeviceIP)) {
                        showToast(getString(R.string.please_enter_ip));
                    } else {
                        alertDialog.dismiss();
                        mySharedPreferences.putData(AppConstants.SMARTLOCK_IP, mDeviceIP);
                        startHomeActivity();
                    }
                }
            });

            alertDialog.show();
        }

    /**
     * Initialize the view and show the animation on view.
     */
    private void initView() {
        ImageView imageView = findViewById(R.id.image);
        Animation anim = new AlphaAnimation(0.0f, 1.0f);
        anim.setDuration(500); //You can manage the blinking time with this parameter
        anim.setStartOffset(20);
        anim.setRepeatMode(Animation.REVERSE);
        anim.setRepeatCount(Animation.INFINITE);
        imageView.startAnimation(anim);
    }

    /**
     * Called whenever the application comes to foreground.
     */
    @Override
    protected void onResume() {
        super.onResume();
        mRadioGroupConnection.clearCheck();
        if (mRadioButtonBLE.isChecked())
            Log.e(TAG,"BLE checked");
        if(mRadioButtonNetwork.isChecked())
            Log.e(TAG, "netWork checked");
        //hideSystemUI();
        Log.e(TAG,"onResume");
    }

    /**
     * View not visible
     */
    @Override
    protected void onPause() {
        super.onPause();
        showSystemUI();
        Log.e(TAG,"onPause");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.e(TAG,"OnDestroy");
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.e(TAG,"onStart");
        mRadioButtonNetwork.setChecked(false);
        mRadioButtonBLE.setChecked(false);
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.e(TAG,"onStop");
    }

    /**
     * Handler for load the login screen.
     */
    private void splashDelay() {
        new Handler().postDelayed(() -> {
            Intent intent = new Intent(SplashActivity.this, HomeActivity.class);
            startActivity(intent);
            finish();
        }, 2000);
    }

    /**
     * Show system ui with navigation bar.
     */
    private void showSystemUI() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
    }

    /**
     * Hide system ui navigation bar and status bar.
     */
    private void hideSystemUI() {
        // Enables regular immersive mode.
        // For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
        // Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE
                        // Set the content to appear under the system bars so that the
                        // content doesn't resize when the system bars hide and show.
                        | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        // Hide the nav bar and status bar
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }
}
