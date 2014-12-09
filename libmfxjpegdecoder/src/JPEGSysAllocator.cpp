/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#include "JPEGSysAllocator.h"

#define MSDK_ALIGN32(X) (((mfxU32)((X)+31)) & (~ (mfxU32)31))
#define ID_BUFFER MFX_MAKEFOURCC('B','U','F','F')
#define ID_FRAME  MFX_MAKEFOURCC('F','R','M','E')


/* Abstract buffer allocator */

BaseBufferAllocator::BaseBufferAllocator()
{
    pthis = this;
    Alloc = Alloc_;
    Lock  = Lock_;
    Free  = Free_;
    Unlock = Unlock_;
}

BaseBufferAllocator::~BaseBufferAllocator()
{
}

mfxStatus BaseBufferAllocator::Alloc_(mfxHDL pthis, mfxU32 nbytes, mfxU16 type, mfxMemId *mid)
{
    if (0 == pthis)
        return MFX_ERR_MEMORY_ALLOC;

    BaseBufferAllocator& self = *(BaseBufferAllocator *)pthis;
    return self.AllocBuffer(nbytes, type, mid);
}

mfxStatus BaseBufferAllocator::Lock_(mfxHDL pthis, mfxMemId mid, mfxU8 **ptr)
{
    if (0 == pthis)
        return MFX_ERR_MEMORY_ALLOC;

    BaseBufferAllocator& self = *(BaseBufferAllocator *)pthis;
    return self.LockBuffer(mid, ptr);
}

mfxStatus BaseBufferAllocator::Unlock_(mfxHDL pthis, mfxMemId mid)
{
    if (0 == pthis)
        return MFX_ERR_MEMORY_ALLOC;

    BaseBufferAllocator& self = *(BaseBufferAllocator *)pthis;
    return self.UnlockBuffer(mid);
}

mfxStatus BaseBufferAllocator::Free_(mfxHDL pthis, mfxMemId mid)
{
    if (0 == pthis)
        return MFX_ERR_MEMORY_ALLOC;

    BaseBufferAllocator& self = *(BaseBufferAllocator *)pthis;
    return self.FreeBuffer(mid);
}

JpegSysBufferAllocator::JpegSysBufferAllocator()
{
}

JpegSysBufferAllocator::~JpegSysBufferAllocator(){}

mfxStatus JpegSysBufferAllocator::AllocBuffer(mfxU32 nbytes, mfxU16 type, mfxMemId *mid)
{
    if (!mid)
        return MFX_ERR_NULL_PTR;

    if (0 == (type & MFX_MEMTYPE_SYSTEM_MEMORY))
        return MFX_ERR_UNSUPPORTED;

    mfxU32 header_size = MSDK_ALIGN32(sizeof(sBuffer));
    mfxU8 *buffer_ptr  = (mfxU8 *)calloc(header_size + nbytes + 32, 1);

    if (!buffer_ptr)
        return MFX_ERR_MEMORY_ALLOC;

    sBuffer *bs = (sBuffer *)buffer_ptr;
    bs->id = ID_BUFFER;
    bs->type = type;
    bs->nbytes = nbytes;
    *mid = (mfxHDL) bs;
    return MFX_ERR_NONE;
}

mfxStatus JpegSysBufferAllocator::LockBuffer(mfxMemId mid, mfxU8 **ptr)
{
    if (!ptr)
        return MFX_ERR_NULL_PTR;

    sBuffer *bs = (sBuffer *)mid;
    if (!bs)
        return MFX_ERR_INVALID_HANDLE;
    if (ID_BUFFER != bs->id)
        return MFX_ERR_INVALID_HANDLE;

    *ptr = (mfxU8*)((size_t)((mfxU8 *)bs+MSDK_ALIGN32(sizeof(sBuffer))+31)&(~((size_t)31)));
    return MFX_ERR_NONE;
}

mfxStatus JpegSysBufferAllocator::UnlockBuffer(mfxMemId mid)
{
    sBuffer *bs = (sBuffer *)mid;
    if (!bs || ID_BUFFER != bs->id)
        return MFX_ERR_INVALID_HANDLE;

    return MFX_ERR_NONE;
}

