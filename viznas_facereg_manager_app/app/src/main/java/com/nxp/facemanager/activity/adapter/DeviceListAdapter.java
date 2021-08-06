package com.nxp.facemanager.activity.adapter;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;

import com.nxp.facemanager.BR;
import com.nxp.facemanager.activity.OnItemClickListener;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.databinding.DeviceItemBinding;
import com.nxp.facemanager.swipeRevealLayout.SwipeRevealLayout;
import com.nxp.facemanager.swipeRevealLayout.ViewBinderHelper;

import java.util.ArrayList;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.databinding.ViewDataBinding;
import androidx.recyclerview.widget.RecyclerView;


/**
 * class for dashboard recycler view
 */
public class DeviceListAdapter extends RecyclerView.Adapter<DeviceListAdapter.MyViewHolder> implements Filterable {
    /**
     * Local reference of the User Information.
     */
    private List<DeviceInfo> originalDeviceList = new ArrayList<>();
    /**
     * Filter data reference of the User Information.
     */
    private List<DeviceInfo> filterDeviceList = new ArrayList<>();
    /**
     * Row item click listener.
     */
    private OnItemClickListener itemClickListener;
    private final ViewBinderHelper binderHelper = new ViewBinderHelper();

    public List<DeviceInfo> getFilterDeviceList() {
        return filterDeviceList;
    }

    private String mac_address;

    /**
     * Constructor for access the class.
     *
     * @param listener    reference of accessing class.
     * @param mac_address mac address
     */
    public DeviceListAdapter(OnItemClickListener listener, String mac_address) {
        this.itemClickListener = listener;
        this.mac_address = mac_address;
        binderHelper.setOpenOnlyOne(true);
    }

    /**
     * Bind view with xml.
     *
     * @param parent   {@link ViewGroup}
     * @param viewType int
     * @return MyViewHolder
     */
    @NonNull
    @Override
    public MyViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        DeviceItemBinding deviceItemBinding = DeviceItemBinding.inflate(LayoutInflater.from(parent.getContext()), parent, false);
        return new MyViewHolder(deviceItemBinding);
    }

    /**
     * Only if you need to restore open/close state when the orientation is changed.
     * Call this method in {@link android.app.Activity#onSaveInstanceState(Bundle)}
     */
    public void saveStates(Bundle outState) {
        binderHelper.saveStates(outState);
    }

    /**
     * Only if you need to restore open/close state when the orientation is changed.
     * Call this method in {@link android.app.Activity#onRestoreInstanceState(Bundle)}
     */
    public void restoreStates(Bundle inState) {
        binderHelper.restoreStates(inState);
    }

    /**
     * Finding the view references and bind the data.
     *
     * @param holder   {@link MyViewHolder}
     * @param position int
     */
    @Override
    public void onBindViewHolder(@NonNull final MyViewHolder holder, final int position) {
        final DeviceInfo deviceInfo = filterDeviceList.get(position);
        holder.getBinding().setVariable(BR.device, deviceInfo);
        if (deviceInfo != null) {
            if (!TextUtils.isEmpty(deviceInfo.get_id())) {
                binderHelper.bind(holder.swipeLayout, deviceInfo.get_id());
            } else {
                binderHelper.bind(holder.swipeLayout, deviceInfo.getMac_address());
            }

            if (deviceInfo.getMac_address().equals(mac_address)) {
                holder.imgBleConnected.setVisibility(ViewGroup.VISIBLE);
            } else {
                holder.imgBleConnected.setVisibility(ViewGroup.GONE);
            }
        }
        holder.delete_layout.setTag(position);
        holder.getBinding().executePendingBindings();
        holder.delete_layout.setOnClickListener(view -> itemClickListener.onItemClick(deviceInfo, holder.delete_layout));
        holder.edit_layout.setOnClickListener(view -> itemClickListener.onItemClick(deviceInfo, holder.edit_layout));

    }

    /**
     * Total displayed item count.
     *
     * @return int
     */
    @Override
    public int getItemCount() {
        if (filterDeviceList == null)
            return 0;
        return filterDeviceList.size();
    }

    /**
     * Update device list and notify the adapter.
     *
     * @param words List<{@link DeviceInfo}>
     */
    public void setDevice(List<DeviceInfo> words) {
        originalDeviceList = words;
        filterDeviceList = originalDeviceList;
        notifyDataSetChanged();
    }

    public void removeItem(int position) {
        filterDeviceList.remove(position);
        notifyItemRemoved(position);
    }

    public void restoreItem(DeviceInfo item, int position) {
        filterDeviceList.add(position, item);
        notifyItemInserted(position);
    }

    /**
     * Include the Search bar filter which filter based on the name and mac address.
     *
     * @return Filter
     */
    @Override
    public Filter getFilter() {
        return new Filter() {
            @Override
            protected FilterResults performFiltering(CharSequence charSequence) {
                String charString = charSequence.toString();
                if (charString.isEmpty()) {
                    filterDeviceList = originalDeviceList;
                } else {
                    List<DeviceInfo> filteredList = new ArrayList<>();
                    for (DeviceInfo row : originalDeviceList) {

                        // name match condition. this might differ depending on your requirement
                        // here we are looking for name or phone number match
                        if (row.getDevice_name().toLowerCase().contains(charString.toLowerCase()) || row.getMac_address().contains(charSequence)) {
                            filteredList.add(row);
                        }
                    }

                    filterDeviceList = filteredList;
                }

                FilterResults filterResults = new FilterResults();
                filterResults.values = filterDeviceList;
                return filterResults;
            }

            @SuppressWarnings("unchecked")
            @Override
            protected void publishResults(CharSequence charSequence, FilterResults filterResults) {
                filterDeviceList = (ArrayList<DeviceInfo>) filterResults.values;
                notifyDataSetChanged();
            }
        };
    }


    /**
     * Custom class which is used to bind with RecyclerView.
     */
    class MyViewHolder extends RecyclerView.ViewHolder {
        FrameLayout delete_layout, edit_layout;
        TextView device_name, mac_address;
        ImageView imgBleConnected;
        private SwipeRevealLayout swipeLayout;
        private ViewDataBinding viewBinding;

        MyViewHolder(DeviceItemBinding binding) {
            super(binding.getRoot());
            viewBinding = binding;
            device_name = binding.txtDeviceName;
            mac_address = binding.tvMacAdd;
            imgBleConnected = binding.imgBleConnected;
            swipeLayout = binding.swipeLayout;

            delete_layout = binding.deleteLayout;
            edit_layout = binding.editLayout;
        }

        public ViewDataBinding getBinding() {
            return viewBinding;
        }
    }


}
