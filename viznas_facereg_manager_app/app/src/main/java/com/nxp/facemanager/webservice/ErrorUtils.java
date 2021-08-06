package com.nxp.facemanager.webservice;

import com.google.gson.Gson;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;

import retrofit2.Response;

public class ErrorUtils {

    public static ApiError parseError(Response<?> response) {


        ApiError error = null;
        try {
            JSONObject jObjError = new JSONObject(response.errorBody() != null ? response.errorBody().string() : null);

            Gson gson = new Gson();
            error = gson.fromJson(jObjError.toString(), ApiError.class);
        } catch (JSONException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }


        return error;
    }
}