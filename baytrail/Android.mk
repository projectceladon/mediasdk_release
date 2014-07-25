LOCAL_PATH := $(call my-dir)

ifneq ($(filter $(TARGET_BOARD_PLATFORM),baytrail gmin),)

  # Include files
  include $(CLEAR_VARS)

  LOCAL_COPY_HEADERS_TO := mediasdk

  LOCAL_COPY_HEADERS := \
    include/mfxastructures.h \
    include/mfxaudio++.h \
    include/mfxaudio.h \
    include/mfxcamera.h \
    include/mfxcommon.h \
    include/mfxdefs.h \
    include/mfxenc.h \
    include/mfxfei.h \
    include/mfxhdcp.h \
    include/mfxjpeg.h \
    include/mfxla.h \
    include/mfxmvc.h \
    include/mfxpcp.h \
    include/mfxplugin++.h \
    include/mfxplugin.h \
    include/mfxplugin_internal.h \
    include/mfxsession.h \
    include/mfxstructures.h \
    include/mfxvideo++.h \
    include/mfxvideo.h \
    include/mfxvp8.h \
    include/mfxvp9.h \
    include/mfxvstructures.h \
    include/mfxwidi.h

  include $(BUILD_COPY_HEADERS)

  # Documents
  include $(CLEAR_VARS)

  LOCAL_MODULE := mediasdk_release_notes.pdf
  LOCAL_MODULE_OWNER := intel
  LOCAL_SRC_FILES := doc/$(LOCAL_MODULE)
  LOCAL_MODULE_TAGS := optional
  LOCAL_MODULE_CLASS := ETC
  LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/mediasdk

  include $(BUILD_PREBUILT)

  # If Media SDK sources repo presents - no need to execute below block
  ifneq ($(BOARD_HAVE_MEDIASDK_SRC),true)
    # Shared libraries
    include $(CLEAR_VARS)
    LOCAL_MODULE := mfx_prebuilts
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_OWNER := intel
    LOCAL_REQUIRED_MODULES :=
    LOCAL_REQUIRED_MODULES += libmfx_omx_core
    LOCAL_REQUIRED_MODULES += libmfx_omx_components_hw
    LOCAL_REQUIRED_MODULES += libmfx_omx_components_sw
    LOCAL_REQUIRED_MODULES += libgabi++-mfx
    LOCAL_REQUIRED_MODULES += libstlport-mfx
    ifeq ($(TARGET_ARCH), x86)
        LOCAL_REQUIRED_MODULES += libmfxhw32
        LOCAL_REQUIRED_MODULES += libmfxsw32
    else
        LOCAL_REQUIRED_MODULES += libmfxhw64
    endif
    include $(BUILD_PHONY_PACKAGE)

  ifeq ($(TARGET_ARCH), x86)
    include $(CLEAR_VARS)
    LOCAL_MODULE := libmfxhw32
    LOCAL_MODULE_CLASS := SHARED_LIBRARIES
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
    LOCAL_MODULE_STEM := libmfxhw32
    LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
    LOCAL_MODULE_OWNER := intel
    LOCAL_SRC_FILES := lib/$(TARGET_ARCH)/libmfxhw32$(LOCAL_MODULE_SUFFIX)
    include $(BUILD_PREBUILT)

    include $(CLEAR_VARS)
    LOCAL_MODULE := libmfxsw32
    LOCAL_MODULE_CLASS := SHARED_LIBRARIES
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
    LOCAL_MODULE_STEM := libmfxsw32
    LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
    LOCAL_MODULE_OWNER := intel
    LOCAL_SRC_FILES := lib/$(TARGET_ARCH)/libmfxsw32$(LOCAL_MODULE_SUFFIX)
    include $(BUILD_PREBUILT)
  else
    include $(CLEAR_VARS)
    LOCAL_MODULE := libmfxhw64
    LOCAL_MODULE_CLASS := SHARED_LIBRARIES
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
    LOCAL_MODULE_STEM := libmfxhw64
    LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
    LOCAL_MODULE_OWNER := intel
    LOCAL_SRC_FILES := lib/$(TARGET_ARCH)/libmfxhw64$(LOCAL_MODULE_SUFFIX)
    include $(BUILD_PREBUILT)
  endif

    include $(CLEAR_VARS)
    LOCAL_MODULE := libmfx_omx_core
    LOCAL_MODULE_CLASS := SHARED_LIBRARIES
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
    LOCAL_MODULE_STEM := libmfx_omx_core
    LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
    LOCAL_MODULE_OWNER := intel
    LOCAL_SRC_FILES := lib/$(TARGET_ARCH)/libmfx_omx_core$(LOCAL_MODULE_SUFFIX)
    include $(BUILD_PREBUILT)

    include $(CLEAR_VARS)
    LOCAL_MODULE := libmfx_omx_components_hw
    LOCAL_MODULE_CLASS := SHARED_LIBRARIES
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
    LOCAL_MODULE_STEM := libmfx_omx_components_hw
    LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
    LOCAL_MODULE_OWNER := intel
    LOCAL_SRC_FILES := lib/$(TARGET_ARCH)/libmfx_omx_components_hw$(LOCAL_MODULE_SUFFIX)
    include $(BUILD_PREBUILT)

    include $(CLEAR_VARS)
    LOCAL_MODULE := libmfx_omx_components_sw
    LOCAL_MODULE_CLASS := SHARED_LIBRARIES
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
    LOCAL_MODULE_STEM := libmfx_omx_components_sw
    LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
    LOCAL_MODULE_OWNER := intel
    LOCAL_SRC_FILES := lib/$(TARGET_ARCH)/libmfx_omx_components_sw$(LOCAL_MODULE_SUFFIX)
    include $(BUILD_PREBUILT)

    include $(CLEAR_VARS)
    LOCAL_MODULE := libgabi++-mfx
    LOCAL_MODULE_CLASS := SHARED_LIBRARIES
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
    LOCAL_MODULE_STEM := libgabi++-mfx
    LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
    LOCAL_MODULE_OWNER := intel
    LOCAL_SRC_FILES := lib/$(TARGET_ARCH)/libgabi++-mfx$(LOCAL_MODULE_SUFFIX)
    include $(BUILD_PREBUILT)

    include $(CLEAR_VARS)
    LOCAL_MODULE := libstlport-mfx
    LOCAL_MODULE_CLASS := SHARED_LIBRARIES
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
    LOCAL_MODULE_STEM := libstlport-mfx
    LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
    LOCAL_MODULE_OWNER := intel
    LOCAL_SRC_FILES := lib/$(TARGET_ARCH)/libstlport-mfx$(LOCAL_MODULE_SUFFIX)
    include $(BUILD_PREBUILT)

    # Executables
    ifeq ($(MEDIASDK_TEST_APPS),true)
      include $(CLEAR_VARS)

      LOCAL_PREBUILT_EXECUTABLES := \
        bin/$(TARGET_ARCH)/sample_decode \
        bin/$(TARGET_ARCH)/sample_encode \
        bin/$(TARGET_ARCH)/mfx_player \
        bin/$(TARGET_ARCH)/mfx_transcoder

      LOCAL_MODULE_TAGS := optional
      include $(BUILD_MULTI_PREBUILT)
    endif
  endif

endif
