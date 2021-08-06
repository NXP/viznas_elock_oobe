package com.nxp.facemanager.activity;


import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;

import com.nxp.facemanager.R;
import com.nxp.facemanager.ble.BleService;
import com.nxp.facemanager.databinding.ActivitySenddataBinding;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.viewModels.SendDataModule;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Objects;

import androidx.annotation.Nullable;
import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.ViewModelProviders;

/**
 * Training data send activity which send data to ble device.
 */
public class TrainingDataSendActivity extends BaseActivity {

    /**
     * Data binding object reference of current activity which bind with {@link xml }
     */
    private ActivitySenddataBinding activityTrainingDataSendBinding;

    /**
     * Send data to ble in chunk of data.
     *
     * @param data data which you want to send ble
     * @return array list of byte.
     */
    public static ArrayList<byte[]> convertStringToByteArrayChunks(String data) {
        ArrayList<byte[]> chunks = new ArrayList<>();

        byte[] wholeData = data.getBytes();
        int PAYLOAD_BYTES_LENGTH = 250; // Enter Max allowed payload size here

        if (wholeData.length > PAYLOAD_BYTES_LENGTH) {

            int totalLength = wholeData.length / PAYLOAD_BYTES_LENGTH;

            for (int i = 0; i < totalLength; i++) {
                int from = i * PAYLOAD_BYTES_LENGTH;
                int to = from + PAYLOAD_BYTES_LENGTH;

                byte[] chunk = Arrays.copyOfRange(wholeData, from, to);

                chunks.add(chunk);

                AppLogger.e("chunk", "size : " + chunk.length);
            }

            int remainingChunkSize = wholeData.length % PAYLOAD_BYTES_LENGTH;

            if (remainingChunkSize > 0) {
                int from = wholeData.length - remainingChunkSize;
                int to = wholeData.length;

                byte[] chunk = Arrays.copyOfRange(wholeData, from, to);
                AppLogger.e("lastchunk", "size : " + chunk.length);

                chunks.add(chunk);
            }

        } else {
            // less than equal to max bytes payload
            chunks.add(wholeData);
        }

        return chunks;
    }

    /**
     * Initialize the variable and binding
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SendDataModule sendDataModule = ViewModelProviders.of(this)
                .get(SendDataModule.class);
        activityTrainingDataSendBinding = DataBindingUtil.setContentView(this, R.layout.activity_senddata);
        activityTrainingDataSendBinding.setVm(sendDataModule);
        AppLogger.d("Device address " + getIntent().getStringExtra(BleService.EXTRA_DEVICE_ADDRESS) + "  " + getIntent().getStringExtra(BleService.EXTRA_DEVICE_NAME));
        activityTrainingDataSendBinding.getVm().getMacaddress().set(getIntent().getStringExtra(BleService.EXTRA_DEVICE_ADDRESS));
        activityTrainingDataSendBinding.getVm().getDevicename().set(getIntent().getStringExtra(BleService.EXTRA_DEVICE_NAME));
        activityTrainingDataSendBinding.setLifecycleOwner(this);
        setToolbar();

    }

    /**
     * Initialize the toolbar
     */
    private void setToolbar() {
        setSupportActionBar(activityTrainingDataSendBinding.toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(false);
        getSupportActionBar().setDisplayShowHomeEnabled(true);
        activityTrainingDataSendBinding.toolbar.setNavigationOnClickListener(view -> onBackPressed());
    }


    @Override
    public void onBackPressed() {
        if (activityTrainingDataSendBinding != null) {
            if (!activityTrainingDataSendBinding.getVm().getIsRunning().get()) {
                super.onBackPressed();
            }
        }

    }


//    // for starting the uart service
//    private void startService() {
//        final Intent service = new Intent(this, UARTService.class);
//        service.putExtra(BleService.EXTRA_DEVICE_ADDRESS, activityTrainingDataSendBinding.getVm().getMacaddress().get());
//        service.putExtra(BleService.EXTRA_DEVICE_NAME, activityTrainingDataSendBinding.getVm().getDevice_name().get());
//        startService(service);
//        serviceConnectionBle = new ServiceConnectionBle();
//        bindService(service, serviceConnectionBle, 0);
//        mIsBound = true;
//    }

    /**
     * Sending data to ble using service binder object.
     */
    private void sampleSend() {
        if (mServiceBinder != null) {
            mServiceBinder.sendBytes("0123456789\n".getBytes());
        }
    }

    /**
     * Manage the click listener for start training.
     *
     * @param view pass view
     */
    public void onClickStart(View view) {
        activityTrainingDataSendBinding.getVm().getIsRunning().set(true);
        activityTrainingDataSendBinding.getVm().getIsVisible().set(false);
        activityTrainingDataSendBinding.getVm().getUserImageString().set("Hello");
        sampleSend();
    }


    /**
     * show dialog after data send successfully to ble.
     */
    private void showDialogForSuccess() {
        LayoutInflater li = LayoutInflater.from(this);
        @SuppressLint("InflateParams") View dialogView = li.inflate(R.layout.success_dialog, null);
        AlertDialog alertDialog;
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                this);
        // set title
        alertDialogBuilder.setTitle(null);
        alertDialogBuilder.setView(dialogView);
        // set custom dialog icon
        Button button = dialogView.findViewById(R.id.btnOkay);
        button.setOnClickListener(v -> {
            //disconnectService();
            finish();
        });
        // create alert dialog
        alertDialog = alertDialogBuilder.create();
        // show it
        alertDialog.show();
    }
}
