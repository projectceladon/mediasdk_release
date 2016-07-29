LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := manage_test.cpp
LOCAL_MODULE := manage_test
LOCAL_SHARED_LIBRARIES := \
    libutils libcutils libdl \
    libhardware libexpat libva libva-android \
    libui libgui libbinder libisv
LOCAL_CFLAGS := -DLOG_TAG=\"ISVDPYTEST\"
LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/libisv \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/profile

include $(BUILD_EXECUTABLE)
