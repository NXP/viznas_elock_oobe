package com.nxp.facemanager.fragment;

import android.app.SearchManager;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.PopupMenu;

import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.BaseActivity;
import com.nxp.facemanager.activity.CreateUserActivity;
import com.nxp.facemanager.activity.HomeActivity;
import com.nxp.facemanager.activity.OnItemClickListener;
import com.nxp.facemanager.activity.OnItemLongClickListener;
import com.nxp.facemanager.activity.StartFGServiceActivity;
import com.nxp.facemanager.activity.TrainingActivity;
import com.nxp.facemanager.activity.adapter.UserListAdapter;
import com.nxp.facemanager.ble.BleService;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.databinding.FragmentSmartUsersBinding;
import com.nxp.facemanager.model.BleScanBackgroundServiceStartScanEvent;
import com.nxp.facemanager.model.BleServiceDisableEvent;
import com.nxp.facemanager.model.BleStatusEvent;
import com.nxp.facemanager.model.ChangeHomeIcon;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.utility.RecyclerItemClickListener;
import com.nxp.facemanager.viewModels.UserInformationListViewModel;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONObject;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.widget.SearchView;
import androidx.databinding.DataBindingUtil;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProviders;

import static com.nxp.facemanager.viewModels.CreateUserViewModel.IS_EDIT_USER;

/**
 * A simple {@link Fragment} subclass.
 * Use the {@link SmartUsersFragment#newInstance} factory method to
 * create an instance of this fragment. This class has links to the tasks of:
 * 1. Create new guest user
 * 2. List all existing guest users
 * 3. Send face data of single or all users(via long press and title menu option selection)
 */
public class SmartUsersFragment extends BaseFragment implements OnItemClickListener, OnItemLongClickListener, View.OnClickListener {
    public static final String IS_SKIPPABLE = "is_skippable";
    /**
     * Reference of Search view.
     */
    private SearchView searchView = null;
    /**
     * Listener for the text change on search bar.
     */
    private SearchView.OnQueryTextListener queryTextListener;
    /**
     * Adapter reference for user list.
     */
    private UserListAdapter userListAdapter;
    /**
     * Binding reference smart user xml file. fragment_smart_users
     */
    private FragmentSmartUsersBinding fragmentSmartUsersBinding;
    /***
     * Reference object of menu items
     */
    private MenuItem menu_search, menu_ble;
    /**
     * Local object reference of {@link UserInformation}
     */
    private List<UserInformation> userInformationList = new ArrayList<>();
    private boolean isMultiSelect = false;
    private ActionMode mActionMode;
    private Menu context_menu;
    private ArrayList<UserInformation> multiselectUserList = new ArrayList<>();

    public SmartUsersFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment SmartLocksFragment.
     */
    public static SmartUsersFragment newInstance() {
        return new SmartUsersFragment();
    }

    /**
     * Default Method.
     *
     * @param savedInstanceState @{@link Bundle}
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
    }

    /**
     * Default Method.
     *
     * @param inflater           {@link LayoutInflater}
     * @param container          {@link ViewGroup}
     * @param savedInstanceState {@link Bundle}
     * @return View
     */
    @SuppressWarnings("NullableProblems")
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        fragmentSmartUsersBinding = DataBindingUtil.inflate(inflater, R.layout.fragment_smart_users, container, false);
        View view = fragmentSmartUsersBinding.getRoot();
        UserInformationListViewModel userListViewModel = ViewModelProviders.of(this, new ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(UserInformationListViewModel.class);

        fragmentSmartUsersBinding.setUser(userListViewModel);
        userListViewModel.setContext(context);
        fragmentSmartUsersBinding.fabUser.setOnClickListener(this);
        fragmentSmartUsersBinding.swipeContainer.setColorSchemeResources(R.color.icon_color);
        bindRecyclerView();
        try {
            userData = CommonUtils.readAssetFile(context, "facerec1.txt");
        } catch (Exception e) {
            e.printStackTrace();
        }
        String connection_type = mySharedPreferences.getStringData(AppConstants.CONNECTION_TYPE);
        if (connection_type.compareTo(AppConstants.CONNECTION_BLE) == 0) {
            Objects.requireNonNull(fragmentSmartUsersBinding.getUser()).getUsersListFromBle();
        } else{
            Objects.requireNonNull(fragmentSmartUsersBinding.getUser()).getUsersListFromCloud();
        }



        observer(userListViewModel);
        setUpNetworkError();
        return view;
    }

