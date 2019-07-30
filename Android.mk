ifeq ($(USE_MEDIASDK),true)
  ifneq ($(BOARD_HAVE_OMX_SRC),true)
    LOCAL_PATH := $(call my-dir)
    include $(CLEAR_VARS)
    
    LOCAL_PROPRIETARY_MODULE := true

    MEDIASDK_BIN_REPO := $(LOCAL_PATH)

    # Call appropriate Android.mk for the platform
    include $(MEDIASDK_BIN_REPO)/mediasdk/Android.mk
  endif
endif
