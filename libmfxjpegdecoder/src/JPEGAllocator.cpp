/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#include "JPEGCommon.h"
#include "JPEGUtils.h"
#include "JPEGAllocator.h"

#include <ufo/graphics.h>

#include <stdio.h>

/*------------------------------------------------------------------------------*/

#undef JPEG_MODULE_NAME
#define JPEG_MODULE_NAME "JPEG_allocator"

#define va_to_mfx_status(sts) ( (VA_STATUS_SUCCESS == sts)? MFX_ERR_NONE: MFX_ERR_UNKNOWN )

unsigned int ConvertMfxFourccToVAFormat(mfxU32 fourcc)
{
    switch (fourcc)
    {
        case MFX_FOURCC_NV12:
            return VA_FOURCC_NV12;
        case MFX_FOURCC_YUY2:
            return VA_FOURCC_YUY2;
        case MFX_FOURCC_YV12:
            return VA_FOURCC_YV12;
        case MFX_FOURCC_RGB4:
            return VA_FOURCC_ARGB;
        case MFX_FOURCC_P8:
            return VA_FOURCC_P208;
        default:
            return 0;
    }
}

mfxU32 ConvertVAFourccToMfxFormat(unsigned int fourcc)
{
    switch (fourcc)
    {
        case VA_FOURCC_NV12:
            return MFX_FOURCC_NV12;
        case VA_FOURCC_YUY2:
            return MFX_FOURCC_YUY2;
        case VA_FOURCC_YV12:
            return MFX_FOURCC_YV12;
        case VA_FOURCC_RGBA:
        case VA_FOURCC_BGRA:
        case VA_FOURCC_RGBX:
            return MFX_FOURCC_RGB4;
        case VA_FOURCC_P208:
            return MFX_FOURCC_P8;
        default:
            return 0;
    }
}

unsigned int ConvertGrallocFourccToVAFormat(int fourcc)
{
    switch (fourcc)
    {
        case JPEG_PIXEL_FORMAT_NV12_TILED_INTEL:
            return VA_FOURCC_NV12;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return VA_FOURCC_RGBA;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            return VA_FOURCC_RGBX;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return VA_FOURCC_BGRA;
        default:
            return 0;
    }
}

JpegVaapiFrameAllocator::JpegVaapiFrameAllocator(VADisplay dpy):
    m_dpy(dpy)
    ,m_MIDs(NULL)
{
    hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (hw_module_t const**)&m_pGralloc);
}

JpegVaapiFrameAllocator::~JpegVaapiFrameAllocator()
{
    JPEG_AUTO_TRACE_FUNC();

    while (!m_extMIDs.empty())
    {
        vaapiMemId* pmid = m_extMIDs.back();
        if (pmid)
        {
            if (VA_INVALID_ID != *pmid->m_pSurface)
                vaDestroySurfaces(m_dpy, pmid->m_pSurface, 1);
            free(pmid);
        }
        m_extMIDs.pop_back();
    }
}

mfxStatus JpegVaapiFrameAllocator::CheckRequestType(mfxFrameAllocRequest *request)
{
    JPEG_AUTO_TRACE_FUNC();
    mfxStatus sts = JpegFrameAllocator::CheckRequestType(request);
    if (MFX_ERR_NONE != sts)
        return sts;

    if ((request->Type & (MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET | MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET)) != 0)
        return MFX_ERR_NONE;
    else
        return MFX_ERR_UNSUPPORTED;
}

