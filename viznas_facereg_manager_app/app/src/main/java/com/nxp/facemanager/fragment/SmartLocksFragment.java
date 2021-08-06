package com.nxp.facemanager.fragment;

import android.app.SearchManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.ActionProvider;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.google.android.material.snackbar.Snackbar;
import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.BaseActivity;
import com.nxp.facemanager.activity.CreateUserActivity;
import com.nxp.facemanager.activity.DeviceScanActivity;
import com.nxp.facemanager.activity.OnItemClickListener;
import com.nxp.facemanager.activity.SocketActivity;
import com.nxp.facemanager.activity.StartFGServiceActivity;
import com.nxp.facemanager.activity.adapter.DeviceListAdapter;
import com.nxp.facemanager.ble.BleScanningService;
import com.nxp.facemanager.ble.BleSendDataModel;
import com.nxp.facemanager.ble.BleService;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.databinding.FragmentSmartLocksBinding;
import com.nxp.facemanager.model.BleEnableEvent;
import com.nxp.facemanager.model.BleScanBackgroundServiceStartScanEvent;
import com.nxp.facemanager.model.BleServiceDisableEvent;
import com.nxp.facemanager.model.BleStatusEvent;
import com.nxp.facemanager.model.ChangeHomeIcon;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.utility.SwipeToDeleteCallback;
import com.nxp.facemanager.viewModels.DeviceInformationListViewModel;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import androidx.annotation.NonNull;
import androidx.appcompat.widget.SearchView;
import androidx.core.content.ContextCompat;
import androidx.databinding.DataBindingUtil;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProviders;
import androidx.recyclerview.widget.ItemTouchHelper;
import androidx.recyclerview.widget.RecyclerView;

import static com.nxp.facemanager.viewModels.CreateUserViewModel.IS_EDIT_USER;

