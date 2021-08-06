package com.nxp.facemanager;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;

import org.junit.Test;
import org.junit.runner.RunWith;

import androidx.test.InstrumentationRegistry;
import androidx.test.espresso.Espresso;
import androidx.test.espresso.action.ViewActions;
import androidx.test.espresso.matcher.ViewMatchers;
import androidx.test.filters.SdkSuppress;
import androidx.test.runner.AndroidJUnit4;
import androidx.test.uiautomator.By;
import androidx.test.uiautomator.SearchCondition;
import androidx.test.uiautomator.UiDevice;
import androidx.test.uiautomator.UiObject;
import androidx.test.uiautomator.UiObject2;
import androidx.test.uiautomator.UiObjectNotFoundException;
import androidx.test.uiautomator.UiSelector;
import androidx.test.uiautomator.Until;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.clearText;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.action.ViewActions.pressImeActionButton;
import static androidx.test.espresso.action.ViewActions.typeText;
import static androidx.test.espresso.assertion.ViewAssertions.matches;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static androidx.test.espresso.matcher.ViewMatchers.withText;

@RunWith(AndroidJUnit4.class)
@SdkSuppress(minSdkVersion = 18)
public class FaceRecInstrumentedTest {

    private static final String BASIC_SAMPLE_PACKAGE
            = "com.nxp.facemanager";
    private static final int LAUNCH_TIMEOUT = 5000;
    private static final String STRING_TO_BE_TYPED = "UiAutomator";
    private UiDevice mDevice;
    // Open drawer content description for accessibility
    private static final String CONTENT_DESCRIPTION_OPEN_DRAWER = "Open drawer";

    // Close drawer content description for accessibility
    private static final String CONTENT_DESCRIPTION_CLOSE_DRAWER = "Close drawer";

    @Test
    public void startRegister() {
        // Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);


        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        PackageManager manager = context.getPackageManager();
        try {
            Intent i = manager.getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
            if (i == null) {
                //throw new ActivityNotFoundException();
            }
            i.addCategory(Intent.CATEGORY_LAUNCHER);
            context.startActivity(i);
        } catch (ActivityNotFoundException e) {
        }
//        final Intent intent = context.getPackageManager()
//                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
//        if (intent != null) {
//            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
//        }
//        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        //Code to hide softkeyboard
        Espresso.closeSoftKeyboard();

        UiObject registerButtonByText = mDevice.findObject(new UiSelector()
                .text("Don't have an account? Register")
                .className("android.widget.TextView"));
        UiObject2 registerButtonById = mDevice.findObject(By.res("com.nxp.facemanager:id/txtAccountSignInLabel"));
        //Simulate a user-click on the OK button, if found.
        if (registerButtonById.isEnabled()) {
            registerButtonById.click();
        }

        //Another way of accessing EditText
        UiObject2 emailEditText = mDevice.findObject(By.res("com.nxp.facemanager:id/edtEmail"));


//        emailEditText.setText("roma.chaudhary@volansystech.com");
        onView(withId(R.id.edtEmail))
                .perform(typeText("roma.chaudhary@volansystech.com"), pressImeActionButton());


        onView(withId(R.id.edtPassword))
                .perform(typeText("Test@1"), pressImeActionButton());

        onView(withId(R.id.edtConfirmPassword))
                .perform(typeText("Test@1"), pressImeActionButton());


        onView(withId(R.id.edtPhone))
                .perform(typeText("123"), pressImeActionButton());

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }

