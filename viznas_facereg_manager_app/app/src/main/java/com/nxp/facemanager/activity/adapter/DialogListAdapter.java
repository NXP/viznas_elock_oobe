package com.nxp.facemanager.activity.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.ViewGroup;

import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.databinding.UserMacDialogItemBinding;

import java.util.List;

import androidx.annotation.NonNull;
import androidx.databinding.ViewDataBinding;
import androidx.recyclerview.widget.RecyclerView;

/**
 * Binding adapter for the dialog display when user press on specific user list.
 */
public class DialogListAdapter extends RecyclerView.Adapter<DialogListAdapter.ViewHolder> {
    /**
     * Inflater reference.
     */
    private LayoutInflater inflater;
    /**
     * Holder reference.
     */
    private ViewHolder holder;
    /**
     * Accessed context reference.
     */
    private Context myContext;
    /**
     * Local reference of device information stored with specific user.
     */
    private List<DeviceInfo> deviceList;

    /**
     * Constructor use for access the class.
     *
     * @param context access context
     * @param list    list of device.
     */
    public DialogListAdapter(Context context, List<DeviceInfo> list) {
        myContext = context;
        deviceList = list;
        inflater = LayoutInflater.from(context);
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {

        // UserMacDialogItemBinding customUserMacDialogBinding = UserMacDialogItemBinding.inflate(LayoutInflater.from(parent.getContext()), parent, false);
        return null;
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {

//        DeviceInfo deviceInfo = deviceList.get(position);
//        holder.getBinding().setVariable(BR.deviceInfo, deviceInfo);
//        holder.getBinding().executePendingBindings();

    }

    /**
     * Access the current position.
     *
     * @param i i
     * @return long
     */
    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public int getItemCount() {
        return deviceList.size();
    }


    /**
     * View reference class.
     */
    class ViewHolder extends RecyclerView.ViewHolder {

        private ViewDataBinding viewBinding;

        public ViewHolder(UserMacDialogItemBinding binding) {
            super(binding.getRoot());
            viewBinding = binding;
        }

        public ViewDataBinding getBinding() {
            return viewBinding;
        }
    }
}
