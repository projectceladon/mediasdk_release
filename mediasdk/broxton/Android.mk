LOCAL_PATH := $(call my-dir)

ifneq ($(filter $(TARGET_BOARD_PLATFORM),broxton gmin joule minnowboardv3),)

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
    LOCAL_PROPRIETARY_MODULE := true
    include $(BUILD_COPY_HEADERS)

    # Documents
    include $(CLEAR_VARS)
    LOCAL_MODULE := mediasdk_release_notes.pdf
    LOCAL_PROPRIETARY_MODULE := true
    LOCAL_MODULE_OWNER := intel
    LOCAL_SRC_FILES := doc/$(LOCAL_MODULE)
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_RELATIVE_PATH := mediasdk
    include $(BUILD_PREBUILT)

    # Prebuilt module
    include $(CLEAR_VARS)
    LOCAL_MODULE := mfx_prebuilts
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_OWNER := intel
    LOCAL_REQUIRED_MODULES += libmfx_omx_core
    LOCAL_REQUIRED_MODULES += libmfx_omx_components_hw
    include $(BUILD_PHONY_PACKAGE)

    # Shared libraries
    ifneq ($(BOARD_HAVE_OMX_SRC),true)
        include $(CLEAR_VARS)
        LOCAL_MODULE := libmfx_omx_core
        LOCAL_PROPRIETARY_MODULE := true
        LOCAL_MODULE_CLASS := SHARED_LIBRARIES
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE_STEM := libmfx_omx_core
        LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
        LOCAL_MODULE_OWNER := intel
        LOCAL_MULTILIB := both
        LOCAL_SRC_FILES_32 := lib/x86/libmfx_omx_core$(LOCAL_MODULE_SUFFIX)
        LOCAL_SRC_FILES_64 := lib/x86_64/libmfx_omx_core$(LOCAL_MODULE_SUFFIX)
        include $(BUILD_PREBUILT)

        include $(CLEAR_VARS)
        LOCAL_MODULE := libmfx_omx_components_hw
        LOCAL_PROPRIETARY_MODULE := true
        LOCAL_MODULE_CLASS := SHARED_LIBRARIES
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE_STEM := libmfx_omx_components_hw
        LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
        LOCAL_MODULE_OWNER := intel
        LOCAL_MULTILIB := both
        LOCAL_SRC_FILES_32 := lib/x86/libmfx_omx_components_hw$(LOCAL_MODULE_SUFFIX)
        LOCAL_SRC_FILES_64 := lib/x86_64/libmfx_omx_components_hw$(LOCAL_MODULE_SUFFIX)
        include $(BUILD_PREBUILT)
    endif

endif