mfxStatus JpegVaapiFrameAllocator::RegisterSurface(VASurfaceID surface, mfxMemId* mid, const mfxU8* key, bool bUseBufferDirectly, mfxU32 mfxFourCC)
{
    JPEG_AUTO_TRACE_FUNC();
    JpegAutoLock lock(m_mutex);

    mfxStatus mfx_res = MFX_ERR_NONE;
    mfxMemId out_mid = NULL;

    JPEG_AUTO_TRACE_I32(surface);

    if (NULL == mid) mfx_res = MFX_ERR_NULL_PTR;

    if (MFX_ERR_NONE == mfx_res)
    {
        for (std::list<vaapiMemId*>::iterator it = m_extMIDs.begin(); it != m_extMIDs.end(); it++)
        { // if surface already exist
            vaapiMemId* pmid = (*it);

            if (surface == *pmid->m_pSurface)
            {
                out_mid = pmid;
                break;
            }
        }

        if (NULL == out_mid)
        { // add new extMID
            vaapiMemId* pmid = (vaapiMemId*)malloc(sizeof(vaapiMemId) + sizeof(VASurfaceID));
            mfxU8* ddd = (mfxU8*)pmid;
            if (pmid != NULL)
            {
                memset(pmid, 0, sizeof(vaapiMemId) + sizeof(VASurfaceID));
                pmid->m_pSurface = (VASurfaceID*)(mfxU8*)(ddd + sizeof(vaapiMemId));//(VASurfaceID*)(pmid + 1);
                *pmid->m_pSurface = surface;
                pmid->m_fourcc = mfxFourCC;
                pmid->m_unused = false;
                pmid->m_key = key;
                pmid->m_bUseBufferDirectly = bUseBufferDirectly;
                out_mid = pmid;
                m_extMIDs.push_back(pmid);
            }
        }

        if (out_mid)
            *mid = out_mid;
        else
            mfx_res = MFX_ERR_UNKNOWN;
    }
    JPEG_AUTO_TRACE_I32(mfx_res);
    return mfx_res;
}

mfxStatus JpegVaapiFrameAllocator::LoadSurface(const buffer_handle_t handle,
                                               bool bIsDecodeTarget,
                                               mfxFrameInfo & mfx_info,
                                               mfxMemId* pmid)
{
    JPEG_AUTO_TRACE_FUNC();
    mfxStatus mfx_res = MFX_ERR_NONE;

    mfx_res = ConvertGrallocBuffer2MFXMemId(handle, bIsDecodeTarget, mfx_info, pmid);

    JPEG_AUTO_TRACE_I32(mfx_res);
    return mfx_res;
}

mfxStatus JpegVaapiFrameAllocator::ConvertGrallocBuffer2MFXMemId(buffer_handle_t handle, bool bIsDecodeTarget, mfxFrameInfo &mfxInfo, mfxMemId* pmid)
{
    JPEG_AUTO_TRACE_FUNC();
    mfxStatus mfx_res = MFX_ERR_NONE;

    VASurfaceID surface;

    bool bUseBufferDirectly = true;

    if (handle) mfx_res = MapGrallocBufferToSurface((const mfxU8*)handle, bIsDecodeTarget, surface, mfxInfo, bUseBufferDirectly);
    else surface = VA_INVALID_ID;

    if (MFX_ERR_NONE == mfx_res)
    {
        mfx_res = RegisterSurface(surface, pmid, (const mfxU8*)handle, bUseBufferDirectly, mfxInfo.FourCC);
    }

    JPEG_AUTO_TRACE_I32(mfx_res);
    return mfx_res;
}

