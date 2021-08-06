package com.nxp.facemanager.activity.adapter;

import android.content.Context;
import android.util.SparseBooleanArray;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.nxp.facemanager.BR;
import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.OnItemClickListener;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.databinding.UserItemBinding;
import com.nxp.facemanager.swipeRevealLayout.SwipeRevealLayout;
import com.nxp.facemanager.swipeRevealLayout.ViewBinderHelper;

import java.util.ArrayList;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import androidx.databinding.ViewDataBinding;
import androidx.recyclerview.widget.RecyclerView;


/**
 * class for dashboard recycler view
 */
public class UserListAdapter extends RecyclerView.Adapter<UserListAdapter.MyViewHolder> implements Filterable {
    private final Context context;
    /**
     * Local reference of the Device Information.
     */
    public List<UserInformation> originalUserList = new ArrayList<>();
    /**
     * Filter reference of the Device Information.
     */
    private List<UserInformation> filteredUserList = new ArrayList<>();

    public ArrayList<UserInformation> selected_usersList=new ArrayList<>();
    /**
     * Row item click listener.
     */
    private OnItemClickListener itemClickListener;
    /**
     * Binding helper object
     */
    private final ViewBinderHelper binderHelper = new ViewBinderHelper();
    /**
     * boolean for isDeviceConnected
     */
    private boolean isDeviceConnected;
    private SparseBooleanArray mSelectedItems;
    private boolean mIsInChoiceMode;


    /**
     * Constructor for access the class.
     *
     * @param listener reference of accessing class.
     */
    public UserListAdapter(OnItemClickListener listener, boolean isDeviceConnected, Context context) {
        this.itemClickListener = listener;
        this.isDeviceConnected = isDeviceConnected;
        this.context =context;
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
        UserItemBinding userItemBinding = UserItemBinding.inflate(LayoutInflater.from(parent.getContext()), parent, false);
        return new MyViewHolder(userItemBinding);
    }

    /**
     * Finding the view references and bind the data.
     *
     * @param holder   {@link MyViewHolder}
     * @param position int
     */
    @Override
    public void onBindViewHolder(@NonNull final MyViewHolder holder, final int position) {
        final UserInformation userInformation = filteredUserList.get(position);
        holder.getBinding().setVariable(BR.user, userInformation);
        if (userInformation != null) {
            binderHelper.bind(holder.swipeLayout, "" + userInformation.getLocal_user_id());
        }
        holder.delete_layout.setTag(position);
        if (isDeviceConnected) {
            holder.relSync.setVisibility(ViewGroup.VISIBLE);
        } else {
            holder.relSync.setVisibility(ViewGroup.GONE);
        }
        holder.getBinding().executePendingBindings();
        holder.imgOverFlow.setOnClickListener(view ->
                itemClickListener.onItemClick(userInformation, holder.imgOverFlow));
        holder.delete_layout.setOnClickListener(view -> itemClickListener.onItemClick(userInformation, holder.delete_layout));
        holder.relDetail.setOnClickListener(view -> itemClickListener.onItemClick(userInformation, holder.relDetail));
        holder.relSync.setOnClickListener(view -> itemClickListener.onItemClick(userInformation, holder.relSync));

        if(selected_usersList.contains(originalUserList.get(position)))
            holder.front_layout.setBackgroundColor(ContextCompat.getColor(context, R.color.icon_color_transparent));
        else
            holder.front_layout.setBackgroundColor(ContextCompat.getColor(context, R.color.white));



    }

    /**
     * Total displayed item count.
     *
     * @return int
     */
    @Override
    public int getItemCount() {
        if (filteredUserList == null) {
            return 0;
        }
        return filteredUserList.size();
    }


    /**
     * Update user list and notify the adapter.
     *
     * @param userInformationList List<{@link DeviceInfo}>
     */
    public void setUser(List<UserInformation> userInformationList) {
        originalUserList = userInformationList;
        filteredUserList = originalUserList;
        notifyDataSetChanged();
    }

    /**
     * Include Search bar filter and filter based on the name and email id.
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
                    filteredUserList = originalUserList;
                } else {
                    List<UserInformation> filteredList = new ArrayList<>();
                    for (UserInformation row : originalUserList) {

                        // name match condition. this might differ depending on your requirement
                        // here we are looking for name or phone number match
                        if (row.getName().toLowerCase().contains(charString.toLowerCase()) || row.getEmail().contains(charSequence)) {
                            filteredList.add(row);
                        }
                    }

                    filteredUserList = filteredList;
                }

                FilterResults filterResults = new FilterResults();
                filterResults.values = filteredUserList;
                return filterResults;
            }

            @Override
            protected void publishResults(CharSequence charSequence, FilterResults filterResults) {
                filteredUserList = (ArrayList<UserInformation>) filterResults.values;
                notifyDataSetChanged();
            }
        };
    }


    public boolean isSelected(int position) {
        return getSelectedItems().contains(position);
    }

    public void switchSelectedState(int position) {
        if (mSelectedItems.get(position)) {       // item has been selected, de-select it.
            mSelectedItems.delete(position);
        } else {
            mSelectedItems.put(position, true);
        }
        notifyItemChanged(position);
    }

    public void clearSelectedState() {
        List<Integer> selection = getSelectedItems();
        mSelectedItems.clear();
        for (Integer i : selection) {
            notifyItemChanged(i);
        }
    }

    public int getSelectedItemCount() {
        return mSelectedItems.size();
    }

    public List<Integer> getSelectedItems() {
        List<Integer> items = new ArrayList<>(mSelectedItems.size());
        for (int i = 0; i < mSelectedItems.size(); ++i) {
            items.add(mSelectedItems.keyAt(i));
        }
        return items;
    }

    public void setIsInChoiceMode(boolean isInChoiceMode) {
        this.mIsInChoiceMode = isInChoiceMode;
    }

    public boolean getIsInChoiceMode() {
        return mIsInChoiceMode;
    }


    /**
     * Custom class which is used to bind with RecyclerView.
     */
    class MyViewHolder extends RecyclerView.ViewHolder {
        FrameLayout delete_layout;
        FrameLayout front_layout;
        RelativeLayout relDetail;
        TextView user_name, email;
        ImageView imageUser, imgOverFlow;
        ViewDataBinding viewBinding;
        SwipeRevealLayout swipeLayout;
        RelativeLayout relSync;

        MyViewHolder(UserItemBinding binding) {
            super(binding.getRoot());
            viewBinding = binding;
            user_name = binding.txtUserName;
            email = binding.txtEmail;
            imageUser = binding.imgUser;
            delete_layout = binding.deleteLayout;
            front_layout = binding.frontLayout;
            swipeLayout = binding.swipeLayout;
            imgOverFlow = binding.imgOverFlow;
            relDetail = binding.relDetail;
            relSync = binding.relSync;
        }

        public ViewDataBinding getBinding() {
            return viewBinding;
        }

    }


}
