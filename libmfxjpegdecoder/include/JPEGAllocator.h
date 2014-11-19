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

// VAAPI Allocator internal Mem ID
struct vaapiMemId
{
    VASurfaceID* m_pSurface;
    VAImage      m_image;
    mfxU32       m_fourcc;
    const mfxU8* m_key;         // to store HW handle from which this surface has been created (for search already created surface by key)
    mfxU8        m_unused;      // to mark created surface which already unused
    bool         m_bIsTiledFormat; // if true - we don't need to load data manually from input handle
};

class JpegVaapiFrameAllocator: public JpegFrameAllocator
{
public:
    JpegVaapiFrameAllocator(VADisplay dpy);
    virtual ~JpegVaapiFrameAllocator();

    mfxStatus ConvertIMBtoMID(mfxU8* data, mfxU32 length, mfxFrameInfo &mfxInfo, mfxMemId* mid);
    mfxStatus ConvertGHDLtoMID(const mfxU8* ghandle, mfxFrameInfo &mfxInfo, mfxMemId* pmid);
    mfxStatus AddExtMID(VASurfaceID surface, mfxMemId* mid, const mfxU8* key, bool bIsTiledFormat, mfxU32 mfxFourCC);
    mfxStatus FreeExtMID(mfxMemId mid);
    mfxStatus RegisterBuffer(mfxU8* pBuffer, mfx_pvr_buffer_details_t* pInfo);

protected:
    virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData *ptr);
    virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData *ptr);
    virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL *handle);

    virtual mfxStatus CheckRequestType(mfxFrameAllocRequest *request);
    virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse *response);
    virtual mfxStatus AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);

    void upload_yuv_to_surface(unsigned char *newImageBuffer, VASurfaceID surface_id,
                               int picture_width, int picture_height);
    VADisplay m_dpy;

private:
    mfxStatus CreateSurfaceFromDRMHandle(const buffer_handle_t handle, mfx_pvr_buffer_details_t* pInfo, VASurfaceID &surface);

    mfxStatus MapGrallocBufferToSurface(const mfxU8* handle, VASurfaceID &surface, mfxFrameInfo &mfxInfo, bool &bIsTiledFormat);
    mfxStatus CreateSurfaceFromGralloc(const mfxU8* handle, VASurfaceID &surface, mfxFrameInfo &mfxInfo, const intel_ufo_buffer_details_t &info);
    mfxStatus LoadGrallocBuffer(const mfxU8* handle, const VASurfaceID surface);
    mfxStatus CreateSurface(VASurfaceID &surface, mfxU16 width, mfxU16 height);

    mfxStatus TouchSurface(VASurfaceID surface);

    // external buffers
    std::list<vaapiMemId*> m_extMIDs;
    mfxMemId* m_MIDs;
    int numMIDs;
    JpegMutex m_mutex;


    JPEG_CLASS_NO_COPY(JpegVaapiFrameAllocator)
};


#endif
