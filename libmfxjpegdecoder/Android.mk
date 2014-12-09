LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(addprefix src/, $(notdir $(wildcard $(LOCAL_PATH)/src/*.cpp)))

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/include \
    $(TARGET_OUT_HEADERS)/mediasdk \
    $(TARGET_OUT_HEADERS)/libva \
    ndk/sources/cxx-stl/stlport/stlport

LOCAL_SHARED_LIBRARIES += \
    libcutils \
    libutils \
    libskia \
    libui \
    libdrm \
    libdrm_intel \
    libva-android \
    libva \
    libva-tpi   \
    libhardware \
    libexpat \
    libmfxhw32 \
    libstlport-mfx \
    libgabi++-mfx \
    libdl

LOCAL_CFLAGS := -fexceptions -frtti

LOCAL_COPY_HEADERS_TO  := libmfxjpegdecoder

LOCAL_COPY_HEADERS += \
    include/JPEGAllocator.h \
    include/JPEGBitstream.h \
    include/JPEGDecoder.h \
    include/JPEGUtils.h \
    include/JPEGBaseAllocator.h \
    include/JPEGCommon.h \
    include/JPEGSysAllocator.h \

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_MODULE := libmfxjpegdecoder
LOCAL_MODULE_OWNER := intel

include $(BUILD_SHARED_LIBRARY)

