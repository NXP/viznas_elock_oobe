package com.nxp.facemanager.dagger;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;

import com.nxp.facemanager.R;
import com.nxp.facemanager.utility.AppConstants;

import java.security.Key;

import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

/**
 * SharedPreference class for storing and retrieving data.
 */
public class MySharedPreferences {

    /**
     * Local reference of {@link SharedPreferences}
     */
    private final SharedPreferences mSharedPreferences;

    /**
     * Constructor to access this class.
     *
     * @param mSharedPreferences {@link SharedPreferences}
     */
    public MySharedPreferences(SharedPreferences mSharedPreferences) {
        this.mSharedPreferences = mSharedPreferences;
    }

    /**
     * Called when we want to insert data in {@link SharedPreferences} below mention formate.
     *
     * @param key  {@link com.nxp.facemanager.utility.AppConstants} key
     * @param data Value
     */
    public void putData(String key, Object data) {
        if (data instanceof String) {
            mSharedPreferences.edit().putString(key, (String) data).apply();
        } else if (data instanceof Integer) {
            mSharedPreferences.edit().putInt(key, (Integer) data).apply();
        } else if (data instanceof Boolean) {
            mSharedPreferences.edit().putBoolean(key, (Boolean) data).apply();
        } else if (data instanceof Float) {
            mSharedPreferences.edit().putFloat(key, (Float) data).apply();
        } else if (data instanceof Long) {
            mSharedPreferences.edit().putLong(key, (Long) data).apply();
        }
    }

    /**
     * Reset Shared Preference data.
     */
    public void resetPreference() {
        mSharedPreferences.edit().clear().apply();
    }

    /**
     * Get {@link SharedPreferences} for integer value.
     *
     * @param key {@link com.nxp.facemanager.utility.AppConstants} key
     * @return value
     */
    public int getIntData(String key) {
        return mSharedPreferences.getInt(key, 0);
    }

    /**
     * Get {@link SharedPreferences} for Long value.
     *
     * @param key {@link com.nxp.facemanager.utility.AppConstants} key
     * @return value
     */
    public long getLongData(String key) {
        return mSharedPreferences.getLong(key, 0L);
    }

    /**
     * Get {@link SharedPreferences} for Float value.
     *
     * @param key {@link com.nxp.facemanager.utility.AppConstants} key
     * @return value
     */
    public Float getFloatData(String key) {
        return mSharedPreferences.getFloat(key, 0f);
    }

    /**
     * Get {@link SharedPreferences} for String value.
     *
     * @param key {@link com.nxp.facemanager.utility.AppConstants} key
     * @return value
     */
    public String getStringData(String key) {
        return mSharedPreferences.getString(key, "");
    }

    /**
     * Get {@link SharedPreferences} for Boolean value.
     *
     * @param key {@link com.nxp.facemanager.utility.AppConstants} key
     * @return value
     */
    public Boolean getBooleanData(String key) {
        return mSharedPreferences.getBoolean(key, false);
    }


    public boolean encryptKey(Context context, String value) {
        try {
            String key = context.getString(R.string.app_name); // 128 bit key
            // Create key and cipher
            Key aesKey = new SecretKeySpec(key.getBytes(), "AES");
            @SuppressLint("GetInstance") Cipher cipher = Cipher.getInstance("AES");
            // encrypt the text
            cipher.init(Cipher.ENCRYPT_MODE, aesKey);
            byte[] encrypted = cipher.doFinal(value.getBytes());
            putData(AppConstants.TOKEN, new String(encrypted));
            return true;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    public String decryptKey(Context context, String value) {
        try {
            String key = context.getString(R.string.app_name); // 128 bit key
            // Create key and cipher
            Key aesKey = new SecretKeySpec(key.getBytes(), "AES");
            @SuppressLint("GetInstance") Cipher cipher = Cipher.getInstance("AES");
//            System.err.println(new String(encrypted));
            // decrypt the text
            cipher.init(Cipher.DECRYPT_MODE, aesKey);
            return new String(cipher.doFinal(value.getBytes()));
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
}
