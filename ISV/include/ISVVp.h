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

#ifndef __ISV_VP_H
#define __ISV_VP_H

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <utils/Vector.h>
#include <system/window.h>
#include "ISVType.h"

namespace intel {
namespace isv {

using namespace android;

class IISVBuffer
{
public:
    typedef enum {
        ISV_BUFFERTYPE_GRALLOC,
        ISV_BUFFERTYPE_METADATA,
    } ISV_BUFFERTYPE;

    typedef enum {
        ISV_BUFFERFLAG_DIRTY            = 0x00000001,
        ISV_BUFFERFLAG_CROP_CHANGED     = 0x00000002,
        ISV_BUFFERFLAG_EOS              = 0x00000004,
    } ISV_BUFFERFLAG;

public:
    IISVBuffer() {}
    virtual ~IISVBuffer() {}

    // init buffer info
    // FIXME: hackFormat is for VP9, should be removed in future
    virtual status_t initBufferInfo(uint32_t hackFormat) = 0;

    // get va surface
    virtual int32_t getSurface() = 0;
    // get buffer handle
    virtual unsigned long getHandle() = 0;
    // get omx buffer handle
    virtual unsigned long getOMXHandle() = 0;
    // get/set time stamp
    virtual int64_t getTimeStamp() = 0;
    virtual void setTimeStamp(int64_t timeStamp) = 0;
    // set/clear/get flag
    virtual uint32_t getFlags() = 0;
    virtual void setFlag(uint32_t flag) = 0;
    virtual void unsetFlag(uint32_t flag) = 0;
    virtual status_t clearIfNeed() = 0;
};

/* for async mode, need app implement this interface to release intput/output buffer
 *
 */
class ISVProcessorCallBack
{
public:
    ISVProcessorCallBack() {}
    virtual ~ISVProcessorCallBack() {}

public:
    virtual status_t releaseBuffer(PORT_INDEX index, IISVBuffer* pBuffer, bool bFlush) = 0;
    virtual status_t reportOutputCrop() = 0;
};


/* common interface for VPP
 * 1. support two methods to configure vpp filters, from user or from profile
 * 2. support both sync and async mode
 */
class IISVProcessor
{
public:
    typedef enum {
        ISV_ASYNC_MODE = 0,
        ISV_SYNC_MODE,
    } ISV_WORKMODE;

    IISVProcessor() {}
    virtual ~IISVProcessor() {}

public:
    //init/deinit func
    virtual status_t init(uint32_t width, uint32_t height) = 0;
    virtual status_t deinit() = 0;

    // get vpp settings
    virtual bool isVPPOn() = 0;
    virtual bool isFRCOn() = 0;
    // get platform filter caps
    virtual status_t getFilterCaps(ISVFilterCapability *caps, uint32_t *count) = 0;
    // config vpp filters
    virtual status_t configFilters(ISVFilterParameter *filterParam, uint32_t count, int64_t timeStamp, uint32_t flags) = 0;

    // set mBuffers size
    virtual status_t setBufferCount(int32_t size) = 0;
    // register/unregister IISVBuffers to mBuffers
    virtual status_t useBuffer(const sp<ANativeWindowBuffer> nativeBuffer, unsigned long omxHandle) = 0;
    virtual status_t useBuffer(unsigned long handle, unsigned long omxHandle) = 0;
    virtual status_t freeBuffer(unsigned long handle) = 0;
    // Map to IISVBuffer
    virtual IISVBuffer* mapBuffer(unsigned long handle) = 0;
    // set all buffers's flag.
    virtual status_t setBuffersFlag(uint32_t flag) = 0;

    // for aync mode, add output buffer
    virtual status_t addOutput(IISVBuffer* output) = 0;
    // for async mode, add intput buffer
    virtual status_t addInput(IISVBuffer* input) = 0;
    // for async mode, notify buffer queue flash
    virtual status_t notifyFlush() = 0;
    // for async mode, wait buffer queue flash finished
    virtual status_t waitFlushFinished() = 0;

    //for sync mode, directly fill output
    virtual status_t process(IISVBuffer* input, Vector<IISVBuffer*> output, uint32_t outputCount, uint32_t flags) = 0;
    virtual void setMetaDataMode(bool metaDataMode) = 0;
};

} // namespace intel
} // namespace isv

#endif //#define __ISV_VP_H
