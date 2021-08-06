package com.nxp.facemanager.activity;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.graphics.Canvas;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.nxp.facemanager.R;
import com.nxp.facemanager.ble.BleModel;
import com.nxp.facemanager.ble.BleScanningService;
import com.nxp.facemanager.ble.BleSendDataModel;
import com.nxp.facemanager.ble.BleService;
import com.nxp.facemanager.ble.LeDeviceListAdapter;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.databinding.ActivityStartFgserviceBinding;
import com.nxp.facemanager.model.AddDeviceResponseModel;
import com.nxp.facemanager.model.BelStartTrainingDataSendEvent;
import com.nxp.facemanager.model.BleDataSentEvent;
import com.nxp.facemanager.model.BleEnableEvent;
import com.nxp.facemanager.model.BleNewDeviceEvent;
import com.nxp.facemanager.model.BleSendingProgress;
import com.nxp.facemanager.model.BleServiceDisableEvent;
import com.nxp.facemanager.model.BleStatusEvent;
import com.nxp.facemanager.model.PairingEventModel;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.viewModels.DeviceScanModule;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

import javax.inject.Inject;

import androidx.annotation.NonNull;
import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.ViewModelProviders;
import androidx.recyclerview.widget.RecyclerView;

import static com.nxp.facemanager.utility.AppConstants.ACTION_DISABLE_SERVICE;
import static com.nxp.facemanager.utility.AppConstants.LOCATION_PERMISSION_REQUEST;

/**
 * Class with BLE scanning results and connect functionality
 * 1. Has method for pairing with device
 * 2. Event handling for new BLE device added (bleNewDeviceEvent)
 * 3. Receive ble status event
 * 4. Checks for bleService event so as to update title if service is disconnected
 * 5. Receives pairing event for autoconnect in case if device is bonded
 */

public class StartFGServiceActivity extends BaseActivity implements OnItemClickListener {

    /**
     * Tag for log.
     */
    private static final String TAG = StartFGServiceActivity.class.getSimpleName();
    /**
     * Inject the face database
     */
    @Inject
    FaceDatabase faceDatabase;
    /**
     * Instance of menu scan and menu refresh.
     */
    MenuItem menu_scan, menu_refresh;
    /**
     * Binding object for the view.
     */
    private ActivityStartFgserviceBinding activityStartFgserviceBinding;
    /**
     * Adapter to bind the ble device.
     */
    private LeDeviceListAdapter mLeDeviceListAdapter;
    /**
     * List of paired devices.
     */
    private Set<BluetoothDevice> pairedDevices = new HashSet<>();

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

    RecyclerView mRecycleView;
    RecyclerView.Adapter mAdapter;
    Set<String> tempSet;
    ArrayList<DeviceInfo> tempInfoList;

    /**
     * First method when class called and initialize all object here and binding .
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        AppLogger.d(TAG + "   onCreate" + null);
        startAutoConnectService(null);
        DeviceScanModule deviceScanModule = ViewModelProviders.of(this, new ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(DeviceScanModule.class);
        activityStartFgserviceBinding = DataBindingUtil.setContentView(StartFGServiceActivity.this, R.layout.activity_start_fgservice);
        activityStartFgserviceBinding.setVm(deviceScanModule);
        AppLogger.d("Test", activityStartFgserviceBinding.getVm().getDeviceInformation());
        activityStartFgserviceBinding.getVm().setActivityContext(this);
        setUpNetworkError();

    }

    /**
     * Check the device is Paired or not.
     *
     * @param bleModel {@link BleModel}
     * @return true/false
     */
    private boolean checkIsDevicePaired(String bleModel) {
        pairedDevices = mBluetoothAdapter.getBondedDevices();
        // If there are paired devices
        if (pairedDevices != null && pairedDevices.size() > 0) {
            // Loop through paired devices
            for (BluetoothDevice device : pairedDevices) {
                // Add the name and address to an array adapter to show in a ListView
                if (bleModel.equalsIgnoreCase(device.getAddress()))
                    return true;
            }
        }
        return false;
    }


