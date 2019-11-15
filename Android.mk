ifeq ($(USE_MEDIASDK),true)
  ifneq ($(BOARD_HAVE_OMX_SRC), true)
    LOCAL_PATH := $(call my-dir)
    include $(CLEAR_VARS)

    LOCAL_COPY_HEADERS_TO := msdk/openmax

    LOCAL_COPY_HEADERS := \
      openmax/OMX_IntelColorFormatExt.h \
      openmax/OMX_IntelErrorTypes.h \
      openmax/OMX_IntelIndexExt.h \
      openmax/OMX_IntelVideoExt.h

    LOCAL_PROPRIETARY_MODULE := true

    include $(BUILD_COPY_HEADERS)

    MEDIASDK_BIN_REPO := $(LOCAL_PATH)

    # Call appropriate Android.mk for the platform
    include $(MEDIASDK_BIN_REPO)/mediasdk/broxton/Android.mk
    include $(MEDIASDK_BIN_REPO)/mediasdk/icelake/Android.mk
  endif
endif
