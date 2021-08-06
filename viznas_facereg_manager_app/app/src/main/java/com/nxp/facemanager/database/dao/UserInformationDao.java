package com.nxp.facemanager.database.dao;

import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.UserInformation;

import java.util.List;

import androidx.lifecycle.LiveData;
import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.Query;
import androidx.room.Update;

/**
 * User Information interface for operation database.
 */
@Dao
public interface UserInformationDao {

    /**
     * Insert/replace the User Information.
     *
     * @param user {@link UserInformation}
     */
    @Insert
    long insertUserInfo(UserInformation user);

    @Update
    void updateUserData(UserInformation user);

    /**
     * Return the list of user information from {@link androidx.room.Room} database.
     *
     * @return LiveData object of user information.
     */
    @Query("SELECT * FROM user_information where isAdmin = 0")
    LiveData<List<UserInformation>> getAll();

    /**
     * Return the {@link UserInformation} by user name.
     *
     * @param name user name
     * @return UserInformation
     */
    @Query("Select * FROM user_information where name = :name")
    UserInformation getByName(String name);


    /**
     * Return the admin user data {@link UserInformation} by user name.
     *
     * @return UserInformation
     */
    @Query("Select * FROM user_information where _id = :id")
    LiveData<UserInformation> getLoginUser(String id);

    /**
     * Return the {@link UserInformation} by user name.
     *
     * @param email user email
     * @return UserInformation
     */
    @Query("Select * FROM user_information where email = :email")
    LiveData<UserInformation> getByMail(String email);

    /**
     * Delete selected User Information from database.
     *
     * @param local_user_id {@link DeviceInfo}
     */
    @Query("Delete from user_information where local_user_id = :local_user_id")
    void delete(int local_user_id);


    /**
     * Delete sub User Information from database.
     */
    @Query("Delete from user_information where isAdmin = 0")
    void deleteSubUsers();

    /**
     * Delete all data.
     */
    @Query("DELETE FROM user_information")
    void deleteAll();
}
