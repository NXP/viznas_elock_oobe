package com.nxp.facemanager.activity;

import android.Manifest;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.location.LocationManager;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.provider.Settings;
import android.util.Base64;
import android.view.View;
import android.widget.Toast;

import com.google.android.material.snackbar.Snackbar;
import com.nxp.facemanager.MyApplication;
import com.nxp.facemanager.R;
import com.nxp.facemanager.ble.BleScanningService;
import com.nxp.facemanager.ble.BleService;
import com.nxp.facemanager.ble.UARTService;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.model.BleEnableEvent;
import com.nxp.facemanager.model.PairingEventModel;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.webservice.ApiInterface;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;

import javax.inject.Inject;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import dagger.android.AndroidInjection;
import dagger.android.support.DaggerAppCompatActivity;


public abstract class BaseActivity extends DaggerAppCompatActivity {
    public static final int SETTING_LOCATION_REQUEST_CODE = 99;
    public static final int BLE_ENABLE_REQUEST_CODE = 100;
    public static final int SETTING_REQUEST_CODE = 101;
    public static final int ACCESS_FINE_LOCATION_REQUEST_CODE = 102;
    UserInformation userInformation;
    DeviceInfo deviceInfo;
    /**
     * UART binding object which is used to bind the service to application context.
     */
    protected UARTService.UARTBinder mServiceBinder;

    /**
     * Database object to access the tables.
     */
    @Inject
    FaceDatabase faceDatabase;

    @Inject
    ApiInterface apiInterface;
    /**
     * System default adapter to get the ble device information.
     */
    protected BluetoothAdapter mBluetoothAdapter;
    /**
     * Local preference to store the some key value information like e.g is_login.
     */
    @Inject
    MySharedPreferences mySharedPreferences;

    /**
     * Application object.
     */
    private MyApplication myApplication;
    @Inject
    Context context;
    /**
     * Check the bond state with ble device{@link BroadcastReceiver}
     */
    private PairingRequest pairingRequest;

    /**
     * Method start service for auto connect the device
     *
     * @param mDevice Bluetooth device
     */
    public void startAutoConnectService(BluetoothDevice mDevice) {
        Intent intent = new Intent(this, BleScanningService.class);
        intent.putExtra(BleService.EXTRA_DEVICE, mDevice);
        startService(intent);
    }

    @Subscribe
    public void bleEnableEvent(BleEnableEvent bleEnableEvent) {
        if (bleEnableEvent != null && bleEnableEvent.isBleOn()) {
            startAutoConnectService(null);
        }
    }


    private boolean isGPSEnable() {
        LocationManager locationManager;
        boolean gps_enabled = false, network_enabled = false;

        locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        try {
            if (locationManager != null) {
                gps_enabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
            }
        } catch (Exception ex) {
            //do nothing...
        }

        try {
            if (locationManager != null) {
                network_enabled = locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
            }
        } catch (Exception ex) {
            //do nothing...
        }

        return gps_enabled || network_enabled;
    }

