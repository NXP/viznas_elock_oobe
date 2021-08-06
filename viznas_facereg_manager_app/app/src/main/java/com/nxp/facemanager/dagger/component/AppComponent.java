package com.nxp.facemanager.dagger.component;


import android.app.Application;

import com.nxp.facemanager.MyApplication;
import com.nxp.facemanager.dagger.module.ActivityBuilderModule;
import com.nxp.facemanager.dagger.module.AppModule;
import com.nxp.facemanager.dagger.module.FragmentBuilderModule;
import com.nxp.facemanager.dagger.module.ServiceBuilderModule;

import javax.inject.Singleton;

import dagger.BindsInstance;
import dagger.Component;
import dagger.android.AndroidInjector;
import dagger.android.support.AndroidSupportInjectionModule;

/**
 * A dagger component can be seen as an intermediate object which allows accessing to objects defined in Dagger modules.
 */
@Singleton
@Component(modules = {AppModule.class, AndroidSupportInjectionModule.class, ActivityBuilderModule.class, FragmentBuilderModule.class, ServiceBuilderModule.class})
public interface AppComponent extends AndroidInjector<MyApplication> {

    @Component.Builder
    interface Builder {

        @BindsInstance
        Builder application(Application application);

        AppComponent build();
    }

    void inject(MyApplication app);
}

