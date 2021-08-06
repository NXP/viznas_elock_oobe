package com.nxp.facemanager.database;

import com.nxp.facemanager.database.dao.DeviceInfoDao;
import com.nxp.facemanager.database.dao.UserInformationDao;
import com.nxp.facemanager.database.entity.DataConverter;
import com.nxp.facemanager.database.entity.DeviceInfo;
import com.nxp.facemanager.database.entity.SetConverter;
import com.nxp.facemanager.database.entity.UserInformation;

import androidx.room.Database;
import androidx.room.RoomDatabase;
import androidx.room.TypeConverters;

/**
 * Database class which contains getter method of entity/tables and version.
 */
@Database(entities = {UserInformation.class, DeviceInfo.class}, version = 1, exportSchema = false)
@TypeConverters({DataConverter.class, SetConverter.class})
public abstract class FaceDatabase extends RoomDatabase {

    public abstract UserInformationDao userDao();

    public abstract DeviceInfoDao deviceInformationDao();
}
