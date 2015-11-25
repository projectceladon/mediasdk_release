/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#ifndef __JPEGALLOCATOR_H__
#define __JPEGALLOCATOR_H__

#include "JPEGCommon.h"
#include "JPEGUtils.h"
#include "JPEGBaseAllocator.h"

struct vaapiMemId
{
    VASurfaceID* m_pSurface;
    VAImage      m_image;
    mfxU32       m_fourcc;
    const mfxU8* m_key;         // to store HW handle from which this surface has been created (for search already created surface by key)
    mfxU8        m_unused;      // to mark created surface which already unused
    bool         m_bUseBufferDirectly; // if true - we don't need to load data manually from input handle
};

class JpegVaapiFrameAllocator: public JpegFrameAllocator
{
public:
    JpegVaapiFrameAllocator(VADisplay dpy);
    virtual ~JpegVaapiFrameAllocator();

    mfxStatus LoadSurface(const buffer_handle_t handle, bool bIsDecodeTarget, mfxFrameInfo & mfx_info, mfxMemId* pmid);
    mfxStatus RegisterSurface(VASurfaceID surface, mfxMemId* mid, const mfxU8* key, bool bUseBufferDirectly, mfxU32 mfxFourCC);

protected:
    virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData *ptr);
    virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData *ptr);
    virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL *handle);

    virtual mfxStatus CheckRequestType(mfxFrameAllocRequest *request);
    virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse *response);
    virtual mfxStatus AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);

    VADisplay m_dpy;

private:
    mfxStatus ConvertGrallocBuffer2MFXMemId(buffer_handle_t handle, bool bIsDecodeTarget, mfxFrameInfo &mfxInfo, mfxMemId* mid);
    mfxStatus MapGrallocBufferToSurface(const mfxU8* handle, bool bIsDecodeTarget, VASurfaceID &surface, mfxFrameInfo &mfxInfo, bool &bUseBufferDirectly);
    mfxStatus CreateSurfaceFromGralloc(const mfxU8* handle, bool bIsDecodeTarget, VASurfaceID &surface, mfxFrameInfo &mfxInfo, const intel_ufo_buffer_details_t & info);
    mfxStatus CreateSurface(VASurfaceID &surface, mfxU16 width, mfxU16 height);

    mfxStatus LoadGrallocBuffer(const mfxU8* handle, const VASurfaceID surface);

    mfxStatus TouchSurface(VASurfaceID surface);

    // external buffers
    std::list<vaapiMemId*> m_extMIDs;
    mfxMemId* m_MIDs;
    int numMIDs;
    JpegMutex m_mutex;
    gralloc_module_t* m_pGralloc;

    JPEG_CLASS_NO_COPY(JpegVaapiFrameAllocator)
};


#endif
