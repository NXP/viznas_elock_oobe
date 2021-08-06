LOCAL_PATH := $(call my-dir)

OASIS_INSTALL_PATH := $(LOCAL_PATH)/oasis

include $(CLEAR_VARS)
LOCAL_MODULE := oasis
LOCAL_SRC_FILES := $(OASIS_INSTALL_PATH)/lib/liboasis_lite_apk.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := JniLib
LOCAL_SRC_FILES := JniLibv3.cpp

LOCAL_C_INCLUDES := $(OASIS_INSTALL_PATH)/include
LOCAL_CPP_INCLUDES := $(OASIS_INSTALL_PATH)/include

LOCAL_STATIC_LIBRARIES := oasis

LOCAL_CFLAGS := -O2 -fvisibility=hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math  -Os -s
LOCAL_CPPFLAGS := -O2 -fvisibility=hidden -fvisibility-inlines-hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math  -Os -s
LOCAL_LDFLAGS += -Wl,--gc-sections

LOCAL_CFLAGS += -fopenmp -static-openmp
LOCAL_CPPFLAGS += -fopenmp -static-openmp
LOCAL_LDFLAGS += -fopenmp -static-openmp

ifeq ($(OS),Windows_NT)
    LOCAL_CFLAGS += -static-openmp
    LOCAL_CPPFLAGS += -static-openmp
    LOCAL_LDFLAGS += -static-openmp
endif

LOCAL_LDLIBS := -lz -llog -ljnigraphics

include $(BUILD_SHARED_LIBRARY)