mfxStatus JpegVaapiFrameAllocator::MapGrallocBufferToSurface(const mfxU8* handle, bool bIsDecodeTarget, VASurfaceID &surface, mfxFrameInfo &mfxInfo, bool &bUseBufferDirectly)
{
    JPEG_AUTO_TRACE_FUNC();
    mfxStatus mfx_res = MFX_ERR_NONE;

    surface = VA_INVALID_ID;

    JPEG_AUTO_TRACE_P(handle);

    for (std::list<vaapiMemId*>::iterator it = m_extMIDs.begin(); it != m_extMIDs.end(); it++)
    {
        vaapiMemId* pmid = (*it);
        if (handle != NULL && pmid->m_key == handle)
        {
            if (pmid->m_key == handle)
            {
                surface = *pmid->m_pSurface;
                bUseBufferDirectly = pmid->m_bUseBufferDirectly;
                JPEG_AUTO_TRACE_MSG("Surface is found");
                JPEG_AUTO_TRACE_I32(surface);
                break;
            }
        }
    }

    if (MFX_ERR_NONE == mfx_res && VA_INVALID_ID == surface)
    {
        intel_ufo_buffer_details_t info;
        JPEG_ZERO_MEMORY(info);
        *reinterpret_cast<uint32_t*>(&info) = sizeof(info);

        int err = 0;
        if (m_pGralloc) err = m_pGralloc->perform(m_pGralloc, INTEL_UFO_GRALLOC_MODULE_PERFORM_GET_BO_INFO, (buffer_handle_t)handle, &info);
        if (0 != err || !m_pGralloc) mfx_res = MFX_ERR_UNKNOWN;
        JPEG_AUTO_TRACE_U32(info.format);

        if (bIsDecodeTarget)
        {
            bUseBufferDirectly = true;
        }
        if (JPEG_PIXEL_FORMAT_NV12_TILED_INTEL == info.format   || // HAL_PIXEL_FORMAT_NV12_TILED_INTEL = 0x100
            JPEG_INTEL_COLOR_FormatVa_NV12_Tiled == info.format ||
            HAL_PIXEL_FORMAT_RGBA_8888 == info.format)            // HAL_PIXEL_FORMAT_RGBA_8888 = 1
        {
            bUseBufferDirectly = true;
        }
        else
            bUseBufferDirectly = false;

        if (MFX_ERR_NONE == mfx_res)
        {
            if (bUseBufferDirectly)
                mfx_res = CreateSurfaceFromGralloc(handle, bIsDecodeTarget, surface, mfxInfo, info);
            else
                mfx_res = CreateSurface(surface, info.width, info.height);
        }
    }

    if (MFX_ERR_NONE == mfx_res && false == bUseBufferDirectly)
        mfx_res = LoadGrallocBuffer(handle, surface);

    JPEG_AUTO_TRACE_I32(mfx_res);
    return mfx_res;
}

mfxStatus JpegVaapiFrameAllocator::CreateSurfaceFromGralloc(const mfxU8* handle, bool bIsDecodeTarget, VASurfaceID &surface, mfxFrameInfo &mfxInfo, const intel_ufo_buffer_details_t & info)
{
    JPEG_AUTO_TRACE_FUNC();
    mfxStatus mfx_res = MFX_ERR_NONE;

    JPEG_AUTO_TRACE_P(handle);

    JPEG_AUTO_TRACE_I32(info.width);
    JPEG_AUTO_TRACE_I32(info.height);
    JPEG_AUTO_TRACE_I32(info.allocWidth);
    JPEG_AUTO_TRACE_I32(info.allocHeight);
    JPEG_AUTO_TRACE_I32(info.pitch);

    mfxU32 width = bIsDecodeTarget ? info.allocWidth : info.width;
    mfxU32 height = bIsDecodeTarget ? info.allocHeight : info.height;

    VASurfaceAttrib attrib;
    JPEG_ZERO_MEMORY(attrib);

    VASurfaceAttribExternalBuffers surfExtBuf;
    JPEG_ZERO_MEMORY(surfExtBuf);

    mfxInfo.FourCC = ConvertVAFourccToMfxFormat(ConvertGrallocFourccToVAFormat(info.format));

    surfExtBuf.pixel_format = ConvertGrallocFourccToVAFormat(info.format);
    surfExtBuf.width = width;
    surfExtBuf.height = height;
    surfExtBuf.pitches[0] = info.pitch;
    surfExtBuf.num_planes = 2;
    surfExtBuf.num_buffers = 1;
    surfExtBuf.buffers = (long unsigned int*)&handle;
    surfExtBuf.flags = VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC;

    attrib.type = (VASurfaceAttribType)VASurfaceAttribExternalBufferDescriptor;
    attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;
    attrib.value.type = VAGenericValueTypePointer;
    attrib.value.value.p = &surfExtBuf;

    VAStatus va_res = vaCreateSurfaces(m_dpy, VA_RT_FORMAT_YUV420,
        width, height,
        &surface, 1, &attrib, 1);
    mfx_res = va_to_mfx_status(va_res);

    if (MFX_ERR_NONE == mfx_res)
    {
        // workaround for a 4kx2k playback performance issue
        if (bIsDecodeTarget && ((mfxInfo.Width >= 2048) || (mfxInfo.Height >= 2048)))
            mfx_res = TouchSurface(surface);
    }

    JPEG_AUTO_TRACE_I32(surface);
    JPEG_AUTO_TRACE_I32(mfx_res);
    return mfx_res;
}

