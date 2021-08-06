package com.nxp.facemanager.utility;


import android.annotation.SuppressLint;
import android.widget.ImageView;
import android.widget.TextView;

import com.nxp.facemanager.R;
import com.nxp.facemanager.ble.BleModel;
import com.nxp.facemanager.database.entity.UserInformation;

import androidx.databinding.BindingAdapter;

/**
 * This is custom data binding class and used for  and image binding.
 */
public class BindingUtils {

    private BindingUtils() {
        // This class is not publicly instantiable
    }


    @BindingAdapter("macAddress")
    public static void setMacAddress(TextView txtView, UserInformation userInformation) {
//        List<DeviceInfo> list = userInformation.getDeviceIds();
//        String address = (list != null && list.size() > 0) ? list.get(0).getMac_address() : "N/A";
//        txtView.setText(address);
    }

    @BindingAdapter("showMore")
    public static void showMoreVisibilty(ImageView imgView, UserInformation userInformation) {
//        List<DeviceInfo> list = userInformation.getDevices();
//        if (list != null && list.size() > 1) {
//            imgView.setVisibility(View.VISIBLE);
//        } else {
//            imgView.setVisibility(View.GONE);
//        }
    }



    @SuppressLint("DefaultLocale")
    @BindingAdapter("macAddressRssi")
    public static void setMacAddressRssi(TextView txtView, BleModel bleModel) {
        txtView.setText(String.format("%s    %ddbm", bleModel.getMacAddress(), bleModel.getRssi()));
    }


    @BindingAdapter("deviceName")
    public static void setDeviceName(TextView txtView, BleModel bleModel) {
        final String deviceName = bleModel.getDevicename();
        if (deviceName != null && deviceName.length() > 0)
            txtView.setText(deviceName);
        else
            txtView.setText(R.string.unknown_device);
    }

}
