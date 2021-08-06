package com.nxp.facemanager.database.dao;

import com.nxp.facemanager.database.entity.DeviceInfo;

import java.util.List;

import androidx.lifecycle.LiveData;
import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.OnConflictStrategy;
import androidx.room.Query;

/**
 * Device information interface for operation database.
 */
@Dao
public interface DeviceInfoDao {
    /**
     * Insert/replace the device information.
     *
     * @param user {@link DeviceInfo}
     */
    @Insert(onConflict = OnConflictStrategy.REPLACE)
    void insertDeviceInfo(DeviceInfo user);

    /**
     * Return the list of device information from {@link androidx.room.Room} database.
     *
     * @return LiveData object of device information.
     */
    @Query("SELECT * FROM device_info")
    LiveData<List<DeviceInfo>> getAllDeviceInformation();

    /**
     * Return the {@link DeviceInfo} by mac address.
     *
     * @param mac_address device mac address
     * @return DeviceInfo
     */
    @Query("Select * FROM device_info where mac_address = :mac_address")
    DeviceInfo getDeviceInfo(String mac_address);

    /**
     * Delete selected device from database.
     *
     * @param mac_address {@link DeviceInfo}
     */
    @Query("Delete from device_info where mac_address = :mac_address")
    void delete(String mac_address);

    /**
     * Delete all data.
     */
    @Query("DELETE FROM device_info")
    void deleteAll();


    @Query("UPDATE device_info SET isConnected=:isConnected")
    void update(boolean isConnected);

    @Query("UPDATE device_info SET isConnected=:isConnected where mac_address=:mac_address")
    void updateIsConnected(boolean isConnected, String mac_address);

}
