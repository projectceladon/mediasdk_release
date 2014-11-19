ifeq ($(USE_MEDIASDK),true)
  LOCAL_PATH := $(call my-dir)
  include $(CLEAR_VARS)

  LOCAL_PROPRIETARY_MODULE := true

  MEDIASDK_BIN_REPO := $(LOCAL_PATH)

  UFO_ENABLE_GEN ?= gen7

  # Call appropriate Android.mk for the platform
  include $(MEDIASDK_BIN_REPO)/mediasdk/Android.mk


  ifeq ($(strip $(UFO_ENABLE_GEN)), gen8)
    include $(MEDIASDK_BIN_REPO)/cherrytrail/Android.mk
    include $(MEDIASDK_BIN_REPO)/libmfxjpegdecoder/Android.mk
  endif

endif
