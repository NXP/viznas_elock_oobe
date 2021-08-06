package com.nxp.facemanager.dagger.module;

import com.nxp.facemanager.ble.BleScanningService;

import dagger.Module;
import dagger.android.ContributesAndroidInjector;

@Module
public abstract class ServiceBuilderModule {

    @ContributesAndroidInjector
    abstract BleScanningService contributeBleScanningService();

}