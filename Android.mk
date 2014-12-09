ifeq ($(USE_MEDIASDK),true)
  MEDIASDK_BIN_REPO := $(call my-dir)

  UFO_ENABLE_GEN ?= gen7

  # Call appropriate Android.mk for the platform
  ifeq ($(strip $(UFO_ENABLE_GEN)), gen7)
    include $(MEDIASDK_BIN_REPO)/baytrail/Android.mk
  endif

  ifeq ($(strip $(UFO_ENABLE_GEN)), gen8)
    include $(MEDIASDK_BIN_REPO)/cherrytrail/Android.mk
    include $(MEDIASDK_BIN_REPO)/libmfxjpegdecoder/Android.mk
  endif

endif
