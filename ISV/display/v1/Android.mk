# Author: tianyang.zhu@intel.com

#$(warning $(INTEL_FEATURE_VIDEO_EXT_MODE))
#$(warning $(TARGET_HAS_MULTIPLE_DISPLAY))
#$(warning $(TARGET_BOARD_PLATFORM))
#$(warning $(TARGET_DEVICE))
#$(warning $(TARGET_OUT_HEADERS))

LOCAL_PATH:= $(call my-dir)

# Build libextmodejni library
include $(CLEAR_VARS)

LOCAL_SRC_FILES := jni/com_intel_extmode_ExtMode.cpp

LOCAL_MODULE:= libextmodejni
LOCAL_MODULE_TAGS:=optional

LOCAL_SHARED_LIBRARIES := \
     libcutils \
     libutils \
     libbinder \
     libandroid_runtime \
     libisv \
     libnativehelper

LOCAL_C_INCLUDES := \
     $(TARGET_OUT_HEADERS)/libisv \
     $(TARGET_OUT_HEADERS)/ufo \
     $(JNI_H_INCLUDE)

LOCAL_CFLAGS += -DLOG_TAG=\"ISVDPY\"
LOCAL_CFLAGS += -DINTEL_FEATURE_VIDEO_EXT_MODE
include $(BUILD_SHARED_LIBRARY)

# Build libextmodeobserer
include $(CLEAR_VARS)

LOCAL_SRC_FILES := java/com/intel/extmode/IExtModeObserver.aidl
LOCAL_SRC_FILES += \
     java/com/intel/extmode/ExtModeJni.java \
     java/com/intel/extmode/ExtMode.java \
     java/com/intel/extmode/ExtModeSocket.java \
     java/com/intel/extmode/ExtModeObserver.java
LOCAL_JAVA_LIBRARIES := services
LOCAL_MODULE:= libextmodeobserver
LOCAL_MODULE_TAGS:=optional
include $(BUILD_STATIC_JAVA_LIBRARY)

# Build com.intel.extmode.jar
include $(CLEAR_VARS)

LOCAL_JNI_SHARED_LIBRARIES := libextmodejni
LOCAL_JAVA_LIBRARIES := services
LOCAL_MODULE:= com.intel.extmode
LOCAL_MODULE_TAGS:=optional
LOCAL_CFLAGS += -DLOG_TAG=\"ISVEXTMODE\"
LOCAL_STATIC_JAVA_LIBRARIES := libextmodeobserver

include $(BUILD_JAVA_LIBRARY)
