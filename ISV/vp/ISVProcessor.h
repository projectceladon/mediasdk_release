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

#ifndef __ISV_PROCESSOR_H
#define __ISV_PROCESSOR_H


#include <utils/Errors.h>
#include "ISVType.h"
#include "ISVProfile.h"
#include "ISVBufferManager.h"
#include "ISVThread.h"
#include "ISVWorker.h"

#define ISV_PROCESSOR_DEBUG 0

namespace intel {
namespace isv {

class ISVBufferManager;
class ISVThread;

class ISVProcessor : public IISVProcessor
{
public:
    /*
     * observer == NULL only for sync mode.
     */
    ISVProcessor(ISV_WORKMODE mode, ISVProcessorCallBack* callback);
    virtual ~ISVProcessor();

    //init/deinit func
    status_t init(uint32_t width, uint32_t height);
    status_t deinit();

    // get vpp settings
    bool isVPPOn() { return ISVProfile::isVPPOn(); }
    bool isFRCOn() { return ISVProfile::isFRCOn(); }
    // query platform filter caps
    status_t getFilterCaps(ISVFilterCapability *caps, uint32_t *count);
    // config vpp filters
    status_t configFilters(ISVFilterParameter *filterParam, uint32_t count, int64_t timeStamp, uint32_t flags);

    // set mBuffers size
    status_t setBufferCount(int32_t size);
    // register/unregister ISVBuffers to mBuffers
    status_t useBuffer(const sp<ANativeWindowBuffer> nativeBuffer, unsigned long omxHandle);
    status_t useBuffer(unsigned long handle, unsigned long omxHandle);
    status_t freeBuffer(unsigned long handle);
    // Map to ISVBuffer
    IISVBuffer* mapBuffer(unsigned long handle);
    // set all buffers's flag.
    status_t setBuffersFlag(uint32_t flag);

    // for aync mode, add output buffer
    status_t addOutput(IISVBuffer* output);
    // for async mode, add intput buffer
    status_t addInput(IISVBuffer* input);
    // for async mode, notify buffer queue flash
    status_t notifyFlush();
    // for async mode, wait buffer queue flash finished
    status_t waitFlushFinished();

    //for sync mode, directly fill output
    status_t process(IISVBuffer* input, Vector<IISVBuffer*> output, uint32_t outputCount, uint32_t flags);
    void setMetaDataMode(bool metaDataMode);

private:
    //return whether this fps is valid
    inline bool isFrameRateValid(float fps);
    //auto calculate fps if the framework doesn't set the correct fps
    status_t calculateFps(int64_t timeStamp, float* fps);

private:
    ISV_WORKMODE mWorkMode;
    sp<ISVWorker> mWorker;
    sp<ISVBufferManager> mBufferManager;

    ISVProfile* mProfile;
    // for async mode
    sp<ISVThread> mThread;
    ISVProcessorCallBack* mpOwner;

    uint32_t mNumRetry;
    const static uint32_t WINDOW_SIZE = 4;  // must >= 2
    Vector<int64_t>mTimeWindow;
};

} // namespace intel
} // namespace isv

#endif /* __ISV_PROCESSOR_H*/
