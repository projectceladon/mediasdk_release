
/*
 * Copyright (C) 2012 Intel Corporation.  All rights reserved.
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
 */

#ifndef __ISVDISPLAY_STATUS_H__
#define __ISVDISPLAY_STATUS_H__

#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/RefBase.h>

#include <hardware/hwcomposer_defs.h>

namespace intel {
namespace isv {

using namespace android;

#define IDS_MAX_NUM 3
#define BUILT_IN_SCREEN_FLAG 0x00750042
#define EXTERNAL_SCREEN_FLAG 0x00440048
#define VIRTUAL_SCREEN_FLAG  0x0039003a
#define REAL_INFO_START_POSITION 6

class ISVDisplayStatus : public RefBase {
private:
    bool mPriConnected;
    bool mVirConnected;
    bool mExtConnected;
    static String16 get_interface_name(sp<IBinder> service);
    int32_t getLogicalIds(int32_t* ids, int32_t size);
public:
    ISVDisplayStatus();
    ~ISVDisplayStatus();
    bool getConnected(int dpyID);
};

}// namespace isv
}// namespace intel

#endif // __ISVDISPLAY_STATUS_H__
