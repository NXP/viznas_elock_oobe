package com.nxp.facemanager.activity;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.PopupMenu;
import android.widget.Toast;

import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.adapter.DashboardAdapter;
import com.nxp.facemanager.ble.BleReceiveDataModel;
import com.nxp.facemanager.ble.BleScanningService;
import com.nxp.facemanager.ble.BleService;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.databinding.ActivityDashboardBinding;
import com.nxp.facemanager.model.BelStartTrainingDataSendEvent;
import com.nxp.facemanager.model.BleSendingProgress;
import com.nxp.facemanager.model.BleServiceDisableEvent;
import com.nxp.facemanager.model.BleSnapshotEvent;
import com.nxp.facemanager.model.BleStatusEvent;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.utility.ScreenUtils;
import com.nxp.facemanager.viewModels.UserInformationListViewModel;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Objects;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.ViewModelProviders;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.ItemTouchHelper;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import static com.nxp.facemanager.utility.AppConstants.ACTION_DISABLE_SERVICE;

/**
 * Shows the list of user and we can add new from here.
 */
public class DashboardActivity extends BaseActivity implements OnItemClickListener, View.OnClickListener {

    private static final String TAG = DashboardActivity.class.getSimpleName();
    /**
     * Data binding object reference of current activity which bind with {@link xml }
     */
    ActivityDashboardBinding activityDashboardBinding;
    /**
     * Adapter object for binding the {@link RecyclerView}.
     */
    DashboardAdapter adapter;
    /**
     * Menu item for ble disconnect.
     */
    MenuItem menu_disconnect;

    private Paint p = new Paint();
    String userData = "";

    /**
     * Object of {@link ProgressDialog}
     */
    protected ProgressDialog progressDialogIndeterminate, determinateProgressDialog;

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


