package com.nxp.facemanager.fragment;

import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.view.View;

import com.google.android.material.snackbar.Snackbar;
import com.nxp.facemanager.R;
import com.nxp.facemanager.activity.BaseActivity;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.webservice.ApiInterface;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import javax.inject.Inject;

import androidx.core.content.ContextCompat;
import dagger.android.support.DaggerFragment;

/**
 * Base abstract class to be extended by all fragment elements. It consists of dagger injected
 * objects of faceDatabase, sharedprefs which will be readily available in child classes of it
 */
public abstract class BaseFragment extends DaggerFragment {
    protected final String TRACE_TAG = "BaseFragment";
    protected BaseActivity context;

    @Inject
    FaceDatabase faceDatabase;
    String userData;


    @Inject
    ApiInterface apiInterface;

    @Inject
    MySharedPreferences mySharedPreferences;

    public BaseFragment() {
        // Required empty public constructor
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        this.context = (BaseActivity) context;
    }

    @Override
    public void onStart() {
        super.onStart();
        EventBus.getDefault().register(this);
    }

    @Override
    public void onStop() {
        EventBus.getDefault().unregister(this);
        super.onStop();
    }

    @Subscribe
    public void genericEvent(Object event) {
        // DO NOT code here, it is a generic catch event method
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }

    protected void showSnackbar(final String text) {

        View rootView = context.getWindow().getDecorView().findViewById(android.R.id.content);
        Snackbar.make(rootView, text, Snackbar.LENGTH_SHORT).show();


    }

    protected void showNoNetworkSnackbar() {


        View rootView = context.getWindow().getDecorView().findViewById(android.R.id.content);
        Snackbar.make(rootView, getString(R.string.check_internet), Snackbar.LENGTH_SHORT)
                .setAction(getString(R.string.setting_label), v -> startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS)))
                .setActionTextColor(ContextCompat.getColor(context, R.color.icon_color)).show();


    }


}