    /**
     * Observer method for {@link UserInformation}
     *
     * @param userListViewModel {@link UserInformationListViewModel}
     */
    private void observer(UserInformationListViewModel userListViewModel) {
        userListViewModel.getUserList().observe(this, userInformation -> {
            // Update the cached copy of the words in the adapter.
            userInformationList = userInformation;
            userListAdapter.setUser(userInformation);
        });

        userListViewModel.getDbOperation().getAllDeviceInfo().observe(this,
                userListViewModel::setDeviceInfoList);
    }


    /**
     * Bind the RecyclerView with adapter.
     */
    private void bindRecyclerView() {
        if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
            userListAdapter = new UserListAdapter(SmartUsersFragment.this, true, context);
        } else {
            userListAdapter = new UserListAdapter(SmartUsersFragment.this, false, context);
        }
        userListAdapter.setUser(userInformationList);
        fragmentSmartUsersBinding.recyclerViewUser.setAdapter(userListAdapter);
        fragmentSmartUsersBinding.fabUser.setOnClickListener(this);
        fragmentSmartUsersBinding.recyclerViewUser.addOnItemTouchListener(new RecyclerItemClickListener(context, fragmentSmartUsersBinding.recyclerViewUser, new RecyclerItemClickListener.OnItemClickListener() {
            @Override
            public void onItemClick(View view, int position) {
                if (isMultiSelect) {
                    multi_select(position);
                } else {
//                    Toast.makeText(context, "Details Page", Toast.LENGTH_SHORT).show();
                }
            }

            @Override
            public void onItemLongClick(View view, int position) {
                if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
                    if (!isMultiSelect) {


                        multiselectUserList = new ArrayList<>();
                        isMultiSelect = true;

                        if (mActionMode == null) {
                            mActionMode = context.startActionMode(mActionModeCallback);
                        }


                    }

                    multi_select(position);

                } else {
                    showAlertForConnection();
                    isMultiSelect = false;
                }
            }

        }));


    }

    private void showAlertForConnection() {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(getString(R.string.connection_label));
        builder.setMessage(getString(R.string.connect_to_device));
        builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
            //if no device in db then start scanning activity
            if (fragmentSmartUsersBinding.getUser() != null && fragmentSmartUsersBinding.getUser().getDeviceInfoList().size() == 0) {
                Intent intent = new Intent(context, StartFGServiceActivity.class);
                context.startActivity(intent);

            } else {
                ((BaseActivity) Objects.requireNonNull(getActivity())).checkPermission();
            }
        });
        builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> dialogInterface.dismiss());
        builder.setCancelable(false);
        //2. now setup to change color of the button

        AlertDialog dialog = builder.create();

        dialog.setOnShowListener(arg0 -> {
            dialog.getButton(AlertDialog.BUTTON_NEGATIVE).setTextColor(context.getResources().getColor(R.color.icon_color));
            dialog.getButton(AlertDialog.BUTTON_POSITIVE).setTextColor(context.getResources().getColor(R.color.icon_color));
        });
        dialog.show();
        userListAdapter.notifyDataSetChanged();


    }

    /**
     * Called when any specific row item object click from User List adapter.
     * Contains BLE icon click,mac address click and show more button.
     */
    @Override
    public void onItemClick(Object item, View view) {
        UserInformation userInformation = (UserInformation) item;
        switch (view.getId()) {
            case R.id.delete_layout:
                // delete user locally and delete user from ble, sync needed?
                fragmentSmartUsersBinding.getUser().deleteUserFromBle(userInformation); // delete form remote end
                fragmentSmartUsersBinding.getUser().deleteUser(userInformation); //delete locally
                /*
                if (userInformation.get_id().equals("")) {
                    if (fragmentSmartUsersBinding.getUser() != null) {
                        fragmentSmartUsersBinding.getUser().deleteUser(userInformation);
                    }
                    EventBus.getDefault().post(new BleScanBackgroundServiceStartScanEvent(false));
                } else {
                    if (fragmentSmartUsersBinding.getUser() != null) {
                        fragmentSmartUsersBinding.getUser().deleteUserFromApi(userInformation);
                    }
                }
                */
                break;
            case R.id.relDetail:
                Intent intent = new Intent(context, CreateUserActivity.class);
                intent.putExtra(AppConstants.IS_EDIT_USER, false);
                intent.putExtra(AppConstants.USER_DATA, userInformation);
                startActivity(intent);
                break;
            case R.id.relSync:
                ((HomeActivity) Objects.requireNonNull(getActivity())).sendData(userInformation.getTrainingData());

                if (fragmentSmartUsersBinding.getUser() != null) {
                    ArrayList<UserInformation> userInformationsSingle = new ArrayList<>();
                    userInformationsSingle.add(userInformation);
                    fragmentSmartUsersBinding.getUser().updateDeviceListWithSelectedUsers(userInformationsSingle);
                }

                break;
            default:
                break;
        }

    }

    private void setUpNetworkError() {
        fragmentSmartUsersBinding.getUser().getInternetErrorMessage().observe(this, s -> showNoNetworkSnackbar());
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
            bindRecyclerView();
        }
    }

    /**
     * Ble status change event and performing operations in the current screen based on the same
     * @param bleStatusEvent: Event model
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void bleStatusEvent(BleStatusEvent bleStatusEvent) {
        int connectionState = bleStatusEvent.getStatus();
        BluetoothDevice device = bleStatusEvent.getBluetoothDevice();
        if (connectionState == BleService.STATE_LINK_LOSS
                || connectionState == BleService.STATE_DISCONNECTED
                || connectionState == BleService.STATE_DISCONNECTING) {
            bindRecyclerView();
        } else if (connectionState == BleService.STATE_CONNECTING) {

        } else if (connectionState == BleService.STATE_CONNECTED) {

        } else if (connectionState == BleService.STATE_DEVICE_READY) {
            bindRecyclerView();
        }
    }

    /**
     * Fab button click which is used to add more user.
     *
     * @param view {@link View}
     */
    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.fabUser:
                //Start the activity of training with skip button
                Intent createUserActivity = new Intent(context, CreateUserActivity.class);
                createUserActivity.putExtra(IS_EDIT_USER, false);
                startActivity(createUserActivity);
