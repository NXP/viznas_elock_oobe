package com.nxp.facemanager.activity;

import android.Manifest;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageButton;

//import com.amitshekhar.DebugDB;
import com.google.android.material.navigation.NavigationView;
import com.nxp.facemanager.R;
import com.nxp.facemanager.ble.BleScanningService;
import com.nxp.facemanager.ble.BleSendDataModel;
import com.nxp.facemanager.ble.BleService;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.databinding.ActivityHomeBinding;
import com.nxp.facemanager.databinding.NavHeaderHomeBinding;
import com.nxp.facemanager.fragment.ChangePasswordFragment;
import com.nxp.facemanager.fragment.HelpFragment;
import com.nxp.facemanager.fragment.RemoteControlFragment;
import com.nxp.facemanager.fragment.SmartLocksFragment;
import com.nxp.facemanager.fragment.SmartUsersFragment;

import com.nxp.facemanager.model.BleScanBackgroundServiceStartScanEvent;
import com.nxp.facemanager.model.BleSendingProgress;
import com.nxp.facemanager.model.BleServiceDisableEvent;
import com.nxp.facemanager.model.BleStatusEvent;
import com.nxp.facemanager.model.ChangeHomeIcon;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.viewModels.HomeViewModel;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;
import com.nxp.facemanager.fragment.Camera2BasicFragment;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.time.Instant;
import java.util.ArrayList;
import java.util.Objects;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.core.content.ContextCompat;
import androidx.core.view.GravityCompat;
import androidx.databinding.DataBindingUtil;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.lifecycle.ViewModelProviders;

import static androidx.core.app.ActivityCompat.requestPermissions;
import static androidx.core.app.ActivityCompat.shouldShowRequestPermissionRationale;
import static com.nxp.facemanager.ble.BleScanningService.convertStringToByteArrayChunks;
import static com.nxp.facemanager.utility.AppConstants.ACTION_DISABLE_SERVICE;

/**
 * Works as the base screen with Navigation Side Menu to link all other screens
 * 1. Ble connected/disconnected state is updated in title bar from here
 * 2. Events for ble status will be obtained here to take any specific update in title bar
 * to show case if mobile is connected to device or not
 * 3. User information is send to ble device via sendData method in this class
 */
