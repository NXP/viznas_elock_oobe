package com.nxp.facemanager;

import com.nxp.facemanager.dagger.component.AppComponent;
import com.nxp.facemanager.dagger.component.DaggerAppComponent;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.utility.AppLogger;

import dagger.android.AndroidInjector;
import dagger.android.DaggerApplication;

/**
 * Android Application class where we can declare the thing which will be used as global level
 */
public class MyApplication extends DaggerApplication {
    /**
     * Store User information when user press any object from {@link UserInformation} list.
     */
    private UserInformation userInformation;

    /**
     * Initialize of application class.
     * Here we can declare all thing which are needed at application level.
     */
    @Override
    public void onCreate() {
        super.onCreate();
        AppLogger.init();
    }

    /**
     * Dagger AndroidInjector
     *
     * @return AndroidInjector
     */
    @Override
    protected AndroidInjector<? extends DaggerApplication> applicationInjector() {
        AppComponent appComponent = DaggerAppComponent.builder().application(this).build();
        appComponent.inject(this);
        return appComponent;
    }

    /**
     * {@link UserInformation} which is set by user when press on {@link UserInformation} list in {@link com.nxp.facemanager.activity.DashboardActivity}
     *
     * @return UserInformation
     */
    public UserInformation getUserInformation() {
        return userInformation;
    }

    /**
     * Store {@link UserInformation} from {@link com.nxp.facemanager.activity.DashboardActivity}.
     *
     * @param userInformation {@link UserInformation}
     */
    public void setUserInformation(UserInformation userInformation) {
        this.userInformation = userInformation;
    }
}