//                Intent startTrainingActivity = new Intent(context, TrainingActivity.class);
//                startTrainingActivity.putExtra(IS_SKIPPABLE, true);
//                startActivity(startTrainingActivity);

//                Intent intent = new Intent(context, CreateUserActivity.class);
//                startActivity(intent);
                break;
            default:
                break;
        }
    }

    /**
     * Bind menu items.
     *
     * @param menu     {@link Menu}
     * @param inflater {@link MenuInflater}
     */
    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        // Do something that differs the Activity's menu here
        inflater.inflate(R.menu.smartlocks_menu, menu);
        menu_search = menu.findItem(R.id.menu_search);
        menu_ble = menu.findItem(R.id.menu_ble);
        menu_ble.setVisible(false);
        SearchManager searchManager = (SearchManager) Objects.requireNonNull(getActivity()).getSystemService(Context.SEARCH_SERVICE);

        if (menu_search != null) {
            searchView = (SearchView) menu_search.getActionView();
        }
        if (searchView != null) {
            if (searchManager != null) {
                searchView.setSearchableInfo(searchManager.getSearchableInfo(getActivity().getComponentName()));
            }
            queryTextListener = new SearchView.OnQueryTextListener() {
                @Override
                public boolean onQueryTextChange(String newText) {
                    AppLogger.i("onQueryTextChange", newText);
                    userListAdapter.getFilter().filter(newText);
                    return true;
                }

                @Override
                public boolean onQueryTextSubmit(String query) {
                    AppLogger.i("onQueryTextSubmit", query);
                    userListAdapter.getFilter().filter(query);
                    return true;
                }


            };
            ImageView closeButton = searchView.findViewById(R.id.search_close_btn);
            closeButton.setImageResource(R.drawable.ic_clear);
            searchView.setOnQueryTextListener(queryTextListener);
            searchView.setOnSearchClickListener(v -> {
                fragmentSmartUsersBinding.fabUser.hide();
                EventBus.getDefault().post(new ChangeHomeIcon(true));
            });
            searchView.setOnCloseListener(() -> {
                fragmentSmartUsersBinding.fabUser.show();
                EventBus.getDefault().post(new ChangeHomeIcon(false));
                getActivity().invalidateOptionsMenu();
                return false;
            });
        }
        super.onCreateOptionsMenu(menu, inflater);
    }

    /**
     * Listener of menu items.
     *
     * @param item {@link MenuItem}
     * @return Boolean
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_search:
                // Not implemented here
                EventBus.getDefault().post(new ChangeHomeIcon(true));
                return false;
            case R.id.menu_ble:
                // Not implemented here
                if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
                    EventBus.getDefault().post(new BleScanBackgroundServiceStartScanEvent(false));
                } else {
                    EventBus.getDefault().post(new BleScanBackgroundServiceStartScanEvent(true));
                }
                return false;
            default:
                break;
        }
        searchView.setOnQueryTextListener(queryTextListener);
        return false;
    }

    /**
     * Event bus to hide search view on click of hamburger icons.
     *
     * @param changeHomeIcon {@link ChangeHomeIcon}
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void ChangeHomeIcon(ChangeHomeIcon changeHomeIcon) {
        if (!changeHomeIcon.isBackEnable()) {
            fragmentSmartUsersBinding.fabUser.show();
            searchView.onActionViewCollapsed();
        }
    }

    /**
     * Method for showing pop up menu with icons, this method uses reflection in order to call a
     * system level method which is not publicly accessible
     *
     * @param popupMenu: Menu object
     */
    private void setForceShowIcon(PopupMenu popupMenu) {
        try {
            Field[] fields = popupMenu.getClass().getDeclaredFields();
            for (Field field : fields) {
                if ("mPopup".equals(field.getName())) {
                    field.setAccessible(true);
                    Object menuPopupHelper = field.get(popupMenu);
                    Class<?> classPopupHelper = Class.forName(menuPopupHelper
                            .getClass().getName());
                    Method setForceIcons = classPopupHelper.getMethod(
                            "setForceShowIcon", boolean.class);
                    setForceIcons.invoke(menuPopupHelper, true);
                    break;
                }
            }
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onItemLongClick(Object item, View view) {


    }

    private void multi_select(int position) {
        //Todo check if the user has training data or not
        if (mActionMode != null && position >= 0) {
            if (!multiselectUserList.isEmpty() && multiselectUserList.contains(userInformationList.get(position)))
                multiselectUserList.remove(userInformationList.get(position));
            else {
                if (userInformationList.get(position).getTrainingData() != null && !multiselectUserList.contains(userInformationList.get(position))) {
                    multiselectUserList.add(userInformationList.get(position));
                } else {
                    showSnackbar(userInformationList.get(position).getName() + " does not have training data");
                }
            }

            if (multiselectUserList.size() > 0) {
                mActionMode.setTitle(getString(R.string.send_training_data_label));
                mActionMode.setSubtitle(mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS));
            } else {
                mActionMode.setTitle("");
                mActionMode.setSubtitle("");
                mActionMode.finish();

            }
            refreshAdapter();
        }
    }


    private void refreshAdapter() {
        userListAdapter.selected_usersList = multiselectUserList;
        userListAdapter.originalUserList = userInformationList;
        userListAdapter.notifyDataSetChanged();
    }


    private ActionMode.Callback mActionModeCallback = new ActionMode.Callback() {

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            // Inflate a menu resource providing context menu items
            MenuInflater inflater = mode.getMenuInflater();
            inflater.inflate(R.menu.user_list_action_menu, menu);
            context_menu = menu;
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            return false; // Return false if nothing is done
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
                case R.id.action_send:
                    try {


                        JSONObject mergedObj = new JSONObject();

                        for (UserInformation userInformation : multiselectUserList) {
                            if (null != userInformation.getTrainingData()) {
                                JSONObject jsonObj = new JSONObject(userInformation.getTrainingData());
                                Iterator i1 = jsonObj.keys();

                                while (i1.hasNext()) {
                                    String tmp_key = (String) i1.next();
                                    mergedObj.put(userInformation.getEmail(), jsonObj.get(tmp_key));
                                }


                            }

                        }
                        String resultJsonUsersTrainingData = mergedObj.toString();
                        ((HomeActivity) Objects.requireNonNull(getActivity())).sendData(resultJsonUsersTrainingData);
                        if (fragmentSmartUsersBinding.getUser() != null) {
                            fragmentSmartUsersBinding.getUser().updateDeviceListWithSelectedUsers(multiselectUserList);
                        }

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    mActionMode.finish();

                    return true;
                default:
                    return false;
            }
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            mActionMode = null;
            isMultiSelect = false;
            multiselectUserList = new ArrayList<>();
            refreshAdapter();

        }
    };




}
