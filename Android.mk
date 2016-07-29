ifeq ($(USE_MEDIASDK),true)
  LOCAL_PATH := $(call my-dir)
  include $(CLEAR_VARS)

  LOCAL_COPY_HEADERS_TO := msdk/openmax

  LOCAL_COPY_HEADERS := \
    openmax/OMX_IntelColorFormatExt.h \
    openmax/OMX_IntelErrorTypes.h \
    openmax/OMX_IntelIndexExt.h \
    openmax/OMX_IntelVideoExt.h

  include $(BUILD_COPY_HEADERS)

  MEDIASDK_BIN_REPO := $(LOCAL_PATH)

  UFO_ENABLE_GEN ?= gen7

  # Call appropriate Android.mk for the platform
  include $(MEDIASDK_BIN_REPO)/mediasdk/Android.mk


  ifeq ($(strip $(UFO_ENABLE_GEN)), gen8)
    include $(MEDIASDK_BIN_REPO)/mediasdk/cherrytrail/Android.mk
    include $(MEDIASDK_BIN_REPO)/libmfxjpegdecoder/Android.mk
  endif

  ifeq ($(strip $(UFO_ENABLE_GEN)), gen9)
    include $(MEDIASDK_BIN_REPO)/mediasdk/broxton/Android.mk
    include $(MEDIASDK_BIN_REPO)/libmfxjpegdecoder/Android.mk
  endif

  include $(MEDIASDK_BIN_REPO)/ISV/Android.mk

endif
