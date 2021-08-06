package com.nxp.facemanager.database;

import android.annotation.SuppressLint;
import android.os.AsyncTask;
import android.util.Log;

import com.nxp.facemanager.database.dao.DeviceInfoDao;
import com.nxp.facemanager.database.dao.UserInformationDao;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;

import java.util.List;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

/**
 * This class is used for database operation on different table.
 * e.g {@link androidx.room.Insert},{@link androidx.room.Delete}
 */
public class DatabaseOperations {

    /**
     * User Information interface method access.
     */
    private UserInformationDao userInformationDao;
    /**
     * User Information interface method access.
     */
    private DeviceInfoDao deviceInfoDao;

    /**
     * Constructor to access the class
     *
     * @param faceDatabase {@link FaceDatabase}
     */
    public DatabaseOperations(FaceDatabase faceDatabase) {
        userInformationDao = faceDatabase.userDao();
        deviceInfoDao = faceDatabase.deviceInformationDao();
    }

    /**
     * Return {@link LiveData} list of {@link UserInformation}
     *
     * @return LiveData
     */
    public LiveData<List<UserInformation>> getAllUsers() {
        return userInformationDao.getAll();
    }

    /**
     * Insert {@link UserInformation or {@link DeviceInfo}} in RoomDatabase.
     *
     * @param object object to detect which object is used to insert into database.
     */
    public void insert(Object object) {
        if (object instanceof UserInformation) {
            new insertAsyncTask(userInformationDao).execute(object);
        } else if (object instanceof DeviceInfo) {
            new insertAsyncTask(deviceInfoDao).execute(object);
        }
    }

    public void updateUser(UserInformation userInformation) {
        new UpdateUserAsyncTask(userInformationDao).execute(userInformation);
    }


    /**
     * Delete {@link UserInformation} or {@link DeviceInfo} from RoomDatabase.
     *
     * @param object userInformation
     */
    public void delete(Object object) {
        if (object instanceof UserInformation) {
            new DeleteAsyncTask(userInformationDao).execute(object);
        } else if (object instanceof DeviceInfo) {
            new DeleteAsyncTask(deviceInfoDao).execute(object);
        }
    }


    /**
     * Check user exits or not in database.
     *
     * @param username {@link String}
     * @return true/false
     */
    public boolean userExits(String username) {
        return userInformationDao.getByName(username) != null;
    }


    /**
     * Check user exits or not in database.
     *
     * @return true/false
     */
    public LiveData<UserInformation> getLoginUser(String _id) {
        return userInformationDao.getLoginUser(_id);
    }

    /**
     * Return {@link LiveData} list of device information.
     *
     * @return LiveData
     */
    public LiveData<List<DeviceInfo>> getAllDeviceInfo() {
        return (deviceInfoDao != null && deviceInfoDao.getAllDeviceInformation() != null) ? deviceInfoDao.getAllDeviceInformation() : new MutableLiveData<>();
    }



    /**
     * Check provided mac address is stored in local database
     *
     * @param macAdd mac address of device
     * @return Device information in the form of DeviceInfo model
     */
    public DeviceInfo checkDeviceExitsInDb(String macAdd) {
        if (getAllDeviceInfo().getValue() != null && getAllDeviceInfo().getValue().size() > 0) {
            for (DeviceInfo deviceInformation : getAllDeviceInfo().getValue()) {
                if (deviceInformation.getMac_address().equalsIgnoreCase(macAdd)) {
                    return deviceInformation;
                }
            }
        }
        return null;
    }


    @SuppressLint("StaticFieldLeak")
    public void updateConnected(String mac_address) {
        new AsyncTask<Void, Void, Void>() {

            @Override
            protected Void doInBackground(Void... voids) {
                if (mac_address == null) {
                    deviceInfoDao.update(false);
                } else {
                    deviceInfoDao.updateIsConnected(true, mac_address);
                }
                return null;
            }
        }.execute();
    }

    /**
     * Delete {@link UserInformation} and {@link DeviceInfo}
     */
    public void deleteSubUsers() {
        deleteSubUserInfoDatabase();
    }


    /**
     * Thread for insert the {@link UserInformation and {@link DeviceInfo}} in {@link androidx.room.Room} database.
     */
    private static class insertAsyncTask extends AsyncTask<Object, Void, Void> {

        /**
         * Object in database table dao.
         */
        private Object mAsyncTaskDao;

        insertAsyncTask(Object dao) {
            mAsyncTaskDao = dao;
        }