mfxStatus JpegVaapiFrameAllocator::LoadGrallocBuffer(const mfxU8* handle, const VASurfaceID surface)
{
    JPEG_AUTO_TRACE_FUNC();
    mfxStatus mfx_res = MFX_ERR_NONE;

    intel_ufo_buffer_details_t info;
    JPEG_ZERO_MEMORY(info);
    *reinterpret_cast<uint32_t*>(&info) = sizeof(info);

    int err = 0;
    if (m_pGralloc) err = m_pGralloc->perform(m_pGralloc, INTEL_UFO_GRALLOC_MODULE_PERFORM_GET_BO_INFO, (buffer_handle_t)handle, &info);

    if (0 != err || !m_pGralloc)
    {
        JPEG_AUTO_TRACE_MSG("Failed to get gralloc module or get handle info");
        mfx_res = MFX_ERR_UNKNOWN;
    }

    mfxU8 *img = NULL;
    if (MFX_ERR_NONE == mfx_res)
    {
        JPEG_AUTO_TRACE_I32(info.width);
        JPEG_AUTO_TRACE_I32(info.height);
        const android::Rect rect(info.width, info.height);
        android::status_t res = android::GraphicBufferMapper::get().lock((buffer_handle_t)handle,
                                                                        GRALLOC_USAGE_HW_VIDEO_ENCODER,
                                                                        rect, (void**)&img);
        if (res != android::OK)
        {
            JPEG_AUTO_TRACE_MSG("Unable to lock image buffer");
            mfx_res = MFX_ERR_UNKNOWN;
        }
    }

    if (MFX_ERR_NONE == mfx_res)
    {
        VAImage image;
        VAStatus va_res = vaDeriveImage(m_dpy, surface, &image);
        if (VA_STATUS_SUCCESS == va_res)
        {
            mfxU8 *pBuffer = NULL;
            mfxU8 *usrptr;
            va_res = vaMapBuffer(m_dpy, image.buf, (void **) &pBuffer);
            if (VA_STATUS_SUCCESS == va_res)
            {
                mfxFrameSurface1 dst, src;

                memset(&dst, 0, sizeof(mfxFrameSurface1));
                memset(&src, 0, sizeof(mfxFrameSurface1));
                switch (image.format.fourcc)
                {
                case VA_FOURCC_NV12:
                    JPEG_AUTO_TRACE_I32(image.width);
                    JPEG_AUTO_TRACE_I32(image.height);
                    dst.Info.Width = image.width;
                    dst.Info.Height = image.height;
                    dst.Data.Y = pBuffer + image.offsets[0];
                    dst.Data.U = pBuffer + image.offsets[1];
                    dst.Data.V = dst.Data.U + 1;
                    dst.Data.Pitch = (mfxU16)image.pitches[0];
                    JPEG_AUTO_TRACE_I32(dst.Data.Pitch);
                    src.Info.Width = info.width;
                    src.Info.Height = info.height;
                    usrptr = (mfxU8 *)img;
                    src.Data.Y = usrptr;
                    src.Data.U = usrptr + info.width * info.height;
                    src.Data.V = src.Data.U + 1;
                    src.Data.Pitch = info.pitch;
                    JPEG_AUTO_TRACE_I32(src.Data.Pitch);

                    jpeg_copy_nv12(&dst, &src);

                    break;
                default:
                    va_res = VA_STATUS_ERROR_OPERATION_FAILED;
                    break;
                }
                vaUnmapBuffer(m_dpy, image.buf);
            }
            vaDestroyImage(m_dpy, image.image_id);
        }
        mfx_res = va_to_mfx_status(va_res);
    }

    android::GraphicBufferMapper::get().unlock((buffer_handle_t)handle);

    JPEG_AUTO_TRACE_I32(mfx_res);
    return mfx_res;
}

