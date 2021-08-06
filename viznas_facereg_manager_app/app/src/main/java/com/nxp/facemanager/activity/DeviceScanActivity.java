package com.nxp.facemanager.activity;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.ParcelUuid;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;

import com.nxp.facemanager.R;
import com.nxp.facemanager.ble.BleModel;
import com.nxp.facemanager.ble.LeDeviceListAdapter;
import com.nxp.facemanager.ble.UARTManager;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.databinding.ActivityDeviceScanBinding;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.viewModels.DeviceScanModule;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.ViewModelProviders;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;

/**
 * Device Scanning view.
 */
public class DeviceScanActivity extends BaseActivity implements OnItemClickListener {
    private static final String TAG = DeviceScanActivity.class.getSimpleName();

    /**
     * Reference of enable ble request.
     */
    private static final int REQUEST_ENABLE_BT = 1;
    /**
     * Ble scanning time.
     */
    private static final long SCAN_PERIOD = 10000;
    /**
     * Data binding object reference of current activity which bind with {@link xml }
     */
    private ActivityDeviceScanBinding activityDeviceScanBinding;
    /**
     * Ble Scanned class object.
     */
    private Object mLeScanner = null;
    /**
     * Ble device binding adapter class reference.
     */
    private LeDeviceListAdapter mLeDeviceListAdapter;
    /**
     * Default class of ble which provides the ble object list adapter.
     */
    private BluetoothAdapter mBluetoothAdapter;
    /**
     * User to post the delay in scanning.
     */
    private Handler mHandler;
    /**
     * Object of list of paired devices.
     */
    private Set<BluetoothDevice> pairedDevices = new HashSet<>();
    /**
     * Broadcast receiver for paring request.
     */
    private PairingRequestBroadcast pairingRequestBroadcast;
    /**
     * Model class for storing ble meta data.
     * eg. Device Name, Mac Address...
     */
    private BleModel scanResult; 
    /**
     * Ble scanning call back which return the BluetoothDevice reference which are range of the device.
     * This method all check the current device is already exits in database and paired or not.
     * If device exits and already paired the try to Auto Connect with device.
     */
    public ScanCallback mScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            if (null != result) {
                BluetoothDevice device = result.getDevice();
                boolean isPaired = mBluetoothAdapter.getBondedDevices().contains(device);
                DeviceInfo deviceInformation = checkDeviceExitsInDb(device.getAddress());
                BleModel bleModel = new BleModel(device.getAddress(), (deviceInformation != null && deviceInformation.getDevice_name() != null) ? deviceInformation.getDevice_name() : device.getName(), isPaired, result.getRssi(), device, "");
                mLeDeviceListAdapter.addDevice(bleModel);
                mLeDeviceListAdapter.notifyDataSetChanged();

                if (deviceInformation != null) {
                    activityDeviceScanBinding.getVm().setDeviceInformation(deviceInformation);
                    if (isPaired) {
                        if (mServiceBinder != null && !mServiceBinder.isConnected() && device.getBondState() == BluetoothDevice.BOND_BONDED) {
                            scanResult = bleModel;
//                             mServiceBinder.connect(scanResult.getMacAddress(), scanResult.getDevice_name());
                        }
                    }
                }
                if (mLeDeviceListAdapter != null && mServiceBinder != null)
                    mLeDeviceListAdapter.setmServiceBinder(mServiceBinder);
            } else {
                AppLogger.e(TAG, "onScanResult() : Cannot get ScanResult !!!");
            }
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            super.onBatchScanResults(results);
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
        }
    };

    /**
     * Check the selected mac address is already exits in database.
     *
     * @param macAdd selected mac address.
     * @return device information stored in database.
     */
    private DeviceInfo checkDeviceExitsInDb(String macAdd) {

        if (activityDeviceScanBinding.getVm() != null
                && activityDeviceScanBinding.getVm().getDeviceInfoList() != null) {
            for (DeviceInfo deviceInformation : activityDeviceScanBinding.getVm().getDeviceInfoList()) {
                if (deviceInformation.getMac_address().equalsIgnoreCase(macAdd)) {
                    return deviceInformation;
                }
            }
        }
        return null;
    }

    /**
     * Initialize of activity context.
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        DeviceScanModule deviceScanModule = ViewModelProviders.of(this, new ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(DeviceScanModule.class);
        activityDeviceScanBinding = DataBindingUtil.setContentView(this, R.layout.activity_device_scan);
        activityDeviceScanBinding.setVm(deviceScanModule);
        activityDeviceScanBinding.setLifecycleOwner(this);
        activityDeviceScanBinding.getVm().getUserList().observe(this, userInformations -> {
            // Update the cached copy of the words in the adapter.
            if (userInformations != null && userInformations.size() > 0) {
                for (UserInformation userInformation : userInformations) {
                    if (userInformation.getName().equalsIgnoreCase(getMyApplication().getUserInformation().getName())) {
                        // activityDeviceScanBinding.getVm().setDeviceInfoList(userInformation.getDevices());
                    }
                }
            }
        });
        setToolbar();
        setUpBle();
        setUpNetworkError();


    }

    /**
     * Check if the current device is already paired not from ble adapter class.
     *
     * @param bleModel String
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
     * Called when application destroyed and release resource.
     */
    @Override
    protected void onDestroy() {
        unregisterReceiver(pairingRequestBroadcast);
        super.onDestroy();
    }

    /**
     * Initialize the toolbar
     */
    private void setToolbar() {
        setSupportActionBar(activityDeviceScanBinding.toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(false);
        getSupportActionBar().setDisplayShowHomeEnabled(true);
        activityDeviceScanBinding.toolbar.setNavigationOnClickListener(view -> finish());
    }

    /**
     * Bind menu to the view.
     *
     * @param menu {@link Menu}
     * @return true/false
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.ble_menu, menu);
        if (!activityDeviceScanBinding.getVm().getmScanning().get()) {
            menu.findItem(R.id.menu_stop).setVisible(false);
            menu.findItem(R.id.menu_scan).setVisible(true);
            menu.findItem(R.id.menu_refresh).setActionView(null);

        } else {
            menu.findItem(R.id.menu_stop).setVisible(true);
            menu.findItem(R.id.menu_scan).setVisible(false);
            menu.findItem(R.id.menu_refresh).setActionView(
                    R.layout.actionbar_indeterminate_progress);
        }
        return true;
    }

    /**
     * Menu item click event.
     *
     * @param item {@link MenuItem}
     * @return true/false
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_scan:
                mLeDeviceListAdapter.clear();
                scanLeDevice(true);
                break;
            case R.id.menu_stop:
                scanLeDevice(false);
                break;
        }
        return true;
    }

    /**
     * Called whenever the application comes to foreground.
     */
    @Override
    protected void onResume() {
        super.onResume();

        // Ensures Bluetooth is enabled on the device.  If Bluetooth is not currently enabled,
        // fire an intent to display a dialog asking the user to grant permission to enable it.
        if (!mBluetoothAdapter.isEnabled()) {
            if (!mBluetoothAdapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }
        }

        // Initializes list view adapter.
        activityDeviceScanBinding.recyclerView.setLayoutManager(new LinearLayoutManager(this));
        activityDeviceScanBinding.recyclerView.addItemDecoration(new DividerItemDecoration(this, 0));
        mLeDeviceListAdapter = new LeDeviceListAdapter(this);
        activityDeviceScanBinding.recyclerView.setAdapter(mLeDeviceListAdapter);
        scanLeDevice(true);

    }


    /**
     * Screen not visible to user.
     */
    @Override
    protected void onPause() {
        super.onPause();
        scanLeDevice(false);
        mLeDeviceListAdapter.clear();
    }

    /**
     * Scanning the device with specific UUID.
     *
     * @param enable boolean for to enable/disable scan.
     */
    private void scanLeDevice(final boolean enable) {
        if (enable) {
            // Stops scanning after a pre-defined scan period.
            mHandler.postDelayed(() -> {
                activityDeviceScanBinding.getVm().getmScanning().set(false);
                if (null != mLeScanner) {
                    ((BluetoothLeScanner) mLeScanner).stopScan(mScanCallback);
                }
                invalidateOptionsMenu();
            }, SCAN_PERIOD);

            activityDeviceScanBinding.getVm().getmScanning().set(true);
            mLeScanner = mBluetoothAdapter.getBluetoothLeScanner();

            if (null != mLeScanner) {
                List<ScanFilter> scanFilters = new ArrayList<>();
                //default setting.
                final ScanSettings settings = new ScanSettings.Builder().build();
                ScanFilter scanFilter = new ScanFilter.Builder().setServiceUuid(new ParcelUuid(UARTManager.UART_SERVICE_UUID)).build();
                scanFilters.add(scanFilter);
                ((BluetoothLeScanner) mLeScanner).startScan(scanFilters, settings, mScanCallback);
            }
        } else {
            activityDeviceScanBinding.getVm().getmScanning().set(false);
            if (null != mLeScanner) {
                ((BluetoothLeScanner) mLeScanner).stopScan(mScanCallback);
            }
        }
        invalidateOptionsMenu();
    }

    /**
     * RecycleView item click listener.
     *
     * @param item   Object access by casting.
     * @param viewId resource id.
     */
    @Override
    public void onItemClick(Object item, View viewId) {
        if (item == null) return;
        scanResult = (BleModel) item;
        activityDeviceScanBinding.getVm().getmScanning().set(false);
        if (null != mLeScanner) {
            AppLogger.d("Device address On click    " + scanResult.getMacAddress() + "  " + scanResult.getDevicename());
            ((BluetoothLeScanner) mLeScanner).stopScan(mScanCallback);
            pairDevice(scanResult);
        }

    }

    /**
     * Pairing the ble device to show the dialog for pairing.
     *
     * @param device custom model class of adapter to store ble information.
     */
    private void pairDevice(BleModel device) {
        try {
            if (!checkIsDevicePaired(device.getBluetoothDevice().getAddress())) {
                mBluetoothAdapter.startDiscovery();
                ProgressDialog progressDialog = new ProgressDialog(this);
                progressDialog.setMessage("Pairing request...");
                progressDialog.show();
                //Give it some time before cancelling the discovery
                new Handler().postDelayed(() -> {
                    // Do delayed stuff!
                    mBluetoothAdapter.cancelDiscovery();
                    device.getBluetoothDevice().createBond();
                    progressDialog.dismiss();
                }, 500);
            }  //                navigateView();
            //      mServiceBinder.connect(scanResult.getMacAddress(), scanResult.getDevice_name());

        } catch (Exception e) {
            e.printStackTrace();
        }


    }

    /**
     * View navigation.
     */
    private void navigateView() {
//        Intent intent1 = new Intent(DeviceScanActivity.this, TrainingDataSendActivity.class);
//        intent1.putExtra(BleService.EXTRA_DEVICE_ADDRESS, "" + scanResult.getMacAddress());
//        intent1.putExtra(BleService.EXTRA_DEVICE_NAME, "" + scanResult.getDevice_name());
//        startActivity(intent1);
    }

    // send data to ble device
    private void sampleSend() {
        if (mServiceBinder != null) {
            mServiceBinder.sendBytes("0123456789\n".getBytes());
        }
    }

    /**
     * Display dialog for the ask for name and passcode.
     *
     * @param device {@link BluetoothDevice}
     */
    private void showDialogForDeviceNameAndPasscode(BluetoothDevice device) {
        LayoutInflater li = LayoutInflater.from(this);
        @SuppressLint("InflateParams")
        View dialogView = li.inflate(R.layout.dialog_passcode, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        alertDialogBuilder.setTitle(getString(R.string.smartlock_information));
        alertDialogBuilder.setIcon(R.drawable.ic_bluetooth_white_conneted);
        alertDialogBuilder.setMessage("\n" + "Your lock is already paired with " + device.getAddress());
        alertDialogBuilder.setView(dialogView);
        final EditText edtDeviceName = dialogView
                .findViewById(R.id.edtDeviceName);
        final EditText edtPassCode = dialogView
                .findViewById(R.id.edtPassCode);

        // set dialog message
        alertDialogBuilder
                .setCancelable(false)
                .setPositiveButton(getString(R.string.ok),
                        (dialog, id) -> {
                            // Put here database name validation.
                            String devicename = edtDeviceName.getText().toString();
                            String passcode = edtPassCode.getText().toString();
                            if (TextUtils.isEmpty(devicename)) {
                                showToast(getString(R.string.please_enter_device_name));
                            } else if (TextUtils.isEmpty(passcode)) {
                                showToast(getString(R.string.please_enter_passcode));
                            } else {
                                DeviceInfo deviceInformation = new DeviceInfo();
                                deviceInformation.setDevice_name(devicename);
                                deviceInformation.setMac_address(scanResult.getMacAddress());
                                deviceInformation.setPass_code(Integer.parseInt(passcode));
                                activityDeviceScanBinding.getVm().setDeviceInformation(deviceInformation);
                                sendData();
                                dialog.dismiss();
                            }
                        })
                .setNegativeButton(getString(R.string.cancel),
                        (dialog, id) -> {
                            dialog.cancel();
                            finish();
                        });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    /**
     * Called when
     *
     * @param deviceInformation {@link DeviceInfo}
     */
    private void sendDataToDevice(DeviceInfo deviceInformation) {
        //finish();
    }


    /**
     * Send User name and pass code data to ble device and store database if not exits.
     */
    private void sendData() {
//        if (activityDeviceScanBinding.getVm().getDeviceInformation() != null) {
//            BleSendDataModel bleSendDataModel = new BleSendDataModel(activityDeviceScanBinding.getVm().getDeviceInformation().getDevice_name(),
//                    activityDeviceScanBinding.getVm().getDeviceInformation().getPass_code(), "");
//            String strData = new Gson().toJson(bleSendDataModel, BleSendDataModel.class);
//            mServiceBinder.sendBytes(strData.getBytes());
//            if (getMyApplication().getUserInformation().getDevices() == null) {
//                List<DeviceInfo> deviceInfoList = new ArrayList<>();
//                deviceInfoList.add(activityDeviceScanBinding.getVm().getDeviceInformation());
//                getMyApplication().getUserInformation().setDevices(deviceInfoList);
//                activityDeviceScanBinding.getVm().insertItem(getMyApplication().getUserInformation());
//            } else {
//                if (activityDeviceScanBinding.getVm().isDeviceExitsInUserInformation(activityDeviceScanBinding.getVm().getDeviceInformation().getMac_address())) {
//                    activityDeviceScanBinding.getVm().insertItem(getMyApplication().getUserInformation());
//                } else {
//                    getMyApplication().getUserInformation().getDevices().add(activityDeviceScanBinding.getVm().getDeviceInformation());
//                }
//            }
//        }
    }

    /**
     * Broadcast receiver for pairing request.
     */
    public class PairingRequestBroadcast extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (action != null && action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {
                BluetoothDevice mDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                //case1: bonded already
                if (mDevice.getBondState() == BluetoothDevice.BOND_BONDED) {
                    AppLogger.d("BroadcastReceiver: BOND_BONDED.");
//                    navigateView();
                    //          mServiceBinder.connect(scanResult.getMacAddress(), scanResult.getDevice_name());
                }
                //case2: creating a bone
                if (mDevice.getBondState() == BluetoothDevice.BOND_BONDING) {
                    AppLogger.d("BroadcastReceiver: BOND_BONDING.");
                }
                //case3: breaking a bond
                if (mDevice.getBondState() == BluetoothDevice.BOND_NONE) {
                    AppLogger.d("BroadcastReceiver: BOND_NONE.");
                }
            }
        }
    }

    private void setUpNetworkError() {
        activityDeviceScanBinding.getVm().getInternetErrorMessage().observe(this, s -> showNoNetworkSnackbar());
    }


}
