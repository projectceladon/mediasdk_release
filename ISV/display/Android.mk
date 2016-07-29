# Author: tianyang.zhu@intel.com

LOCAL_PATH:= $(call my-dir)

# Build libextmodejni library
include $(CLEAR_VARS)

ifeq ($(TARGET_HAS_MULTIPLE_DISPLAY),true)
include $(LOCAL_PATH)/v0/Android.mk
else
ifeq ($(INTEL_FEATURE_VIDEO_EXT_MODE),true)
include $(LOCAL_PATH)/v1/Android.mk
endif
endif