mfxStatus JpegVaapiFrameAllocator::CreateSurface(VASurfaceID &surface, mfxU16 width, mfxU16 height)
{
    JPEG_AUTO_TRACE_FUNC();
    mfxStatus mfx_res = MFX_ERR_NONE;
    VAStatus va_res = VA_STATUS_SUCCESS;

    VASurfaceAttrib attrib;

    JPEG_ZERO_MEMORY(attrib);
    attrib.type = VASurfaceAttribPixelFormat;
    attrib.value.type = VAGenericValueTypeInteger;
    attrib.value.value.i = VA_FOURCC_NV12;
    attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;

    va_res = vaCreateSurfaces(
        m_dpy,
        VA_RT_FORMAT_YUV420,
        width, height,
        &surface, 1,
        &attrib, 1);
    mfx_res = va_to_mfx_status(va_res);
    JPEG_AUTO_TRACE_I32(surface);

    JPEG_AUTO_TRACE_I32(mfx_res);
    return mfx_res;
}

mfxStatus JpegVaapiFrameAllocator::TouchSurface(VASurfaceID surface)
{
    VAImage image;
    unsigned char* buffer;
    VAStatus va_res;

    if (VA_INVALID_ID == surface) return MFX_ERR_UNKNOWN;

    va_res = vaDeriveImage(m_dpy, surface, &image);
    if (VA_STATUS_SUCCESS == va_res)
    {
        va_res = vaMapBuffer(m_dpy, image.buf, (void **) &buffer);
        if (VA_STATUS_SUCCESS == va_res)
        {
            *buffer = 0x0; // can have any value
            vaUnmapBuffer(m_dpy, image.buf);
        }
        vaDestroyImage(m_dpy, image.image_id);
     }

    return MFX_ERR_NONE;
}

