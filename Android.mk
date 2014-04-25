ifeq ($(USE_MEDIASDK),true)
  MEDIASDK_BIN_REPO := $(call my-dir)

  # Call appropriate Android.mk for the platform
  ifneq ($(filter $(TARGET_BOARD_PLATFORM),baytrail gmin),)
    include $(MEDIASDK_BIN_REPO)/baytrail/Android.mk
  endif

endif
