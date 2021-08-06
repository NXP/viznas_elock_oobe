package com.nxp.facemanager.dagger.module;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import com.google.gson.GsonBuilder;
import com.nxp.facemanager.dagger.MySharedPreferences;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.utility.CommonUtils;
import com.nxp.facemanager.webservice.ApiInterface;

import javax.inject.Singleton;
import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;

import androidx.room.Room;
import dagger.Module;
import dagger.Provides;
import okhttp3.OkHttpClient;
import okhttp3.logging.HttpLoggingInterceptor;
import retrofit2.Retrofit;
import retrofit2.converter.gson.GsonConverterFactory;

import static com.nxp.facemanager.utility.AppConstants.BASE_URL_SECURE;

/**
 * A dagger module provides the way that constructs the objects which will be injected.
 * In order to define a dagger module, create a class and annotate it with
 * <p>
 * Module annotation and define the provider methods that return the instances.
 * Provider methods have to be annotated by @Provider annotation
 * <p>
 * We provide  Context,Retrofit,Room db, shared pref etc here. There is an important detail here.
 */
@Module
public class AppModule {


    /**
     * Application level context.
     *
     * @param application {@link com.nxp.facemanager.MyApplication}
     * @return Context
     */
    @Provides
    @Singleton
    Context provideContext(Application application) {
        return application;
    }

    /**
     * SharedPreference object creation.
     *
     * @param context {@link Context}
     * @return SharedPreference
     */
    @Provides
    @Singleton
    SharedPreferences providesSharedPreferences(Context context) {
        return PreferenceManager.getDefaultSharedPreferences(context);
    }

    /**
     * SharedPreference operation class creation.
     *
     * @param sharedPreferences {@link SharedPreferences}
     * @return MySharedPreferences
     */
    @Singleton
    @Provides
    MySharedPreferences provideObjectManager(SharedPreferences sharedPreferences) {
        return new MySharedPreferences(sharedPreferences);
    }

    /**
     * Room Database object creation.
     *
     * @param context {@link Context}
     * @return FaceDatabase
     */
    @Provides
    @Singleton
    FaceDatabase providesDataBaseManager(Context context) {
        return Room.databaseBuilder(context, FaceDatabase.class, AppConstants.DB_NAME).build();
    }


    /**
     * Retrofit api reference.
     *
     * @return ApiInterface
     */
    @Provides
    @Singleton
    ApiInterface providesApiManager(Context context) {

        HttpLoggingInterceptor logging = new HttpLoggingInterceptor();
        logging.setLevel(HttpLoggingInterceptor.Level.BODY);
        OkHttpClient.Builder httpClient = new OkHttpClient.Builder();
        httpClient.addInterceptor(logging);
        SSLContext sslContext = CommonUtils.setCaCert(context);
        httpClient.sslSocketFactory(sslContext.getSocketFactory());
        httpClient.hostnameVerifier(new HostnameVerifier() {
            @Override
            public boolean verify(String hostname, SSLSession session) {
                boolean value = true;
                return value;
            }
        });

        Retrofit retrofit = new Retrofit.Builder()
                .baseUrl(BASE_URL_SECURE)
                .client(httpClient.build())
                .addConverterFactory(GsonConverterFactory.create(new GsonBuilder().excludeFieldsWithoutExposeAnnotation().create()))
                .build();
        return retrofit.create(ApiInterface.class);
    }
}