mfxStatus JpegVaapiFrameAllocator::AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    JPEG_AUTO_TRACE_FUNC();
    JpegAutoLock lock(m_mutex);
    mfxStatus mfx_res = MFX_ERR_NONE;
    VAStatus  va_res  = VA_STATUS_SUCCESS;
    unsigned int va_fourcc = 0;
    VASurfaceID* surfaces = NULL;
    VASurfaceAttrib attrib;
    vaapiMemId *vaapi_mids = NULL, *vaapi_mid = NULL;
    mfxMemId* mids = NULL;
    mfxU32 fourcc = request->Info.FourCC;
    mfxU16 surfaces_num = request->NumFrameSuggested, numAllocated = 0, i = 0;
    bool bCreateSrfSucceeded = false;

    memset(response, 0, sizeof(mfxFrameAllocResponse));

    response->reserved[1] = request->Type;

    va_fourcc = ConvertMfxFourccToVAFormat(fourcc);
    if (!va_fourcc || ((VA_FOURCC_NV12 != va_fourcc) &&
                       (VA_FOURCC_YV12 != va_fourcc) &&
                       (VA_FOURCC_YUY2 != va_fourcc) &&
                       (VA_FOURCC_ARGB != va_fourcc) &&
                       (VA_FOURCC_P208 != va_fourcc)))
    {
        return MFX_ERR_MEMORY_ALLOC;
    }
    if (!surfaces_num)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }
    if (!(request->Type & JPEG_MEMTYPE_GRALLOC))
    {
        /* at the moment: encoder plug-in */
        if (MFX_ERR_NONE == mfx_res)
        {
            surfaces = (VASurfaceID*)calloc(surfaces_num, sizeof(VASurfaceID));
            vaapi_mids = (vaapiMemId*)calloc(surfaces_num, sizeof(vaapiMemId));
            mids = (mfxMemId*)calloc(surfaces_num, sizeof(mfxMemId));
            if ((NULL == surfaces) || (NULL == vaapi_mids) || (NULL == mids)) mfx_res = MFX_ERR_MEMORY_ALLOC;
        }
        if (MFX_ERR_NONE == mfx_res)
        {
            if (VA_FOURCC_P208 != va_fourcc)
            {
                attrib.type = VASurfaceAttribPixelFormat;
                attrib.value.type = VAGenericValueTypeInteger;
                attrib.value.value.i = va_fourcc;
                attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;

                va_res = vaCreateSurfaces(m_dpy,
                                        VA_RT_FORMAT_YUV420,
                                        request->Info.Width, request->Info.Height,
                                        surfaces,
                                        surfaces_num,
                                        &attrib, 1);
                mfx_res = va_to_mfx_status(va_res);
                bCreateSrfSucceeded = (MFX_ERR_NONE == mfx_res);
            }
            else
            {
                VAContextID context_id = request->reserved[0];
                mfxU32 codedbuf_size = (request->Info.Width * request->Info.Height) * 400LL / (16 * 16);

                for (numAllocated = 0; numAllocated < surfaces_num; numAllocated++)
                {
                    VABufferID coded_buf;

                    va_res = vaCreateBuffer(m_dpy,
                                          context_id,
                                          VAEncCodedBufferType,
                                          codedbuf_size,
                                          1,
                                          NULL,
                                          &coded_buf);
                    mfx_res = va_to_mfx_status(va_res);
                    if (MFX_ERR_NONE != mfx_res) break;
                    surfaces[numAllocated] = coded_buf;
                }
            }
        }
        if (MFX_ERR_NONE == mfx_res)
        {
            for (i = 0; i < surfaces_num; ++i)
            {
                vaapi_mid = &(vaapi_mids[i]);
                vaapi_mid->m_fourcc = fourcc;
                vaapi_mid->m_pSurface = &(surfaces[i]);
                mids[i] = vaapi_mid;
            }
        }
        if (MFX_ERR_NONE == mfx_res)
        {
            response->mids = mids;
            response->NumFrameActual = surfaces_num;
        }
        else
        {
            response->mids = NULL;
            response->NumFrameActual = 0;
            if (VA_FOURCC_P208 != va_fourcc)
            {
                if (bCreateSrfSucceeded) vaDestroySurfaces(m_dpy, surfaces, surfaces_num);
            }
            else
            {
                for (i = 0; i < numAllocated; i++)
                    vaDestroyBuffer(m_dpy, surfaces[i]);
            }
            if (mids)
            {
                free(mids);
                mids = NULL;
            }
            if (vaapi_mids) { free(vaapi_mids); vaapi_mids = NULL; }
            if (surfaces) { free(surfaces); surfaces = NULL; }
        }
    }
    else // if (!(request->Type & JPEG_MEMTYPE_GRALLOC))
    {
        /* at the moment: decoder plug-in */
        if (MFX_ERR_NONE == mfx_res && m_MIDs == NULL)
        {
            numMIDs = surfaces_num;
            m_MIDs = (mfxMemId*)calloc(numMIDs, sizeof(mfxMemId));
            if ((NULL == m_MIDs)) mfx_res = MFX_ERR_MEMORY_ALLOC;
        }
        if (MFX_ERR_NONE == mfx_res)
        {
            int i = 0;
            for (std::list<vaapiMemId*>::iterator it = m_extMIDs.begin(); it != m_extMIDs.end(); it++)
            { // search for free extMID
                vaapiMemId* pmid = (*it);

                if (VA_INVALID_ID != *pmid->m_pSurface)
                {
                    if(i < numMIDs)
                    {
                        m_MIDs[i] = pmid;
                    }
                    else
                    {
                        mfx_res = MFX_ERR_MEMORY_ALLOC;
                        break;
                    }
                    i++;
                }
            }
        }
        if (MFX_ERR_NONE == mfx_res)
        {
            response->mids = m_MIDs;
            response->NumFrameActual = surfaces_num;
        }
        else
        {
            response->mids = NULL;
            response->NumFrameActual = 0;
            if (m_MIDs)
            {
                free(m_MIDs);
                m_MIDs = NULL;
            }
        }
    } // if (!(request->Type & JPEG_MEMTYPE_GRALLOC))
    return mfx_res;
}

