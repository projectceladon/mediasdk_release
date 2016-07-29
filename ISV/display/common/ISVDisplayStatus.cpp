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
#include "ISVDisplayStatus.h"

#undef LOG_TAG
#define LOG_TAG "ISVDPY"

namespace intel {
namespace isv {

ISVDisplayStatus::ISVDisplayStatus() {
    mPriConnected = false;
    mVirConnected = false;
    mExtConnected = false;
}

ISVDisplayStatus::~ISVDisplayStatus() {
    mPriConnected = false;
    mVirConnected = false;
    mExtConnected = false;
}

String16 ISVDisplayStatus::get_interface_name(sp<IBinder> service) {
    if (service != NULL) {
        Parcel data, reply;
        status_t err = service->transact(IBinder::INTERFACE_TRANSACTION, data, &reply);
        if (err == NO_ERROR) {
            return reply.readString16();
        }
    }
    return String16();
}

int32_t ISVDisplayStatus::getLogicalIds(int32_t* ids, int32_t size) {
    if (ids == NULL || size != IDS_MAX_NUM)
        return 0;
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == NULL) {
        ALOGE("Unable to get default service manager!");
        return -1;
    }
    sp<IBinder> service = sm->checkService(String16("display"));
    if (sm == NULL) {
        ALOGE("Unable to get display service");
        return 0;
    }
    String16 ifName = get_interface_name(service);
    if (ifName.size() <= 0)
        return 0;
    Parcel dataIds, replyIds;
    dataIds.writeInterfaceToken(ifName);
    int32_t code = 2;
    service->transact(code, dataIds, &replyIds);
    int ret = replyIds.readInt32();
    int ssize = replyIds.dataSize();
    int num = replyIds.readInt32();
    if (ret != 0 || ssize <= 0 || num <= 0){
        ALOGW("size: %d, dpy num: %d, ret %d", ssize, num, ret);
        return 0;
    }
    for (int i = 0; i < num; i++) {
        ids[i] = replyIds.readInt32();
        //ALOGI("Logiccal ID[%d] : %d, num: %d", i, ids[i], num);
    }
    return num;
}

bool ISVDisplayStatus::getConnected(int dpyID) {
    //ALOGI("Entering");
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == NULL) {
        ALOGE("Unable to get default service manager!");
        return false;
    }
    sp<IBinder> service = sm->checkService(String16("display"));
    if (sm == NULL) {
        ALOGE("Unable to get display service");
        return false;
    }
    String16 ifName = get_interface_name(service);
    if (ifName.size() <= 0)
        return false;
    // getIds and num;
    bool pri = false;
    bool ext = false;
    bool vir = false;
    int32_t ids[IDS_MAX_NUM] = {-1, -1, -1};
    int32_t num = getLogicalIds(ids, IDS_MAX_NUM);
    for (int i = 0; i < num; i++) {
        int32_t id = ids[i];
        if (id == -1)
            continue;
        Parcel data, reply;
        data.writeInterfaceToken(ifName);
        data.writeInt32(id);
        int code = 1;
        service->transact(code, data, &reply);
        int ret = reply.readInt32();
        int size = reply.dataSize();
        if (ret != 0 || size <= 0)
            return false;
        for (int i = 0; i < REAL_INFO_START_POSITION; i++) {
            int value = reply.readInt32();
            //ALOGI("Data[%d] is 0x%x", i+1, value);
        }
        int32_t flag = reply.readInt32();
        if (flag == BUILT_IN_SCREEN_FLAG) {
            pri = true;
        } else if (flag == EXTERNAL_SCREEN_FLAG) {
            ext = true;
        } else if (flag == VIRTUAL_SCREEN_FLAG) {
            vir = true;
        }
    }
    bool result = false;
    if (dpyID == HWC_DISPLAY_PRIMARY) {
        result = mPriConnected = pri;
    } else if (ext && dpyID == HWC_DISPLAY_EXTERNAL) {
        result = mExtConnected = ext;
    } else if (vir && dpyID == HWC_DISPLAY_VIRTUAL) {
        result = mVirConnected = vir;
    }
    ALOGV("Display[%d] is connected %d", dpyID, result);
    return result;
}

}//
}//