/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * {@link SmartLocksFragment.OnFragmentInteractionListener} interface
 * to handle interaction events.
 * Use the {@link SmartLocksFragment#newInstance} factory method to
 * create an instance of this fragment.
 * This class contains methods for :
 * 1. Add new device
 * 2. Connect disconnect to the device added
 */
public class SmartLocksFragment extends BaseFragment implements OnItemClickListener, View.OnClickListener {

    /**
     * Object of smart lock xml binding reference.
     */
    private FragmentSmartLocksBinding fragmentSmartLocksBinding;
    /**
     * Adapter reference of load device list.
     */
    private DeviceListAdapter deviceListAdapter;
    /**
     * Search view object for menu item.
     */
    private SearchView searchView = null;
    /***
     * Listener for the Search view.
     */
    private SearchView.OnQueryTextListener queryTextListener;
    private List<DeviceInfo> deviceInfoList = new ArrayList<>();
    /**
     * Menu items for search and ble connection.
     */
    private MenuItem menu_search;
    private MenuItem menu_ble;

    private boolean isMenuClick = false;

    public SmartLocksFragment() {
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment SmartLocksFragment.
     */
    public static SmartLocksFragment newInstance() {
        return new SmartLocksFragment();
    }

    /**
     * Default method.
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
    }

    /**
     * Default method.
     *
     * @param inflater           {@link LayoutInflater}
     * @param container          {@link ViewGroup}
     * @param savedInstanceState {@link Bundle}
     * @return View
     */
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        fragmentSmartLocksBinding = DataBindingUtil.inflate(inflater, R.layout.fragment_smart_locks, container, false);
        View view = fragmentSmartLocksBinding.getRoot();
        DeviceInformationListViewModel deviceInformationListViewModel = ViewModelProviders.of(this, new ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(DeviceInformationListViewModel.class);
        fragmentSmartLocksBinding.setDevice(deviceInformationListViewModel);
        deviceInformationListViewModel.setContext(getActivity());
        fragmentSmartLocksBinding.fabDevice.setOnClickListener(this);
        fragmentSmartLocksBinding.swipeContainer.setColorSchemeResources(R.color.icon_color);
        bindRecyclerView();
        setUpNetworkError();
        //fragmentSmartLocksBinding.getDevice().getDeviceListFromCloud();
        observer(deviceInformationListViewModel);
        return view;
    }

    /**
     * Observer and manage and the menu and toolbar based on data in database.
     *
     * @param deviceInformationListViewModel {@link DeviceInformationListViewModel}
     */
    private void observer(DeviceInformationListViewModel deviceInformationListViewModel) {
        deviceInformationListViewModel.getDeviceList().observe(this, deviceInfo -> {
            deviceInfoList = deviceInfo;
            String mac_address = mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS);
            if (!mac_address.isEmpty()) {
                if (menu_ble != null) menu_ble.setIcon(R.drawable.ic_bluetooth_disabled);
                deviceListAdapter = new DeviceListAdapter(SmartLocksFragment.this, mac_address);
            } else {
                if (menu_ble != null) menu_ble.setIcon(R.drawable.ic_bluetooth_searching);
                deviceListAdapter = new DeviceListAdapter(SmartLocksFragment.this, mac_address);
            }
            fragmentSmartLocksBinding.recyclerViewDevice.setAdapter(deviceListAdapter);
            deviceListAdapter.setDevice(deviceInfoList);
            if (deviceInfo != null && deviceInfo.size() > 0) {
                if (menu_ble != null) menu_ble.setVisible(true);
            } else {
                if (menu_ble != null) menu_ble.setVisible(false);
            }
        });
    }

    /**
     * Bind the RecyclerView with adapter.
     */
    private void bindRecyclerView() {
        String mac_address = mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS);
        if (!mac_address.isEmpty()) {
            deviceListAdapter = new DeviceListAdapter(SmartLocksFragment.this, mac_address);
        } else {
            deviceListAdapter = new DeviceListAdapter(SmartLocksFragment.this, mac_address);
        }
        deviceListAdapter.setDevice(deviceInfoList);
        fragmentSmartLocksBinding.recyclerViewDevice.setAdapter(deviceListAdapter);

    }

    /**
     * Called when any specific row item object click from User List adapter.
     * Contains BLE icon click,mac address click and show more button.
     */
    @Override
    public void onItemClick(Object item, View viewId) {
        DeviceInfo deviceInfo = (DeviceInfo) item;
        switch (viewId.getId()) {
            case R.id.delete_layout:
                if (deviceInfo.get_id().equals("")) {
                    fragmentSmartLocksBinding.getDevice().delete(deviceInfo);
                    EventBus.getDefault().post(new BleScanBackgroundServiceStartScanEvent(false));
                } else {
                    fragmentSmartLocksBinding.getDevice().deleteDevice(deviceInfo);
                }

                break;
            case R.id.edit_layout:
                Intent testIntent = new Intent(context, CreateUserActivity.class);
                testIntent.putExtra(IS_EDIT_USER, false);
                // TODO: change back to device info handling


            default:
                break;
        }
    }

    /**
     * Fab button click which is used to add more device.
     *
     * @param view {@link View}
     */
    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.fabDevice:
                Intent scanActivity = new Intent(context, StartFGServiceActivity.class);
                startActivity(scanActivity);
//                ((BaseActivity) Objects.requireNonNull(getActivity())).checkPermission();
                break;
            default:
                break;
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        showAndHideMenuItems();
    }

    private void showAndHideMenuItems() {
        if (deviceListAdapter != null && deviceListAdapter.getItemCount() > 0) {
            if (menu_ble != null) {
                if (!mySharedPreferences.getStringData(AppConstants.CONNECTED_MAC_ADDRESS).isEmpty()) {
                    menu_ble.setIcon(R.drawable.ic_bluetooth_disabled);
                    bindRecyclerView();
                } else {
                    menu_ble.setIcon(R.drawable.ic_bluetooth_searching);
                    bindRecyclerView();
                }
                menu_ble.setVisible(true);
            }
        } else {
            if (menu_ble != null) {
                menu_ble.setVisible(false);
            }
        }
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
            showAndHideMenuItems();
        }
    }

    /**
     * Eventbus for ble enable event.
     *
     * @param bleEnableEvent boolean flag that event handle condition of ble enabled.
     */
    @Subscribe
    public void bleEnableEvent(BleEnableEvent bleEnableEvent) {
        if (bleEnableEvent != null && bleEnableEvent.isBleOn()) {
            if (isMenuClick) {

                isMenuClick = false;
                if (menu_ble != null && Objects.equals(menu_ble.getIcon().getConstantState(), getResources().getDrawable(R.drawable.ic_bluetooth_searching).getConstantState())) {
                    menu_ble.setIcon(R.drawable.ic_bluetooth_disabled);
                    BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                    BluetoothDevice bluetoothDevice = null;
                    if (mBluetoothAdapter != null) {
                        bluetoothDevice = mBluetoothAdapter.getRemoteDevice(deviceListAdapter.getFilterDeviceList().get(0).getMac_address());
                    }


                    BleScanBackgroundServiceStartScanEvent event = new BleScanBackgroundServiceStartScanEvent(true);
                    event.setBluetoothDevice(bluetoothDevice);
                    EventBus.getDefault().post(event);
                } else if (menu_ble != null && Objects.equals(menu_ble.getIcon().getConstantState(), getResources().getDrawable(R.drawable.ic_bluetooth_disabled).getConstantState())) {
                    bindRecyclerView();
                    menu_ble.setIcon(R.drawable.ic_bluetooth_searching);
                    EventBus.getDefault().post(new BleScanBackgroundServiceStartScanEvent(false));
                }
            } else {
                //if (deviceListAdapter.getItemCount() == 0) {
                startActivity(new Intent(context, StartFGServiceActivity.class));
                if (!((BaseActivity) Objects.requireNonNull(getActivity())).isMyServiceRunning(BleScanningService.class)) {
                    ((BaseActivity) Objects.requireNonNull(getActivity())).startAutoConnectService(null);
                }

            }
        }
    }

    @SuppressWarnings("StatementWithEmptyBody")
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void bleStatusEvent(BleStatusEvent bleStatusEvent) {
        int connectionState = bleStatusEvent.getStatus();
        BluetoothDevice device = bleStatusEvent.getBluetoothDevice();
        if (connectionState == BleService.STATE_LINK_LOSS
                || connectionState == BleService.STATE_DISCONNECTING) {
            showAndHideMenuItems();
            if (null != menu_ble) {
                menu_ble.setEnabled(true);
            }
        } else if (connectionState == BleService.STATE_DISCONNECTED) {
            showAndHideMenuItems();
            if (null != menu_ble) {
                menu_ble.setEnabled(true);
            }
        } else if (connectionState == BleService.STATE_CONNECTING) {
            if (null != menu_ble) {
                menu_ble.setEnabled(false);
            }

        } else if (connectionState == BleService.STATE_CONNECTED) {

        } else if (connectionState == BleService.STATE_DEVICE_READY) {
            observer(fragmentSmartLocksBinding.getDevice());
            showAndHideMenuItems();
            sendData(AppConstants.USER_INFORMATION);
            if (null != menu_ble) {
                menu_ble.setEnabled(true);
            }
        }
    }

    /**
     * Bind the menu items for Smart locks list screen.
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
        SearchManager searchManager = (SearchManager) Objects.requireNonNull(getActivity()).getSystemService(Context.SEARCH_SERVICE);
        showAndHideMenuItems();
        if (menu_search != null) {
            searchView = (SearchView) menu_search.getActionView();
        }
        if (searchView != null) {
            if (searchManager != null) {
                searchView.setSearchableInfo(searchManager.getSearchableInfo(getActivity().getComponentName()));
            }
            ImageView closeButton = searchView.findViewById(R.id.search_close_btn);
            closeButton.setImageResource(R.drawable.ic_clear);
            queryTextListener = new SearchView.OnQueryTextListener() {
                @Override
                public boolean onQueryTextChange(String newText) {
                    AppLogger.i("onQueryTextChange", newText);
                    deviceListAdapter.getFilter().filter(newText);
                    return true;
                }

                @Override
                public boolean onQueryTextSubmit(String query) {
                    AppLogger.i("onQueryTextSubmit", query);
                    deviceListAdapter.getFilter().filter(query);
                    return true;
                }


            };
            searchView.setOnQueryTextListener(queryTextListener);
            searchView.setOnSearchClickListener(v -> {
                menu_ble.setVisible(false);
                fragmentSmartLocksBinding.fabDevice.hide();
                EventBus.getDefault().post(new ChangeHomeIcon(true));
            });
            searchView.setOnCloseListener(() -> {
                fragmentSmartLocksBinding.fabDevice.show();
                EventBus.getDefault().post(new ChangeHomeIcon(false));
                return false;
            });
        }
        super.onCreateOptionsMenu(menu, inflater);
    }

    /**
     * Listener for menu items.
     *
     * @param item {@link MenuItem}
     * @return Boolean
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_search:
                // Not implemented here
                return false;
            case R.id.menu_ble:
                isMenuClick = true;
                ((BaseActivity) Objects.requireNonNull(getActivity())).checkPermission();
                return false;
            default:
                break;
        }
        searchView.setOnQueryTextListener(queryTextListener);
        return false;
    }


    @Override
    public void onDetach() {
        super.onDetach();
//        mListener = null;
    }

    /**
     * Event bus for the show and hide hamburger icon when search view is visible.
     *
     * @param changeHomeIcon {@link ChangeHomeIcon}
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void ChangeHomeIcon(ChangeHomeIcon changeHomeIcon) {
        if (!changeHomeIcon.isBackEnable()) {
            Objects.requireNonNull(getActivity()).invalidateOptionsMenu();
            fragmentSmartLocksBinding.fabDevice.show();
            showAndHideMenuItems();
            searchView.onActionViewCollapsed();
        }
    }


    /**
     * This interface must be implemented by activities that contain this
     * fragment to allow an interaction in this fragment to be communicated
     * to the activity and potentially other fragments contained in that
     * activity.
     * <p>
     * See the Android Training lesson <a href=
     * "http://developer.android.com/training/basics/fragments/communicating.html"
     * >Communicating with Other Fragments</a> for more information.
     */
    public interface OnFragmentInteractionListener {
        void onFragmentInteraction(Uri uri);
    }


    /**
     * Implement swipe to delete functionality.
     */
    private void enableSwipeToDeleteAndUndo() {
        SwipeToDeleteCallback swipeToDeleteCallback = new SwipeToDeleteCallback(context) {
            @Override
            public void onSwiped(@NonNull RecyclerView.ViewHolder viewHolder, int i) {
                final int position = viewHolder.getAdapterPosition();
                final DeviceInfo item = deviceListAdapter.getFilterDeviceList().get(position);
//                deviceListAdapter.removeItem(position);
                Snackbar snackbar = Snackbar
                        .make(fragmentSmartLocksBinding.root, "Are you sure you want to delete this " + item.getDevice_name() + "?", Snackbar.LENGTH_LONG);
                snackbar.setAction(getString(R.string.yes), view -> {
//                    deviceListAdapter.restoreItem(item, position);
//                    fragmentSmartLocksBinding.recyclerViewDevice.scrollToPosition(position);
                    if (item.get_id().equals("")) {
                        deviceListAdapter.removeItem(position);
                        fragmentSmartLocksBinding.recyclerViewDevice.scrollToPosition(position);
                    } else {
                        fragmentSmartLocksBinding.getDevice().deleteDevice(item);
                    }
                });

                snackbar.setAction(getString(R.string.no), view -> snackbar.dismiss());
                snackbar.getView().setBackgroundColor(ContextCompat.getColor(context, R.color.statusbar_color));
                snackbar.setActionTextColor(ContextCompat.getColor(context, R.color.white));
                snackbar.show();
            }
        };

        ItemTouchHelper itemTouchhelper = new ItemTouchHelper(swipeToDeleteCallback);
        itemTouchhelper.attachToRecyclerView(fragmentSmartLocksBinding.recyclerViewDevice);
    }

    private void sendData(String requestType) {
        if (requestType.equalsIgnoreCase(AppConstants.USER_INFORMATION)) {
            DeviceInfo deviceInfo = deviceListAdapter.getFilterDeviceList().get(0);
            if (deviceInfo != null) {
                BleSendDataModel bleSendDataModel = new BleSendDataModel(
                        AppConstants.READY_INDICATION,
                        deviceInfo.getDevice_name() + deviceInfo.getPass_code()
                );
//                bleSendDataModel.setCommand(AppConstants.USER_INFORMATION);
//                bleSendDataModel.setDeviceName(deviceInfo.getDevice_name());
//                bleSendDataModel.setPass_code("" + deviceInfo.getPass_code());
//                bleSendDataModel.setCookie("");
//                EventBus.getDefault().post(bleSendDataModel);
            }
        }
    }

    private void setUpNetworkError() {
        fragmentSmartLocksBinding.getDevice().getInternetErrorMessage().observe(this, s -> showNoNetworkSnackbar());
    }

}