    /**
     * Initialize the toolbar
     */
    private void setToolbar() {
        activityStartFgserviceBinding.toolbar.setTitle(getString(R.string.smart_scanning));
        setSupportActionBar(activityStartFgserviceBinding.toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
            activityStartFgserviceBinding.toolbar.setSubtitle(mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS));
        } else {
            activityStartFgserviceBinding.toolbar.setSubtitle(null);
        }
        activityStartFgserviceBinding.toolbar.setNavigationOnClickListener(view -> finish());
    }

    /**
     * Bind menu item here
     *
     * @param menu ble_menu
     * @return true/false
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.ble_menu, menu);
        menu_scan = menu.findItem(R.id.menu_scan);
        menu_refresh = menu.findItem(R.id.menu_refresh);
        menu_refresh.setActionView(
                R.layout.actionbar_indeterminate_progress);
        if (mBluetoothAdapter != null && mBluetoothAdapter.isEnabled()) {
            menu.findItem(R.id.menu_refresh).setActionView(
                    R.layout.actionbar_indeterminate_progress);
        }
        return true;
    }

    /**
     * Menu item click events
     *
     * @param item {@link MenuItem}
     * @return true/false
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_scan:
                mLeDeviceListAdapter.clear();
                menu_scan.setVisible(false);
                menu_refresh.setVisible(true);
                AppLogger.d(TAG + "  menu_scan" + null);
                startAutoConnectService(null);
//                scanLeDevice(true);
                break;
            case R.id.menu_stop:
                AppLogger.e(TAG+" menu_stop");
//                scanLeDevice(false);
                break;
        }
        return true;
    }

    /**
     * This method called when application come back to foreground again.
     */
    @Override
    protected void onResume() {
        super.onResume();
        setToolbar();
        setUpBle();
        // Initializes list view adapter.
        mLeDeviceListAdapter = new LeDeviceListAdapter(this);
        activityStartFgserviceBinding.recyclerView.setAdapter(mLeDeviceListAdapter);
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        AppLogger.e(TAG+" on destroy");
        // stop ble scanning
//        Intent intent = new Intent(this, BleScanningService.class);
//        intent.setAction(ACTION_DISABLE_SERVICE);
//        startService(intent);
    }

    /**
     * When view goes in background.
     */
    @Override
    protected void onPause() {
        super.onPause();
//        scanLeDevice(false);
        if (mLeDeviceListAdapter != null) {
            mLeDeviceListAdapter.clear();
        }
        if (determinateProgressDialog != null && determinateProgressDialog.isShowing()) {
            determinateProgressDialog.dismiss();
            determinateProgressDialog = null;
        }
        if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
            progressDialogIndeterminate.dismiss();
            progressDialogIndeterminate = null;
        }
    }

    /**
     * Item click listener of Recycler view
     *
     * @param item   object of item type
     * @param viewId view id.
     */
    @Override
    public void onItemClick(Object item, View viewId) {
        try {
            if (item == null) return;
            BleModel scanResult = (BleModel) item;
            AppLogger.d("FaceRec:Selected Device" + scanResult.getMacAddress() + "  " + scanResult.getDevicename());
            pairDevice(scanResult.getBluetoothDevice());
        } catch (Exception e) {
            AppLogger.e(e.toString());
        }
    }

    /**
     * Pairing the ble device to show the dialog for pairing.
     *
     * @param device bluetooth device for pairing
     */
    private void pairDevice(BluetoothDevice device) {
        try {
            showProgressDialog(getString(R.string.connecting));
            activityStartFgserviceBinding.toolbar.setSubtitle(null);
            startAutoConnectService(device);
            if (checkIsDevicePaired(device.getAddress())) {
                showProgressDialog(getString(R.string.connecting));
                activityStartFgserviceBinding.toolbar.setSubtitle(null);
                startAutoConnectService(device);
            } else {
                mBluetoothAdapter.startDiscovery();
                showProgressDialog(getString(R.string.pairing_request));
                //Give it some time before cancelling the discovery
                new Handler().postDelayed(() -> {
                    // Do delayed stuff!
                    mBluetoothAdapter.cancelDiscovery();
                    device.createBond();
                    if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
                        progressDialogIndeterminate.dismiss();
                        progressDialogIndeterminate = null;
                    }
                }, 1000);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    /**
     * Show indeterminate progress dialog.
     *
     * @param msg Message.
     */
    protected void showProgressDialog(String msg) {
        if (progressDialogIndeterminate == null) {
            progressDialogIndeterminate = new ProgressDialog(this, R.style.AlertDialogTheme);
            progressDialogIndeterminate.setMessage(msg);
            if (!isFinishing())
                progressDialogIndeterminate.show();
        } else {
            if (progressDialogIndeterminate.isShowing()) {
                progressDialogIndeterminate.setMessage(msg);
            }
        }
    }

    /**
     * Event for adding new device in adapter after received from scanning class in {@link BleScanningService}
     *
     * @param bleModel provide details of device using BleModel Class
     */
    @Subscribe
    public void bleNewDeviceEvent(BleNewDeviceEvent bleModel) {
        //  AppLogger.d("onMessageEvent: " + bleModel.getBleModel().getDevice_name() + " " + bleModel.getBleModel().getMacAddress());
        mLeDeviceListAdapter.addDevice(bleModel.getBleModel());
        mLeDeviceListAdapter.notifyDataSetChanged();
        if (bleModel.getDeviceInformation() != null) {
            activityStartFgserviceBinding.getVm().setDeviceInformation(bleModel.getDeviceInformation());
        }
    }


    @Subscribe
    public void belStartTrainingDataSendEvent(BelStartTrainingDataSendEvent belStartTrainingDataSendEvent) {
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

    private void setUpNetworkError() {
        activityStartFgserviceBinding.getVm().getInternetErrorMessage().observe(this, s -> showNoNetworkSnackbar());
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
            activityStartFgserviceBinding.toolbar.setSubtitle("");
            AppLogger.d(TAG + "Link Loss disconnectService");
            disconnectService();
            mLeDeviceListAdapter.clear();
            menu_refresh.setVisible(false);
            menu_scan.setVisible(true);
        } else if (connectionState == BleService.STATE_CONNECTING) {
            if (determinateProgressDialog != null && determinateProgressDialog.isShowing()) {
                determinateProgressDialog.dismiss();
                determinateProgressDialog = null;
            }
            showProgressDialog(getString(R.string.connecting));
        } else if (connectionState == BleService.STATE_CONNECTED) {
            activityStartFgserviceBinding.toolbar.setSubtitle(device.getAddress());
        } else if (connectionState == BleService.STATE_DEVICE_READY) {
            activityStartFgserviceBinding.getVm().getDbOperation().updateConnected(device.getAddress());
            //To notify adapter
            if (mLeDeviceListAdapter != null) {
                BluetoothDevice bluetoothDevice = bleStatusEvent.getBluetoothDevice();
                boolean isPaired = mBluetoothAdapter.getBondedDevices().contains(bluetoothDevice);
                DeviceInfo deviceInformation = activityStartFgserviceBinding.getVm().getDbOperation().checkDeviceExitsInDb(bluetoothDevice.getAddress());
                BleModel bleModel = new BleModel(bluetoothDevice.getAddress(),
                        (deviceInformation != null && deviceInformation.getDevice_name() != null) ? deviceInformation.getDevice_name() : bluetoothDevice.getName(),
                        isPaired, 0, bluetoothDevice, "");
                mLeDeviceListAdapter.removeDevice(bleModel);
            }

            if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
                progressDialogIndeterminate.dismiss();
                progressDialogIndeterminate = null;
            }
            if (activityStartFgserviceBinding != null)
                activityStartFgserviceBinding.toolbar.setSubtitle(bleStatusEvent.getBluetoothDevice().getAddress());

            if (activityStartFgserviceBinding != null && activityStartFgserviceBinding.getVm() != null
                    && activityStartFgserviceBinding.getVm().getDeviceInformation() == null) {
                if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
                    progressDialogIndeterminate.dismiss();
                    progressDialogIndeterminate = null;
                }
                showDialogForDeviceNameAndPassCode(bleStatusEvent.getBluetoothDevice());
            } else {
                if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
                    progressDialogIndeterminate.dismiss();
                    progressDialogIndeterminate = null;
                }
                sendData(AppConstants.USER_INFORMATION);
            }
        }
    }

    /**
     * Receive event after sending data to Ble device to store into Database and post event from {@link BleScanningService}
     *
     * @param bleDataSentEvent get device status
     */
    @Subscribe
    void bleDataSentEvent(BleDataSentEvent bleDataSentEvent) {
//        if (bleDataSentEvent.isDataSent()) {
//            if (getMyApplication().getUserInformation().getDevices() == null) {
//                List<DeviceInfo> deviceInfoList = new ArrayList<>();
//                deviceInfoList.add(activityStartFgserviceBinding.getVm().getDeviceInformation());
//                getMyApplication().getUserInformation().setDevices(deviceInfoList);
//                activityStartFgserviceBinding.getVm().insertItem(getMyApplication().getUserInformation());
//            } else {
//                if (activityStartFgserviceBinding.getVm().isDeviceExitsInUserInformation(activityStartFgserviceBinding.getVm().getDeviceInformation().getMac_address())) {
//                    activityStartFgserviceBinding.getVm().insertItem(getMyApplication().getUserInformation());
//                } else {
//                    getMyApplication().getUserInformation().getDevices().add(activityStartFgserviceBinding.getVm().getDeviceInformation());
//                }
//            }
//        }
    }


    /**
     * Eventbus for ble enable event.
     *
     * @param bleEnableEvent boolean flag that event handle condition of ble enabled.
     */
    @Subscribe
    public void bleEnableEvent(BleEnableEvent bleEnableEvent) {
        if (bleEnableEvent != null && bleEnableEvent.isBleOn()) {

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
            mySharedPreferences.putData(AppConstants.CONNECTED_MAC_ADDRESS, "");
            activityStartFgserviceBinding.toolbar.setSubtitle(null);
            if (determinateProgressDialog != null && determinateProgressDialog.isShowing()) {
                determinateProgressDialog.dismiss();
                determinateProgressDialog = null;
            }
            if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
                progressDialogIndeterminate.dismiss();
                progressDialogIndeterminate = null;
            }
        }
    }

    /**
     * Create sending data object and send to {@link BleScanningService} class
     */
    public void sendData(String requestType) {
        if (requestType.equalsIgnoreCase(AppConstants.USER_INFORMATION)) {
            if (activityStartFgserviceBinding.getVm().getDeviceInformation() != null) {
//                BleSendDataModel bleSendDataModel = new BleSendDataModel(
//                        BleRemoteCommand.READY_INDICATION,
//                        activityStartFgserviceBinding.getVm().getDeviceInformation().getDevice_name() + activityStartFgserviceBinding.getVm().getDeviceInformation().getPass_code()
//                );
//                bleSendDataModel.setCommand(AppConstants.USER_INFORMATION);
//                bleSendDataModel.setDeviceName(activityStartFgserviceBinding.getVm().getDeviceInformation().getDevice_name());
//                bleSendDataModel.setPass_code("" + activityStartFgserviceBinding.getVm().getDeviceInformation().getPass_code());
//                bleSendDataModel.setCookie("");
                activityStartFgserviceBinding.getVm().getDbOperation().insert(activityStartFgserviceBinding.getVm().getDeviceInformation());
//                EventBus.getDefault().post(bleSendDataModel);
            }
        }
//        else {
//            AlertDialog.Builder builder = new android.app.AlertDialog.Builder(StartFGServiceActivity.this);
//            builder.setTitle(activityStartFgserviceBinding.getVm().getUserInformation().getName() + "'s" + "Trained Data");
//            builder.setMessage("Do you want send training data?");
//            builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
//                BleSendDataModel bleSendDataModel = new BleSendDataModel(AppConstants.TRAINING_INFORMATION,
//                        activityStartFgserviceBinding.getVm().getDeviceInformation().getDevice_name(),
//                        activityStartFgserviceBinding.getVm().getDeviceInformation().getPass_code(),
//                        activityStartFgserviceBinding.getVm().getUserInformation().getCookie(),
//                        activityStartFgserviceBinding.getVm().getUserInformation().getTrainingData().getBytes().length,
//                        activityStartFgserviceBinding.getVm().getUserInformation().getTrainingData());
//                EventBus.getDefault().post(bleSendDataModel);
//            });
//            builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> dialogInterface.dismiss());
//            builder.setCancelable(false);
//            android.app.AlertDialog dialog = builder.create();
//            dialog.show();
//        }
    }

    /**
     * This alert shows when user deny the permission.
     */
    private void showAlertForSettingScreen() {
        AlertDialog.Builder builder = new AlertDialog.Builder(StartFGServiceActivity.this, R.style.AlertDialogTheme);
        builder.setTitle(R.string.title_location_permission);
        builder.setMessage(R.string.msg_location_permission);
        builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
            Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            Uri uri = Uri.fromParts("package", getPackageName(), null);
            intent.setData(uri);
            startActivityForResult(intent, LOCATION_PERMISSION_REQUEST);
        });
        builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> {
            dialogInterface.dismiss();
            showAlertForSettingScreen();
        });
        builder.setCancelable(false);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * Represent Dialog to provide device name and passcode.
     *
     * @param device is bluetooth device details.
     */
    private void showDialogForDeviceNameAndPassCode(BluetoothDevice device) {
        @SuppressLint("InflateParams") View dialogView = LayoutInflater.from(this).inflate(R.layout.dialog_passcode, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this, R.style.AlertDialogTheme);
        alertDialogBuilder.setTitle(getString(R.string.smartlock_information));
        alertDialogBuilder.setIcon(R.drawable.ic_bluetooth_connected);
        alertDialogBuilder.setMessage(getString(R.string.msg_dialog_passcode) + " " + device.getAddress());
        alertDialogBuilder.setView(dialogView);
        final EditText edtDeviceName = dialogView
                .findViewById(R.id.edtDeviceName);
        final EditText edtPassCode = dialogView
                .findViewById(R.id.edtPassCode);

        // set dialog message
        alertDialogBuilder
                .setCancelable(false)
                .setPositiveButton(getString(R.string.ok), null)
                .setNegativeButton(getString(R.string.cancel), null);
        AlertDialog alertDialog = alertDialogBuilder.create();

        alertDialog.setOnShowListener(dialog -> {

            Button button = ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE);
            button.setOnClickListener(view -> {
                String device_name = edtDeviceName.getText().toString();
                String pass_code = edtPassCode.getText().toString();
                if (TextUtils.isEmpty(device_name)) {
                    showToast(getString(R.string.please_enter_device_name));
                } else if (TextUtils.isEmpty(pass_code)) {
                    showToast(getString(R.string.please_enter_passcode));
                } else {
                    DeviceInfo deviceInformation = new DeviceInfo();
                    deviceInformation.setDevice_name(device_name);
                    deviceInformation.setMac_address(device.getAddress());
                    deviceInformation.setPass_code(Integer.parseInt(pass_code));
                    Set<String> list = new HashSet<>();
                    list.add(mySharedPreferences.getStringData(AppConstants.LOGIN_USER_ID));
                    deviceInformation.setUserIdList(list);
                    activityStartFgserviceBinding.getVm().setDeviceInformation(deviceInformation);
                    sendData(AppConstants.USER_INFORMATION);
                    activityStartFgserviceBinding.getVm().performAddDevice(new AddDeviceListener() {
                        @Override
                        public void callFinishMethod() {
                            dialog.dismiss();
                            finish();
                        }
                    });

                }
            });
            Button btnNegative = ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_NEGATIVE);
            btnNegative.setOnClickListener(view -> {
                AppLogger.d(TAG + " Fill data disconnectService");
                disconnectService();
                dialog.cancel();

            });
        });
        alertDialog.show();
    }


    private void disconnectService() {
        activityStartFgserviceBinding.toolbar.setSubtitle(null);
        Intent disableIntent = new Intent(this, BleScanningService.class);
        disableIntent.setAction(ACTION_DISABLE_SERVICE);
        startService(disableIntent);
    }


    /**
     * Event bus for pairing request.
     *
     * @param pairingEventModel {@link PairingEventModel}
     */
    @Subscribe
    public void pairingEventModel(PairingEventModel pairingEventModel) {
        if (pairingEventModel != null) {
            if (pairingEventModel.getBluetoothDevice().getBondState() == BluetoothDevice.BOND_BONDED) {
                AppLogger.d("FaceRec:PairingRequest BOND_BONDED.");
                showProgressDialog("Connecting...");
                startAutoConnectService(pairingEventModel.getBluetoothDevice());
            } else if (pairingEventModel.getBluetoothDevice().getBondState() == BluetoothDevice.BOND_NONE) {
                if (progressDialogIndeterminate != null && progressDialogIndeterminate.isShowing()) {
                    progressDialogIndeterminate.dismiss();
                    progressDialogIndeterminate = null;
                }
                Toast.makeText(StartFGServiceActivity.this, "Please reconnect..", Toast.LENGTH_SHORT).show();
                AppLogger.d("FaceRec:PairingRequest BOND_NONE.");
            }
        }
    }

}