    /**
     * Initialize of activity context.
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        activityDashboardBinding = DataBindingUtil.setContentView(this, R.layout.activity_dashboard);
        UserInformationListViewModel userListViewModel = ViewModelProviders.of(this, new ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(UserInformationListViewModel.class);
        activityDashboardBinding.setUser(userListViewModel);
        setToolbar();
        bindRecyclerView();
        setUpNetworkError();
        userListViewModel.getUserList().observe(this, userInformations -> {
            // Update the cached copy of the words in the adapter.
            adapter.setUsers(userInformations);
        });
        try {
            userData = readAssetFile("facerec1.txt");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    /**
     * This method called when user come back to view again.
     */
    @Override
    protected void onResume() {
        super.onResume();
        String connected_mac_address = mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS);
        if (!connected_mac_address.isEmpty()) {
            activityDashboardBinding.toolbar.setSubtitle(connected_mac_address);
            if (menu_disconnect != null) {
                menu_disconnect.setVisible(true);
            }
        } else {
            activityDashboardBinding.toolbar.setSubtitle(null);
            if (menu_disconnect != null) {
                menu_disconnect.setVisible(false);
            }
        }
    }

    /**
     * Initialize the toolbar
     */
    private void setToolbar() {
        setSupportActionBar(activityDashboardBinding.toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setDisplayShowHomeEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(false);

    }


    /**
     * Called when any specific row item object click from User List adapter.
     * Contains BLE icon click,mac address click and show more button.
     */
    @Override
    public void onItemClick(Object item, View viewId) {
        UserInformation userInformation = (UserInformation) item;
        switch (viewId.getId()) {
            case R.id.img_ble:
                getMyApplication().setUserInformation(userInformation);
                checkPermission();
                break;
            case R.id.tv_mac_add:
                Toast.makeText(this, "mac Click", Toast.LENGTH_SHORT).show();
                break;
            case R.id.tv_show_more:
                if (userInformation.getDeviceIds().size() > 0) {
                    showMacAddressesDialog(userInformation);
                } else {
                    Toast.makeText(this, R.string.msg_error, Toast.LENGTH_SHORT).show();
                }
                break;
            case R.id.txtOptions:
                View view = findViewById(R.id.txtOptions);
                PopupMenu popup = new PopupMenu(this, view);
                //inflating menu from xml resource
                popup.inflate(R.menu.user_menu);
                MenuItem menuConnect = popup.getMenu().findItem(R.id.menuConnect);
                MenuItem menuSnapshot = popup.getMenu().findItem(R.id.menuSnapshot);
                if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
                    menuConnect.setVisible(false);
                    menuSnapshot.setVisible(true);
                } else if (userInformation.getDeviceIds() == null) {
                    menuConnect.setVisible(false);
                    menuSnapshot.setVisible(true);
                } else {
                    menuSnapshot.setVisible(false);
                }
                //adding click listener
                popup.setOnMenuItemClickListener(item2 -> {

                    switch (item2.getItemId()) {
                        case R.id.menuConnect:
                            BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                            if (mBluetoothAdapter != null) {
                                getMyApplication().setUserInformation(userInformation);
                                //startAutoConnectService(mBluetoothAdapter.getRemoteDevice(userInformation.getDevices().get(0).getMac_address()));
                            }
                            break;
                        case R.id.menuTraining:
                            EventBus.getDefault().unregister(this);
                            startActivity(new Intent(DashboardActivity.this, TrainingActivity.class));
                            break;
                        case R.id.menuFace:
                            if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
                                android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(DashboardActivity.this);
                                builder.setTitle(userInformation.getName() + "'s" + "Trained Data");
                                builder.setMessage("Do you want send training data?");
                                builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
//                                    BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.TRAINING_INFORMATION,
//                                            userInformation.getDevices().get(0).getDevice_name(),
//                                            userInformation.getDevices().get(0).getPass_code(),
//                                            userInformation.getCookie(),
//                                            userInformation.getTrainingData().getBytes().length,
//                                            userInformation.getTrainingData());
//                                    EventBus.getDefault().post(bleSendDataModel);
                                });
                                builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> dialogInterface.dismiss());
                                builder.setCancelable(false);
                                android.app.AlertDialog dialog = builder.create();
                                dialog.show();
                            } else {
                                EventBus.getDefault().unregister(this);
                                getMyApplication().setUserInformation(userInformation);
                                startActivity(new Intent(this, StartFGServiceActivity.class));
                            }
                            break;
                        case R.id.menuDelete:
                            //handle menu3 click
                            activityDashboardBinding.getUser().deleteUser(userInformation);
                            mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                            disconnectService();
                            if (mBluetoothAdapter != null) {
//                                if (userInformation.getDevices() != null && userInformation.getDevices().size() > 0) {
//                                    BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(userInformation.getDevices().get(0).getMac_address());
//                                    if (device != null) removePairing(device);
//                                }
                            }
                            break;


                        case R.id.menuSnapshot:
                            //Request Snapshot
                            if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
                                android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(DashboardActivity.this);
                                builder.setTitle(userInformation.getName());
                                builder.setMessage("Do you want to request Snapshot data?");
                                builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
//                                    BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.SNAPSHOT_INFORMATION,
//                                            userInformation.getDevices().get(0).getDevice_name(),
//                                            userInformation.getDevices().get(0).getPass_code(),
//                                            userInformation.getCookie(),
//                                            0,
//                                            userInformation.getTrainingData());
                                    // EventBus.getDefault().post(bleSendDataModel);
                                });
                                builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> dialogInterface.dismiss());
                                builder.setCancelable(false);
                                android.app.AlertDialog dialog = builder.create();
                                dialog.show();
                            }

                            break;
                    }
                    return false;
                });
                //displaying the popup
                popup.show();
                break;
        }
    }


    /**
     * Fab button click which is used to add more user.
     *
     * @param view {@link View}
     */
    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.fab:
                if (activityDashboardBinding.getUser().getUserList() != null
                        && activityDashboardBinding.getUser().getUserList().getValue() != null) {
                    if (activityDashboardBinding.getUser().getUserList().getValue().size() > 0) {
                        showToast("You can add only one user for demo.");
                    } else {
                        showDialogForUserName();
                    }
                }
