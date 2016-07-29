ifeq ($(TARGET_HAS_ISV),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    isv_buffer.cpp \
    isv_bufmanager.cpp \
    isv_processor.cpp \
    isv_worker.cpp \
    isv_thread.cpp \
    isv_factory.cpp \
    isv_profile.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libisv

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libdl \
    libhardware \
    libexpat \
    libva \
    libva-android

LOCAL_C_INCLUDES := \
    $(call include-path-for, frameworks-native)/media/openmax \
    $(TARGET_OUT_HEADERS)/libisv \
    $(TARGET_OUT_HEADERS)/libva

LOCAL_CFLAGS += -DTARGET_VPP_USE_GEN

include $(BUILD_SHARED_LIBRARY)

endif
