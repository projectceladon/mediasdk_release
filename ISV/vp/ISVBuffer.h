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

#ifndef __ISV_BUFFER_H
#define __ISV_BUFFER_H

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <hardware/gralloc.h>
#include "ISVVp.h"
#include "ISVWorker.h"
#ifndef TARGET_VPP_USE_GEN
#include "hal_public.h"
#endif

namespace intel {
namespace isv {

class ISVWorker;

class ISVBuffer : public IISVBuffer
{
public:
    virtual ~ISVBuffer();

    // init buffer info
    // FIXME: hackFormat is for VP9, should be removed in future
    status_t initBufferInfo(uint32_t hackFormat);

    // get va surface
    int32_t getSurface() { return mSurface; }
    // get buffer handle
    unsigned long getHandle() { return mBuffer; }
    // get omx buffer handle
    unsigned long getOMXHandle() { return mOMXHandle; }
    // get/set time stamp
    int64_t getTimeStamp() { return mTimeStamp; }
    void setTimeStamp(int64_t timeStamp) { mTimeStamp = timeStamp; return; }
    // set/clear/get flag
    uint32_t getFlags() { return mFlags; }
    void setFlag(uint32_t flag) { mFlags |= flag; return; }
    void unsetFlag(uint32_t flag) { mFlags &= ~flag; return; }
    status_t clearIfNeed();

protected:
    ISVBuffer(sp<ISVWorker> worker,
            unsigned long buffer,
            unsigned long grallocHandle,
            unsigned long omxHandle,
            uint32_t width, uint32_t height,
            uint32_t stride, uint32_t colorFormat,
            ISV_BUFFERTYPE type, uint32_t flag)
        :mWorker(worker),
        mBuffer(buffer),
        mGrallocHandle(grallocHandle),
        mOMXHandle(omxHandle),
        mWidth(width),
        mHeight(height),
        mSurfaceHeight(0),
        mStride(stride),
        mColorFormat(colorFormat),
        mType(type),
        mSurface(-1),
        mFlags(flag),
        mTimeStamp(-1),
        mpGralloc(NULL) {}

    ISVBuffer(sp<ISVWorker> worker,
            unsigned long buffer,
            unsigned long omxHandle,
            ISV_BUFFERTYPE type,
            uint32_t flag)
        :mWorker(worker),
        mBuffer(buffer),
        mGrallocHandle(0),
        mOMXHandle(omxHandle),
        mWidth(0),
        mHeight(0),
        mSurfaceHeight(0),
        mStride(0),
        mColorFormat(0),
        mType(type),
        mSurface(-1),
        mFlags(flag),
        mTimeStamp(-1),
        mpGralloc(NULL) {}

private:
    friend class ISVBufferManager;
    sp<ISVWorker> mWorker;
    unsigned long mBuffer;
    unsigned long mGrallocHandle;
    unsigned long mOMXHandle;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mSurfaceHeight;
    uint32_t mStride;
    uint32_t mColorFormat;
    ISV_BUFFERTYPE mType;
    int32_t mSurface;
    uint32_t mFlags;
    int64_t mTimeStamp;
    gralloc_module_t* mpGralloc;
};

} // namespace intel
} // namespace isv

#endif //#define __ISV_BUFFER_H
