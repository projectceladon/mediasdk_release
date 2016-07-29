ifeq ($(TARGET_HAS_ISV),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := libisv

LOCAL_COPY_HEADERS := \
    include/ISVVp.h \
    include/ISVDisplay.h \
    include/ISVType.h \
    include/ISVFactory.h \
    include/ISVManager.h

include $(BUILD_COPY_HEADERS)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    vp/ISVBufferManager.cpp \
    vp/ISVProcessor.cpp \
    vp/ISVWorker.cpp \
    vp/ISVBuffer.cpp \
    vp/ISVThread.cpp \
    vp/ISVFactory.cpp \
    profile/ISVProfile.cpp \
    manager/ISVManager.cpp \
    display/common/ISVDisplay.cpp \
    display/common/ISVDisplayStatus.cpp \
    display/common/ISVExtModeFactory.cpp

ifeq ($(TARGET_HAS_MULTIPLE_DISPLAY),true)
LOCAL_SRC_FILES += display/v0/wrapper/ISVExtModeBaseOnPrivateService.cpp
else
ifeq ($(INTEL_FEATURE_VIDEO_EXT_MODE),true)
LOCAL_SRC_FILES += display/v1/wrapper/ISVExtModeBaseOnHwcService.cpp
endif
endif


LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libisv

LOCAL_SHARED_LIBRARIES := \
    libutils libcutils libdl \
    libhardware libexpat libva libva-android \
    libui libgui libbinder

ifeq ($(TARGET_HAS_MULTIPLE_DISPLAY),true)
LOCAL_SHARED_LIBRARIES += libmultidisplay
else
ifeq ($(INTEL_FEATURE_VIDEO_EXT_MODE),true)
LOCAL_SHARED_LIBRARIES += libhwcservice
endif
endif

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/profile \
    $(LOCAL_PATH)/manager \
    $(call include-path-for, frameworks-native)/media/openmax \
    $(TARGET_OUT_HEADERS)/libisv \
    $(TARGET_OUT_HEADERS)/libva \
    $(TARGET_OUT_HEADERS)/ufo

ifeq ($(TARGET_HAS_MULTIPLE_DISPLAY),true)
LOCAL_C_INCLUDES += \
     $(LOCAL_PATH)/display/common \
     $(LOCAL_PATH)/display/v0/wrapper \
     $(TARGET_OUT_HEADERS)/display \
     $(call include-path-for, frameworks-av)
LOCAL_CFLAGS += -DTARGET_HAS_MULTIPLE_DISPLAY
else
ifeq ($(INTEL_FEATURE_VIDEO_EXT_MODE),true)
LOCAL_C_INCLUDES += \
     $(LOCAL_PATH)/display/common \
     $(LOCAL_PATH)/display/v1/wrapper
LOCAL_CFLAGS += -DINTEL_FEATURE_VIDEO_EXT_MODE
endif
endif

LOCAL_CFLAGS += -DTARGET_VPP_USE_GEN

include $(BUILD_SHARED_LIBRARY)

# omx
include $(CLEAR_VARS)

ISV_ROOT := $(LOCAL_PATH)
include $(ISV_ROOT)/omx/Android.mk
include $(ISV_ROOT)/display/Android.mk

endif
