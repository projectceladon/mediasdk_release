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

#include <media/hardware/HardwareAPI.h>
#include <system/graphics.h>
#include "ISVBufferManager.h"
#include "ISVBuffer.h"
#ifndef TARGET_VPP_USE_GEN
#include "hal_public.h"
#endif

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "isv-omxil"

namespace intel {
namespace isv {

status_t ISVBufferManager::setBufferCount(int32_t size)
{
    Mutex::Autolock autoLock(mBufferLock);
#if 0
    if (!mBuffers.isEmpty()) {
        ALOGE("%s: the buffer queue should be empty before we set its size", __func__);
        return STATUS_ERROR;
    }
#endif
    mBuffers.setCapacity(size);

    return OK;
}

status_t ISVBufferManager::freeBuffer(unsigned long handle)
{
    Mutex::Autolock autoLock(mBufferLock);
    for (uint32_t i = 0; i < mBuffers.size(); i++) {
        IISVBuffer* isvBuffer = mBuffers.itemAt(i);
        if (isvBuffer->getHandle() == handle) {
            delete isvBuffer;
            mBuffers.removeAt(i);
            ALOGD_IF(ISV_BUFFER_MANAGER_DEBUG, "%s: remove handle 0x%08x, and then mBuffers.size() %d", __func__,
                    handle, mBuffers.size());
            return OK;
        }
    }

    ALOGW("%s: can't find buffer %u", __func__, handle);
    return UNKNOWN_ERROR;
}

status_t ISVBufferManager::useBuffer(
        unsigned long handle,
        unsigned long omxHandle)
{
    Mutex::Autolock autoLock(mBufferLock);
    if (handle == 0 || mBuffers.size() >= mBuffers.capacity())
        return BAD_VALUE;

    for (uint32_t i = 0; i < mBuffers.size(); i++) {
        IISVBuffer* isvBuffer = mBuffers.itemAt(i);
        if (isvBuffer->getHandle() == handle) {
            ALOGE("%s: this buffer 0x%08x has already been registered", __func__, handle);
            return UNKNOWN_ERROR;
        }
    }
    IISVBuffer* isvBuffer = new ISVBuffer(mWorker, handle, omxHandle,
            mMetaDataMode ? IISVBuffer::ISV_BUFFERTYPE_METADATA : IISVBuffer::ISV_BUFFERTYPE_GRALLOC,
            mNeedClearBuffers ? IISVBuffer::ISV_BUFFERFLAG_DIRTY : 0);
    ALOGD_IF(ISV_BUFFER_MANAGER_DEBUG, "%s: add handle 0x%08x, and then mBuffers.size() %d", __func__,
            handle, mBuffers.size());
    mBuffers.push_back(isvBuffer);
    return OK;

}

status_t ISVBufferManager::useBuffer(
        const sp<ANativeWindowBuffer> nativeBuffer,
        unsigned long omxHandle)
{
    Mutex::Autolock autoLock(mBufferLock);
    if (nativeBuffer == NULL || mBuffers.size() >= mBuffers.capacity())
        return BAD_VALUE;

    for (uint32_t i = 0; i < mBuffers.size(); i++) {
        IISVBuffer* isvBuffer = mBuffers.itemAt(i);
        if (isvBuffer->getHandle() == (unsigned long)nativeBuffer->handle) {
            ALOGE("%s: this buffer 0x%08x has already been registered", __func__, nativeBuffer->handle);
            return UNKNOWN_ERROR;
        }
    }

    IISVBuffer* isvBuffer = new ISVBuffer(mWorker,
            (unsigned long)nativeBuffer->handle,
            (unsigned long)nativeBuffer->handle,
            omxHandle,
            nativeBuffer->width, nativeBuffer->height,
            nativeBuffer->stride, nativeBuffer->format,
            mMetaDataMode ? IISVBuffer::ISV_BUFFERTYPE_METADATA : IISVBuffer::ISV_BUFFERTYPE_GRALLOC,
            mNeedClearBuffers ? IISVBuffer::ISV_BUFFERFLAG_DIRTY : 0);

    ALOGD_IF(ISV_BUFFER_MANAGER_DEBUG, "%s: add handle 0x%08x, and then mBuffers.size() %d", __func__,
            nativeBuffer->handle, mBuffers.size());
    mBuffers.push_back(isvBuffer);
    return OK;
}

IISVBuffer* ISVBufferManager::mapBuffer(unsigned long handle)
{
    Mutex::Autolock autoLock(mBufferLock);
    for (uint32_t i = 0; i < mBuffers.size(); i++) {
        IISVBuffer* isvBuffer = mBuffers.itemAt(i);
        if (isvBuffer->getHandle() == handle)
            return isvBuffer;
    }
    return NULL;
}

status_t ISVBufferManager::setBuffersFlag(uint32_t flag)
{
    Mutex::Autolock autoLock(mBufferLock);

    if (flag & IISVBuffer::ISV_BUFFERFLAG_DIRTY) {
        if (mBuffers.size() == 0)
            mNeedClearBuffers = true;
        else {
            for (uint32_t i = 0; i < mBuffers.size(); i++) {
                IISVBuffer* isvBuffer = mBuffers.itemAt(i);
                isvBuffer->setFlag(IISVBuffer::ISV_BUFFERFLAG_DIRTY);
            }
        }
    }
    return OK;
}

} // namespace intel
} // namespace isv
