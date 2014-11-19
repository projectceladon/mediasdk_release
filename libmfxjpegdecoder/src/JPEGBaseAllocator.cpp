/********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

*********************************************************************************/

#include "JPEGBaseAllocator.h"

/*------------------------------------------------------------------------------*/

static const mfxU32 MEMTYPE_FROM_MASK =
    MFX_MEMTYPE_FROM_ENCODE |
    MFX_MEMTYPE_FROM_DECODE |
    MFX_MEMTYPE_FROM_VPPIN |
    MFX_MEMTYPE_FROM_VPPOUT;

/*------------------------------------------------------------------------------*/

inline JpegFrameAllocator* _wrapper_get_allocator(mfxHDL pthis)
{
    return (JpegFrameAllocator*)pthis;
}

static mfxStatus _wrapper_alloc(
  mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    JpegFrameAllocator* a = _wrapper_get_allocator(pthis);
    return (a)? a->AllocFrames(request, response): MFX_ERR_INVALID_HANDLE;
}

static mfxStatus _wrapper_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
    JpegFrameAllocator* a = _wrapper_get_allocator(pthis);

    if (a) return a->LockFrame(mid, ptr);

    return MFX_ERR_INVALID_HANDLE;
}

static mfxStatus _wrapper_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
    JpegFrameAllocator* a = _wrapper_get_allocator(pthis);
    return (a)? a->UnlockFrame(mid, ptr): MFX_ERR_INVALID_HANDLE;
}

static mfxStatus _wrapper_free(mfxHDL pthis, mfxFrameAllocResponse *response)
{
    JpegFrameAllocator* a = _wrapper_get_allocator(pthis);
    return (a)? a->FreeFrames(response): MFX_ERR_INVALID_HANDLE;
}

static mfxStatus _wrapper_get_hdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle)
{
    JpegFrameAllocator* a = _wrapper_get_allocator(pthis);
    return (a)? a->GetFrameHDL(mid, handle): MFX_ERR_INVALID_HANDLE;
}

/*------------------------------------------------------------------------------*/

JpegFrameAllocator::JpegFrameAllocator()
{
    mfxFrameAllocator& allocator = *((mfxFrameAllocator*)this);

    memset(&allocator, 0, sizeof(mfxFrameAllocator));
    allocator.pthis  = this;
    allocator.Alloc  = _wrapper_alloc;
    allocator.Free   = _wrapper_free;
    allocator.Lock   = _wrapper_lock;
    allocator.Unlock = _wrapper_unlock;
    allocator.GetHDL = _wrapper_get_hdl;

    memset(&m_DecoderResponse, 0, sizeof(JpegFrameAllocResponse));
}

JpegFrameAllocator::~JpegFrameAllocator()
{
}

mfxStatus JpegFrameAllocator::CheckRequestType(mfxFrameAllocRequest *request)
{
    if (!request)
    {
        return MFX_ERR_NULL_PTR;
    }
    // check that Media SDK component is specified in request
    if ((request->Type & MEMTYPE_FROM_MASK) != 0)
        return MFX_ERR_NONE;
    else
        return MFX_ERR_UNSUPPORTED;
}

mfxStatus JpegFrameAllocator::AllocFrames(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    mfxStatus mfx_res = MFX_ERR_NONE;
    if (!request || !response)
        return MFX_ERR_NULL_PTR;

    if ((MFX_ERR_NONE == mfx_res) && (0 == request->NumFrameSuggested))
        return MFX_ERR_MEMORY_ALLOC;

    if (MFX_ERR_NONE == mfx_res)
        mfx_res = CheckRequestType(request);

    if (MFX_ERR_NONE == mfx_res)
    {
        bool bDecoderResponse = (request->Type & MFX_MEMTYPE_EXTERNAL_FRAME) &&
                                (request->Type & MFX_MEMTYPE_FROM_DECODE);
        if (bDecoderResponse && (0 != m_DecoderResponse.response.NumFrameActual))
        {
            if (request->NumFrameSuggested > m_DecoderResponse.response.NumFrameActual)
            {
                mfx_res = MFX_ERR_MEMORY_ALLOC;
            }
            else
            {
                memcpy(response, &(m_DecoderResponse.response), sizeof(mfxFrameAllocResponse));
            }
        }
        else
        {
            mfx_res = AllocImpl(request, response);
            if (bDecoderResponse && MFX_ERR_NONE == mfx_res)
            {
                memcpy(&(m_DecoderResponse.response), response, sizeof(mfxFrameAllocResponse));
            }
        }
        if ((MFX_ERR_NONE == mfx_res) && bDecoderResponse) ++(m_DecoderResponse.refcount);
    }
    return mfx_res;
}

mfxStatus JpegFrameAllocator::FreeFrames(mfxFrameAllocResponse *response)
{
    if (!response) return MFX_ERR_NULL_PTR;

    if (!memcmp(response, &(m_DecoderResponse.response), sizeof(mfxFrameAllocResponse)))
    {
        if (!m_DecoderResponse.refcount) return MFX_ERR_UNKNOWN; // should not occur, just in case

        --(m_DecoderResponse.refcount);
        if (m_DecoderResponse.refcount) return MFX_ERR_NONE;

        memset(&(m_DecoderResponse), 0, sizeof(JpegFrameAllocResponse));
    }
    return ReleaseResponse(response);
}
