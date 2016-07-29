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
#include "ISVDisplay.h"
#include "ISVDisplayStatus.h"
#include <ISVType.h>

#undef LOG_TAG
#define LOG_TAG "ISVDPY"


namespace intel {
namespace isv {

ISVDisplay::ISVDisplay() {
    ALOGV("Entering %s, %p", __func__, this);
    mExtControl = ISVExtModeFactory::getExtMode();
}

ISVDisplay::~ISVDisplay() {
    ALOGV("Entering %s, %p", __func__, this);
    if (mExtControl != NULL) {
        delete mExtControl;
        mExtControl = NULL;
    }
}

void ISVDisplay::dumpDpyInfo(DisplayInfo* info) {
    if (info == NULL)
        return;
    ALOGV("Info(%d): [%dx%d@%fHz]", sizeof(DisplayInfo),
            info->w, info->h, info->fps);
}

status_t ISVDisplay::getDpyState(uint32_t dpyID, bool* connected) {
    *connected = false;
    if (dpyID != HWC_DISPLAY_PRIMARY &&
            dpyID != HWC_DISPLAY_EXTERNAL &&
            dpyID != HWC_DISPLAY_VIRTUAL)
        return UNKNOWN_ERROR;
    sp<ISVDisplayStatus> edpy = new ISVDisplayStatus();
    *connected = edpy->getConnected(dpyID);
    return NO_ERROR;
}

status_t ISVDisplay::getActiveDpyInfo(uint32_t dpyID, DisplayInfo* info) {
    status_t ret = UNKNOWN_ERROR;
    if (info == NULL)
        return ret;
    if (dpyID == HWC_DISPLAY_VIRTUAL) {
        ALOGW("Couldn't get virtual display info");
        return ret;
    }
    if (dpyID == HWC_DISPLAY_EXTERNAL) {
        sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
                    ISurfaceComposer::eDisplayIdHdmi));
        if (display != NULL && display.get() != NULL)
            ret = SurfaceComposerClient::getDisplayInfo(display, info);
    } else {
        sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
                    ISurfaceComposer::eDisplayIdMain));
        if (display != NULL && display.get() != NULL)
            ret = SurfaceComposerClient::getDisplayInfo(display, info);
    }
    if (ret == NO_ERROR) {
       dumpDpyInfo(info);
    }
    return ret;
}

status_t ISVDisplay::getDpyInfo(uint32_t dpyID, Vector<DisplayInfo>* info) {
    status_t ret = UNKNOWN_ERROR;
    if (info == NULL || info->size() <= 0)
        return ret;
    if (dpyID == HWC_DISPLAY_VIRTUAL) {
        ALOGW("Couldn't get virtual display info");
        return ret;
    }
    if (dpyID == HWC_DISPLAY_EXTERNAL) {
        sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
                    ISurfaceComposer::eDisplayIdHdmi));
        if (display != NULL && display.get() != NULL)
            ret = SurfaceComposerClient::getDisplayConfigs(display, info);
    } else {
        sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
                    ISurfaceComposer::eDisplayIdMain));
        if (display != NULL && display.get() != NULL)
            ret = SurfaceComposerClient::getDisplayConfigs(display, info);
    }
    if (ret == NO_ERROR) {
        for (int i = 0; i < info->size(); i++) {
            DisplayInfo tmp = info->itemAt(i);
            dumpDpyInfo(&tmp);
        }
    }
    return ret;
}

} // namespace isv
} // namespace intel

