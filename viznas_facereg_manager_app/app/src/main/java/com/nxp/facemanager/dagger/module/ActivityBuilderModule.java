package com.nxp.facemanager.dagger.module;

import com.nxp.facemanager.activity.BaseActivity;
import com.nxp.facemanager.activity.CreateUserActivity;
import com.nxp.facemanager.activity.DashboardActivity;
import com.nxp.facemanager.activity.DeviceScanActivity;
import com.nxp.facemanager.activity.HomeActivity;
import com.nxp.facemanager.activity.LoginActivity;
import com.nxp.facemanager.activity.RegistrationActivity;
import com.nxp.facemanager.activity.SplashActivity;
import com.nxp.facemanager.activity.StartFGServiceActivity;
import com.nxp.facemanager.activity.TrainingActivity;
import com.nxp.facemanager.activity.TrainingDataSendActivity;

import dagger.Module;
import dagger.android.ContributesAndroidInjector;

/**
 * We map all our activities here and Dagger know our activities in compile time.
 */
@Module
public abstract class ActivityBuilderModule {

    @ContributesAndroidInjector()
    abstract BaseActivity bindBaseActivity();

    @ContributesAndroidInjector()
    abstract LoginActivity bindLoginActivity();

    @ContributesAndroidInjector()
    abstract CreateUserActivity bindCreateUserActivity();

    @ContributesAndroidInjector()
    abstract SplashActivity bindSplashActivity();


    @ContributesAndroidInjector()
    abstract RegistrationActivity bindRegistrationActivity();

    @ContributesAndroidInjector()
    abstract TrainingActivity bindTrainingActivity();

    @ContributesAndroidInjector()
    abstract DeviceScanActivity bindDeviceScanActivity();

    @ContributesAndroidInjector()
    abstract HomeActivity bindHomeAtivity();

    @ContributesAndroidInjector()
    abstract DashboardActivity bindDashboardAtivity();

    @ContributesAndroidInjector()
    abstract TrainingDataSendActivity bindTrainingDataSendAtivity();

    @ContributesAndroidInjector()
    abstract StartFGServiceActivity bindStartFGServiceActivity();

}

