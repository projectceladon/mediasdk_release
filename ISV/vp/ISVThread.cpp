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

#include <utils/Errors.h>
#include "ISVThread.h"

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "isv-omxil"

namespace intel {
namespace isv {

ISVThread::ISVThread(bool canCallJava,
        sp<ISVBufferManager> bufferManager,
        sp<ISVWorker> worker,
        ISVProcessorCallBack* owner)
    :Thread(canCallJava),
    mpOwner(owner),
    mThreadId(NULL),
    mThreadRunning(false),
    mISVWorker(worker),
    mBufferManager(bufferManager),
    mOutputProcIdx(0),
    mInputProcIdx(0),
    mNumTaskInProcesing(0),
    mError(false),
    mbFlush(false),
    mbBypass(false),
    mFlagEnd(false)
{
    mOutputBuffers.clear();
    mInputBuffers.clear();
}

ISVThread::~ISVThread() {
    ALOGV("ISVThread is deleted");
    flush();
    mOutputBuffers.clear();
    mInputBuffers.clear();
}

status_t ISVThread::readyToRun()
{
    mThreadId = androidGetThreadId();
    //do init ops here
    return Thread::readyToRun();
}

void ISVThread::start()
{
    ALOGD_IF(ISV_THREAD_DEBUG, "ISVThread::start");

    this->run("ISVProcessor", ANDROID_PRIORITY_NORMAL);
    mThreadRunning = true;
    return;
}

void ISVThread::stop()
{
    ALOGD_IF(ISV_THREAD_DEBUG, "ISVThread::stop");

    if(mThreadRunning) {
        this->requestExit();
        {
            Mutex::Autolock autoLock(mLock);
            mRunCond.signal();
        }
        this->requestExitAndWait();
        mThreadRunning = false;
    }

    return;
}

bool ISVThread::getBufForFirmwareOutput(Vector<IISVBuffer*> *fillBufList,uint32_t *fillBufNum){
    uint32_t i = 0;
    // output buffer number for filling
    *fillBufNum = 0;
    uint32_t needFillNum = 0;
    IISVBuffer *outputBuffer;

    //output data available
    needFillNum = mISVWorker->getFillBufCount();
    if (mOutputProcIdx < needFillNum ||
            mInputProcIdx < 1) {
        ALOGE("%s: no enough input or output buffer which need to be sync", __func__);
        return false;
    }

    if ((needFillNum == 0) || (needFillNum > 4))
       return false;

    Mutex::Autolock autoLock(mOutputLock);
    for (i = 0; i < needFillNum; i++) {
        //fetch the render buffer from the top of output buffer queue
        outputBuffer = mOutputBuffers.itemAt(i);
        if (!outputBuffer) {
            ALOGE("%s: failed to fetch output buffer for sync.", __func__);
            return false;
        }
        fillBufList->push_back(outputBuffer);
    }

    *fillBufNum  = i;
    return true;
}


status_t ISVThread::updateFirmwareOutputBufStatus(uint32_t fillBufNum) {
    int64_t timeUs = 0;
    int64_t baseTimeUs = 0;
    IISVBuffer *outputBuffer = NULL;
    IISVBuffer *inputBuffer = NULL;
    status_t err = UNKNOWN_ERROR;
    bool cropChanged = false;

    if (mInputBuffers.empty()) {
        ALOGE("%s: input buffer queue is empty. no buffer need to be sync", __func__);
        return UNKNOWN_ERROR;
    }

    if (mOutputBuffers.size() < fillBufNum) {
        ALOGE("%s: no enough output buffer which need to be sync", __func__);
        return UNKNOWN_ERROR;
    }
    // remove one buffer from intput buffer queue
    {
        Mutex::Autolock autoLock(mInputLock);
        inputBuffer = mInputBuffers.itemAt(0);

        if (inputBuffer != NULL) {
            inputBuffer->clearIfNeed();
            uint32_t flags = inputBuffer->getFlags();
            if (flags & IISVBuffer::ISV_BUFFERFLAG_CROP_CHANGED) {
                err = mpOwner->reportOutputCrop();
                if (err != OK) {
                    ALOGE("%s: failed to reportOutputCrop", __func__);
                    return err;
                }
                cropChanged = true;
                inputBuffer->unsetFlag(IISVBuffer::ISV_BUFFERFLAG_CROP_CHANGED);
            }
        }

        err = mpOwner->releaseBuffer(kPortIndexInput, inputBuffer, false);
        if (err != OK) {
            ALOGE("%s: failed to fillInputBuffer", __func__);
            return err;
        }

        mInputBuffers.removeAt(0);
        ALOGD_IF(ISV_THREAD_DEBUG, "%s: fetch buffer %u from input buffer queue for fill to decoder, and then queue size is %d", __func__,
                inputBuffer, mInputBuffers.size());
        mInputProcIdx--;
    }

    //set the time stamp for interpreted frames
    {
        Mutex::Autolock autoLock(mOutputLock);
        baseTimeUs = mOutputBuffers[0]->getTimeStamp();

        for(uint32_t i = 0; i < fillBufNum; i++) {
            outputBuffer = mOutputBuffers.itemAt(i);
            if (fillBufNum > 1) {
                if (mISVWorker->getFrameRate() == 24.0) {
                    if (fillBufNum == 2) {
                        timeUs = baseTimeUs + 1000000ll * (i + 1) / 60 - 1000000ll * 1 / 24;
                    } else if (fillBufNum == 3) {
                        timeUs = baseTimeUs + 1000000ll * (i + 3) / 60 - 1000000ll * 2 / 24;
                    }
                }
                else
                    timeUs = baseTimeUs - 1000000ll * (fillBufNum - i - 1) / (mISVWorker->getFrameRate() * 2);
                outputBuffer->setTimeStamp(timeUs);
            }

            //return filled buffers for rendering
            //skip rendering for crop change
            err = mpOwner->releaseBuffer(kPortIndexOutput, outputBuffer, cropChanged);

            if (err != OK) {
                ALOGE("%s: failed to releaseOutputBuffer", __func__);
                return err;
            }

            ALOGD_IF(ISV_THREAD_DEBUG, "%s: fetch buffer %u(timestamp %.2f ms) from output buffer queue for render, and then queue size is %d", __func__,
                    outputBuffer, outputBuffer->getTimeStamp()/1E3, mOutputBuffers.size());
        }
        // remove filled buffers from output buffer queue
        mOutputBuffers.removeItemsAt(0, fillBufNum);
        mOutputProcIdx -= fillBufNum;
    }
    return OK;
}


bool ISVThread::getBufForFirmwareInput(Vector<IISVBuffer*> *procBufList,
                                   IISVBuffer **inputBuf,
                                   uint32_t *procBufNum,
                                   uint32_t *flags)
{
    IISVBuffer *outputBuffer;
    IISVBuffer *inputBuffer;

    if (mbFlush) {
        *inputBuf = NULL;
        *procBufNum = 0;
        *flags = 0;
        return true;
    }

    int32_t procBufCount = mISVWorker->getProcBufCount();
    if ((procBufCount == 0) || (procBufCount > 4)) {
       return false;
    }

    //fetch a input buffer for processing
    {
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: acqiring mInputLock", __func__);
        Mutex::Autolock autoLock(mInputLock);
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: acqired mInputLock", __func__);
        inputBuffer = mInputBuffers.itemAt(mInputProcIdx);
        if (!inputBuffer) {
            ALOGE("%s: failed to get input buffer for processing.", __func__);
            return false;
        }
        *inputBuf = inputBuffer;

        unsigned long inHandle = inputBuffer->getOMXHandle();
        OMX_BUFFERHEADERTYPE* pBuffer = reinterpret_cast<OMX_BUFFERHEADERTYPE*>(inHandle);
        if (pBuffer == NULL) {
            ALOGE("%s: failed to get input buffer's omx buffer header.", __func__);
            return false;
        }
        *flags = pBuffer->nFlags;
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: releasing mInputLock", __func__);
    }

    //fetch output buffers for processing
    {
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: acqiring mOutputLock", __func__);
        Mutex::Autolock autoLock(mOutputLock);
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: acqired mOutputLock", __func__);
        for (int32_t i = 0; i < procBufCount; i++) {
            outputBuffer = mOutputBuffers.itemAt(mOutputProcIdx + i);
            if (!outputBuffer) {
                ALOGE("%s: failed to get output buffer for processing.", __func__);
                return false;
            }
            procBufList->push_back(outputBuffer);
        }
        *procBufNum = procBufCount;
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: releasing mOutputLock", __func__);
    }

    return true;
}


status_t ISVThread::updateFirmwareInputBufStatus(uint32_t procBufNum)
{
    IISVBuffer *isvOutputBuffer;
    IISVBuffer *isvInputBuffer;

    isvInputBuffer = mInputBuffers.itemAt(mInputProcIdx);
    unsigned long inHandle = isvInputBuffer->getOMXHandle();
    OMX_BUFFERHEADERTYPE* inputBuffer = reinterpret_cast<OMX_BUFFERHEADERTYPE*>(inHandle);
    mInputProcIdx++;

    Mutex::Autolock autoLock(mOutputLock);
    for(uint32_t i = 0; i < procBufNum; i++) {
        isvOutputBuffer = mOutputBuffers.editItemAt(mOutputProcIdx + i);
        isvOutputBuffer->setTimeStamp(inputBuffer->nTimeStamp);

        unsigned long outHandle = isvOutputBuffer->getOMXHandle();
        OMX_BUFFERHEADERTYPE* outputBuffer = reinterpret_cast<OMX_BUFFERHEADERTYPE*>(outHandle);
        // set output buffer timestamp as the same as input
        outputBuffer->nTimeStamp = inputBuffer->nTimeStamp;
        outputBuffer->nFilledLen = inputBuffer->nFilledLen;
        outputBuffer->nOffset = inputBuffer->nOffset;
        outputBuffer->nFlags = inputBuffer->nFlags;
        //outputBuffer->nTickCount = inputBuffer->nTickCount;
        //outputBuffer->pMarkData = inputBuffer->pMarkData;
    }
    mOutputProcIdx += procBufNum;
    return OK;
}


bool ISVThread::isReadytoRun()
{
    ALOGD_IF(ISV_THREAD_DEBUG, "%s: mISVWorker->getProcBufCount() return %d", __func__,
            mISVWorker->getProcBufCount());
    if (mInputProcIdx < mInputBuffers.size() 
            && (mOutputBuffers.size() - mOutputProcIdx) >= mISVWorker->getProcBufCount())
       return true;
    else
       return false;
}

bool ISVThread::isCurrentThread() const {
    return mThreadId == androidGetThreadId();
}

bool ISVThread::threadLoop() {
    uint32_t procBufNum = 0, fillBufNum = 0;
    IISVBuffer* inputBuf;
    Vector<IISVBuffer*> procBufList;
    Vector<IISVBuffer*> fillBufList;
    uint32_t flags = 0;
    bool bGetBufSuccess = true;

    Mutex::Autolock autoLock(mLock);

    if (!isReadytoRun() && !mbFlush) {
        mRunCond.wait(mLock);
    }

    if (isReadytoRun() || mbFlush) {
        procBufList.clear();
        bool bGetInBuf = getBufForFirmwareInput(&procBufList, &inputBuf, &procBufNum, &flags);
        if (bGetInBuf) {
            status_t ret = mISVWorker->process(inputBuf, procBufList, procBufNum, mbFlush, false, flags);
            if (ret == STATUS_OK) {
                // for seek and EOS
                if (mbFlush) {
                    mISVWorker->reset();
                    flush();

                    mNumTaskInProcesing = 0;
                    mInputProcIdx = 0;
                    mOutputProcIdx = 0;

                    mbFlush = false;

                    Mutex::Autolock endLock(mEndLock);
                    mEndCond.signal();
                    return true;
                }
                mNumTaskInProcesing++;
                updateFirmwareInputBufStatus(procBufNum);
            } else {
                mbBypass = true;
                flush();
                ALOGE("VSP process error %d .... ISV changes to bypass mode", __LINE__);
            }
        }
    }

    ALOGV("mNumTaskInProcesing %d", mNumTaskInProcesing);
    while ((mNumTaskInProcesing > 0) && mNumTaskInProcesing >= mISVWorker->mNumForwardReferences && bGetBufSuccess ) {
        fillBufList.clear();
        bGetBufSuccess = getBufForFirmwareOutput(&fillBufList, &fillBufNum);
        ALOGD_IF(ISV_THREAD_DEBUG, "%s: bGetOutput %d, buf num %d", __func__,
                bGetBufSuccess, fillBufNum);
        if (bGetBufSuccess) {
            status_t ret = mISVWorker->fill(fillBufList, fillBufNum);
            if (ret == STATUS_OK) {
                mNumTaskInProcesing--;
                ALOGV("mNumTaskInProcesing: %d ...", mNumTaskInProcesing);
                updateFirmwareOutputBufStatus(fillBufNum);
            } else {
                mError = true;
                ALOGE("ISV read firmware data error! Thread EXIT...");
                return false;
            }
        }
    }

    return true;
}

void ISVThread::addInput(IISVBuffer* input)
{
    if (mbFlush) {
        mpOwner->releaseBuffer(kPortIndexInput, input, true);
        return;
    }

    if (mbBypass) {
        // return this buffer to framework
        mpOwner->releaseBuffer(kPortIndexOutput, input, false);
        return;
    }

    uint32_t flags = input->getFlags();
    if (flags & IISVBuffer::ISV_BUFFERFLAG_EOS) {
        mpOwner->releaseBuffer(kPortIndexInput, input, true);
        notifyFlush();
        input->unsetFlag(IISVBuffer::ISV_BUFFERFLAG_EOS);
        return;
    }

    {
        //put the decoded buffer into fill buffer queue
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: acqiring mInputLock", __func__);
        Mutex::Autolock autoLock(mInputLock);
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: acqired mInputLock", __func__);

        mInputBuffers.push_back(input);
        ALOGD_IF(ISV_THREAD_DEBUG, "%s: hold pBuffer %u in input buffer queue. Intput queue size is %d, mInputProIdx %d.\
                Output queue size is %d, mOutputProcIdx %d", __func__,
                input, mInputBuffers.size(), mInputProcIdx,
                mOutputBuffers.size(), mOutputProcIdx);
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: releasing mInputLock", __func__);
    }

    {
        Mutex::Autolock autoLock(mLock);
        mRunCond.signal();
    }
    return;
}

void ISVThread::addOutput(IISVBuffer* output)
{
    if (mbFlush) {
        mpOwner->releaseBuffer(kPortIndexOutput, output, true);
        return;
    }

    if (mbBypass) {
        // return this buffer to decoder
        mpOwner->releaseBuffer(kPortIndexInput, output, false);
        return;
    }

    {
        //push the buffer into the output queue if it is not full
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: acqiring mOutputLock", __func__);
        Mutex::Autolock autoLock(mOutputLock);
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: acqired mOutputLock", __func__);

        mOutputBuffers.push_back(output);
        ALOGD_IF(ISV_THREAD_DEBUG, "%s: hold pBuffer %u in output buffer queue. Input queue size is %d, mInputProIdx %d.\
                Output queue size is %d, mOutputProcIdx %d", __func__,
                output, mInputBuffers.size(), mInputProcIdx,
                mOutputBuffers.size(), mOutputProcIdx);
        ALOGD_IF(ISV_THREAD_LOCK_DEBUG, "%s: releasing mOutputLock", __func__);
    }

    {
        Mutex::Autolock autoLock(mLock);
        mRunCond.signal();
    }
    return;
}

void ISVThread::notifyFlush()
{
    if (mInputBuffers.empty() && mOutputBuffers.empty()) {
        ALOGD_IF(ISV_THREAD_DEBUG, "%s: input and ouput buffer queue is empty, nothing need to do", __func__);
        return;
    }

    Mutex::Autolock autoLock(mLock);
    mbFlush = true;
    mRunCond.signal();
    ALOGD_IF(ISV_THREAD_DEBUG, "wake up proc thread");
    return;
}

void ISVThread::waitFlushFinished()
{
    Mutex::Autolock endLock(mEndLock);
    ALOGD_IF(ISV_THREAD_DEBUG, "waiting mEnd lock(seek finish) ");
    while(mbFlush) {
        mEndCond.wait(mEndLock);
    }
    return;
}

void ISVThread::flush()
{
    IISVBuffer* pBuffer = NULL;
    {
        Mutex::Autolock autoLock(mInputLock);
        while (!mInputBuffers.empty()) {
            pBuffer = mInputBuffers.itemAt(0);
            mpOwner->releaseBuffer(kPortIndexInput, pBuffer, true);
            ALOGD_IF(ISV_THREAD_DEBUG, "%s: Flush the pBuffer %u in input buffer queue.", __func__, pBuffer);
            mInputBuffers.removeAt(0);
        }
    }
    {
        Mutex::Autolock autoLock(mOutputLock);
        while (!mOutputBuffers.empty()) {
            pBuffer = mOutputBuffers.itemAt(0);
            mpOwner->releaseBuffer(kPortIndexOutput, pBuffer, true);
            ALOGD_IF(ISV_THREAD_DEBUG, "%s: Flush the pBuffer %u in output buffer queue.", __func__, pBuffer);
            mOutputBuffers.removeAt(0);
        }
    }
    //flush finished.
    return;
}

} // namespace intel
} // namespace isv
