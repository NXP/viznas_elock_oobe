package com.nxp.facemanager.ble;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.TextView;

import com.nxp.facemanager.BR;
import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.OnItemClickListener;
import com.nxp.facemanager.databinding.BleListitemBinding;
import com.nxp.facemanager.utility.AppLogger;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.databinding.ViewDataBinding;
import androidx.recyclerview.widget.RecyclerView;

/**
 * Ble scanning device list adapter.
 */
public class LeDeviceListAdapter extends RecyclerView.Adapter<LeDeviceListAdapter.MyViewHolder> {
    /**
     * Inject the context.
     */
    private Context context;
    /**
     * Local reference of ble devices.
     */
    private List<BleModel> bluetoothDevicesList;
    /**
     * List item click listener passed to view.
     */
    private OnItemClickListener itemClickListener;
    /**
     * Service binder object to check the connected or not.
     */
    private UARTService.UARTBinder mServiceBinder;

    /**
     * Constructor for access the adapter class.
     *
     * @param context view listener reference.
     */
    public LeDeviceListAdapter(Context context) {
        this.context = context;
        this.itemClickListener = (OnItemClickListener) context;
        bluetoothDevicesList = new ArrayList<>();
    }

    /**
     * Set the binder object.
     *
     * @param mServiceBinder UARTService.UARTBinder
     */
    public void setmServiceBinder(UARTService.UARTBinder mServiceBinder) {
        this.mServiceBinder = mServiceBinder;
        notifyDataSetChanged();
    }

    /**
     * Bind xml to adapter {@link RecyclerView.ViewHolder}
     *
     * @param parent   {@link ViewGroup}
     * @param viewType int
     * @return MyViewHolder
     */
    @NonNull
    @Override
    public MyViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        BleListitemBinding bleListitemBinding = BleListitemBinding.inflate(LayoutInflater.from(parent.getContext()), parent, false);
        return new LeDeviceListAdapter.MyViewHolder(bleListitemBinding);
    }

    /**
     * Binding the data to xml files.
     *
     * @param holder   {@link MyViewHolder}
     * @param position int
     */
    @SuppressLint("DefaultLocale")
    @Override
    public void onBindViewHolder(@NonNull final MyViewHolder holder, final int position) {
        final BleModel device = bluetoothDevicesList.get(position);

        holder.getBinding().setVariable(BR.bleModel, device);
        holder.getBinding().executePendingBindings();

        if (mServiceBinder != null) {
            if (mServiceBinder.isConnected()) {
                if (mServiceBinder.getDeviceAddress() != null && mServiceBinder.getDeviceAddress().equalsIgnoreCase(device.getMacAddress())) {

                    holder.txtConnect.setClickable(false);
                    holder.txtConnect.setText(context.getString(R.string.connected));

                } else {
                    holder.txtConnect.setClickable(true);
                    holder.txtConnect.setText(context.getString(R.string.connect));
                }
            }
        } else {
            holder.txtConnect.setClickable(true);
            holder.txtConnect.setText(context.getString(R.string.connect));
        }
        holder.txtConnect.setOnClickListener(view -> itemClickListener.onItemClick(device, holder.txtConnect));
    }

    /**
     * on scanning device this method called to add device in list adapter.
     *
     * @param device {@link BleModel}
     */
    public synchronized void addDevice(BleModel device) {
        if (!isContains(device)) {
            AppLogger.d("Mac Address: " + device.getBluetoothDevice());
            bluetoothDevicesList.add(device);
            Collections.sort(bluetoothDevicesList, (obj1, obj2) -> {
                return Integer.compare(obj2.getRssi(), obj1.getRssi()); // To compare integer values

            });

        }
    }

    /**
     * Remove device form device list and notify adapter
     *
     * @param bleModel to remove
     */
    public synchronized void removeDevice(BleModel bleModel) {
        if (isContains(bleModel)) {
            bluetoothDevicesList.remove(bleModel);
            notifyDataSetChanged();
        }
    }

    /**
     * Check device is already in list or not.
     *
     * @param scanResultNew {@link BleModel}
     * @return boolean true/false.
     */
    private synchronized boolean isContains(BleModel scanResultNew) {
        for (BleModel scanResult : bluetoothDevicesList) {
            if (scanResult.getMacAddress().equalsIgnoreCase(scanResultNew.getMacAddress())) {
                if (scanResult.getRssi() != scanResultNew.getRssi()) {
                    scanResult.setRssi(scanResultNew.getRssi());
                    notifyDataSetChanged();
                }
                return true;
            }
        }
        return false;
    }

    /**
     * Clear ble list.
     */
    public void clear() {
        bluetoothDevicesList.clear();
        notifyDataSetChanged();
    }

    /**
     * Total displayed item count.
     *
     * @return int
     */
    @Override
    public int getItemCount() {
        return bluetoothDevicesList.size();
    }

    /**
     * Finding view reference.
     */
    class MyViewHolder extends RecyclerView.ViewHolder {

        private ViewDataBinding viewBinding;
        TextView txtDeviceName;
        TextView txtMacAddress;
        TextView txtConnect;


        MyViewHolder(BleListitemBinding binding) {
            super(binding.getRoot());
            viewBinding = binding;
            txtMacAddress = binding.txtDeviceName;
            txtDeviceName = binding.txtMacAddress;
            txtConnect = binding.txtConnect;

        }

        public ViewDataBinding getBinding() {
            return viewBinding;
        }

    }


}