mfxStatus JpegVaapiFrameAllocator::ReleaseResponse(mfxFrameAllocResponse *response)
{
    JPEG_AUTO_TRACE_FUNC();
    vaapiMemId *vaapi_mids = NULL;
    VASurfaceID* surfaces = NULL;
    mfxU32 i = 0;
    bool isBitstreamMemory = false;

    if (!response) return MFX_ERR_NULL_PTR;

    mfxU16 type = (mfxU16)response->reserved[1];
    if (response->mids)
    {
        if (!(type & JPEG_MEMTYPE_GRALLOC))
        {
            vaapi_mids = (vaapiMemId*)(response->mids[0]);
            isBitstreamMemory = (MFX_FOURCC_P8 == vaapi_mids->m_fourcc) ? true : false;
            surfaces = vaapi_mids->m_pSurface;
            for (i = 0; i < response->NumFrameActual; ++i)
            {
                if (MFX_FOURCC_P8 == vaapi_mids[i].m_fourcc) vaDestroyBuffer(m_dpy, surfaces[i]);
            }
            free(vaapi_mids);
            free(response->mids);
            response->mids = NULL;

            if (!isBitstreamMemory)
            {
                vaDestroySurfaces(m_dpy, surfaces, response->NumFrameActual);
                free(surfaces);
            }
        }
        else
        {
            while (!m_extMIDs.empty())
            {
                vaapiMemId* pmid = m_extMIDs.back();
                if (pmid)
                {
                    if (VA_INVALID_ID != *pmid->m_pSurface)
                        vaDestroySurfaces(m_dpy, pmid->m_pSurface, 1);
                    free(pmid);
                }
                m_extMIDs.pop_back();
            }
            if (m_MIDs)
            {
                free(m_MIDs);
                m_MIDs = NULL;
            }
        }
    }
    response->NumFrameActual = 0;
    return MFX_ERR_NONE;
}