    @Test
    public void startPerformLogin() {
        // Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);

        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        SearchCondition<UiObject2> isAvailable =
                Until.findObject(By.clazz("foo"));


        onView(withId(R.id.edtEmail))
                .perform(typeText("roma.chaudhary@volansystech.com"), pressImeActionButton());


        onView(withId(R.id.edtPassword))
                .perform(typeText("test1"), pressImeActionButton());


        try {
            Thread.sleep(4000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        try {
            onView(withId(com.google.android.material.R.id.snackbar_text))
                    .check(matches(withText("Password is wrong")));

//            onView(withTagKey(100))
//                    .check(matches(withText("Password is wrong")));

            onView(withId(R.id.edtEmail))
                    .perform(clearText());

            onView(withId(R.id.edtPassword))
                    .perform(clearText());

            onView(withId(R.id.edtEmail))
                    .perform(typeText("roma.chaudhary@volansystech.com"), pressImeActionButton());


            onView(withId(R.id.edtPassword))
                    .perform(typeText("test"), pressImeActionButton());

        } catch (AssertionError e) {
            e.printStackTrace();
        }


        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }

    @Test
    public void startPerformForgotPassword() {
        // Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);


        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        //Code to hide softkeyboard
        Espresso.closeSoftKeyboard();

        onView(withId(R.id.txtForgotPassword))
                .perform(click());


        onView(withId(R.id.edtEmail))
                .perform(typeText("roma.chaudhary@volansystech.com"), pressImeActionButton());


        onView(withId(R.id.btnResetPassword))
                .perform(click());


        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }


    @Test
    public void startPerformChangePassword() {
        // Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);


        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        onView(withId(R.id.edtEmail))
                .perform(typeText("roma.chaudhary@volansystech.com"), pressImeActionButton());


        onView(withId(R.id.edtPassword))
                .perform(typeText("570285"), pressImeActionButton());


        UiObject navDrawer = mDevice.findObject(new UiSelector()
                .descriptionContains("Open navigation drawer")
                .className("android.widget.ImageButton"));
        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        UiObject changePasswordMenu = mDevice.findObject(new UiSelector()
                .text("Change Password")
                .className("android.widget.CheckedTextView"));
        try {
            changePasswordMenu.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        onView(withId(R.id.edtPassword)).perform(typeText("570285"), pressImeActionButton());
        onView(withId(R.id.edtNewPassword)).perform(typeText("Test@1"), pressImeActionButton());
        onView(withId(R.id.edtConfirmNewPassword)).perform(typeText("Test@1"), pressImeActionButton());


        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }


    @Test
    public void startPerformCreateUser() {
// Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);

        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        SearchCondition<UiObject2> isAvailable =
                Until.findObject(By.clazz("foo"));


        onView(withId(R.id.edtEmail))
                .perform(typeText("roma.chaudhary@volansystech.com"), pressImeActionButton());


        onView(withId(R.id.edtPassword))
                .perform(typeText("Test@1"), pressImeActionButton());


        UiObject navDrawer = mDevice.findObject(new UiSelector()
                .descriptionContains("Open navigation drawer")
                .className("android.widget.ImageButton"));
        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        UiObject manageUsers = mDevice.findObject(new UiSelector()
                .text("Manage Users")
                .className("android.widget.CheckedTextView"));
        try {
            manageUsers.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        onView(withId(R.id.fabUser))
                .perform(click());


        UiObject allow = mDevice.findObject(new UiSelector()
                .resourceId("com.android.packageinstaller:id/permission_allow_button")
                .className("android.widget.Button"));
        try {
            allow.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        try {
            allow.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        onView(withId(R.id.menu_skip))
                .perform(click());

        onView(withId(R.id.edtName))
                .perform(typeText("Test User"), pressImeActionButton());

        onView(withId(R.id.edtEmail))
                .perform(typeText("roma.chaudhary+4@volansystech.com"), pressImeActionButton());

        onView(withId(R.id.edtPhone))
                .perform(typeText("123456"), pressImeActionButton());

        Espresso.closeSoftKeyboard();
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        onView(withId(R.id.btnCreate))
                .perform(click());

        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }


    @Test
    public void startPerformAddSmartLock() {
// Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);

        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        SearchCondition<UiObject2> isAvailable =
                Until.findObject(By.clazz("foo"));


        onView(withId(R.id.edtEmail))
                .perform(typeText("roma.chaudhary@volansystech.com"), pressImeActionButton());


        onView(withId(R.id.edtPassword))
                .perform(typeText("Test@1"), pressImeActionButton());


        UiObject navDrawer = mDevice.findObject(new UiSelector()
                .descriptionContains("Open navigation drawer")
                .className("android.widget.ImageButton"));
        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        UiObject manageUsers = mDevice.findObject(new UiSelector()
                .text("Manage Smart Locks")
                .className("android.widget.CheckedTextView"));
        try {
            manageUsers.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        onView(withId(R.id.fabDevice))
                .perform(click());


        UiObject allow = mDevice.findObject(new UiSelector()
                .resourceId("com.android.packageinstaller:id/permission_allow_button")
                .className("android.widget.Button"));
        try {
            allow.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }


    @Test
    public void startPerformAddSmartLockWoLogin() {
// Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);

        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        UiObject navDrawer = mDevice.findObject(new UiSelector()
                .descriptionContains("Open navigation drawer")
                .className("android.widget.ImageButton"));
        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        UiObject manageUsers = mDevice.findObject(new UiSelector()
                .text("Manage Smart Locks")
                .className("android.widget.CheckedTextView"));
        try {
            manageUsers.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        onView(withId(R.id.fabDevice))
                .perform(click());


        UiObject allow = mDevice.findObject(new UiSelector()
                .resourceId("com.android.packageinstaller:id/permission_allow_button")
                .className("android.widget.Button"));
        try {
            allow.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


//        UiObject allowTurningBluetooth = mDevice.findObject(new UiSelector()
//                .resourceId("android:id/button1")
//                .className("android.widget.Button"));
//
//        try {
//            allowTurningBluetooth.click();
//        } catch (UiObjectNotFoundException e) {
//            e.printStackTrace();
//        }
//
//
//        try {
//            Thread.sleep(500);
//        } catch (InterruptedException e) {
//            e.printStackTrace();
//        }


        UiObject connectDevice = mDevice.findObject(new UiSelector()
                .resourceId("com.nxp.facemanager:id/txtConnect")
                .className("android.widget.TextView"));

        try {
            connectDevice.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        onView(withId(R.id.edtDeviceName))
                .perform(typeText("Test Device"), pressImeActionButton());


        onView(withId(R.id.edtPassCode))
                .perform(typeText("123456"), pressImeActionButton());

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        UiObject addDevice = mDevice.findObject(new UiSelector()
                .resourceId("android:id/button1")
                .className("android.widget.Button"));

        try {
            addDevice.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }


    @Test
    public void startPerformDeleteSmartLockWoLogin() {
// Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);

        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        UiObject navDrawer = mDevice.findObject(new UiSelector()
                .descriptionContains("Open navigation drawer")
                .className("android.widget.ImageButton"));
        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        UiObject manageUsers = mDevice.findObject(new UiSelector()
                .text("Manage Smart Locks")
                .className("android.widget.CheckedTextView"));
        try {
            manageUsers.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        Espresso.onView(ViewMatchers.withId(R.id.relDetail)).perform(ViewActions.swipeLeft());

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        Espresso.onView(ViewMatchers.withId(R.id.imgDelete)).perform(ViewActions.click());

        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }


    @Test
    public void startConnectSmartLockSendAdminFaceDataWoLogin() {
// Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);

        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        UiObject navDrawer = mDevice.findObject(new UiSelector()
                .descriptionContains("Open navigation drawer")
                .className("android.widget.ImageButton"));
        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        UiObject manageUsers = mDevice.findObject(new UiSelector()
                .text("Manage Smart Locks")
                .className("android.widget.CheckedTextView"));
        try {
            manageUsers.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        Espresso.onView(ViewMatchers.withId(R.id.menu_ble)).perform(click());

        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        UiObject sendFaceData = mDevice.findObject(new UiSelector()
                .text("Send Face Data")
                .className("android.widget.CheckedTextView"));
        try {
            sendFaceData.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }

    @Test
    public void startConnectSmartLockSendAGuestFaceDataWoLogin() {
// Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);

        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        UiObject navDrawer = mDevice.findObject(new UiSelector()
                .descriptionContains("Open navigation drawer")
                .className("android.widget.ImageButton"));
        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        UiObject manageUsers = mDevice.findObject(new UiSelector()
                .text("Manage Smart Locks")
                .className("android.widget.CheckedTextView"));
        try {
            manageUsers.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        Espresso.onView(ViewMatchers.withId(R.id.menu_ble)).perform(click());

        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        UiObject sendFaceData = mDevice.findObject(new UiSelector()
                .text("Manage Users")
                .className("android.widget.CheckedTextView"));
        try {
            sendFaceData.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        try {
            UiObject2 listview = mDevice.findObject(By.res("com.nxp.facemanager:id/recyclerViewUser"));

            UiObject2 imageview = listview.getChildren().get(1).findObject(By.res("com.nxp.facemanager:id/imageView2"));
            imageview.click();
        } catch (Exception e) {
            e.printStackTrace();
        }


//        onData(anything()).inAdapterView(withId(R.id.recyclerViewUser)).atPosition(0).perform(longClick());

//        onData(instanceOf(UserListAdapter.class))
//                .atPosition(0)
//                .perform(longClick());

        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


//        onData(anything()).inAdapterView(withId(R.id.recyclerViewUser)).atPosition(1).perform(click());


        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        Espresso.onView(ViewMatchers.withId(R.id.search_button)).perform(click());


        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }


    @Test
    public void startPerformEditLoggedInUser() {
// Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);

        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        onView(withId(R.id.edtEmail))
                .perform(typeText("roma.chaudhary@volansystech.com"), pressImeActionButton());


        onView(withId(R.id.edtPassword))
                .perform(typeText("test"), pressImeActionButton());


        UiObject navDrawer = mDevice.findObject(new UiSelector()
                .descriptionContains("Open navigation drawer")
                .className("android.widget.ImageButton"));
        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        onView(withId(R.id.imgEditProfile))
                .perform(click());

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        onView(withId(R.id.edtName))
                .perform(clearText());


        //Code to hide softkeyboard
        Espresso.closeSoftKeyboard();


        onView(withId(R.id.edtName))
                .perform(typeText("Roma"), pressImeActionButton());

        //Code to hide softkeyboard
        Espresso.closeSoftKeyboard();

        onView(withId(R.id.btnCreate))
                .perform(click());


        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }


    @Test
    public void startCompleteIntegratedTest() {

        // Initialize UiDevice instance
        mDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());


        //Wait for launcher
        final String launcherPackage = mDevice.getLauncherPackageName();
        //assertThat(launcherPackage, notNullValue());
        mDevice.wait(Until.hasObject(By.pkg(launcherPackage).depth(0)),
                LAUNCH_TIMEOUT);

        // Launch the app
        Context context = InstrumentationRegistry.getContext();
        final Intent intent = context.getPackageManager()
                .getLaunchIntentForPackage(BASIC_SAMPLE_PACKAGE);
        // Clear out any previous instances
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        }
        context.startActivity(intent);

        // Wait for the app to appear
        mDevice.wait(Until.hasObject(By.pkg(BASIC_SAMPLE_PACKAGE).depth(0)),
                LAUNCH_TIMEOUT);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        onView(withId(R.id.edtEmail))
                .perform(typeText("roma.chaudhary@volansystech.com"), pressImeActionButton());


        onView(withId(R.id.edtPassword))
                .perform(typeText("Test@1"), pressImeActionButton());


        UiObject navDrawer = mDevice.findObject(new UiSelector()
                .descriptionContains("Open navigation drawer")
                .className("android.widget.ImageButton"));
        try {
            navDrawer.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();


        }


        UiObject manageUsers = mDevice.findObject(new UiSelector()
                .text("Manage Smart Locks")
                .className("android.widget.CheckedTextView"));
        try {
            manageUsers.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        onView(withId(R.id.fabDevice))
                .perform(click());


        UiObject allow = mDevice.findObject(new UiSelector()
                .resourceId("com.android.packageinstaller:id/permission_allow_button")
                .className("android.widget.Button"));
        try {
            allow.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        UiObject allowTurningBluetooth = mDevice.findObject(new UiSelector()
                .resourceId("android:id/button1")
                .className("android.widget.Button"));

        try {
            allowTurningBluetooth.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        UiObject connectDevice = mDevice.findObject(new UiSelector()
                .resourceId("com.nxp.facemanager:id/txtConnect")
                .className("android.widget.TextView"));

        try {
            connectDevice.click();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }


        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }


}