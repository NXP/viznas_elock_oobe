package com.nxp.facemanager.viewModels;

import com.nxp.facemanager.database.DatabaseOperations;
import com.nxp.facemanager.database.FaceDatabase;
import com.nxp.facemanager.database.entity.UserInformation;

import java.util.ArrayList;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.databinding.ObservableBoolean;
import androidx.databinding.ObservableField;
import androidx.databinding.ObservableInt;
import androidx.lifecycle.LifecycleOwner;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModel;

/**
 * This class used in training mode and bind with {@link com.nxp.facemanager.activity.TrainingActivity}
 */
public class TrainingModel extends ViewModel {
    /**
     * Progress of left face.
     */
    private ObservableInt progressLeft;
    /**
     * Progress of center face.
     */
    private ObservableInt progressCenter;
    /**
     * Progress of right face.
     */
    private ObservableInt progressRight;
    /**
     * Store Training data as string formate.
     */
    private ObservableField<String> userInfo = new ObservableField<>("");
    /**
     * Store the user name.
     */
    private ObservableField<String> userName = new ObservableField<>("");
    /**
     * Store user image data in string.
     */
    private ObservableField<String> userPhoto = new ObservableField<>("");
    /**
     * Verify camera is running or not.
     */
    private ObservableBoolean isCameraRunning = new ObservableBoolean(false);
    /**
     * Verify camera preview is created or not.
     */
    private ObservableBoolean previewDisplayCreated = new ObservableBoolean(false);
    /**
     * List of {@link UserInformation}
     */
    private List<UserInformation> userInformationArrayList = new ArrayList<>();

    /**
     * Return training progress of left face.
     *
     * @return ObservableField
     */
    public ObservableInt getProgressLeft() {
        return progressLeft;
    }

    public void setProgressLeft(ObservableInt progressLeft) {
        this.progressLeft = progressLeft;
    }

    /**
     * Return training progress of center face.
     *
     * @return ObservableField
     */
    public ObservableInt getProgressCenter() {
        return progressCenter;
    }

    public void setProgressCenter(ObservableInt progressCenter) {
        this.progressCenter = progressCenter;
    }

    /**
     * Return training progress of right face.
     *
     * @return ObservableField
     */
    public ObservableInt getProgressRight() {
        return progressRight;
    }

    public void setProgressRight(ObservableInt progressRight) {
        this.progressRight = progressRight;
    }

    /**
     * Return user training data as string format.
     *
     * @return ObservableField
     */
    public ObservableField<String> getUserInfo() {
        return userInfo;
    }

    public void setUserInfo(ObservableField<String> userInfo) {
        this.userInfo = userInfo;
    }

    /**
     * Return user name.
     *
     * @return ObservableField
     */
    public ObservableField<String> getUserName() {
        return userName;
    }

    public void setUserName(ObservableField<String> userName) {
        this.userName = userName;
    }

    /**
     * Return user phone bitmap as base64 String format.
     *
     * @return ObservableField
     */
    public ObservableField<String> getUserPhoto() {
        return userPhoto;
    }

    public void setUserPhoto(ObservableField<String> userPhoto) {
        this.userPhoto = userPhoto;
    }

    /**
     * boolean for camera is running or not.
     *
     * @return ObservableField
     */
    public ObservableBoolean getIsCameraRunning() {
        return isCameraRunning;
    }

    public void setIsCameraRunning(ObservableBoolean isCameraRunning) {
        this.isCameraRunning = isCameraRunning;
    }

    /**
     * boolean for  camera preview is display or not.
     *
     * @return ObservableField
     */
    public ObservableBoolean getPreviewDisplayCreated() {
        return previewDisplayCreated;
    }

    public void setPreviewDisplayCreated(ObservableBoolean previewDisplayCreated) {
        this.previewDisplayCreated = previewDisplayCreated;
    }

    /**
     * Get the userInformation list
     *
     * @return List {@link UserInformation}
     */
    public List<UserInformation> getList() {
        return userInformationArrayList;
    }

    /**
     * Set/update {@link UserInformation} list
     *
     * @param list List {@link UserInformation}
     */
    public void setList(List<UserInformation> list) {
        this.userInformationArrayList = list;
    }

    /**
     * check to see if same name is exits in local database.
     *
     * @param username String
     * @return true/false
     */
    public boolean isSameNameExitsInDatabase(String username) {
        if (userInformationArrayList != null && userInformationArrayList.size() > 0) {
            for (UserInformation userInformation : userInformationArrayList) {
                if (userInformation.getName().equals(username)) return true;
            }
        }
        return false;
    }

    private DatabaseOperations dbOperation;
    /**
     * {@link LiveData} object reference of {@link UserInformation}
     */
    private LiveData<List<UserInformation>> userLiveData = new LiveData<List<UserInformation>>() {
        @Override
        public void observe(@NonNull LifecycleOwner owner, @NonNull Observer<? super List<UserInformation>> observer) {
            super.observe(owner, observer);
        }
    };


    public LiveData<List<UserInformation>> getUserLiveData() {
        return userLiveData;
    }

    public void setUserLiveData(FaceDatabase faceDatabase) {
        dbOperation = new DatabaseOperations(faceDatabase);
        this.userLiveData = dbOperation.getAllUsers();
    }
}