//                startActivity(new Intent(this, TrainingActivity.class));
                break;
            default:
                break;
        }
    }

    /**
     * Bind menu to the view.
     *
     * @param menu {@link MenuItem}
     * @return true/false
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.theme_menu, menu);
        menu_disconnect = menu.findItem(R.id.menu_disconnect);
        //To update toolbar
        if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
            menu_disconnect.setVisible(true);
            activityDashboardBinding.toolbar.setSubtitle(mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS));
        } else {
            menu_disconnect.setVisible(false);
        }
        return true;
    }

    /**
     * Called when application destroyed and release resource.
     */
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    /**
     * Menu item click event.
     *
     * @param item {@link MenuItem}
     * @return true/false
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Intent intent = new Intent(this, SplashActivity.class);
        switch (item.getItemId()) {
            case R.id.theme_blue_black:
                mySharedPreferences.putData(AppConstants.THEME, AppConstants.THEME_BLUE_BLACK);
                startActivity(intent);
                finish();
                break;
            case R.id.theme_gray_white:
                mySharedPreferences.putData(AppConstants.THEME, AppConstants.THEME_GRAY_WHITE);
                startActivity(intent);
                finish();
                break;
            case R.id.menu_disconnect:
                AppLogger.d(TAG + " Menu Disconnect:");
                disconnectService();
                break;
        }
        return true;
    }

    private void disconnectService() {
        menu_disconnect.setVisible(false);
        activityDashboardBinding.toolbar.setSubtitle(null);
        // Disconnect service from foreground
        Intent disableIntent = new Intent(this, BleScanningService.class);
        disableIntent.setAction(ACTION_DISABLE_SERVICE);
        startService(disableIntent);
    }

    /**
     * Check service disconnection and update toolBar
     *
     * @param bleServiceDisableEvent boolean flag that event handle condition of service disconnection
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void bleServiceDisableEvent(BleServiceDisableEvent bleServiceDisableEvent) {
        if (bleServiceDisableEvent.isServiceDisable()) {
            if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
                activityDashboardBinding.toolbar.setSubtitle(mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS));
            } else {
                activityDashboardBinding.toolbar.setSubtitle(null);
                if (menu_disconnect != null) {
                    menu_disconnect.setVisible(false);
                }
            }
        }

    }

    /**
     * Display dialog of device list which bind with user.
     *
     * @param userInformation User Information
     */
    private void showMacAddressesDialog(UserInformation userInformation) {
//        List<DeviceInfo> deviceInfoList = userInformation.getDevices();
//        AlertDialog.Builder alertDialog = new AlertDialog.Builder(DashboardActivity.this);
//        alertDialog.setCancelable(true);
//        CustomUserMacDialogBinding dialogBinding = DataBindingUtil.inflate(LayoutInflater.from(this), R.layout.custom_user_mac_dialog, null, false);
//        alertDialog.setView(dialogBinding.getRoot());
//        DialogListAdapter listAdapter = new DialogListAdapter(DashboardActivity.this, deviceInfoList);
//        RecyclerView ls = dialogBinding.getRoot().findViewById(R.id.dialog_list);
//        CircleImageView profileImage = dialogBinding.getRoot().findViewById(R.id.img_dialog_user);
//        profileImage.setImageDrawable(ContextCompat.getDrawable(DashboardActivity.this, R.drawable.ic_bluetooth_white_conneted));
//        TextView tvDialogUser = dialogBinding.getRoot().findViewById(R.id.tv_dialog_user_name);
//        tvDialogUser.setText(userInformation.getName());
//        Button btnCancel = dialogBinding.getRoot().findViewById(R.id.dialog_btn_cancel);
//        ls.setLayoutManager(new LinearLayoutManager(this));
////        activityDashboardBinding.dashboardRecyclerView.addItemDecoration(new DividerItemDecoration(this, 0));
////        adapter = new DashboardAdapter(this);
////        activityDashboardBinding.dashboardRecyclerView.setAdapter(adapter);
//        ls.setAdapter(listAdapter);
//
//        final AlertDialog alert = alertDialog.create();
//        alert.show();
//        btnCancel.setOnClickListener(view -> alert.dismiss());
    }


    /**
     * Check and validate the permission based on android version and start the preview.
     */
    public void checkPermission() {
        if (android.os.Build.VERSION.SDK_INT > 23) {
            if (ContextCompat.checkSelfPermission(this,
                    Manifest.permission.ACCESS_FINE_LOCATION)
                    != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.ACCESS_FINE_LOCATION},
                        102);
            } else {
                EventBus.getDefault().unregister(this);
                startActivity(new Intent(this, StartFGServiceActivity.class));

            }
        } else {
            EventBus.getDefault().unregister(this);
            startActivity(new Intent(this, StartFGServiceActivity.class));

        }
    }

    /**
     * Enable permission callback
     *
     * @param requestCode  intent code
     * @param permissions  enable permission list
     * @param grantResults integer type to check results.
     */
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case 102:
                if (grantResults.length <= 0
                        || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                    // permission was not granted
                    if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.ACCESS_FINE_LOCATION)) {
                        checkPermission();
                    } else {
                        showAlertForSettingScreen();
                    }
                } else {
                    EventBus.getDefault().unregister(this);
                    startActivity(new Intent(this, StartFGServiceActivity.class));
                }

                break;
        }
    }

    private void setUpNetworkError() {
        activityDashboardBinding.getUser().getInternetErrorMessage().observe(this, s -> showNoNetworkSnackbar());
    }


    /**
     * This alert shows when user deny the permission.
     */
    private void showAlertForSettingScreen() {
        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(DashboardActivity.this);
        builder.setTitle(R.string.title_location_permission);
        builder.setMessage(R.string.msg_location_permission);
        builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
            Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            Uri uri = Uri.fromParts("package", getPackageName(), null);
            intent.setData(uri);
            startActivityForResult(intent, 100);
        });
        builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> {

        });
        builder.setCancelable(false);
        android.app.AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * Display for adding new user and store user name into database.
     */
    private void showDialogForUserName() {
        LayoutInflater li = LayoutInflater.from(this);
        @SuppressLint("InflateParams") View dialogView = li.inflate(R.layout.username_dialog, null);
        android.app.AlertDialog.Builder alertDialogBuilder = new android.app.AlertDialog.Builder(this);
        alertDialogBuilder.setTitle(R.string.face_recognition_name);
        alertDialogBuilder.setMessage("\n" + getString(R.string.please_enter_unique_user_name_for_the_face_recognition));
        alertDialogBuilder.setIcon(R.drawable.ic_man_user_colored);
        alertDialogBuilder.setView(dialogView);
        final EditText userInput = dialogView.findViewById(R.id.et_input);
        alertDialogBuilder
                .setCancelable(false)
                .setPositiveButton(getString(R.string.ok),
                        (dialog, id) -> {
                            // Put here database name validation.
                            String username = userInput.getText().toString();
                            if (TextUtils.isEmpty(username)) {
                                showToast("Please enter name");
                            } else if (activityDashboardBinding.getUser().userExits(username) != null) {
                                showToast(username + " is already exits.");
                            } else {
                                UserInformation userInformation = new UserInformation();
                                userInformation.setName(username);
                                userInformation.setCookie(username + CommonUtils.getDeviceId(this));
                                if (userData != null) {
                                    userInformation.setTrainingData(userData);
                                } else {
                                    userInformation.setTrainingData("");
                                }
//                                userInformation.setDate(System.currentTimeMillis());
                                activityDashboardBinding.getUser().insert(userInformation);
                                getMyApplication().setUserInformation(userInformation);
                                checkPermission();
                            }
                        })
                .setNegativeButton(getString(R.string.cancel),
                        (dialog, id) -> dialog.cancel());
        // create alert dialog
        android.app.AlertDialog alertDialog = alertDialogBuilder.create();
        // show it
        alertDialog.show();
    }

    /**
     * Bind the RecyclerView with adapter.
     */
    private void bindRecyclerView() {
        activityDashboardBinding.dashboardRecyclerView.setLayoutManager(new LinearLayoutManager(this));
        activityDashboardBinding.dashboardRecyclerView.addItemDecoration(new DividerItemDecoration(this, 0));
        adapter = new DashboardAdapter(this);
        activityDashboardBinding.dashboardRecyclerView.setAdapter(adapter);
        activityDashboardBinding.fab.setOnClickListener(this);
        initSwipe();
    }

    private void initSwipe() {
        ItemTouchHelper.SimpleCallback simpleItemTouchCallback = new ItemTouchHelper.SimpleCallback(0, ItemTouchHelper.LEFT) {

            @Override
            public boolean onMove(@NonNull RecyclerView recyclerView, @NonNull RecyclerView.ViewHolder viewHolder, @NonNull RecyclerView.ViewHolder target) {
                return false;
            }

            @Override
            public void onSwiped(RecyclerView.ViewHolder viewHolder, int direction) {
                int position = viewHolder.getAdapterPosition();

                if (direction == ItemTouchHelper.LEFT) {
                    UserInformation userInformation = Objects.requireNonNull(activityDashboardBinding.getUser().getUserList().getValue()).get(position);
                    activityDashboardBinding.getUser().deleteUser(userInformation);
                    disconnectService();
                    BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                    if (mBluetoothAdapter != null) {
//                        if (userInformation.getDevices() != null && userInformation.getDevices().size() > 0) {
//                            BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(userInformation.getDevices().get(0).getMac_address());
//                            if (device != null) removePairing(device);
//                        }
                    }
                }
            }

            @Override
            public void onChildDraw(@NonNull Canvas c, @NonNull RecyclerView recyclerView, @NonNull RecyclerView.ViewHolder viewHolder, float dX, float dY, int actionState, boolean isCurrentlyActive) {

                Bitmap icon;
                if (actionState == ItemTouchHelper.ACTION_STATE_SWIPE) {

                    View itemView = viewHolder.itemView;
                    float height = (float) itemView.getBottom() - (float) itemView.getTop();
                    float width = height / 3;

                    //noinspection StatementWithEmptyBody
                    if (dX > 0) {
//                        p.setColor(Color.parseColor("#388E3C"));
//                        RectF background = new RectF((float) itemView.getLeft(), (float) itemView.getTop(), dX, (float) itemView.getBottom());
//                        c.drawRect(background, p);
//                        icon = BitmapFactory.decodeResource(getResources(), R.drawable.ic_delete_white_24);
//                        RectF icon_dest = new RectF((float) itemView.getLeft() + width, (float) itemView.getTop() + width, (float) itemView.getLeft() + 2 * width, (float) itemView.getBottom() - width);
//                        c.drawBitmap(icon, null, icon_dest, p);
                    } else {
                        p.setColor(Color.parseColor("#C11913"));
                        RectF background = new RectF((float) itemView.getRight() + dX, (float) itemView.getTop(), (float) itemView.getRight(), (float) itemView.getBottom());
                        c.drawRect(background, p);

                        icon = ScreenUtils.getBitmap(DashboardActivity.this, R.drawable.ic_delete_white_24);
                        RectF icon_dest = new RectF((float) itemView.getRight() - 2 * width, (float) itemView.getTop() + width, (float) itemView.getRight() - width, (float) itemView.getBottom() - width);
                        c.drawBitmap(icon, null, icon_dest, p);
                    }
                }
                super.onChildDraw(c, recyclerView, viewHolder, dX, dY, actionState, isCurrentlyActive);
            }
        };
        ItemTouchHelper itemTouchHelper = new ItemTouchHelper(simpleItemTouchCallback);
        itemTouchHelper.attachToRecyclerView(activityDashboardBinding.dashboardRecyclerView);
    }

    /**
     * Show indeterminate progress dialog.
     *
     * @param msg Message.
     */
    protected void showProgressDialog(String msg) {
        if (progressDialogIndeterminate == null) {
            progressDialogIndeterminate = new ProgressDialog(this);
            progressDialogIndeterminate.setMessage(msg);
            progressDialogIndeterminate.setCancelable(false);
            if (!isFinishing())
                progressDialogIndeterminate.show();
        } else {
            if (progressDialogIndeterminate.isShowing()) {
                progressDialogIndeterminate.setMessage(msg);
            }
        }
    }

    /**
     * Create sending data object and send to {@link BleScanningService} class
     */
    private void sendData(String requestType) {
//        DeviceInfo deviceInfo = getMyApplication().getUserInformation().getDevices().get(0);
//        if (requestType.equalsIgnoreCase(AppConstants.USER_INFORMATION)) {
//            if (deviceInfo != null) {
//                BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.USER_INFORMATION,
//                        deviceInfo.getDevice_name(),
//                        deviceInfo.getPass_code(),
//                        getMyApplication().getUserInformation().getCookie());
//                EventBus.getDefault().post(bleSendDataModel);
//            }
//        } else {
//            android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(DashboardActivity.this);
//            builder.setTitle(getMyApplication().getUserInformation().getName() + "'s" + "Trained Data");
//            builder.setMessage("Do you want send training data?");
//            builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
//                BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.TRAINING_INFORMATION,
//                        deviceInfo.getDevice_name(),
//                        deviceInfo.getPass_code(),
//                        getMyApplication().getUserInformation().getCookie(),
//                        getMyApplication().getUserInformation().getTrainingData().getBytes().length,
//                        getMyApplication().getUserInformation().getTrainingData());
//                EventBus.getDefault().post(bleSendDataModel);
//            });
//            builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> dialogInterface.dismiss());
//            builder.setCancelable(false);
//            android.app.AlertDialog dialog = builder.create();
//            dialog.show();
//        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void bleNewDeviceEvent(BelStartTrainingDataSendEvent belStartTrainingDataSendEvent) {
        AppLogger.d(TAG + "BelStartTrainingDataSendEvent");
        if (belStartTrainingDataSendEvent.isSendDataEnable()) {
            if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
                progressDialogIndeterminate.dismiss();
                progressDialogIndeterminate = null;
            }
            sendData(AppConstants.TRAINING_INFORMATION);
        }
    }

    /**
     * Return the sending progress of ble data.
     *
     * @param bleSendingProgress {@link BleSendingProgress}
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void bleSendingProgress(BleSendingProgress bleSendingProgress) {
//        AppLogger.d("FaceRec:BleSendingProgress: " + bleSendingProgress.getMaxProgress() + " " + bleSendingProgress.getProgress());
        if (determinateProgressDialog == null) {
            determinateProgressDialog = new ProgressDialog(this);
            determinateProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            determinateProgressDialog.setIndeterminate(false);
            determinateProgressDialog.setMax(bleSendingProgress.getMaxProgress() - 1);
            determinateProgressDialog.setMessage(bleSendingProgress.getDialogMessage());
            determinateProgressDialog.setCancelable(false);
            if (!isFinishing())
                determinateProgressDialog.show();
        } else {
            determinateProgressDialog.setProgress(bleSendingProgress.getProgress());
            if (bleSendingProgress.getProgress() == bleSendingProgress.getMaxProgress() - 1) {
                determinateProgressDialog.dismiss();
                determinateProgressDialog = null;
            }
        }
    }


    /**
     * Called when complete stream of snapshot data is available and this method will show dialog having the imageview
     *
     * @param bleSnapshotEvent event which will be posted
     */
    @SuppressWarnings("CatchMayIgnoreException")
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void snapshotDataReceived(BleSnapshotEvent bleSnapshotEvent) {
        AlertDialog.Builder alertadd = new AlertDialog.Builder(DashboardActivity.this);
        LayoutInflater factory = LayoutInflater.from(DashboardActivity.this);
        @SuppressLint("InflateParams") final View view = factory.inflate(R.layout.layout_snap_received, null);
        ImageView imageView = view.findViewById(R.id.dialog_imageview);
        alertadd.setTitle("Received Snapshot");
        alertadd.setNegativeButton("Cancel", (dialog, which) -> {

        });
        ArrayList<String> snapList = bleSnapshotEvent.getSnapShotList();
        StringBuilder listString = new StringBuilder();

        for (String s : snapList) {
            listString.append(s);
        }


        try {
            Bitmap bitmap = Bitmap.createBitmap(480, 272, Bitmap.Config.RGB_565);
            bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(listString.toString().getBytes()));
            imageView.setImageBitmap(bitmap);


        } catch (Exception e) {
            e.getMessage();
        }


        alertadd.setView(view);
        alertadd.setCancelable(false);
        alertadd.show();
    }

    /**
     * Receive event while ble status get changed and posting from {@link BleScanningService}
     *
     * @param bleStatusEvent provide ble status and device details
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void bleStatusEvent(BleStatusEvent bleStatusEvent) {
        AppLogger.d("FaceRec:BleStatusEvent: " + bleStatusEvent.getBluetoothDevice().getAddress() + " " + bleStatusEvent.getStatus());
        int connectionState = bleStatusEvent.getStatus();
        BluetoothDevice device = bleStatusEvent.getBluetoothDevice();
        if (connectionState == BleService.STATE_LINK_LOSS
                || connectionState == BleService.STATE_DISCONNECTED
                || connectionState == BleService.STATE_DISCONNECTING) {
            if (determinateProgressDialog != null && determinateProgressDialog.isShowing()) {
                determinateProgressDialog.dismiss();
                determinateProgressDialog = null;
            }

            if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
                progressDialogIndeterminate.dismiss();
                progressDialogIndeterminate = null;
            }

            activityDashboardBinding.toolbar.setSubtitle("");
            AppLogger.d(TAG + "Link Loss disconnectService");
            disconnectService();
        } else if (connectionState == BleService.STATE_CONNECTING) {
            if (determinateProgressDialog != null && determinateProgressDialog.isShowing()) {
                determinateProgressDialog.dismiss();
                determinateProgressDialog = null;
            }
            showProgressDialog(getString(R.string.connecting));
        } else if (connectionState == BleService.STATE_CONNECTED) {
            activityDashboardBinding.toolbar.setSubtitle(device.getAddress());
        } else if (connectionState == BleService.STATE_DEVICE_READY) {
            menu_disconnect.setVisible(true);
            if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
                progressDialogIndeterminate.dismiss();
                progressDialogIndeterminate = null;
            }
            if (activityDashboardBinding != null)
                activityDashboardBinding.toolbar.setSubtitle(bleStatusEvent.getBluetoothDevice().getAddress());
            sendData(AppConstants.USER_INFORMATION);

        }
    }

}