    /**
     * Called when application destroyed and release resource.
     */
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    /**
     * Check and validate the permission based on android version and start the preview.
     */
    public void checkPermission() {
        if (android.os.Build.VERSION.SDK_INT > 23) {
            if (ContextCompat.checkSelfPermission(context,
                    Manifest.permission.ACCESS_FINE_LOCATION)
                    != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION},
                        ACCESS_FINE_LOCATION_REQUEST_CODE);
            } else {
                if (isGPSEnable()) {
                    if (!setUpBle()) {
                        Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                        startActivityForResult(enableBtIntent, BLE_ENABLE_REQUEST_CODE);
                    } else {
                        EventBus.getDefault().post(new BleEnableEvent(true));
                    }
                } else {
                    showAlertForGPSEnable();
                }
            }
        } else {
            if (!setUpBle()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, BLE_ENABLE_REQUEST_CODE);
            } else {
                EventBus.getDefault().post(new BleEnableEvent(true));
            }
        }
    }


    /**
     * This method will check ble support and register the receiver for the ble pairing.
     */
    protected boolean setUpBle() {
        // Use this check to determine whether BLE is supported on the device.  Then you can
        // selectively disable BLE-related features.
        if (!context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            Toast.makeText(context, R.string.ble_not_supported, Toast.LENGTH_SHORT).show();
            finish();
        }
        // Initializes a Bluetooth adapter.  For API level 18 and above, get a reference to
        // BluetoothAdapter through BluetoothManager.
        final BluetoothManager bluetoothManager =
                (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            mBluetoothAdapter = bluetoothManager.getAdapter();
        }

        // Checks if Bluetooth is supported on the device.
        if (mBluetoothAdapter == null) {
            Toast.makeText(context, R.string.error_bluetooth_not_supported, Toast.LENGTH_SHORT).show();
            finish();
        }

        return mBluetoothAdapter != null && mBluetoothAdapter.isEnabled();

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
            case ACCESS_FINE_LOCATION_REQUEST_CODE:
                if (grantResults.length <= 0
                        || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                    // permission was not granted
                    if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.ACCESS_FINE_LOCATION)) {
                        checkPermission();
                    } else {
                        showAlertForSettingScreen();
                    }
                } else {
                    checkPermission();
                }
                break;
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == SETTING_LOCATION_REQUEST_CODE || requestCode == SETTING_REQUEST_CODE) {
            checkPermission();
        } else if (requestCode == BLE_ENABLE_REQUEST_CODE) {
            if (resultCode == Activity.RESULT_OK) {
                EventBus.getDefault().post(new BleEnableEvent(true));
            } else {
                EventBus.getDefault().post(new BleEnableEvent(false));
            }
        }
    }

    /**
     * This alert shows when user deny the permission.
     */
    private void showAlertForSettingScreen() {
        AlertDialog.Builder builder = new android.app.AlertDialog.Builder(BaseActivity.this, R.style.AlertDialogTheme);
        builder.setTitle(R.string.title_location_permission);
        builder.setMessage(R.string.msg_location_permission);
        builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
            Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            Uri uri = Uri.fromParts("package", context.getPackageName(), null);
            intent.setData(uri);
            startActivityForResult(intent, SETTING_REQUEST_CODE);
        });
        builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> {

        });
        builder.setCancelable(false);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * This alert shows when gps is not enable.
     */
    private void showAlertForGPSEnable() {
        AlertDialog.Builder builder = new android.app.AlertDialog.Builder(BaseActivity.this, R.style.AlertDialogTheme);
        builder.setTitle(R.string.title_location_enabled);
        builder.setMessage(R.string.msg_location_enabled);
        builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
            Intent myIntent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
            startActivityForResult(myIntent, SETTING_LOCATION_REQUEST_CODE);
        });
        builder.setCancelable(false);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * Called whenever the application comes to foreground.
     */
    @Override
    protected void onResume() {
        //Broadcasts when bond state changes (ie:pairing)
        IntentFilter filter = new IntentFilter(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        pairingRequest = new StartFGServiceActivity.PairingRequest();
        registerReceiver(pairingRequest, filter);
        mySharedPreferences.putData(AppConstants.IS_APP_FORGROUND, true);
        super.onResume();
    }


    @SuppressWarnings("JavaReflectionMemberAccess")
    protected void removePairing(BluetoothDevice device) {
        try {
            if (device != null) {
                @SuppressWarnings("ConstantConditions") Method m = device.getClass().getMethod("removeBond", (Class[]) null);
                m.invoke(device, (Object[]) null);
            }
        } catch (Exception e) {
            AppLogger.e("RemovePairing " + e.getMessage());
        }
    }


    /**
     * Application class reference
     *
     * @return MyApplication
     */
    public MyApplication getMyApplication() {
        return myApplication;
    }

    /**
     * Set application class reference to access any where inside the application.
     *
     * @param myApplication {@link MyApplication}
     */
    public void setMyApplication(MyApplication myApplication) {
        this.myApplication = myApplication;
    }

    /**
     * Initialize of activity context.
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        AndroidInjection.inject(this);
        super.onCreate(savedInstanceState);
        setMyApplication((MyApplication) getApplicationContext());
    }

    /**
     * Called when application goes in background.
     */
    @Override
    protected void onPause() {
        if (pairingRequest != null) unregisterReceiver(pairingRequest);
        mySharedPreferences.putData(AppConstants.IS_APP_FORGROUND, false);
        super.onPause();
    }

    protected void showSnackbar(final String text) {

        runOnUiThread(() -> {
            View rootView = getWindow().getDecorView().findViewById(android.R.id.content);
            Snackbar snackbar=Snackbar.make(rootView, text, Snackbar.LENGTH_SHORT);
            snackbar.getView().setTag(100);
            snackbar.show();
        });

    }


    protected void showNoNetworkSnackbar() {

        runOnUiThread(() -> {
            View rootView = getWindow().getDecorView().findViewById(android.R.id.content);
            Snackbar.make(rootView, getString(R.string.check_internet), Snackbar.LENGTH_SHORT)
                    .setAction(getString(R.string.setting_label), v -> startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS)))
                    .setActionTextColor(ContextCompat.getColor(context, R.color.icon_color)).show();
        });

    }


    /**
     * Vibrate device
     * Play the notification sound.
     */
    private void vibrateDevice() {
        Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        // Vibrate for 500 milliseconds
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            if (v != null) {
                v.vibrate(VibrationEffect.createOneShot(500, VibrationEffect.DEFAULT_AMPLITUDE));
            }
        } else {
            //deprecated in API 26
            if (v != null) {
                v.vibrate(500);
            }
        }
        try {
            Uri notification = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
            Ringtone r = RingtoneManager.getRingtone(getApplicationContext(), notification);
            r.play();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Shows a {@link Toast} on the UI thread.
     *
     * @param text The message to show
     */
    protected void showToast(final String text) {
        runOnUiThread(() -> Toast.makeText(BaseActivity.this, text, Toast.LENGTH_SHORT).show());
    }

    /**
     * Broadcast receiver for device pairing
     */
    public class PairingRequest extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (action != null && action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {
                BluetoothDevice mDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                EventBus.getDefault().post(new PairingEventModel(mDevice));
            }
        }
    }

    /**
     * Start activity transition.
     *
     * @param intent {@link Intent}
     */
    @Override
    public void startActivity(Intent intent) {
        super.startActivity(intent);
        overridePendingTransitionEnter();
    }

    /**
     * Whenever user press back press from navigation bar or from the title bar.
     */
    @Override
    public void onBackPressed() {
        super.onBackPressed();
        overridePendingTransitionExit();
    }

    /**
     * Convert Bitmap to String object.
     *
     * @param bitmap {@link Bitmap}
     * @return string
     */
    public String bitmapToString(Bitmap bitmap) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.PNG, 50, baos);
        byte[] b = baos.toByteArray();
        return Base64.encodeToString(b, Base64.DEFAULT);
    }

    /**
     * Convert String to bitmap.
     *
     * @param encodedString image String
     * @return android.graphics.Bitmap
     */
    public Bitmap stringToBitMap(String encodedString) {
        try {
            byte[] encodeByte = Base64.decode(encodedString, Base64.DEFAULT);
            return BitmapFactory.decodeByteArray(encodeByte, 0, encodeByte.length);
        } catch (Exception e) {
            e.getMessage();
            return null;
        }
    }

    /**
     * Overrides the pending Activity transition by performing the "Enter" animation.
     */
    protected void overridePendingTransitionEnter() {
        overridePendingTransition(R.anim.slide_from_right, R.anim.slide_to_left);
    }

    /**
     * Overrides the pending Activity transition by performing the "Exit" animation.
     */
    protected void overridePendingTransitionExit() {
        overridePendingTransition(R.anim.slide_from_left, R.anim.slide_to_right);
    }

    /**
     * Check if service is running or not.
     *
     * @param serviceClass Class
     * @return true/false
     */
    public boolean isMyServiceRunning(Class<?> serviceClass) {
        ActivityManager manager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        if (manager != null) {
            for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
                if (serviceClass.getName().equals(service.service.getClassName())) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Read file from asset folder
     *
     * @param fileName file name
     * @return String
     */
    protected String readAssetFile(String fileName) {
        AssetManager assetManager = getAssets();
        InputStream input;
        try {
            input = assetManager.open(fileName);
            int size = input.available();
            byte[] buffer = new byte[size];
            //noinspection ResultOfMethodCallIgnored
            input.read(buffer);
            input.close();
            return new String(buffer);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return "";
    }
}
