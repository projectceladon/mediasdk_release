/*
 * Copyright (c) 2012-2013, Intel Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: tianyang.zhu@intel.com
 */


//#define LOG_NDEBUG 0

#include <binder/IServiceManager.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/Surface.h>
#include <cutils/sockets.h>
#include "IMDSExtModeControl.h"
#include "IDisplayControl.h"
#include "IService.h"
#include "ISVExtModeBaseOnHwcService.h"
#include <ISVProfile.h>
#include <inttypes.h>
#undef LOG_TAG
#define LOG_TAG "ISVDPY"


using namespace intel::ufo::hwc::services;

namespace intel {
namespace isv {

#define VIDEO_CLIENT_MSG_LENGTH 128
#define EXTMODE_OBSERVER_SERVER_SOCKET "intel_extmode\0"

ISVExtModeBaseOnHwcService::ISVExtModeBaseOnHwcService() {
    ALOGV("Entering %s", __func__);
    mHdmiAutoSet = false;
    mMipiAutOff  = false;
    mState = false;
    mFPSetted = false;
}

ISVExtModeBaseOnHwcService::~ISVExtModeBaseOnHwcService() {
    ALOGV("Entering %s", __func__);
    mHdmiAutoSet = false;
    mMipiAutOff  = false;
    mState = false;
    mFPSetted = false;
}

status_t ISVExtModeBaseOnHwcService::getDefaultExtModeState(bool* hasHDMIAutoSet, bool* hasMIPIAutOff) {
    uint32_t hdmi = 0;
    uint32_t mipi = 0;
    status_t ret = ISVProfile::GetInstance()->getISVSetting(ISV_DISPLAY_HDMI_AUTO, &hdmi);
    if (ret != OK) {
        hdmi = 0;
    }
    ret = ISVProfile::GetInstance()->getISVSetting(ISV_DISPLAY_MIPI_AUTO, &mipi);
    if (ret != OK) {
        mipi = 0;
    }
    *hasHDMIAutoSet = ((hdmi == 1 ? true : false));
    *hasMIPIAutOff = ((mipi == 1 ? true : false));
    return NO_ERROR;
}

status_t ISVExtModeBaseOnHwcService::setRealExtModeState(bool hasHDMIAutoSet, bool hasMIPIAutOff) {
    ALOGV("Entering %s, set real state %d, %d", __func__, hasHDMIAutoSet, hasMIPIAutOff);
    mHdmiAutoSet = hasHDMIAutoSet;
    mMipiAutOff  = hasMIPIAutOff;
    return NO_ERROR;
};

status_t ISVExtModeBaseOnHwcService::setInputState(bool isActive) {
    //ALOGV("Entering %s, real state is %d, %d", __func__, mHdmiAutoSet, mMipiAutOff);
    status_t ret = UNKNOWN_ERROR;
    sp<IService> hwcS = interface_cast<IService>(
           defaultServiceManager()->getService(String16(INTEL_HWC_SERVICE_NAME)));
    if (hwcS == NULL)
        return ret;
    sp<IMDSExtModeControl> hwcExt = hwcS->getMDSExtModeControl();
    if (hwcExt != NULL) {
        hwcExt->updateInputState(isActive);
        ret = NO_ERROR;
    }
    return ret;
}

status_t ISVExtModeBaseOnHwcService::setVideoFPS(int64_t videoHandler, int32_t FPS) {
    status_t ret = UNKNOWN_ERROR;
    if (!mMipiAutOff && !mHdmiAutoSet) {
        ALOGI("Video extended mode feature isn't enbale, %d, %d", mHdmiAutoSet, mMipiAutOff);
        return ret;
    }
    if (mFPSetted) {
        return UNKNOWN_ERROR;
    }
    ALOGV("Entering %s:%d, Set video FPS %d, %p", __func__, __LINE__, FPS, this);
    sp<IService> hwcS = interface_cast<IService>(
           defaultServiceManager()->getService(String16(INTEL_HWC_SERVICE_NAME)));
    if (hwcS == NULL)
        return ret;
    sp<IMDSExtModeControl> hwcExt = hwcS->getMDSExtModeControl();
    if (hwcExt != NULL) {
        hwcExt->updateVideoFPS(videoHandler, FPS);
        ret = NO_ERROR;
    }
    mFPSetted = true;
    ALOGV("Leaving %s:%d, Success to set video FPS, %p",  __func__, __LINE__, this);
    return ret;
}

status_t ISVExtModeBaseOnHwcService::setVideoState(int64_t videoHandler, bool isPlaying) {
    status_t ret = UNKNOWN_ERROR;
    if (!mMipiAutOff && !mHdmiAutoSet) {
        ALOGI("Video extended mode feature isn't enbale, %d, %d", mHdmiAutoSet, mMipiAutOff);
        return ret;
    }
    if (isPlaying == mState) {
        ALOGV("Same playback state %d, Ignore it", isPlaying);
        return UNKNOWN_ERROR;
    }
    ALOGV("Set video session %" PRId64 " state %d", videoHandler, isPlaying);
    sp<IService> hwcS = interface_cast<IService>(
           defaultServiceManager()->getService(String16(INTEL_HWC_SERVICE_NAME)));
    if (hwcS == NULL)
        return ret;
    sp<IMDSExtModeControl> hwcExt = hwcS->getMDSExtModeControl();
    if (hwcExt != NULL) {
        hwcExt->updateVideoState(videoHandler, isPlaying);
        ret = NO_ERROR;
    }
    informObserver(videoHandler, isPlaying);
    mState = isPlaying;
    ALOGV("Leaving %s:%d,Success to set video state, %p", __func__, __LINE__, this);
    return ret;
}

status_t ISVExtModeBaseOnHwcService::informObserver(int64_t videoSessionID, bool isPlaying) {
    status_t ret = UNKNOWN_ERROR;
    if (!mMipiAutOff && !mHdmiAutoSet) {
        ALOGI("Video extended mode feature isn't enbale, %d, %d", mHdmiAutoSet, mMipiAutOff);
        return ret;
    }
    ALOGV("Inform ExtModeObserver");
    char msg[VIDEO_CLIENT_MSG_LENGTH] = "\0";
    int32_t pos = sprintf(msg, "%" PRId64, videoSessionID);
    pos += sprintf(msg + pos, ":%d", (isPlaying ? 1 : 0));
    ALOGV("Inform EXTModeObserver %s", msg);

    int sock = socket_local_client(EXTMODE_OBSERVER_SERVER_SOCKET,
            ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (sock >= 0) {
        if (write(sock, msg, strlen(msg) + 1) > 0) {
            close(sock);
            return (ret = OK);
        }
        ALOGE("Fail to write msg into extmode server socket");
        close(sock);
    }
    ALOGW("Fail to inform ExtModeObserver, %d", sock);
    return ret;
}

status_t ISVExtModeBaseOnHwcService::setPhoneCallState(bool isCalling) {
    status_t ret = UNKNOWN_ERROR;
    sp<IService> hwcS = interface_cast<IService>(
           defaultServiceManager()->getService(String16(INTEL_HWC_SERVICE_NAME)));
    if (hwcS == NULL)
        return ret;
    sp<IDisplayControl> pDpy = hwcS->getDisplayControl(HWC_DISPLAY_EXTERNAL);
    if (pDpy == NULL)
        return ret;
    sp<IDisplayBlankControl> blankC = pDpy->getBlankControl();
    if (blankC != NULL) {
        ret = blankC->enableBlank(!isCalling);
    }
    return ret;
}

} // namespace isv
} // namespace intel

