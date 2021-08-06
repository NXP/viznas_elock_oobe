
package com.nxp.facemanager.model;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;
import com.nxp.facemanager.database.entity.UserInformation;

import java.util.ArrayList;

@SuppressWarnings("unused")
public class GetUsersResponseModel {

    @Expose
    private String message;

    @SerializedName("Success")
    @Expose
    private Boolean success;

    @SerializedName("users")
    @Expose
    private ArrayList<UserInformation> users;

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public Boolean getSuccess() {
        return success;
    }

    public void setSuccess(Boolean success) {
        this.success = success;
    }

    public ArrayList<UserInformation> getUsers() {
        return users;
    }

    public void setUsers(ArrayList<UserInformation> users) {
        this.users = users;
    }


//    public class User {
//
//        @Expose
//        private String _id;
//        @Expose
//        private String adminId;
//        @Expose
//        private List<String> deviceIds;
//        @Expose
//        private String email;
//        @Expose
//        private Boolean isAdmin;
//        @Expose
//        private String name;
//
//        @Expose
//        private String phoneNo;
//
//        public Boolean getLoginUser() {
//            return isAdmin;
//        }
//
//        public void setAdmin(Boolean admin) {
//            isAdmin = admin;
//        }
//
//        public String getPhoneNo() {
//            return phoneNo;
//        }
//
//        public void setPhoneNo(String phoneNo) {
//            this.phoneNo = phoneNo;
//        }
//
//        @Expose
//        private Long status;
//
//        public String get_id() {
//            return _id;
//        }
//
//        public void set_id(String _id) {
//            this._id = _id;
//        }
//
//        public String getAdminId() {
//            return adminId;
//        }
//
//        public void setAdminId(String adminId) {
//            this.adminId = adminId;
//        }
//
//        public List<String> getDeviceIds() {
//            return deviceIds;
//        }
//
//        public void setDeviceIds(List<String> deviceIds) {
//            this.deviceIds = deviceIds;
//        }
//
//        public String getEmail() {
//            return email;
//        }
//
//        public void setEmail(String email) {
//            this.email = email;
//        }
//
//        public Boolean getIsAdmin() {
//            return isAdmin;
//        }
//
//        public void setIsAdmin(Boolean isAdmin) {
//            this.isAdmin = isAdmin;
//        }
//
//        public String getName() {
//            return name;
//        }
//
//        public void setName(String name) {
//            this.name = name;
//        }
//
//        public Long getStatus() {
//            return status;
//        }
//
//        public void setStatus(Long status) {
//            this.status = status;
//        }
//
//    }

}
