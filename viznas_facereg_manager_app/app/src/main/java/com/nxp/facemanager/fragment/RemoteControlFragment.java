package com.nxp.facemanager.fragment;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.AnimationDrawable;
import android.net.Uri;
import android.os.Bundle;

import androidx.databinding.DataBindingUtil;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;
import androidx.lifecycle.ViewModelProviders;

import android.os.Handler;
import android.os.SystemClock;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.nxp.facemanager.R;
import com.nxp.facemanager.SocketSend;
import com.nxp.facemanager.activity.HomeActivity;
import com.nxp.facemanager.ble.BleReceiveDataModel;
import com.nxp.facemanager.ble.BleReceiveListener;
import com.nxp.facemanager.ble.BleScanningService;
import com.nxp.facemanager.ble.BleSendDataModel;
import com.nxp.facemanager.ble.UARTService;
import com.nxp.facemanager.databinding.FragmentRemoteControlBinding;
import com.nxp.facemanager.utility.AppConstants;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.time.Instant;
import java.util.ArrayList;

import static com.nxp.facemanager.ble.BleScanningService.convertStringToByteArrayChunks;

/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * {@link RemoteControlFragment.OnFragmentInteractionListener} interface
 * to handle interaction events.
 * Use the {@link RemoteControlFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class RemoteControlFragment extends BaseFragment {
    private ImageView mLock;
    private TextView mDevice;
    private TextView mAddress;
    private static final String TAG = "RemoteControlFragment";
    String mConnectionType;
    String mSmartLockIP;
    private SocketSend mNetwork;

    private OnFragmentInteractionListener mListener;
    AnimationDrawable unlockAnim;

    FragmentRemoteControlBinding fragmentRemoteControlBinding;
    public RemoteControlFragment() {
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.

     * @return A new instance of fragment RemoteControlFragment.
     */
    // TODO: Rename and change types and number of parameters
    public static RemoteControlFragment newInstance() {
        return new RemoteControlFragment();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
        mConnectionType = mySharedPreferences.getStringData(AppConstants.CONNECTION_TYPE);
        mSmartLockIP = mySharedPreferences.getStringData(AppConstants.SMARTLOCK_IP);
        Log.d(TAG, "++" + "connection[" + mConnectionType + ":" + mSmartLockIP + "]");

    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        fragmentRemoteControlBinding = DataBindingUtil.inflate(inflater, R.layout.fragment_remote_control, container, false);
        View v = inflater.inflate(R.layout.fragment_remote_control, container, false);

        mAddress = (TextView) v.findViewById(R.id.txAddr);
        mDevice = (TextView) v.findViewById(R.id.txDev);
        mLock = (ImageView) v.findViewById(R.id.imLock);
//        mLock.setImageResource(R.drawable.ic_lock);
        mLock.setBackgroundResource(R.drawable.unlock_anim);
        unlockAnim=(AnimationDrawable)mLock.getBackground();

        mDevice.setText(AppConstants.SMARTLOCK_NAME_DEFAULT);
        mAddress.setText(AppConstants.SMARTLOCK_IP);

        mLock.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onOpenReq();
                Log.e(TAG, "unlock req sent");
            }


        });
        return v;
    }

    BleReceiveListener bleReceiveListener = new BleReceiveListener() {
        @Override
        public void onDataReceive(BleReceiveDataModel bleReceiveDataModel) {
            unlockAnim.start();
        }

        @Override
        public void onFailure() {
            CharSequence text = "Couldn't unlock...";
            int duration = Toast.LENGTH_SHORT;
            Toast toast = Toast.makeText(getContext(), text, duration);
            toast.show();
        }
    };

    private void onOpenReq(){
        Log.d("OPEN", "++" + "by ble");
        BleSendDataModel bleSendDataModel = new BleSendDataModel(
                AppConstants.OPEN_DOOR_REQUEST,
                "open door true"
        );
        bleSendDataModel.setBleReceiveListener(bleReceiveListener);
        EventBus.getDefault().post(bleSendDataModel);

        final Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                bleReceiveListener.onFailure();
                bleSendDataModel.setBleReceiveListener(null);
            }
        }, AppConstants.BLE_CB_WAIT_INTERVAL);
    }

 // send remote control req
//     public boolean remoteReqSend() {
//
//            String payload = "";
//            if (mConnectionType.compareTo("connection_network") == 0) {
//                Log.d(TAG, "++" + "by network");
//                int loop_count = 0;
//                mNetwork = new SocketSend(payload, mSmartLockIP, AppConstants.SMARTLOCK_PORT_DEFAULT);
//                while(mNetwork.mSocket == null || !mNetwork.mSocket.isConnected()) {
//                    loop_count++;
//                    if (loop_count > 5) {
//                        return false;
//                    }
//                    SystemClock.sleep(1000);
//                }
//                mNetwork.send();
//            } else {
//                Log.d(TAG, "++" + "by ble");
//                BleSendDataModel bleSendDataModel = new BleSendDataModel(
//                        AppConstants.OPEN_DOOR_REQUEST,
//                        "open door true"
//                );
//                EventBus.getDefault().post(bleSendDataModel);
//                // wait for response?
//
//    //            byte[] byteArray = payload.getBytes();
//                /*Log.e(TAG, ">>sendData:" + byteArray.length);
//                for (int i = 0; i < 46; i++) {
//                    Log.e(TAG, "[" + byteTo16(byteArray[i])  + "]");
//                }
//                Log.e(TAG, "<<sendData");*/
//            }
//            return true;
//        }

    @Subscribe
    public void bleReceiveDataModel(BleReceiveDataModel bleReceiveDataModel){
        if (bleReceiveDataModel.getCmd() == AppConstants.OPEN_DOOR_RESPONSE){
            // change UI icon on remote control success
            if (bleReceiveDataModel.isSuccess()) {
                bleReceiveListener.onDataReceive(bleReceiveDataModel);
            } else{
                bleReceiveListener.onFailure();
            }
        }
    }


    // TODO: Rename method, update argument and hook method into UI event
    public void onButtonPressed(Uri uri) {
        if (mListener != null) {
            mListener.onFragmentInteraction(uri);
        }
    }

//    @Override
//    public void onAttach(Context context) {
//        super.onAttach(context);
//        if (context instanceof OnFragmentInteractionListener) {
//            mListener = (OnFragmentInteractionListener) context;
//        } else {
//            throw new RuntimeException(context.toString()
//                    + " must implement OnFragmentInteractionListener");
//        }
//    }

    @Override
    public void onDetach() {
        super.onDetach();
        mListener = null;
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
        // TODO: Update argument type and name
        void onFragmentInteraction(Uri uri);
    }

}
