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

#include <math.h>
#include <utils/Errors.h>
#include "ISVProcessor.h"

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "isv-omxil"

namespace intel {
namespace isv {

#define MAX_RETRY_NUM   10

ISVProcessor::ISVProcessor(
        ISV_WORKMODE mode,
        ISVProcessorCallBack* owner)
    :mWorkMode(mode),
    mWorker(NULL),
    mBufferManager(NULL),
    mProfile(NULL),
    mThread(NULL),
    mpOwner(owner),
    mNumRetry(0)
{
    if (mWorkMode == ISV_ASYNC_MODE && mpOwner == NULL)
        ALOGE("%s: mpOwner should not be NULL for async mode", __func__);

    mProfile = ISVProfile::GetInstance();
    if (mProfile == NULL)
        ALOGE("%s: failed to get profile instance", __func__);

    if (mBufferManager == NULL)
        mBufferManager = new ISVBufferManager();

    mTimeWindow.clear();
}

ISVProcessor::~ISVProcessor() {
    ALOGV("ISVProcessor is deleted");
    mProfile = NULL;
    mBufferManager = NULL;
}

status_t ISVProcessor::init(uint32_t width, uint32_t height)
{
    ALOGD_IF(ISV_PROCESSOR_DEBUG, "ISVProcessor::start");

    if (mWorker == NULL) {
        mWorker = new ISVWorker();
        if (mWorker != NULL && STATUS_OK != mWorker->init(width, height)) {
            ALOGE("%s: mWorker init failed", __func__);
            return UNKNOWN_ERROR;
        }
        if (mBufferManager != NULL)
            mBufferManager->setWorker(mWorker);
    }

    if (mWorkMode == ISV_ASYNC_MODE &&
            mThread == NULL) {
        mThread = new ISVThread(false, mBufferManager, mWorker, mpOwner);
        if (mThread != NULL)
            mThread->start();
    }
    return OK;
}

status_t ISVProcessor::deinit()
{
    ALOGD_IF(ISV_PROCESSOR_DEBUG, "ISVProcessor::stop");

    if (mThread != NULL) {
        mThread->stop();
        mThread = NULL;
    }

    if (mWorker != NULL) {
        if (STATUS_OK != mWorker->deinit()) {
            ALOGE("%s: mWorker deinit failed", __func__);
            return UNKNOWN_ERROR;
        }
        mWorker = NULL;
    }

    return OK;
}

status_t ISVProcessor::getFilterCaps(
        ISVFilterCapability *caps,
        uint32_t *count)
{
    if (mProfile == NULL)
        return UNKNOWN_ERROR;
    
    return mProfile->getFilterCapability(caps, count);
}

inline bool ISVProcessor::isFrameRateValid(float fps)
{
    return (fps == 15.0f || fps == 24.0f || fps == 25.0f || fps == 30.0f || fps == 50.0f || fps == 60.0f) ? true : false;
}

status_t ISVProcessor::calculateFps(int64_t timeStamp, float* fps)
{
    int32_t i = 0;
    *fps = 0.0f;

    mTimeWindow.push_back(timeStamp);
    if (mTimeWindow.size() > WINDOW_SIZE) {
        mTimeWindow.removeAt(0);
    }
    else if (mTimeWindow.size() < WINDOW_SIZE)
        return NOT_ENOUGH_DATA;

    int64_t delta = mTimeWindow[WINDOW_SIZE-1] - mTimeWindow[0];
    if (delta == 0)
        return NOT_ENOUGH_DATA;

    *fps = ceil(1.0 / delta * 1E6 * (WINDOW_SIZE-1));

    if (!isFrameRateValid(*fps))
        return NOT_ENOUGH_DATA;

    return OK;
}

status_t ISVProcessor::configFilters(
        ISVFilterParameter *filterParam,
        uint32_t count,
        int64_t timeStamp,
        uint32_t flags)
{
    if (mWorker == NULL)
        return UNKNOWN_ERROR;

    for (uint32_t i = 0; i < count; i++) {
        switch (filterParam[i].filter) {
            case ISVFilterFrameRateConversion:
                if (!isFrameRateValid(filterParam[i].values[0])) {
                    if (mNumRetry++ < MAX_RETRY_NUM) {
                        float fps = 0.0f;
                        if (OK != calculateFps(timeStamp, &fps))
                            return NOT_ENOUGH_DATA;

                        if (fps == 50.0f || fps == 60.0f) {
                            ALOGD_IF(ISV_PROCESSOR_DEBUG, "%s: %d fps don't need do FRC, so disable FRC", __func__, fps);
                            filterParam[i].algorithm = FRC_RATE_1X;
                        } else {
                            filterParam[i].values[0] = fps;
                            ALOGD_IF(ISV_PROCESSOR_DEBUG, "%s: fps is set to %f, frc rate is %d", __func__,
                                    filterParam[i].values[0], filterParam[i].algorithm);
                        }
                    } else {
                        ALOGD_IF(ISV_PROCESSOR_DEBUG, "%s: exceed max retry to get a valid frame rate(%d), disable FRC", __func__,
                                filterParam[i].values[0]);
                        filterParam[i].algorithm = FRC_RATE_1X;
                    }
                }
                break;
            case ISVFilterDeinterlacing:
                if ((flags & OMX_BUFFERFLAG_TFF) == 0 &&
                        (flags & OMX_BUFFERFLAG_BFF) == 0)
                    filterParam[i].algorithm = DEINTERLACE_NONE;
                break;
            default:
                break;
        }
    }

    if (mWorker->configFilters(filterParam, count) != STATUS_OK) {
        ALOGE("%s: error to configure filters", __func__);
        return UNKNOWN_ERROR;
    }

    return OK;
}

status_t ISVProcessor::setBufferCount(int32_t size)
{
    if (mBufferManager == NULL)
        return UNKNOWN_ERROR;

    return mBufferManager->setBufferCount(size);
}

status_t ISVProcessor::freeBuffer(unsigned long handle)
{
    if (mBufferManager == NULL)
        return UNKNOWN_ERROR;

    return mBufferManager->freeBuffer(handle);
}

status_t ISVProcessor::useBuffer(
        unsigned long handle,
        unsigned long omxHandle)
{
    if (mBufferManager == NULL)
        return UNKNOWN_ERROR;

    return mBufferManager->useBuffer(handle, omxHandle);
}

status_t ISVProcessor::useBuffer(
        const sp<ANativeWindowBuffer> nativeBuffer,
        unsigned long omxHandle)
{
    if (mBufferManager == NULL)
        return UNKNOWN_ERROR;

    return mBufferManager->useBuffer(nativeBuffer, omxHandle);
}

IISVBuffer* ISVProcessor::mapBuffer(unsigned long handle)
{
    if (mBufferManager == NULL)
        return NULL;

    return mBufferManager->mapBuffer(handle);
}

status_t ISVProcessor::setBuffersFlag(uint32_t flag)
{
    if (mBufferManager == NULL)
        return UNKNOWN_ERROR;

    return mBufferManager->setBuffersFlag(flag);
}

status_t ISVProcessor::addInput(IISVBuffer* input)
{
    if (mWorkMode != ISV_ASYNC_MODE || mThread == NULL) {
        ALOGE("%s: in the wrong work mode: %d", __func__, mWorkMode);
        return UNKNOWN_ERROR;
    }
    mThread->addInput(input);
    return OK;
}

status_t ISVProcessor::addOutput(IISVBuffer* output)
{
    if (mWorkMode != ISV_ASYNC_MODE || mThread == NULL) {
        ALOGE("%s: in the wrong work mode: %d", __func__, mWorkMode);
        return UNKNOWN_ERROR;
    }
    mThread->addOutput(output);
    return OK;
}

status_t ISVProcessor::notifyFlush()
{
    if (mWorkMode != ISV_ASYNC_MODE || mThread == NULL) {
        ALOGE("%s: in the wrong work mode: %d", __func__, mWorkMode);
        return UNKNOWN_ERROR;
    }
    mThread->notifyFlush();
    return OK;
}

status_t ISVProcessor::waitFlushFinished()
{
    if (mWorkMode != ISV_ASYNC_MODE || mThread == NULL) {
        ALOGE("%s: in the wrong work mode: %d", __func__, mWorkMode);
        return UNKNOWN_ERROR;
    }
    mThread->waitFlushFinished();
    return OK;
}

status_t ISVProcessor::process(
        IISVBuffer* input,
        Vector<IISVBuffer*> output,
        uint32_t outputCount,
        uint32_t flags)
{
    if (mWorkMode != ISV_SYNC_MODE || mThread != NULL) {
        ALOGE("%s: in the wrong work mode: %d", __func__, mWorkMode);
        return UNKNOWN_ERROR;
    }

    status_t ret = mWorker->process(input, output, outputCount, false, true, flags);
    if (ret != STATUS_OK) {
        ALOGE("%s: error to process: %d", __func__, ret);
        return UNKNOWN_ERROR;
    }
    return OK;
}

void ISVProcessor::setMetaDataMode(bool metaDataMode)
{
    if (mBufferManager != NULL)
        mBufferManager->setMetaDataMode(metaDataMode);
}

} // namespace intel
} // namespace isv