        @Override
        protected Void doInBackground(final Object... params) {
            if (mAsyncTaskDao instanceof UserInformationDao) {
                UserInformationDao userInformationDao = (UserInformationDao) mAsyncTaskDao;
                userInformationDao.insertUserInfo((UserInformation) params[0]);
            } else if (mAsyncTaskDao instanceof DeviceInfoDao) {
                DeviceInfoDao deviceInfoDao = (DeviceInfoDao) mAsyncTaskDao;
                deviceInfoDao.insertDeviceInfo((DeviceInfo) params[0]);
            }
            return null;
        }
    }


    /**
     * Delete {@link UserInformation} and {@link DeviceInfo}
     */
    public void resetDatabase() {
        resetUserInfoDatabase();
        resetDeviceInfoDatabase();
    }

    /**
     * Delete  {@link DeviceInfo}
     */
    private void deleteSubUserInfoDatabase() {
        new DeleteAllSubUsersAsyncTask(userInformationDao).execute();
    }

    /**
     * Delete  {@link DeviceInfo}
     */
    public void resetDeviceInfoDatabase() {
        new DeleteAllAsyncTask(deviceInfoDao).execute();
    }

    /**
     * Delete  {@link DeviceInfo}
     */
    private void resetUserInfoDatabase() {
        new DeleteAllAsyncTask(userInformationDao).execute();
    }

    /**
     * Thread for Update the {@link UserInformation} in {@link androidx.room.Room} database.
     */
    private static class UpdateUserAsyncTask extends AsyncTask<Object, Void, Void> {

        /**
         * Object in database table dao.
         */
        private Object mAsyncTaskDao;

        UpdateUserAsyncTask(Object dao) {
            mAsyncTaskDao = dao;
        }

        @Override
        protected Void doInBackground(final Object... params) {
            if (mAsyncTaskDao instanceof UserInformationDao) {
                UserInformationDao userInformationDao = (UserInformationDao) mAsyncTaskDao;
                userInformationDao.updateUserData((UserInformation) params[0]);
            }
            return null;
        }
    }

    /**
     * Thread for insert the {@link UserInformation} in {@link androidx.room.Room} database.
     */
    private static class DeleteAsyncTask extends AsyncTask<Object, Boolean, Boolean> {

        private Object mAsyncTaskDao;

        DeleteAsyncTask(Object dao) {
            mAsyncTaskDao = dao;
        }

        @Override
        protected Boolean doInBackground(final Object... params) {
            if (mAsyncTaskDao instanceof UserInformationDao) {
                UserInformationDao userInformationDao = (UserInformationDao) mAsyncTaskDao;
                userInformationDao.delete(((UserInformation) params[0]).getLocal_user_id());
            } else if (mAsyncTaskDao instanceof DeviceInfoDao) {
                Log.e("TAG", "doInBackground: "+"Delete device in db "+((DeviceInfo) params[0]).getDevice_name()+" "+((DeviceInfo) params[0]).getMac_address());
                DeviceInfoDao deviceInfoDao = (DeviceInfoDao) mAsyncTaskDao;
                deviceInfoDao.delete(((DeviceInfo) params[0]).getMac_address());
            }
            return null;
        }
    }

    /**
     * Thread for delete all {@link UserInformation} or {@link DeviceInfo} in {@link androidx.room.Room} database.
     */
    private static class DeleteAllAsyncTask extends AsyncTask<Object, Boolean, Boolean> {

        private Object mAsyncTaskDao;

        DeleteAllAsyncTask(Object dao) {
            mAsyncTaskDao = dao;
        }

        @Override
        protected Boolean doInBackground(final Object... params) {
            if (mAsyncTaskDao instanceof UserInformationDao) {
                UserInformationDao userInformationDao = (UserInformationDao) mAsyncTaskDao;
                userInformationDao.deleteAll();
            } else if (mAsyncTaskDao instanceof DeviceInfoDao) {
                DeviceInfoDao deviceInfoDao = (DeviceInfoDao) mAsyncTaskDao;
                deviceInfoDao.deleteAll();
            }

            return null;
        }
    }

    /**
     * Thread for delete all {@link UserInformation} or {@link DeviceInfo} in {@link androidx.room.Room} database.
     */
    private static class DeleteAllSubUsersAsyncTask extends AsyncTask<Object, Boolean, Boolean> {

        private Object mAsyncTaskDao;

        DeleteAllSubUsersAsyncTask(Object dao) {
            mAsyncTaskDao = dao;
        }

        @Override
        protected Boolean doInBackground(final Object... params) {
            if (mAsyncTaskDao instanceof UserInformationDao) {
                UserInformationDao userInformationDao = (UserInformationDao) mAsyncTaskDao;
                userInformationDao.deleteSubUsers();
            }
            return null;
        }
    }
}
