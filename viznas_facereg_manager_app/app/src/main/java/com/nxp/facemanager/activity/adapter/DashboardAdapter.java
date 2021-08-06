package com.nxp.facemanager.activity.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.nxp.facemanager.BR;
import com.nxp.facemanager.activity.OnItemClickListener;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.databinding.DashboarItemBinding;

import java.util.ArrayList;
import java.util.List;

import javax.inject.Inject;

import androidx.annotation.NonNull;
import androidx.databinding.ViewDataBinding;
import androidx.recyclerview.widget.RecyclerView;
import de.hdodenhof.circleimageview.CircleImageView;


/**
 * class for dashboard recycler view
 */
public class DashboardAdapter extends RecyclerView.Adapter<DashboardAdapter.MyViewHolder> {
    /**
     * Access the context of application with dependency injection.
     */
    @Inject
    Context context;
    /**
     * Local reference of the User Information.
     */
    private List<UserInformation> userInformation = new ArrayList<>();
    /**
     * Row item click listener.
     */
    private OnItemClickListener itemClickListener;

    /**
     * Constructor for access the class.
     *
     * @param listener reference of accessing class.
     */
    public DashboardAdapter(OnItemClickListener listener) {
//        this.userInformation = users;
        this.itemClickListener = listener;
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
        DashboarItemBinding dashboarItemBinding = DashboarItemBinding.inflate(LayoutInflater.from(parent.getContext()), parent, false);
        return new MyViewHolder(dashboarItemBinding);
    }

    /**
     * Finding the view references and bind the data.
     *
     * @param holder   {@link MyViewHolder}
     * @param position int
     */
    @Override
    public void onBindViewHolder(@NonNull final MyViewHolder holder, final int position) {


        final UserInformation userInfo = userInformation.get(position);
        holder.getBinding().setVariable(BR.user, userInfo);
        holder.getBinding().executePendingBindings();

        holder.macAddress.setOnClickListener(view -> itemClickListener.onItemClick(userInformation.get(position), holder.macAddress));
        holder.bleImage.setOnClickListener(view -> itemClickListener.onItemClick(userInformation.get(position), holder.bleImage));
        holder.showMoreMac.setOnClickListener(view -> itemClickListener.onItemClick(userInformation.get(position), holder.showMoreMac));
        holder.txtOptions.setOnClickListener(view -> itemClickListener.onItemClick(userInformation.get(position), holder.txtOptions));
//        holder.userName.setTag(userInformation.get(position));
    }

    /**
     * Total displayed item count.
     *
     * @return int
     */
    @Override
    public int getItemCount() {
        return userInformation.size();
    }

    /**
     * Update user list and notify the adapter.
     *
     * @param words List<UserInformation>
     */
    public void setUsers(List<UserInformation> words) {
        userInformation = words;

        notifyDataSetChanged();
    }

    /**
     * Custom class which is used to bind with RecyclerView.
     */
    class MyViewHolder extends RecyclerView.ViewHolder {

        private ViewDataBinding viewBinding;
        TextView userName, macAddress, txtOptions;
        ImageView bleImage, showMoreMac;
        CircleImageView userImage;

        MyViewHolder(DashboarItemBinding binding) {
            super(binding.getRoot());
            viewBinding = binding;
            userName = binding.txtUsername;
            macAddress = binding.tvMacAdd;
            bleImage = binding.imgBle;
            showMoreMac = binding.tvShowMore;
            userImage = binding.imgUser;
            txtOptions = binding.txtOptions;
        }

        public ViewDataBinding getBinding() {
            return viewBinding;
        }

    }


}
