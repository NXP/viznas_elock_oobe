package com.nxp.facemanager.database.entity;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import java.lang.reflect.Type;
import java.util.Collections;
import java.util.List;

import androidx.room.TypeConverter;

/**
 * This class is used as converter of room database which will convert the model class in json and vice-versa.
 */
public class DataConverter {

    /**
     * Convert the JSON String to list of {@link DeviceInfo}
     *
     * @param data {@link String}
     * @return List
     */
    @TypeConverter
    public static List<DeviceInfo> stringToSomeObjectList(String data) {
        if (data == null) {
            return Collections.emptyList();
        }

        Type listType = new TypeToken<List<DeviceInfo>>() {
        }.getType();

        return new Gson().fromJson(data, listType);
    }

    /**
     * Convert list of {@link DeviceInfo} into String.
     *
     * @param someObjects {@link List}
     * @return String
     */
    @TypeConverter
    public static String someObjectListToString(List<DeviceInfo> someObjects) {
        return new Gson().toJson(someObjects);
    }
}
