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
#include "ISVBuffer.h"
#ifndef TARGET_VPP_USE_GEN
#include "hal_public.h"
#else
#include "gralloc.h"
#endif

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "isv-omxil"

namespace intel {
namespace isv {

#define GRALLOC_SUB_BUFFER_MAX  3
#define RANDOM_BUFFER_SIZE      200
static char random_buf[RANDOM_BUFFER_SIZE];

ISVBuffer::~ISVBuffer() {
    if (mWorker != NULL) {
        ALOGV("%s: mSurface %d", __func__, mSurface);
        mWorker->freeSurface(&mSurface);
    }
}

status_t ISVBuffer::initBufferInfo(uint32_t hackFormat)
{
    ISV_UNUSED(hackFormat);
    if (mType == ISV_BUFFERTYPE_METADATA) {
        VideoDecoderOutputMetaData *metaData =
            reinterpret_cast<VideoDecoderOutputMetaData*>(mBuffer);

        if (metaData->eType != kMetadataBufferTypeGrallocSource) {
            ALOGE("%s: unsupported meta data format eType = %d", __func__, metaData->eType);
            return UNKNOWN_ERROR;
        }

        if (mGrallocHandle != 0) {
            if ((unsigned long)metaData->pHandle != mGrallocHandle) {
                if (STATUS_OK != mWorker->freeSurface(&mSurface)) {
                    ALOGE("%s: free surface %d failed.", __func__, mSurface);
                    return UNKNOWN_ERROR;
                }
            } else
                return OK;
        }
        mGrallocHandle = (unsigned long)metaData->pHandle;
    } else {
        if (mSurface != -1)
            return OK;
        mGrallocHandle = mBuffer;
    }

    int32_t err = 0;
    if (!mpGralloc) {
        err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (hw_module_t const**)&mpGralloc);
        if (0 != err)
            return UNKNOWN_ERROR;
    }
#ifdef TARGET_VPP_USE_GEN
    intel_ufo_buffer_details_t info;

    memset(&info, 0, sizeof(intel_ufo_buffer_details_t));
    info.magic = sizeof(intel_ufo_buffer_details_t);
    err = mpGralloc->perform(mpGralloc, INTEL_UFO_GRALLOC_MODULE_PERFORM_GET_BO_INFO, mGrallocHandle, &info);
    if (0 != err) {
        ALOGE("%s: can't get graphic buffer info, mGrallocHandle %p, mType %d", __func__, mGrallocHandle, mType);
    }
    mWidth = info.width;
    mHeight = info.height;
    mStride = info.pitch;
    mColorFormat = info.format;

    if ((info.usage & GRALLOC_USAGE_PROTECTED) != 0) {
        ALOGV("%s: protected stream can't be supported right now, disable ISV.", __func__);
        return BAD_TYPE;
    }
#else
    IMG_native_handle_t* grallocHandle = (IMG_native_handle_t*)mGrallocHandle;
    mStride = grallocHandle->iWidth;
    mSurfaceHeight = grallocHandle->iHeight;
    mColorFormat = (hackFormat != 0) ? hackFormat : grallocHandle->iFormat;
#endif
    if (mWorker == NULL) {
        ALOGE("%s: mWorker == NULL!!", __func__);
        return UNKNOWN_ERROR;
    }

    if (STATUS_OK != mWorker->allocSurface(&mWidth, &mHeight, mStride, mColorFormat, mGrallocHandle, &mSurface)) {
        ALOGE("%s: alloc surface failed, mGrallocHandle %p", __func__, mGrallocHandle);
        return UNKNOWN_ERROR;
    }

    ALOGD_IF(ISV_BUFFER_MANAGER_DEBUG, "%s: mWidth %d, mHeight %d, mStride %d, mColorFormat %d, mGrallocHandle %p, mSurface %d",
            __func__, mWidth, mHeight, mStride, mColorFormat, mGrallocHandle, mSurface);
    return OK;
}

status_t ISVBuffer::clearIfNeed()
{
#ifndef TARGET_VPP_USE_GEN
    static bool bRandomBufferInit = false;
    if (!bRandomBufferInit) {
        time_t my_time;
        srand((unsigned)time(&my_time));
        for (int32_t i = 0; i < RANDOM_BUFFER_SIZE; i++)
            random_buf[i] = (char)(((double)rand()/(double)RAND_MAX) * 255.0);
        bRandomBufferInit = true;
    }

    if ((mFlags & ISV_BUFFERFLAG_DIRTY) && mpGralloc) {
        int32_t usage = GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN;
        void *vaddr[GRALLOC_SUB_BUFFER_MAX];

        int32_t err = mpGralloc->lock(mpGralloc, (buffer_handle_t)mGrallocHandle, usage, 0, 0, mStride, mSurfaceHeight, &vaddr[0]);

        if (0 != err) {
            ALOGE("%s: get graphic buffer ptr failed", __func__);
            return UNKNOWN_ERROR;
        }

        int32_t buffer_size = mStride * mSurfaceHeight * 3 / 2;
        char* ptr = (char*)vaddr[0];
        for (int32_t i = 0; i < buffer_size/RANDOM_BUFFER_SIZE; i++) {
            memcpy(ptr, random_buf, sizeof(random_buf));
            ptr += sizeof(random_buf);
        }
        mpGralloc->unlock(mpGralloc, (buffer_handle_t)mGrallocHandle);
        ALOGD_IF(ISV_BUFFER_MANAGER_DEBUG, "%s: clear isv buffer %p finished, buffer size %d", __func__, this, buffer_size);
        mFlags &= ~ISV_BUFFERFLAG_DIRTY;
    }
#endif
    return OK;
}

} // namespace intel
} // namespace isv
