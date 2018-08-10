LOCAL_PATH := $(call my-dir)

ifneq ($(filter $(TARGET_BOARD_PLATFORM),broxton gmin joule minnowboardv3 project-celadon),)

    # Shared libraries
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


endif
