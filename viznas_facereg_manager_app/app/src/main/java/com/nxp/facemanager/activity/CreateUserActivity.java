package com.nxp.facemanager.activity;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.util.TypedValue;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.CheckBox;

import com.github.florent37.viewtooltip.ViewTooltip;
import com.nxp.facemanager.R;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.databinding.ActivityCreateUserBinding;
import com.nxp.facemanager.utility.AppConstants;
import com.nxp.facemanager.viewModels.CreateUserViewModel;
import com.nxp.facemanager.viewModels.ViewFactoryCommon;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

import javax.inject.Inject;

import androidx.annotation.Nullable;
import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProviders;

import static com.nxp.facemanager.activity.TrainingActivity.BASE_64_IMAGE;
import static com.nxp.facemanager.activity.TrainingActivity.USERNAME;
import static com.nxp.facemanager.activity.TrainingActivity.USER_FEATURES;
import static com.nxp.facemanager.activity.TrainingActivity.USER_ID;
import static java.sql.Types.NULL;

/**
 * Login view
 */
public class CreateUserActivity extends BaseActivity {


    public static final int REQUEST_CODE_CAPTURE_TRAINING = 10;

    /**
     * Inject FaceDb
     */
    @Inject
    FaceDatabase faceDatabase;
    /**
     * Activity Binding Object
     */
    private ActivityCreateUserBinding activityCreateUserBinding;
    private static final String TAG = "CreateUser";
    private UserInformation userInformation;
    private boolean isEditUser = false;

    private MenuItem menu_edit;
    private String strBase64Image;

    private String userName;
    private String userEmail;
    private int userId;
    CreateUserViewModel createUserViewModel;

    private List<UserInformation> userInformationList;

