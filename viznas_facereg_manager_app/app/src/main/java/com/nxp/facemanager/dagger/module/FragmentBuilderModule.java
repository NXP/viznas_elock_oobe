package com.nxp.facemanager.dagger.module;

import com.nxp.facemanager.fragment.BaseFragment;
import com.nxp.facemanager.fragment.ChangePasswordFragment;
import com.nxp.facemanager.fragment.HelpFragment;
import com.nxp.facemanager.fragment.RemoteControlFragment;
import com.nxp.facemanager.fragment.SmartLocksFragment;
import com.nxp.facemanager.fragment.SmartUsersFragment;

import dagger.Module;
import dagger.android.ContributesAndroidInjector;

/**
 * We map all our activities here and Dagger know our fragment in compile time.
 */
@Module
public abstract class FragmentBuilderModule {

    @ContributesAndroidInjector()
    abstract BaseFragment bindBaseFragment();

    @ContributesAndroidInjector()
    abstract SmartLocksFragment bindSmartLocksFragment();

    @ContributesAndroidInjector()
    abstract SmartUsersFragment bindSmartUsersFragment();

    @ContributesAndroidInjector()
    abstract ChangePasswordFragment bindChangePasswordFragment();

    @ContributesAndroidInjector()
    abstract HelpFragment bindHelpFragment();

    @ContributesAndroidInjector()
    abstract RemoteControlFragment remoteControlFragment();


}

