ifeq ($(TARGET_HAS_ISV),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    ISVOMXCore.cpp \
    ISVComponent.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libisv_omx_core

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libdl \
    libhardware \
    libexpat \
    libisv

LOCAL_C_INCLUDES := \
    $(call include-path-for, frameworks-native)/media/openmax \
    $(TARGET_OUT_HEADERS)/libisv \
    $(TARGET_OUT_HEADERS)/libva

LOCAL_CFLAGS += -DTARGET_VPP_USE_GEN
ifeq ($(TARGET_HAS_MULTIPLE_DISPLAY),true)
LOCAL_CFLAGS += -DTARGET_HAS_MULTIPLE_DISPLAY
endif
ifeq ($(INTEL_FEATURE_VIDEO_EXT_MODE),true)
LOCAL_CFLAGS += -DINTEL_FEATURE_VIDEO_EXT_MODE
endif


include $(BUILD_SHARED_LIBRARY)

endif