mfxStatus JpegSysBufferAllocator::FreeBuffer(mfxMemId mid)
{
    sBuffer *bs = (sBuffer *)mid;
    if (!bs || ID_BUFFER != bs->id)
        return MFX_ERR_INVALID_HANDLE;

    free(bs);
    return MFX_ERR_NONE;
}

JpegSysFrameAllocator::JpegSysFrameAllocator(mfxAllocatorParams *pParams)
: m_pBufferAllocator(0),
  m_bOwnBufferAllocator(false)
{
    // check if any params passed from application
    if (pParams)
    {
        JpegSysAllocatorParams *pSysMemParams = 0;
        pSysMemParams = dynamic_cast<JpegSysAllocatorParams *>(pParams);
        if (!pSysMemParams)
        {
            return;
        }
        m_pBufferAllocator    = pSysMemParams->pBufferAllocator;
        m_bOwnBufferAllocator = false;
    }

    // if buffer allocator wasn't passed from application create own
    if (!m_pBufferAllocator)
    {
        m_pBufferAllocator = new JpegSysBufferAllocator;
        if (!m_pBufferAllocator)
        {
            return;
        }
        m_bOwnBufferAllocator = true;
    }
    return;
}

JpegSysFrameAllocator::~JpegSysFrameAllocator()
{
    Close();
}

mfxStatus JpegSysFrameAllocator::Init(mfxAllocatorParams *pParams)
{
   return MFX_ERR_NONE;
}

mfxStatus JpegSysFrameAllocator::Close()
{
    mfxStatus sts;// = JpegFrameAllocator::Close();

    if (m_bOwnBufferAllocator)
    {
        delete m_pBufferAllocator;
        m_pBufferAllocator = 0;
    }
    return sts;
}

mfxStatus JpegSysFrameAllocator::LockFrame(mfxMemId mid, mfxFrameData *ptr)
{
    JpegAutoLock lock(m_mutex);
    if (!m_pBufferAllocator)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }
    if (!ptr)
    {
        return MFX_ERR_NULL_PTR;
    }
    sFrame *fs = 0;
    mfxStatus sts = m_pBufferAllocator->Lock(m_pBufferAllocator->pthis, mid,(mfxU8 **)&fs);

    if (MFX_ERR_NONE != sts)
    {
        return sts;
    }
    if (ID_FRAME != fs->id)
    {
        m_pBufferAllocator->Unlock(m_pBufferAllocator->pthis, mid);
        return MFX_ERR_INVALID_HANDLE;
    }

    mfxU16 Width2 = (mfxU16)MSDK_ALIGN32(fs->info.Width);
    mfxU16 Height2 = (mfxU16)MSDK_ALIGN32(fs->info.Height);
    ptr->B = ptr->Y = (mfxU8 *)fs + MSDK_ALIGN32(sizeof(sFrame));

    switch (fs->info.FourCC)
    {
    case MFX_FOURCC_NV12:
        ptr->U = ptr->Y + Width2 * Height2;
        ptr->V = ptr->U + 1;
        ptr->Pitch = Width2;
        break;
    case MFX_FOURCC_YV12:
        ptr->V = ptr->Y + Width2 * Height2;
        ptr->U = ptr->V + (Width2 >> 1) * (Height2 >> 1);
        ptr->Pitch = Width2;
        break;
    case MFX_FOURCC_YUY2:
        ptr->U = ptr->Y + 1;
        ptr->V = ptr->Y + 3;
        ptr->Pitch = 2 * Width2;
        break;
    case MFX_FOURCC_RGB3:
        ptr->G = ptr->B + 1;
        ptr->R = ptr->B + 2;
        ptr->Pitch = 3 * Width2;
        break;
    case MFX_FOURCC_RGB4:
        ptr->G = ptr->B + 1;
        ptr->R = ptr->B + 2;
        ptr->A = ptr->B + 3;
        ptr->Pitch = 4 * Width2;
        break;
   case MFX_FOURCC_R16:
        ptr->Y16 = (mfxU16 *)ptr->B;
        ptr->Pitch = 2 * Width2;
        break;
    default:
        return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_NONE;
}

