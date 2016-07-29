# Author: tianyang.zhu@intel.com

LOCAL_PATH:= $(call my-dir)

#$(warning $(TARGET_HAS_MULTIPLE_DISPLAY))
#$(warning $(TARGET_BOARD_PLATFORM))

# Build header files
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := display
LOCAL_COPY_HEADERS := \
    native/include/MultiDisplayType.h \
    native/include/IMultiDisplayListener.h \
    native/include/IMultiDisplayCallback.h \
    native/include/IMultiDisplayHdmiControl.h \
    native/include/IMultiDisplayVideoControl.h \
    native/include/IMultiDisplayEventMonitor.h \
    native/include/IMultiDisplaySinkRegistrar.h \
    native/include/IMultiDisplayCallbackRegistrar.h \
    native/include/IMultiDisplayConnectionObserver.h \
    native/include/IMultiDisplayInfoProvider.h \
    native/include/IMultiDisplayDecoderConfig.h \
    native/include/MultiDisplayService.h

include $(BUILD_COPY_HEADERS)


# Build multidisplay library
include $(CLEAR_VARS)
LOCAL_SRC_FILES:= \
    native/MultiDisplayComposer.cpp \
    native/IMultiDisplayListener.cpp \
    native/IMultiDisplayCallback.cpp \
    native/IMultiDisplayInfoProvider.cpp \
    native/IMultiDisplayConnectionObserver.cpp \
    native/IMultiDisplayHdmiControl.cpp \
    native/IMultiDisplayVideoControl.cpp \
    native/IMultiDisplayEventMonitor.cpp \
    native/IMultiDisplaySinkRegistrar.cpp \
    native/IMultiDisplayCallbackRegistrar.cpp \
    native/IMultiDisplayDecoderConfig.cpp \
    native/MultiDisplayService.cpp

LOCAL_MODULE:= libmultidisplay
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
    libui libcutils libutils libbinder
LOCAL_CFLAGS := -DLOG_TAG=\"ISVDPY\"

LOCAL_SRC_FILES += \
    native/drm_hdmi.cpp

LOCAL_SHARED_LIBRARIES += libdrm

LOCAL_CFLAGS += -DDVI_SUPPORTED

LOCAL_C_INCLUDES += $(call include-path-for, frameworks-av) \
    $(call include-path-for, external-drm)

include $(BUILD_SHARED_LIBRARY)

# Build JNI library
include $(CLEAR_VARS)

LOCAL_SRC_FILES := jni/com_intel_multidisplay_DisplaySetting.cpp

LOCAL_MODULE := libmultidisplayjni
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
     libcutils \
     libutils \
     libbinder \
     libandroid_runtime \
     libmultidisplay \
     libnativehelper

LOCAL_C_INCLUDES := \
     $(JNI_H_INCLUDE) \
     $(call include-path-for, frameworks-base)

LOCAL_CFLAGS += -DLOG_TAG=\"ISVDPY\"

include $(BUILD_SHARED_LIBRARY)


# Build jar library
include $(CLEAR_VARS)
LOCAL_SRC_FILES := java/com/intel/multidisplay/IDisplayObserver.aidl
LOCAL_SRC_FILES += \
     java/com/intel/multidisplay/DisplaySettingJni.java \
     java/com/intel/multidisplay/DisplaySetting.java \
     java/com/intel/multidisplay/DisplayObserver.java
LOCAL_JAVA_LIBRARIES := services
LOCAL_MODULE:= libdisplayobserver
LOCAL_MODULE_TAGS:=optional
include $(BUILD_STATIC_JAVA_LIBRARY)

include $(CLEAR_VARS)
LOCAL_JNI_SHARED_LIBRARIES := libmultidisplayjni
LOCAL_JAVA_LIBRARIES := services
LOCAL_STATIC_JAVA_LIBRARIES := libdisplayobserver
LOCAL_MODULE_TAGS:=optional
LOCAL_MODULE:= com.intel.multidisplay
include $(BUILD_JAVA_LIBRARY)
