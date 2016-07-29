LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dpy_check.cpp
LOCAL_MODULE := isvdpy_check
LOCAL_SHARED_LIBRARIES := \
    libutils libcutils libdl \
    libhardware libexpat libva libva-android \
    libui libgui libbinder libisv
LOCAL_CFLAGS := -DLOG_TAG=\"ISVDPYTEST\"
LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/libisv
include $(BUILD_EXECUTABLE)
