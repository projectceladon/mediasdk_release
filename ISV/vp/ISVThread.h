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

#ifndef __ISV_THREAD_H
#define __ISV_THREAD_H

#include <media/stagefright/MetaData.h>

#include <utils/Mutex.h>
#include <utils/threads.h>
#include <utils/Errors.h>
#include "ISVVp.h"
#include "ISVBufferManager.h"
#include "ISVProcessor.h"
#include "ISVWorker.h"
#define ISV_THREAD_LOCK_DEBUG 0
#define ISV_THREAD_DEBUG 0

namespace intel {
namespace isv {

class ISVProcessorCallBack;

class ISVThread : public Thread
{
public:
    ISVThread(bool canCallJava,
        sp<ISVBufferManager> bufferManager,
        sp<ISVWorker> worker,
        ISVProcessorCallBack* owner);
    virtual ~ISVThread();

    virtual status_t readyToRun();

    void start();
    void stop();

    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
                                                // 1) loop: if threadLoop() returns true, it will be called again if
                                                //          requestExit() wasn't called.
                                                // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool threadLoop();
    bool isCurrentThread() const;
    bool isReadytoRun();

    //add output buffer into mOutputBuffers
    void addOutput(IISVBuffer* output);
    //add intput buffer into mInputBuffers
    void addInput(IISVBuffer* input);
    //notify flush and wait flush finish
    void notifyFlush();
    void waitFlushFinished();

private:
    bool getBufForFirmwareOutput(Vector<IISVBuffer*> *fillBufList,
            uint32_t *fillBufNum);
    status_t updateFirmwareOutputBufStatus(uint32_t fillBufNum);
    bool getBufForFirmwareInput(Vector<IISVBuffer*> *procBufList,
            IISVBuffer **inputBuf,
            uint32_t *procBufNum,
            uint32_t *flags);
    status_t updateFirmwareInputBufStatus(uint32_t procBufNum);
    //flush input&ouput buffer queue
    void flush();

    //config vpp filters
    status_t configFilters(OMX_BUFFERHEADERTYPE* buffer);
private:
    ISVProcessorCallBack* mpOwner;
    sp<ISVWorker> mISVWorker;
    sp<ISVBufferManager> mBufferManager;

    android_thread_id_t mThreadId;
    bool mThreadRunning;

    Vector<IISVBuffer*> mOutputBuffers;
    Mutex mOutputLock; // to protect access to mOutputBuffers
    uint32_t mOutputProcIdx;

    Vector<IISVBuffer*> mInputBuffers;
    Mutex mInputLock; // to protect access to mFillBuffers
    uint32_t mInputProcIdx;

    // conditon for thread running
    Mutex mLock;
    Condition mRunCond;

    // condition for seek finish
    Mutex mEndLock;
    Condition mEndCond;

    uint32_t mNumTaskInProcesing;
    bool mError;
    bool mbFlush;
    bool mbBypass;
    bool mFlagEnd;
};

} // namespace intel
} // namespace isv

#endif /* __ISV_THREAD_H*/