    /**
     * Initialize of activity context.
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        activityCreateUserBinding = DataBindingUtil.setContentView(this, R.layout.activity_create_user);
        setToolbar();
        createUserViewModel = ViewModelProviders.of(this, new
                ViewFactoryCommon(faceDatabase, apiInterface, mySharedPreferences)).get(CreateUserViewModel.class);
        activityCreateUserBinding.setCreateUser(createUserViewModel);
        createUserViewModel.setActivityContext(this);

        createUserViewModel.getUserList().observe(this, new Observer<List<UserInformation>>() {
            @Override
            public void onChanged(List<UserInformation> userInformations) {
                    userInformationList = userInformations;
            }
        });

        Bundle bundle = getIntent().getExtras();
        if (bundle != null) {

            if (bundle.containsKey(BASE_64_IMAGE)) {
                strBase64Image = bundle.getString(BASE_64_IMAGE);
                Bitmap bitmap = stringToBitMap(strBase64Image);
                if (null != bitmap && null != activityCreateUserBinding && !isFinishing()) {
                    Drawable drawable = new BitmapDrawable(getResources(), bitmap);
                    activityCreateUserBinding.imgProfile.setImageDrawable(drawable);
                }

                if (null != activityCreateUserBinding.getCreateUser()) {
                    activityCreateUserBinding.getCreateUser().strProfile.set(strBase64Image);
                }
                Log.e(TAG, "bundle.containsKey(BASE_64_IMAGE)");
            }


            userInformation = bundle.getParcelable(AppConstants.USER_DATA);
            isEditUser = bundle.getBoolean(AppConstants.IS_EDIT_USER);


        }
        if (null != userInformation) {

            createUserViewModel.strName.set(userInformation.getName());
            createUserViewModel.strPhone.set(userInformation.getPhoneNo());
            createUserViewModel.strEmail.set(userInformation.getEmail());
            createUserViewModel.strProfile.set(userInformation.getProfilePic());
            createUserViewModel.intId.set(userInformation.getLocal_user_id());

            activityCreateUserBinding.imgProfile.setImageBitmap(stringToBitMap(userInformation.getProfilePic()));


            createUserViewModel.setEditUser(true);
            createUserViewModel.setUserID(userInformation.get_id());
            createUserViewModel.setLocal_user_id(userInformation.getLocal_user_id());

            strBase64Image = userInformation.getProfilePic();
            Bitmap bitmap = stringToBitMap(strBase64Image);
            if (null != bitmap && null != activityCreateUserBinding && !isFinishing()) {
                Drawable drawable = new BitmapDrawable(getResources(), bitmap);
                activityCreateUserBinding.imgProfile.setImageDrawable(drawable);
                Log.e(TAG, "null != activityCreateUserBinding.getCreateUser()");
            }

            if (isEditUser) {
                activityCreateUserBinding.toolbar.setTitle(getString(R.string.edit_user));
                activityCreateUserBinding.btnCreate.setVisibility(View.VISIBLE);
                activityCreateUserBinding.btnCreate.setText(getString(R.string.btn_update));
//                activityCreateUserBinding.edtEmail.setEnabled(false);
                activityCreateUserBinding.edtEmail.setVisibility(View.GONE);
                activityCreateUserBinding.edtName.setEnabled(true);
                activityCreateUserBinding.edtPhone.setEnabled(true);
                activityCreateUserBinding.imgProfile.setEnabled(true);
            } else {
                activityCreateUserBinding.btnCreate.setVisibility(View.GONE);
                activityCreateUserBinding.toolbar.setTitle(getString(R.string.user_detail));
                activityCreateUserBinding.edtEmail.setEnabled(false);
                activityCreateUserBinding.edtName.setEnabled(false);
                activityCreateUserBinding.edtPhone.setEnabled(false);
                activityCreateUserBinding.imgProfile.setEnabled(false);
            }
        } else {
            activityCreateUserBinding.toolbar.setTitle(getString(R.string.create_user_title));
            createUserViewModel.setEditUser(false);
            activityCreateUserBinding.btnCreate.setText(getString(R.string.create_user));
        }
        setUpEmailError();
        setUpNameError();
        setUpNetworkError();
        setServerError();

//        ViewTooltip
//                .on(this, activityCreateUserBinding.imgVideo)
//                .autoHide(false, 1000)
//                .clickToHide(true)
//                .align(ViewTooltip.ALIGN.CENTER)
//                .position(ViewTooltip.Position.BOTTOM)
//                .text(getString(R.string.msg_tooltip))
//                .textSize(TypedValue.COMPLEX_UNIT_SP, 12)
//                .textColor(Color.WHITE)
//                .color(getResources().getColor(R.color.toolbar_color))
//                .corner(10)
//                .arrowWidth(15)
//                .arrowHeight(15)
//                .padding(10, 5, 10, 5)
//                .distanceWithView(0)
//                //change the opening animation
//                .animation(new ViewTooltip.FadeTooltipAnimation(500))
//                .show();
    }

    /**
     * Set up error for name
     */
    private void setUpNameError() {
        Objects.requireNonNull(activityCreateUserBinding.getCreateUser()).getNameError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                activityCreateUserBinding.name.setError(null);
            } else {
                activityCreateUserBinding.name.setError(s);
                activityCreateUserBinding.edtName.requestFocus();
            }
        });
    }

    /**
     * Server error Live Data observer
     */
    private void setServerError() {
        Objects.requireNonNull(activityCreateUserBinding.getCreateUser()).getServerError().observe(this, this::showSnackbar);
    }

    /**
     * Set observer on error message of invalid email
     */
    private void setUpEmailError() {
        Objects.requireNonNull(activityCreateUserBinding.getCreateUser()).getEmailError().observe(this, s -> {
            if (s.equalsIgnoreCase("")) {
                activityCreateUserBinding.email.setError(null);
            } else {
                activityCreateUserBinding.email.setError(s);
                activityCreateUserBinding.edtEmail.requestFocus();
            }
        });
    }

    @Override
    public boolean onSupportNavigateUp() {
        onBackPressed();
        return true;
    }
    public static String byteToHex(byte bt){
        String[] strHex={"0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f"};
        String resStr="";
        int low =(bt & 15);
        int high = bt>>4 & 15;
        resStr = strHex[high]+strHex[low];
        return resStr;
    }

    public static String bytesToHex(byte[] bts) {
        String[] strHex={"0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f"};
        String resStr="";
        int low, high;

        for (int i = 0; i < bts.length; i++) {
            low =(bts[i] & 15);
            high = bts[i] >> 4 & 15;
            resStr += strHex[high]+strHex[low];
        }
        return resStr;
    }
    @Override
    public void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.e(TAG, "req code: "+requestCode);
        int frame = mySharedPreferences.getIntData(AppConstants.FRAME_NUM);
        if (requestCode == REQUEST_CODE_CAPTURE_TRAINING && data != null) {

            strBase64Image = data.getStringExtra(BASE_64_IMAGE);
            try{
                if (createUserViewModel.strName.get() == ""){
                        userName = data.getStringExtra(USERNAME);
                        createUserViewModel.strName.set(userName);
                }

                userId = data.getIntExtra(USER_ID, -1);
                byte feature_byte[] = data.getByteArrayExtra(USER_FEATURES);
                String feature = bytesToHex(feature_byte);
                Bitmap bm = stringToBitMap(strBase64Image);
                if (frame==1) {
                    createUserViewModel.strProfile.set(strBase64Image);
                    createUserViewModel.strFeature.set(feature);
                    activityCreateUserBinding.imgProfile.setImageBitmap(bm);
                } else if(frame == 2){
                    createUserViewModel.strProfile2.set(strBase64Image);
                    createUserViewModel.strFeature2.set(feature);
                    activityCreateUserBinding.imgProfile2.setImageBitmap(bm);
                } else{
                    createUserViewModel.strProfile3.set(strBase64Image);
                    createUserViewModel.strFeature.set(feature);
                    activityCreateUserBinding.imgSingle.setImageBitmap(bm);
                }
                //need find max id to transform.
//                List<UserInformation> userInformationList = createUserViewModel.getDbOperation().getAllUsers().getValue();
                //List<UserInformation> userInformationList = userInformationList;//faceDatabase.userDao().getAll().getValue();
                if(this.userInformationList != null) {
                    if(this.userInformationList.size() > 0) {
                        List<Integer> idList = new ArrayList<Integer>();
                        for(UserInformation information : userInformationList) {
                            idList.add(Integer.parseInt(information.get_id()));
                        };
                        int max = Collections.max(idList);
                        createUserViewModel.intId.set(max+1);
                    }
                    else
                    {
                        createUserViewModel.intId.set(1);
                    }
                }else {
                    createUserViewModel.intId.set(1);
                }
//                createUserViewModel.intId.set(userId);
//                createUserViewModel.strFeature.set(feature);

//                Bitmap bm = stringToBitMap(strBase64Image);
//                activityCreateUserBinding.imgProfile.setImageBitmap(bm);
            } catch (Exception e){
                e.printStackTrace();
            }

//            if (null != activityCreateUserBinding.getCreateUser()) {
//                activityCreateUserBinding.getCreateUser().setStrProfilePic(strBase64Image);
//            }
//            Bitmap bitmap = stringToBitMap(strBase64Image);
//            if (null != bitmap && null != activityCreateUserBinding && !isFinishing()) {
//                Drawable drawable = new BitmapDrawable(getResources(), bitmap);
//                activityCreateUserBinding.imgProfile.setImageDrawable(drawable);
//            }

        }
    }

    /**
     * Bind menu with current view {@link Menu}
     *
     * @param menu {@link Menu}
     * @return true/false
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.user_edit_menu, menu);
        menu_edit = menu.findItem(R.id.menu_edit);
        if (userInformation != null && !isEditUser) {
            menu_edit.setVisible(true);
        } else {
            menu_edit.setVisible(false);
            activityCreateUserBinding.imgProfile.setEnabled(true);
        }
        return true;
    }

    /**
     * Menu item select listener {@link MenuItem}.
     *
     * @param item {@link MenuItem}
     * @return true/false
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_edit:
                if (menu_edit.isVisible()) {
                    activityCreateUserBinding.btnCreate.setVisibility(View.VISIBLE);
                    activityCreateUserBinding.btnCreate.setText(getString(R.string.btn_update));
                    activityCreateUserBinding.edtEmail.setEnabled(false);
                    activityCreateUserBinding.edtName.setEnabled(true);
                    activityCreateUserBinding.edtPhone.setEnabled(true);
                    activityCreateUserBinding.toolbar.setTitle(getString(R.string.edit_user));
                    activityCreateUserBinding.imgProfile.setEnabled(true);
                    menu_edit.setVisible(false);
                }
                break;
        }
        return true;
    }


    /**
     * Initialize the toolbar
     */
    private void setToolbar() {
        setSupportActionBar(activityCreateUserBinding.toolbar);
        if (null != getSupportActionBar()) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setDisplayShowHomeEnabled(true);
        }
        activityCreateUserBinding.toolbar.setNavigationOnClickListener(view -> finish());
    }


    private void setUpNetworkError() {
        activityCreateUserBinding.getCreateUser().getInternetErrorMessage().observe(this, s -> showNoNetworkSnackbar());
    }

    /**
     * When user done editing/creating user profile, perform submit and return to previous page
     */
    public void onSubmit(View view){
        this.createUserViewModel.performSubmit();
//        finish();
    }

    public void onCheckboxClick(View view) {
        CheckBox c = (CheckBox) view;
        createUserViewModel.bothFeatures.set(c.isChecked());
        Log.d(TAG, "checkbox: "+ createUserViewModel.bothFeatures.get());
    }
}
