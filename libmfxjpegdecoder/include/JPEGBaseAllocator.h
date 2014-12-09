/********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

*********************************************************************************/

#ifndef __JPEG_BASE_ALLOCATOR_H__
#define __JPEG_BASE_ALLOCATOR_H__

#include "JPEGCommon.h"

#include <list>
#include <functional>

struct JpegFrameAllocResponse
{
    mfxU32 refcount;
    mfxFrameAllocResponse response;
};

class JpegFrameAllocator: public mfxFrameAllocator
{
public:
    JpegFrameAllocator();
    virtual ~JpegFrameAllocator();

    virtual mfxStatus AllocFrames(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
    virtual mfxStatus FreeFrames(mfxFrameAllocResponse *response);

    virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData *ptr) = 0;
    virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData *ptr) = 0;

    virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL *handle) = 0;

protected: //functions
    // checks if request is supported
    virtual mfxStatus CheckRequestType(mfxFrameAllocRequest *request);
    // frees memory attached to response
    virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse *response) = 0;
    // allocates memory
    virtual mfxStatus AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response) = 0;

protected: // variables
    JpegFrameAllocResponse m_DecoderResponse;

private:
    JPEG_CLASS_NO_COPY(JpegFrameAllocator)
};

#endif // __JPEG_BASE_ALLOCATOR_H__
