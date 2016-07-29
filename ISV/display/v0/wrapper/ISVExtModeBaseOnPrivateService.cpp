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
#include "ISVExtModeBaseOnPrivateService.h"
#undef LOG_TAG
#define LOG_TAG "ISVDPY"


namespace intel {
namespace isv {

ISVExtModeBaseOnPrivateService::ISVExtModeBaseOnPrivateService() {
    ALOGV("Entering %s, %p", __func__, this);
    mSessionId = -1;
    mVideo = NULL;
    mState = false;
    mFPSetted = false;
}

void ISVExtModeBaseOnPrivateService::close() {
    mSessionId = -1;
    mState = false;
    mVideo = NULL;
    mFPSetted = false;
}

ISVExtModeBaseOnPrivateService::~ISVExtModeBaseOnPrivateService() {
    ALOGV("Entering %s, %p", __func__, this);
    close();
}

sp<IMDService> ISVExtModeBaseOnPrivateService::getService() {
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == NULL) {
        ALOGW("%s: Failed to get service manager", __func__);
        return NULL;
    }
    sp<IMDService> mds = interface_cast<IMDService>(
            sm->getService(String16(INTEL_MDS_SERVICE_NAME)));
    if (mds == NULL) {
        ALOGW("%s: Failed to get MDS service", __func__);
        return NULL;
    }
    return mds;
}


status_t ISVExtModeBaseOnPrivateService::setVideoFPS(int64_t videoHandler, int32_t FPS) {
    ISV_UNUSED(videoHandler)
    status_t ret = UNKNOWN_ERROR;
    if (!mState || mVideo == NULL || mSessionId < 0)
        return UNKNOWN_ERROR;
    if (mFPSetted) {
        return UNKNOWN_ERROR;
    }
    ALOGV("Entering %s:%d, Set video FPS %d, %p", __func__, __LINE__, FPS, this);
    MDSVideoSourceInfo info;
    memset(&info, 0, sizeof(MDSVideoSourceInfo));
    info.frameRate = FPS;
    mVideo->updateVideoSourceInfo(mSessionId, info);

    mFPSetted = true;
    ALOGV("Leaving %s:%d, Success to set video FPS, %p",  __func__, __LINE__, this);
    return NO_ERROR;
}

status_t ISVExtModeBaseOnPrivateService::setVideoState(int64_t videoHandler, bool isPlaying) {
    ISV_UNUSED(videoHandler)
    if (isPlaying == mState) {
        ALOGV("Same playback state %d, Ignore it", isPlaying);
        return UNKNOWN_ERROR;
    }
    status_t ret = UNKNOWN_ERROR;
    ALOGV("Entering %s:%d, Set video state %d, %p", __func__, __LINE__, isPlaying, this);
    if (mVideo == NULL) {
        sp<IMDService> mds = getService();
        if (mds == NULL) {
            return UNKNOWN_ERROR;
        }
        mVideo = mds->getVideoControl();
    }
    if (mSessionId < 0) {
        mSessionId = mVideo->allocateVideoSessionId();
    }
    MDS_VIDEO_STATE state = MDS_VIDEO_PREPARING;
    if (isPlaying)
        state = MDS_VIDEO_PREPARED;
    else
        state = MDS_VIDEO_UNPREPARED;
    mVideo->updateVideoState(mSessionId, state);
    if (!isPlaying)
        close();
    mState = isPlaying;
    ALOGV("Leaving %s:%d,Success to set video state, %p", __func__, __LINE__, this);
    return NO_ERROR;
}

} // namespace isv
} // namespace intel

