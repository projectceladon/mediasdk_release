LOCAL_PATH := $(call my-dir)

ifneq ($(filter $(TARGET_BOARD_PLATFORM), kabylake),)

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

    # If Media SDK sources repo presents - no need to execute below block
    ifneq ($(BOARD_HAVE_MEDIASDK_SRC),true)
        # Shared libraries
        include $(CLEAR_VARS)
        LOCAL_MODULE := mfx_prebuilts
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE_OWNER := intel
        LOCAL_REQUIRED_MODULES :=
        ifneq ($(BOARD_HAVE_OMX_SRC),true)
            LOCAL_REQUIRED_MODULES += libmfx_omx_core
            LOCAL_REQUIRED_MODULES += libmfx_omx_components_hw
            LOCAL_REQUIRED_MODULES += libmfx_omx_components_sw
        endif
        LOCAL_REQUIRED_MODULES += libmfxhw32
        LOCAL_REQUIRED_MODULES += libmfxsw32
        LOCAL_REQUIRED_MODULES += libmfx_hevce_hw32
        #ifeq ($(TARGET_ARCH), x86_64)
            #LOCAL_REQUIRED_MODULES += libmfxhw64
            #LOCAL_REQUIRED_MODULES += libmfxsw64
            #LOCAL_REQUIRED_MODULES += libmfx_hevce_hw64
        #endif
        include $(BUILD_PHONY_PACKAGE)

        include $(CLEAR_VARS)
        LOCAL_MODULE := libmfxhw32
        LOCAL_PROPRIETARY_MODULE := true
        LOCAL_MODULE_CLASS := SHARED_LIBRARIES
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE_STEM := libmfxhw32
        LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
        LOCAL_MODULE_OWNER := intel
        LOCAL_MULTILIB := 32
        LOCAL_SRC_FILES_32 := lib/x86/libmfxhw32$(LOCAL_MODULE_SUFFIX)
        include $(BUILD_PREBUILT)

        include $(CLEAR_VARS)
        LOCAL_MODULE := libmfxsw32
        LOCAL_PROPRIETARY_MODULE := true
        LOCAL_MODULE_CLASS := SHARED_LIBRARIES
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE_STEM := libmfxsw32
        LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
        LOCAL_MODULE_OWNER := intel
        LOCAL_MULTILIB := 32
        LOCAL_SRC_FILES_32 := lib/x86/libmfxsw32$(LOCAL_MODULE_SUFFIX)
        include $(BUILD_PREBUILT)

        include $(CLEAR_VARS)
        LOCAL_MODULE := libmfx_hevce_hw32
        LOCAL_PROPRIETARY_MODULE := true
        LOCAL_MODULE_CLASS := SHARED_LIBRARIES
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE_STEM := libmfx_hevce_hw32
        LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
        LOCAL_MODULE_OWNER := intel
        LOCAL_MULTILIB := 32
        LOCAL_SRC_FILES_32 := lib/x86/libmfx_hevce_hw32$(LOCAL_MODULE_SUFFIX)
        include $(BUILD_PREBUILT)

        include $(CLEAR_VARS)
        LOCAL_MODULE := libmfxhw64
        LOCAL_PROPRIETARY_MODULE := true
        LOCAL_MODULE_CLASS := SHARED_LIBRARIES
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE_STEM := libmfxhw64
        LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
        LOCAL_MODULE_OWNER := intel
        LOCAL_MULTILIB := 64
        LOCAL_SRC_FILES_64 := lib/x86_64/libmfxhw64$(LOCAL_MODULE_SUFFIX)
        include $(BUILD_PREBUILT)

        include $(CLEAR_VARS)
        LOCAL_MODULE := libmfxsw64
        LOCAL_PROPRIETARY_MODULE := true
        LOCAL_MODULE_CLASS := SHARED_LIBRARIES
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE_STEM := libmfxsw64
        LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
        LOCAL_MODULE_OWNER := intel
        LOCAL_MULTILIB := 64
        LOCAL_SRC_FILES_64 := lib/x86_64/libmfxsw64$(LOCAL_MODULE_SUFFIX)
        include $(BUILD_PREBUILT)

        include $(CLEAR_VARS)
        LOCAL_MODULE := libmfx_hevce_hw64
        LOCAL_PROPRIETARY_MODULE := true
        LOCAL_MODULE_CLASS := SHARED_LIBRARIES
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE_STEM := libmfx_hevce_hw64
        LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
        LOCAL_MODULE_OWNER := intel
        LOCAL_MULTILIB := 64
        LOCAL_SRC_FILES_64 := lib/x86_64/libmfx_hevce_hw64$(LOCAL_MODULE_SUFFIX)
        include $(BUILD_PREBUILT)

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

            include $(CLEAR_VARS)
            LOCAL_MODULE := libmfx_omx_components_sw
            LOCAL_PROPRIETARY_MODULE := true
            LOCAL_MODULE_CLASS := SHARED_LIBRARIES
            LOCAL_MODULE_TAGS := optional
            LOCAL_MODULE_STEM := libmfx_omx_components_sw
            LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
            LOCAL_MODULE_OWNER := intel
            LOCAL_MULTILIB := both
            LOCAL_SRC_FILES_32 := lib/x86/libmfx_omx_components_sw$(LOCAL_MODULE_SUFFIX)
            LOCAL_SRC_FILES_64 := lib/x86_64/libmfx_omx_components_sw$(LOCAL_MODULE_SUFFIX)
            include $(BUILD_PREBUILT)
        endif

    endif

endif