mfxStatus JpegVaapiFrameAllocator::LockFrame(mfxMemId mid, mfxFrameData *ptr)
{
    JPEG_AUTO_TRACE_FUNC();
    JpegAutoLock lock(m_mutex);
    mfxStatus mfx_res = MFX_ERR_NONE;
    VAStatus  va_res  = VA_STATUS_SUCCESS;
    vaapiMemId* vaapi_mid = (vaapiMemId*)mid;
    mfxU8* pBuffer = 0;

    if (!vaapi_mid || !(vaapi_mid->m_pSurface)) return MFX_ERR_INVALID_HANDLE;

    if (MFX_FOURCC_P8 == vaapi_mid->m_fourcc)   // bitstream processing
    {
        VACodedBufferSegment *coded_buffer_segment;
        va_res =  vaMapBuffer(m_dpy, *(vaapi_mid->m_pSurface), (void **)(&coded_buffer_segment));
        mfx_res = va_to_mfx_status(va_res);
        ptr->Y = (mfxU8*)coded_buffer_segment->buf;
    }
    else   // Image processing
    {
        va_res = vaSyncSurface(m_dpy, *(vaapi_mid->m_pSurface));
        mfx_res = va_to_mfx_status(va_res);

        if (MFX_ERR_NONE == mfx_res)
        {
            va_res = vaDeriveImage(m_dpy, *(vaapi_mid->m_pSurface), &(vaapi_mid->m_image));
            mfx_res = va_to_mfx_status(va_res);
        }
        if (MFX_ERR_NONE == mfx_res)
        {
            va_res = vaMapBuffer(m_dpy, vaapi_mid->m_image.buf, (void **) &pBuffer);
            mfx_res = va_to_mfx_status(va_res);
        }
        if (MFX_ERR_NONE == mfx_res)
        {
            switch (vaapi_mid->m_image.format.fourcc)
            {
            case VA_FOURCC_NV12:
                if (vaapi_mid->m_fourcc == MFX_FOURCC_NV12)
                {
                    ptr->Pitch = (mfxU16)vaapi_mid->m_image.pitches[0];
                    ptr->Y = pBuffer + vaapi_mid->m_image.offsets[0];
                    ptr->U = pBuffer + vaapi_mid->m_image.offsets[1];
                    ptr->V = ptr->U + 1;
                }
                else
                {
                    mfx_res = MFX_ERR_LOCK_MEMORY;
                }
                break;
            case VA_FOURCC_YV12:
                if (vaapi_mid->m_fourcc == MFX_FOURCC_YV12)
                {
                    ptr->Pitch = (mfxU16)vaapi_mid->m_image.pitches[0];
                    ptr->Y = pBuffer + vaapi_mid->m_image.offsets[0];
                    ptr->V = pBuffer + vaapi_mid->m_image.offsets[1];
                    ptr->U = pBuffer + vaapi_mid->m_image.offsets[2];
                }
                else mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
            case VA_FOURCC_YUY2:
                if (vaapi_mid->m_fourcc == MFX_FOURCC_YUY2)
                {
                    ptr->Pitch = (mfxU16)vaapi_mid->m_image.pitches[0];
                    ptr->Y = pBuffer + vaapi_mid->m_image.offsets[0];
                    ptr->U = ptr->Y + 1;
                    ptr->V = ptr->Y + 3;
                }
                else mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
            case VA_FOURCC_ARGB:
            case VA_FOURCC_ABGR:
                if (vaapi_mid->m_fourcc == MFX_FOURCC_RGB4)
                {
                    ptr->Pitch = (mfxU16)vaapi_mid->m_image.pitches[0];
                    ptr->B = pBuffer + vaapi_mid->m_image.offsets[0];
                    ptr->G = ptr->B + 1;
                    ptr->R = ptr->B + 2;
                    ptr->A = ptr->B + 3;
                }
                else mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
            default:
                mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
            }
        }
    }

    JPEG_AUTO_TRACE_I32(mfx_res);
    return mfx_res;
}

mfxStatus JpegVaapiFrameAllocator::UnlockFrame(mfxMemId mid, mfxFrameData *ptr)
{
    JPEG_AUTO_TRACE_FUNC();
    vaapiMemId* vaapi_mid = (vaapiMemId*)mid;
    JpegAutoLock lock(m_mutex);

    if (!vaapi_mid || !(vaapi_mid->m_pSurface)) return MFX_ERR_INVALID_HANDLE;

    if (MFX_FOURCC_P8 == vaapi_mid->m_fourcc)   // bitstream processing
    {
        vaUnmapBuffer(m_dpy, *(vaapi_mid->m_pSurface));
    }
    else  // Image processing
    {
        vaUnmapBuffer(m_dpy, vaapi_mid->m_image.buf);
        vaDestroyImage(m_dpy, vaapi_mid->m_image.image_id);

        if (NULL != ptr)
        {
            ptr->Pitch = 0;
            ptr->Y     = NULL;
            ptr->U     = NULL;
            ptr->V     = NULL;
            ptr->A     = NULL;
        }
    }
    return MFX_ERR_NONE;
}

mfxStatus JpegVaapiFrameAllocator::GetFrameHDL(mfxMemId mid, mfxHDL *handle)
{
    JPEG_AUTO_TRACE_FUNC();
    JpegAutoLock lock(m_mutex);

    vaapiMemId* vaapi_mid = (vaapiMemId*)mid;

    if (!handle || !vaapi_mid || !(vaapi_mid->m_pSurface)) return MFX_ERR_INVALID_HANDLE;

    *handle = vaapi_mid->m_pSurface; //VASurfaceID* <-> mfxHDL
    return MFX_ERR_NONE;
}
