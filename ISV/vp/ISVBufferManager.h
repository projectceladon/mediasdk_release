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

#ifndef __ISV_BUFMANAGER_H
#define __ISV_BUFMANAGER_H

#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Errors.h>
#include "ISVBuffer.h"
#ifndef TARGET_VPP_USE_GEN
#include "hal_public.h"
#endif

namespace intel {
namespace isv {

#define ISV_BUFFER_MANAGER_DEBUG 0

class ISVWorker;

class ISVBufferManager: public RefBase
{
public:
    ISVBufferManager()
        :mWorker(NULL),
        mMetaDataMode(false),
        mNeedClearBuffers(false) {}

    ~ISVBufferManager() {}
    // set mBuffers size
    status_t setBufferCount(int32_t size);

    // register/unregister ISVBuffers to mBuffers
    status_t useBuffer(const sp<ANativeWindowBuffer> nativeBuffer, unsigned long omxHandle);
    status_t useBuffer(unsigned long handle, unsigned long omxHandle);
    status_t freeBuffer(unsigned long handle);

    // Map to ISVBuffer
    IISVBuffer* mapBuffer(unsigned long handle);
    // set isv worker
    void setWorker(sp<ISVWorker> worker) { mWorker = worker; }
    void setMetaDataMode(bool metaDataMode) { mMetaDataMode = metaDataMode; }
    // set buffer flag.
    status_t setBuffersFlag(uint32_t flag);
private:
    typedef enum {
        GRALLOC_BUFFER_MODE = 0,
        META_DATA_MODE = 1,
    } ISV_WORK_MODE;

    sp<ISVWorker> mWorker;
    bool mMetaDataMode;
    // VPP buffer queue
    Vector<IISVBuffer*> mBuffers;
    Mutex mBufferLock; // to protect access to mBuffers
    bool mNeedClearBuffers;
};

} // namespace intel
} // namespace isv

#endif //#define __ISV_BUFMANAGER_H