mfxStatus JpegSysFrameAllocator::UnlockFrame(mfxMemId mid, mfxFrameData *ptr)
{
    JpegAutoLock lock(m_mutex);
    if (!m_pBufferAllocator)
        return MFX_ERR_NOT_INITIALIZED;

    mfxStatus sts = m_pBufferAllocator->Unlock(m_pBufferAllocator->pthis, mid);

    if (MFX_ERR_NONE != sts)
        return sts;

    if (NULL != ptr)
    {
        ptr->Pitch = 0;
        ptr->Y     = 0;
        ptr->U     = 0;
        ptr->V     = 0;
    }

    return MFX_ERR_NONE;
}

mfxStatus JpegSysFrameAllocator::GetFrameHDL(mfxMemId mid, mfxHDL *handle)
{
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus JpegSysFrameAllocator::CheckRequestType(mfxFrameAllocRequest *request)
{
    mfxStatus sts = JpegFrameAllocator::CheckRequestType(request);
    if (MFX_ERR_NONE != sts)
        return sts;

    if ((request->Type & MFX_MEMTYPE_SYSTEM_MEMORY) != 0)
        return MFX_ERR_NONE;
    else
        return MFX_ERR_UNSUPPORTED;
}

mfxStatus JpegSysFrameAllocator::AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    if (!m_pBufferAllocator)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }

    mfxU32 numAllocated = 0;

    mfxU32 Width2 = MSDK_ALIGN32(request->Info.Width);
    mfxU32 Height2 = MSDK_ALIGN32(request->Info.Height);
    mfxU32 nbytes;

    switch (request->Info.FourCC)
    {
    case MFX_FOURCC_YV12:
    case MFX_FOURCC_NV12:
        nbytes = Width2*Height2 + (Width2>>1)*(Height2>>1) + (Width2>>1)*(Height2>>1);
        break;
    case MFX_FOURCC_RGB3:
        nbytes = Width2*Height2 + Width2*Height2 + Width2*Height2;
        break;
    case MFX_FOURCC_RGB4:
        nbytes = Width2*Height2 + Width2*Height2 + Width2*Height2 + Width2*Height2;
        break;
    case MFX_FOURCC_YUY2:
        nbytes = Width2*Height2 + (Width2>>1)*(Height2) + (Width2>>1)*(Height2);
        break;
    case MFX_FOURCC_R16:
        nbytes = 2*Width2*Height2;
        break;

    default:
        return MFX_ERR_UNSUPPORTED;
    }

    safe_array<mfxMemId> mids(new mfxMemId[request->NumFrameSuggested]);
    if (!mids.get())
    {
        return MFX_ERR_MEMORY_ALLOC;
    }
    // allocate frames
    for (numAllocated = 0; numAllocated < request->NumFrameSuggested; numAllocated ++)
    {
        mfxStatus sts = m_pBufferAllocator->Alloc(m_pBufferAllocator->pthis,
            nbytes + MSDK_ALIGN32(sizeof(sFrame)), request->Type, &(mids.get()[numAllocated]));

        if (MFX_ERR_NONE != sts)
        {
            break;
        }
        sFrame *fs;
        sts = m_pBufferAllocator->Lock(m_pBufferAllocator->pthis, mids.get()[numAllocated], (mfxU8 **)&fs);

        if (MFX_ERR_NONE != sts)
        {
            break;
        }
        fs->id = ID_FRAME;
        fs->info = request->Info;
        m_pBufferAllocator->Unlock(m_pBufferAllocator->pthis, mids.get()[numAllocated]);
    }

    // check the number of allocated frames
    if (numAllocated < request->NumFrameMin)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }

    response->NumFrameActual = (mfxU16) numAllocated;
    response->mids = mids.release();

    return MFX_ERR_NONE;
}

mfxStatus JpegSysFrameAllocator::ReleaseResponse(mfxFrameAllocResponse *response)
{
    if (!response)
        return MFX_ERR_NULL_PTR;

    if (!m_pBufferAllocator)
        return MFX_ERR_NOT_INITIALIZED;

    mfxStatus sts = MFX_ERR_NONE;

    if (response->mids)
    {
        for (mfxU32 i = 0; i < response->NumFrameActual; i++)
        {
            if (response->mids[i])
            {
                sts = m_pBufferAllocator->Free(m_pBufferAllocator->pthis, response->mids[i]);
                if (MFX_ERR_NONE != sts)
                    return sts;
            }
        }
    }

    delete [] response->mids;
    response->mids = 0;

    return sts;
}
