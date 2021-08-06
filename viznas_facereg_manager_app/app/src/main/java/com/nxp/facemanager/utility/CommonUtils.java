package com.nxp.facemanager.utility;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.res.AssetManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.provider.Settings;
import android.util.Patterns;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

import com.nxp.facemanager.R;
import com.nxp.facemanager.utility.HTTPS.AuthenticationParameters;
import com.nxp.facemanager.utility.HTTPS.IOUtil;
import com.nxp.facemanager.utility.HTTPS.SSLContextFactory;

import java.io.IOException;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.net.ssl.SSLContext;

/**
 * This class file used for common function access
 * e.g ANDROID_ID,TimeStamp Converter..
 */
public class CommonUtils {
    private static SSLContext sslContext;

    private CommonUtils() {
        // This utility class is not publicly instantiable
    }

    @SuppressLint("all")
    public static String getDeviceId(Context context) {
        return Settings.Secure.getString(context.getContentResolver(), Settings.Secure.ANDROID_ID);
    }

    public static String getTimestamp() {
        return new SimpleDateFormat(AppConstants.TIMESTAMP_FORMAT, Locale.US).format(new Date());
    }

    /**
     * Get time stamp on long format.
     *
     * @return Long object.
     */
    public static long getTimestampLong() {
        return System.currentTimeMillis();
    }

    /**
     * Check the email address is valid or not.
     *
     * @param email String.
     * @return Boolean
     */
    public static boolean isEmailValid(String email) {
        return Patterns.EMAIL_ADDRESS.matcher(email).matches();
    }

    public static boolean isNetworkConnected(Context context) {
        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (cm != null) {
            NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
            //noinspection deprecation
            return activeNetwork != null && activeNetwork.isConnectedOrConnecting();
        }
        return false;
    }

    /**
     * Read file from asset folder
     *
     * @param fileName file name
     * @return String
     */
    public static String readAssetFile(Context context, String fileName) {
        AssetManager assetManager = context.getAssets();
        InputStream input;
        try {
            input = assetManager.open(fileName);
            int size = input.available();
            byte[] buffer = new byte[size];
            //noinspection ResultOfMethodCallIgnored
            input.read(buffer);
            input.close();
            return new String(buffer);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return "";
    }

    /**
     * Instance of progress dialog.
     *
     * @param context selected context
     * @return ProgressDialog
     */
    public static ProgressDialog progressDialog(Context context, String message) {
        ProgressDialog progressDialog = new ProgressDialog(context, R.style.AlertDialogTheme);
        progressDialog.setIndeterminate(true);
        progressDialog.setMessage(message);
        progressDialog.setCancelable(false);
        progressDialog.setCanceledOnTouchOutside(false);
        progressDialog.show();
        return progressDialog;
    }

    /**
     * Method to hide keyboard
     */
    public static void hideKeyboard(Activity activity) {

        try {
            if (null != activity) {
                InputMethodManager inputMethodManager = (InputMethodManager) activity.getSystemService(Activity.INPUT_METHOD_SERVICE);
                View currentFocus = (activity.getCurrentFocus());
                if (currentFocus != null) {
                    if (inputMethodManager != null) {
                        inputMethodManager.hideSoftInputFromWindow(currentFocus.getWindowToken(), 0);
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    public static ProgressDialog showLoadingDialog(Context context, String message) {
        ProgressDialog progressDialog = new ProgressDialog(context, R.style.ProgressDialogStyle);
        progressDialog.setCancelable(false);
        progressDialog.setMessage(message);
        progressDialog.setCanceledOnTouchOutside(false);
        return progressDialog;
    }

    public static void dismissProgress(ProgressDialog progressDialog) {
        try {
            if (progressDialog != null && progressDialog.isShowing()) {
                progressDialog.dismiss();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    /**
     * method to check whether entered password is valid or not
     *
     * @param password entered password
     * @return boolean value
     */
    public static boolean isValidPassword(final String password) {

        Pattern pattern;
        Matcher matcher;

        final String PASSWORD_PATTERN = "^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[@#$%^&+=])(?=\\S+$).{4,}$";

        pattern = Pattern.compile(PASSWORD_PATTERN);
        matcher = pattern.matcher(password);

        return matcher.matches();

    }

    public static String readCaCert(Context context) throws Exception {
        InputStream inputStream = context.getResources().openRawResource(R.raw.ca);
        return IOUtil.readFully(inputStream);
    }

    public static SSLContext setCaCert(Context context) {
        AuthenticationParameters authParams = new AuthenticationParameters();
        try {
            authParams.setCaCertificate(CommonUtils.readCaCert(context));
        } catch (Exception e) {
            e.printStackTrace();
        }
        return setSslContext(context, authParams);
    }

    public static SSLContext setSslContext(Context context, AuthenticationParameters authParams) {
        try {
            sslContext = SSLContextFactory.getInstance().makeContext(context, authParams.getCaCertificate());

        } catch (Exception e) {
            e.printStackTrace();
        }
        return sslContext;
    }
}