public class HomeActivity extends BaseActivity
        implements NavigationView.OnNavigationItemSelectedListener, View.OnClickListener {
    private static final String TAG = HomeActivity.class.getSimpleName();
    private static final String TAG1 = "HomeActivity";
    public static final String IS_SKIPPABLE = "is_skippable";

    //private Oasis mOasis;
    /**
     * Data binding object reference of current activity which bind with {@link xml }
     */
    ActivityHomeBinding activityHomeBinding;
    /**
     * Reference object of @{@link ActionBarDrawerToggle}
     */
    ActionBarDrawerToggle toggle;

    /**
     * Boolean to check the hamburger/back arrow is enable or not.
     */
    private boolean mToolBarNavigationListenerIsRegistered;
    /**
     * Index of user selected menu.
     */
    int menuPosition;
    String userData = "";

    int mConnectionTypeFlag;
    String mSmartLockIP;

    /**
     * Object of progress dialog.
     */
    private ProgressDialog determinateProgressDialog;

    /**
     * start activity and register
     */
    @Override
    public void onStart() {
        super.onStart();
        EventBus.getDefault().register(this);
    }

    /**
     * stop activity and unregister event
     */
    @Override
    public void onStop() {
        super.onStop();
        EventBus.getDefault().unregister(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        updateToolbar();
    }

    protected void updateToolbar() {
//        activityHomeBinding.navView.getMenu().findItem(R.id.nav_ManageUser).setVisible(false);
        activityHomeBinding.navView.getMenu().findItem(R.id.nav_ChangePassword).setVisible(false);
        activityHomeBinding.navView.getMenu().findItem(R.id.nav_SignOut).setVisible(false);
//        activityHomeBinding.navView.getMenu().findItem(R.id.nav_Registration).setVisible(false);

        if (mConnectionTypeFlag == AppConstants.CONNECTION_NETWORK_FLAG) {
            // network connection
            activityHomeBinding.include1.toolbar.setSubtitle(mSmartLockIP);
//            activityHomeBinding.navView.getMenu().findItem(R.id.nav_SendFaceData).setVisible(true);
            activityHomeBinding.navView.getMenu().findItem(R.id.nav_RemoteControl).setVisible(true);
            activityHomeBinding.navView.getMenu().findItem(R.id.nav_ManageUser).setVisible(true);
            activityHomeBinding.navView.getMenu().findItem(R.id.nav_ChangePassword).setVisible(true);

        } else {
            // BLE connection
            if (!isMyServiceRunning(BleScanningService.class)) {
                mySharedPreferences.putData(AppConstants.CONNECTED_MAC_ADDRESS, "");
//                activityHomeBinding.navView.getMenu().findItem(R.id.nav_SendFaceData).setVisible(false);
                activityHomeBinding.navView.getMenu().findItem(R.id.nav_RemoteControl).setVisible(true);
            } else {
                if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
                    // TODO: Change to if user is logged in
                    activityHomeBinding.navView.getMenu().findItem(R.id.nav_RemoteControl).setVisible(true);
                    activityHomeBinding.include1.toolbar.setSubtitle(mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS));
                    activityHomeBinding.navView.getMenu().findItem(R.id.nav_ManageUser).setVisible(true);
                    activityHomeBinding.navView.getMenu().findItem(R.id.nav_RemoteControl).setVisible(true);
                    activityHomeBinding.navView.getMenu().findItem(R.id.nav_ChangePassword).setVisible(true);
                    activityHomeBinding.navView.getMenu().findItem(R.id.nav_SignOut).setVisible(true);
                    


                } else {
                    activityHomeBinding.include1.toolbar.setSubtitle(null);
//                    activityHomeBinding.navView.getMenu().findItem(R.id.nav_SendFaceData).setVisible(false);
                    activityHomeBinding.navView.getMenu().findItem(R.id.nav_RemoteControl).setVisible(false);
                }
            }
            activityHomeBinding.navView.getMenu().findItem(R.id.nav_ManageUser).setVisible(true);
            activityHomeBinding.navView.getMenu().findItem(R.id.nav_RemoteControl).setVisible(true);
            activityHomeBinding.navView.getMenu().findItem(R.id.nav_ChangePassword).setVisible(true);

        }
    }

    /**
     * Finish view based on back and hamburger condition.
     */
    @Override
    public void onBackPressed() {
        if (activityHomeBinding.drawerLayout.isDrawerOpen(GravityCompat.START)) {
            activityHomeBinding.drawerLayout.closeDrawer(GravityCompat.START);
        } else {
            if (mToolBarNavigationListenerIsRegistered) {
                EventBus.getDefault().post(new ChangeHomeIcon(false));
            } else {
                super.onBackPressed();
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        ///mOasis = new Oasis();
        super.onCreate(savedInstanceState);
        int permissionCheck = ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION);
        if (permissionCheck != PackageManager.PERMISSION_GRANTED){
            if (shouldShowRequestPermissionRationale(Manifest.permission.ACCESS_FINE_LOCATION)){
                Toast.makeText(this, "The permission to get BLE location data is required", Toast.LENGTH_SHORT).show();
            }else{
                requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION, Manifest.permission.ACCESS_FINE_LOCATION}, 1);
            }
        }else{
            Toast.makeText(this, "Location permissions already granted", Toast.LENGTH_SHORT).show();
        }
        activityHomeBinding = DataBindingUtil.setContentView(this, R.layout.activity_home);
        HomeViewModel homeViewModel = ViewModelProviders.of(this, new ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(HomeViewModel.class);
        activityHomeBinding.setHomemodel(homeViewModel);
        NavHeaderHomeBinding headerBinding = NavHeaderHomeBinding.bind(activityHomeBinding.navView.getHeaderView(0));
        setSupportActionBar(activityHomeBinding.include1.toolbar);
        toggle = new ActionBarDrawerToggle(
                this, activityHomeBinding.drawerLayout, activityHomeBinding.include1.toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        activityHomeBinding.drawerLayout.addDrawerListener(toggle);
        toggle.syncState();
        activityHomeBinding.navView.setNavigationItemSelectedListener(this);

        activityHomeBinding.navView.getMenu().getItem(0).setChecked(true);
//        AppLogger.d("DebugDB", DebugDB.getAddressLog());
//        Log.d(TAG1, "DebugDB" + DebugDB.getAddressLog());

        String connectionTypeStr = mySharedPreferences.getStringData(AppConstants.CONNECTION_TYPE);

        //Log.d(TAG1, "++onCreate" + "connection[" + mySharedPreferences.getStringData(AppConstants.CONNECTION_TYPE) + "]" + "[" + mySharedPreferences.getStringData(AppConstants.SMARTLOCK_IP) + "]");

        if (connectionTypeStr.compareTo(AppConstants.CONNECTION_NETWORK) == 0) {
             mConnectionTypeFlag = AppConstants.CONNECTION_NETWORK_FLAG;
             mSmartLockIP = mySharedPreferences.getStringData(AppConstants.SMARTLOCK_IP);
        } else {
            mConnectionTypeFlag = AppConstants.CONNECTION_BLE_FLAG;
            mSmartLockIP = "";
            bindFragment(0);
        }

        Log.d(TAG1, "++[connectionType:" + mConnectionTypeFlag + ":" + mSmartLockIP + "]");

        ImageButton imgButton = activityHomeBinding.navView.getHeaderView(0).findViewById(R.id.imgEditProfile);
        imgButton.setOnClickListener(this);
        ImageView userImage = activityHomeBinding.navView.getHeaderView(0).findViewById(R.id.imageView);
        TextView username = activityHomeBinding.navView.getHeaderView(0).findViewById(R.id.txtUserName);
        TextView userEmail = activityHomeBinding.navView.getHeaderView(0).findViewById(R.id.txtEmail);
        if (AppConstants.USER_LOGGED_IN){
            username.setText(AppConstants.CUR_USERNAME);
            userEmail.setText(AppConstants.CUR_USER_EMAIL);
            userImage.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    // TODO: user profile upload or training activity?
                }
            });

            if (AppConstants.CUR_USER_PROFILE!=null){
                try {
                    Bitmap bitmap = stringToBitMap(AppConstants.CUR_USER_PROFILE);
                    Drawable drawable = new BitmapDrawable(getResources(), bitmap);
                    userImage.setImageDrawable(drawable);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }

        }


        activityHomeBinding.getHomemodel().getUserInformationLiveData().observe(this, userInformation -> {
            Log.d(TAG1, "getUserInformationLiveData().observe");
            if (userInformation != null) {
                Log.d(TAG1, "userInformation is null");
                if (userInformation.getTrainingData() == null) {
                    try {
                        userData = readAssetFile("facerec1.txt");
                        if (userData != null) {
                            userInformation.setTrainingData(userData);
                        } else {
                            userInformation.setTrainingData("");
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                activityHomeBinding.navView.getMenu().findItem(R.id.nav_ManageUser).setVisible(userInformation.isAdmin());
                if (TextUtils.isEmpty(userInformation.getName())) {
                    Log.d(TAG1, "userInformation.getName()");
                    int index = userInformation.getEmail().indexOf("@");
                    if (index > 0) {
                        userInformation.setName(userInformation.getEmail().substring(0, index));
                    }
                }
                this.userInformation = userInformation;
                headerBinding.setUser(userInformation);
            }
        });

        activityHomeBinding.getHomemodel().getDbOperation().getAllDeviceInfo().observe(this,
                deviceInfos -> activityHomeBinding.getHomemodel().setDeviceInfoList(deviceInfos));

        Log.d(TAG1, "--onCreate"); 
    }

    /**
     * Side menu item click listener.
     *
     * @param item {@link MenuItem}
     * @return boolean
     */
    @Override
    public boolean onNavigationItemSelected(@NonNull MenuItem item) {

        // Handle navigation view item clicks here.
        int id = item.getItemId();
        Intent startTrainingActivity;
        switch (id) {
            case R.id.nav_ManageLocks:
                menuPosition = 0;
                bindFragment(0);
                break;
            case R.id.nav_ManageUser:
                menuPosition = 1;
                bindFragment(1);
                break;
//            case R.id.nav_SendFaceData:
//                menuPosition = 2;
//                startTrainingActivity = new Intent(context, TrainingActivity.class);
//                startTrainingActivity.putExtra(IS_SKIPPABLE, false);
//                if (mConnectionTypeFlag == AppConstants.CONNECTION_NETWORK_FLAG) {
//                    startTrainingActivity.putExtra(AppConstants.CONNECTION_TYPE, AppConstants.CONNECTION_NETWORK);
//                } else {
//                    startTrainingActivity.putExtra(AppConstants.CONNECTION_TYPE, AppConstants.CONNECTION_BLE);
//                }
//                startTrainingActivity.putExtra(AppConstants.SMARTLOCK_IP, mSmartLockIP);
//
//                startActivity(startTrainingActivity);
//                Log.d(TAG1, "AddFace onClick");
//                break;
            case R.id.nav_ChangePassword:
                menuPosition = 3;
                bindFragment(3);
                break;
            case R.id.nav_Help:
                menuPosition = 4;
                bindFragment(4);
                break;
            case R.id.nav_SignOut:
                disconnectService();
                if (activityHomeBinding.getHomemodel() != null) {
                    activityHomeBinding.getHomemodel().resetData();
                }
                Intent intent = new Intent(this, LoginActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                startActivity(intent);
                finish();
                break;
//            case R.id.nav_Registration:
//                menuPosition = 6;
//                //bindFragment(6);
//
//                startTrainingActivity = new Intent(context, TrainingActivity.class);
//                startTrainingActivity.putExtra(IS_SKIPPABLE, false);
//                startActivity(startTrainingActivity);
//                //if (mOasis != null) {
//                    //mOasis.Init();
//                //}
//                Log.d(TAG1, "AddFace onClick");
//                break;
            case R.id.nav_RemoteControl:
                menuPosition = 7;
                bindFragment(7);
                break;
            default:
                bindFragment(0);
                menuPosition = 0;
                break;
        }

        activityHomeBinding.drawerLayout.closeDrawers();
        return true;

    }

    /**
     * Send userinformation to ble device via this method
     * @param userInformation string
     */
    public void sendData(String userInformation) {
        if (userInformation != null) {
            if (TextUtils.isEmpty(userInformation)) {
                showDialogForSendFaceData(getString(R.string.msg_start_traning));
            } else {
                if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
                    DeviceInfo deviceInfo = Objects.requireNonNull(activityHomeBinding.getHomemodel()).checkDeviceExitsInDb(mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS));
                    if (deviceInfo != null) {
//                        BleSendDataModel bleSendDataModel = new BleSendDataModel(
//                                BleRemoteCommand.READY_INDICATION,
//                                userInformation);
//                                AppConstants.TRAINING_INFORMATION,
//                                deviceInfo.getDevice_name(),
//                                "" + deviceInfo.getPass_code(),
//                                null,
//                                userInformation.getBytes().length,
//                                userInformation);
//                        EventBus.getDefault().post(bleSendDataModel);

                    }
                } else {
                    checkPermission();
                }
            }
        }
    }


    /**
     * Event bus for the disable ble on menu item click.
     *
     * @param bleScanBackgroundServiceStartScanEvent {@link BleScanBackgroundServiceStartScanEvent}
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void bleScanBackgroundServiceStartScanEvent(BleScanBackgroundServiceStartScanEvent bleScanBackgroundServiceStartScanEvent) {
        if (bleScanBackgroundServiceStartScanEvent.isStartScan()) {

            startAutoConnectService(bleScanBackgroundServiceStartScanEvent.getBluetoothDevice());
        } else {
            disconnectService();
//            activityHomeBinding.navView.getMenu().findItem(R.id.nav_SendFaceData).setVisible(false);
        }
    }

    /**
     * Status update evnet for ble that is any change in ble connection will be notified from here
     *
     * @param bleStatusEvent: Event of ble on which actions will be taken
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void bleStatusEvent(BleStatusEvent bleStatusEvent) {
        int connectionState = bleStatusEvent.getStatus();
        BluetoothDevice device = bleStatusEvent.getBluetoothDevice();
        if (connectionState == BleService.STATE_LINK_LOSS
                || connectionState == BleService.STATE_DISCONNECTED
                || connectionState == BleService.STATE_DISCONNECTING) {
            updateToolbar();
        } else if (connectionState == BleService.STATE_CONNECTING) {
            activityHomeBinding.include1.toolbar.setSubtitle(getString(R.string.connecting));
        } else if (connectionState == BleService.STATE_CONNECTED) {
//            activityHomeBinding.navView.getMenu().findItem(R.id.nav_SendFaceData).setVisible(true);
            activityHomeBinding.include1.toolbar.setSubtitle(getString(R.string.connected));
        } else if (connectionState == BleService.STATE_DEVICE_READY) {
//            activityHomeBinding.navView.getMenu().findItem(R.id.nav_SendFaceData).setVisible(true);
            activityHomeBinding.include1.toolbar.setSubtitle(mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS));

        }
    }

    /**
     * Disconnect ble service.
     */
    private void disconnectService() {
        activityHomeBinding.include1.toolbar.setSubtitle("");
        Intent disableIntent = new Intent(this, BleScanningService.class);
        disableIntent.setAction(ACTION_DISABLE_SERVICE);
        startService(disableIntent);
    }

    /**
     * Bind Fragment Based on position.
     *
     * @param position user selected menu position
     */
    void bindFragment(int position) {
        Fragment fragment = null;
        String title = getString(R.string.smart_locks);
        try {
            switch (position) {
                case 0:
                    fragment = SmartLocksFragment.newInstance();
                    title = getString(R.string.smart_locks);
                    break;
                case 1:
                    fragment = SmartUsersFragment.newInstance();
                    title = getString(R.string.smart_users);
                    break;
                case 2:
                    break;
                case 3:
                    fragment = ChangePasswordFragment.newInstance();
                    title = getString(R.string.change_password);
                    break;
                case 4:
                    fragment = HelpFragment.newInstance();
                    title = getString(R.string.help);
                    break;
                case 5:
                    break;
                case 6:
                    Log.d(TAG1, "bindFragment Camera2BasicFragment");
                    /*getSupportFragmentManager().beginTransaction()
                    .replace(R.id.container, Camera2BasicFragment.newInstance())
                    .commit();*/
                    fragment = Camera2BasicFragment.newInstance();
                    title = "Add Face";
                    break;
                case 7:
                    fragment = RemoteControlFragment.newInstance();
                    title = "Remote Control";
                    break;
                default:
                    fragment = SmartLocksFragment.newInstance();
                    title = "Smart Locks";
                    break;
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

        // Insert the fragment by replacing any existing fragment
        FragmentManager fragmentManager = getSupportFragmentManager();
        if (fragment != null) {

            fragmentManager.beginTransaction().replace(R.id.frameContent, fragment).commit();
        }
        setTitle(title);
    }

    /**
     * This method called from {@link SmartLocksFragment} and {@link SmartUsersFragment}
     * to enable hamburger icons
     *
     * @param changeHomeIcon {@link ChangeHomeIcon}
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void ChangeHomeIcon(ChangeHomeIcon changeHomeIcon) {
        if (changeHomeIcon.isBackEnable()) {
            enableViews(true);
        } else {
            enableViews(false);
        }
    }

    /**
     * This method used to hide/show back arrow.
     *
     * @param enable true/false
     */
    private void enableViews(boolean enable) {

        // To keep states of ActionBar and ActionBarDrawerToggle synchronized,
        // when you enable on one, you disable on the other.
        // And as you may notice, the order for this operation is disable first, then enable - VERY VERY IMPORTANT.
        if (enable) {
            //You may not want to open the drawer on swipe from the left in this case
            activityHomeBinding.drawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_CLOSED);
            // Remove hamburger
            toggle.setDrawerIndicatorEnabled(false);
            // Show back button
            Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
            // when DrawerToggle is disabled i.e. setDrawerIndicatorEnabled(false), navigation icon
            // clicks are disabled i.e. the UP button will not work.
            // We need to add a listener, as in below, so DrawerToggle will forward
            // click events to this listener.
            if (!mToolBarNavigationListenerIsRegistered) {
                toggle.setToolbarNavigationClickListener(v -> {
                    // Doesn't have to be onBackPressed
                    EventBus.getDefault().post(new ChangeHomeIcon(false));
//                        enableViews(false);
                });

                mToolBarNavigationListenerIsRegistered = true;
            }


        } else {
            //You must regain the power of swipe for the drawer.
            activityHomeBinding.drawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_UNLOCKED);
            // Remove back button
            Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(false);
            // Show hamburger
            toggle.setDrawerIndicatorEnabled(true);
            // Remove the/any drawer toggle listener
            toggle.setToolbarNavigationClickListener(null);
            mToolBarNavigationListenerIsRegistered = false;
        }
    }

    /**
     * Check service disconnection and update toolBar
     *
     * @param bleServiceDisableEvent boolean flag that event handle condition of service disconnection
     */
    @Subscribe
    public void bleServiceDisableEvent(BleServiceDisableEvent bleServiceDisableEvent) {
        if (bleServiceDisableEvent.isServiceDisable()) {
            activityHomeBinding.include1.toolbar.setSubtitle("");
        }
    }

    public void setNavigationMenu(int index) {
        activityHomeBinding.navView.getMenu().getItem(index).setChecked(true);
        onNavigationItemSelected(activityHomeBinding.navView.getMenu().getItem(index));
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.imgEditProfile) {
            Intent intent = new Intent(context, CreateUserActivity.class);
            intent.putExtra(AppConstants.IS_EDIT_USER, true);
            if (activityHomeBinding.getHomemodel() != null) {
                intent.putExtra(AppConstants.USER_DATA, activityHomeBinding.getHomemodel().getUserInformationLiveData().getValue());
            }
            startActivity(intent);
        }
    }

    /**
     * Forgot password Dialog.
     */
    private void showDialogForSendFaceData(String message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(HomeActivity.this, R.style.AlertDialogTheme);
        builder.setTitle(R.string.send_face_data);
        builder.setMessage(message);
        builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
            Intent intent = new Intent(context, CreateUserActivity.class);
            intent.putExtra(AppConstants.IS_EDIT_USER, true);
            if (activityHomeBinding.getHomemodel() != null) {
                intent.putExtra(AppConstants.USER_DATA, activityHomeBinding.getHomemodel().getUserInformationLiveData().getValue());
            }
            startActivity(intent);
        });
        builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> dialogInterface.dismiss());
        builder.setCancelable(false);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * Return the sending progress of ble data.
     *
     * @param bleSendingProgress {@link BleSendingProgress}
     */
    @Subscribe
    public void bleSendingProgress(BleSendingProgress bleSendingProgress) {
//        AppLogger.d("FaceRec:BleSendingProgress: " + bleSendingProgress.getMaxProgress() + " " + bleSendingProgress.getProgress());
        if (determinateProgressDialog == null) {
            determinateProgressDialog = new ProgressDialog(this, R.style.AlertDialogTheme);
            determinateProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            determinateProgressDialog.setIndeterminate(false);
            determinateProgressDialog.setMax(bleSendingProgress.getMaxProgress() - 1);
            determinateProgressDialog.setMessage("Sending User Data...");
            determinateProgressDialog.setCancelable(false);
            if (!isFinishing()) {
                determinateProgressDialog.show();
                determinateProgressDialog.setProgress(bleSendingProgress.getProgress());
                if (bleSendingProgress.getProgress() == bleSendingProgress.getMaxProgress() - 1) {
                    determinateProgressDialog.dismiss();
                    determinateProgressDialog = null;
                }
            }
        } else {
            determinateProgressDialog.setProgress(bleSendingProgress.getProgress());
            if (bleSendingProgress.getProgress() == bleSendingProgress.getMaxProgress() - 1) {
                determinateProgressDialog.dismiss();
                determinateProgressDialog = null;
            }
        }
    }
}
