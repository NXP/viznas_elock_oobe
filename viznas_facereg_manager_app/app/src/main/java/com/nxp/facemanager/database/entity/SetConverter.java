package com.nxp.facemanager.database.entity;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import java.lang.reflect.Type;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import androidx.room.TypeConverter;

/**
 * This class is used as converter of room database which will convert the model class in json and vice-versa.
 * Used to convert the integer array to String and vice-versa.
 */
public class SetConverter {


    /**
     * Convert the JSON String to set of {@link Integer}
     *
     * @param data {@link String}
     * @return List
     */
    @SuppressWarnings("unchecked")
    @TypeConverter
    public static Set<String> integersToObjectList(String data) {
        if (data == null) {
            return Collections.EMPTY_SET;
        }

        Type listType = new TypeToken<Set<String>>() {
        }.getType();

        return new Gson().fromJson(data, listType);
    }

    /**
     * Convert Set of {@link Integer} into String.
     *
     * @param someObjects {@link List}
     * @return String
     */
    @TypeConverter
    public static String objectSetToString(Set<String> someObjects) {
        return new Gson().toJson(someObjects);
    }
}
