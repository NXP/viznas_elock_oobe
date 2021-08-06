package com.nxp.facemanager.webservice;

import com.nxp.facemanager.model.AddDeviceRequestModel;
import com.nxp.facemanager.model.AddDeviceResponseModel;
import com.nxp.facemanager.model.ChangePasswordRequestModel;
import com.nxp.facemanager.model.CommonResponseModel;
import com.nxp.facemanager.model.CreateUserRequestModel;
import com.nxp.facemanager.model.DeleteUserRequestModel;
import com.nxp.facemanager.model.DeviceListResponseModel;
import com.nxp.facemanager.model.ForgotPasswordRequestModel;
import com.nxp.facemanager.model.GetUsersRequestModel;
import com.nxp.facemanager.model.GetUsersResponseModel;
import com.nxp.facemanager.model.LoginRequestModel;
import com.nxp.facemanager.model.LoginResponseModel;
import com.nxp.facemanager.model.RegistrationRequestModel;
import com.nxp.facemanager.model.UpdateUserRequestModel;
import com.nxp.facemanager.model.UpdateUserResponseModel;

import java.util.HashMap;

import retrofit2.Call;
import retrofit2.http.Body;
import retrofit2.http.GET;
import retrofit2.http.HTTP;
import retrofit2.http.Header;
import retrofit2.http.POST;
import retrofit2.http.PUT;
import retrofit2.http.Path;

import static com.nxp.facemanager.utility.AppConstants.PATH_ADD_DEVICE;
import static com.nxp.facemanager.utility.AppConstants.PATH_CREATE_USER;
import static com.nxp.facemanager.utility.AppConstants.PATH_DELETE_USER;
import static com.nxp.facemanager.utility.AppConstants.PATH_DEVICE_DELETE;
import static com.nxp.facemanager.utility.AppConstants.PATH_FORGOT_PASSWORD;
import static com.nxp.facemanager.utility.AppConstants.PATH_GET_DEVICES_USER_ID;
import static com.nxp.facemanager.utility.AppConstants.PATH_GET_USERS;
import static com.nxp.facemanager.utility.AppConstants.PATH_LOGIN;
import static com.nxp.facemanager.utility.AppConstants.PATH_REGISTER;
import static com.nxp.facemanager.utility.AppConstants.PATH_RESET_PASSWORD;
import static com.nxp.facemanager.utility.AppConstants.PATH_UPDATE_USER;

/**
 * {@link retrofit2.Retrofit} interface class for access the api.
 */
public interface ApiInterface {


    @POST(PATH_REGISTER)
    Call<CommonResponseModel> registerAdmin(@Body RegistrationRequestModel registrationRequestModel);

    @POST(PATH_LOGIN)
    Call<LoginResponseModel> loginUser(@Body LoginRequestModel loginRequestModel);

    @POST(PATH_ADD_DEVICE)
    Call<AddDeviceResponseModel> addDevice(@Header("token") String token, @Body AddDeviceRequestModel addDeviceRequestModel);

    @GET(PATH_GET_DEVICES_USER_ID)
    Call<DeviceListResponseModel> getDevices(@Header("token") String token, @Path("user_id") String user_id);

    @HTTP(method = "DELETE", path = PATH_DEVICE_DELETE, hasBody = true)
    Call<CommonResponseModel> deleteDevices(@Header("token") String token, @Body HashMap<String, String> deviceId);


    @POST(PATH_CREATE_USER)
    Call<CommonResponseModel> createUser(@Header("token") String token, @Body CreateUserRequestModel createUserRequestModel);

    @PUT(PATH_UPDATE_USER)
    Call<UpdateUserResponseModel> updateUser(@Header("token") String token, @Body UpdateUserRequestModel updateUserRequestModel);


    @POST(PATH_GET_USERS)
    Call<GetUsersResponseModel> getUsers(@Header("token") String token, @Body GetUsersRequestModel getUsersRequestModel);

    @HTTP(method = "DELETE", path = PATH_DELETE_USER, hasBody = true)
    Call<CommonResponseModel> deleteUser(@Header("token") String token, @Body DeleteUserRequestModel deleteUserRequestModel);

    @POST(PATH_RESET_PASSWORD)
    Call<CommonResponseModel> resetPassword(@Header("token") String token, @Body ChangePasswordRequestModel changePasswordRequestModel);


    @POST(PATH_FORGOT_PASSWORD)
    Call<CommonResponseModel> forgotPassword(@Body ForgotPasswordRequestModel forgotPasswordRequestModel);


}
