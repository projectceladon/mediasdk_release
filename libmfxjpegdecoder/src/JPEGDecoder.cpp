/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#include "JPEGDecoder.h"
#include "JPEGAllocator.h"
#include "JPEGBitstream.h"

inline int fourcc2VaFormat(uint32_t fourcc)
{
    switch(fourcc) {
    case VA_FOURCC_422H:
    case VA_FOURCC_422V:
    case VA_FOURCC_YUY2:
        return VA_RT_FORMAT_YUV422;
    case VA_FOURCC_IMC3:
    case VA_FOURCC_YV12:
    case VA_FOURCC_NV12:
        return VA_RT_FORMAT_YUV420;
    case VA_FOURCC_444P:
        return VA_RT_FORMAT_YUV444;
    case VA_FOURCC_411P:
        return VA_RT_FORMAT_YUV411;
    case VA_FOURCC_BGRA:
    case VA_FOURCC_ARGB:
    case VA_FOURCC_RGBA:
        return VA_RT_FORMAT_RGB32;
    default:
        return -1;
    }
}

int fourcc2PixelFormat(uint32_t fourcc)
{
    switch(fourcc) {
    case VA_FOURCC_YV12:
        return HAL_PIXEL_FORMAT_YV12;
    case VA_FOURCC_422H:
        return HAL_PIXEL_FORMAT_YCbCr_422_H_INTEL;
    case VA_FOURCC_YUY2:
        return HAL_PIXEL_FORMAT_YCbCr_422_I;
    case VA_FOURCC_NV12:
        return HAL_PIXEL_FORMAT_NV12_TILED_INTEL;
    case VA_FOURCC_RGBA:
        return HAL_PIXEL_FORMAT_RGBA_8888;
    case VA_FOURCC_422V:
    case VA_FOURCC_411P:
    default:
        return -1;
    }
}
uint32_t pixelFormat2Fourcc(int pixel_format)
{
    switch(pixel_format) {
    case HAL_PIXEL_FORMAT_YV12:
        return VA_FOURCC_YV12;
    case HAL_PIXEL_FORMAT_YCbCr_422_H_INTEL:
        return VA_FOURCC_422H;
    case HAL_PIXEL_FORMAT_YCbCr_422_I:
        return VA_FOURCC_YUY2;
    case HAL_PIXEL_FORMAT_NV12_TILED_INTEL:
        return VA_FOURCC_NV12;
    case HAL_PIXEL_FORMAT_RGBA_8888:
        return VA_FOURCC_RGBA;
    default:
        return 0;
    }
}

JpegDecoder::JpegDecoder()
{
    mInited        = false;
    mDecoderInited = false;
}

JpegDecoder::~JpegDecoder()
{
    if (!mInited) return;
    mDecoder->Close();
    mSession->Close();
    delete mFrameAllocator;
    delete mAllocResponse;
    delete mFrameInfo;
}

JpegDecodeStatus JpegDecoder::init(int width, int height, RenderTarget **targets, int num)
{
    JpegAutoLock lock(m_mutex);
    if (mInited) return JD_ALREADY_INITIALIZED;

    int va_major_version, va_minor_version;
    VAStatus status;
    mfxStatus mfx_status;
    mfxVersion mfxVer;
    mfxVer.Major = 1;
    mfxVer.Minor = 8;
    Display dpy;
    mDisplay = vaGetDisplay(&dpy);
    if (!mDisplay) return JD_INITIALIZATION_ERROR;

    status = vaInitialize(mDisplay, &va_major_version, &va_minor_version);
    if (VA_STATUS_SUCCESS != status) return JD_INITIALIZATION_ERROR;

    mAllocResponse = (mfxFrameAllocResponse *)calloc(1, sizeof(mfxFrameAllocResponse));
    if (! mAllocResponse) return JD_INITIALIZATION_ERROR;

    mFrameInfo  = (mfxFrameInfo *)calloc(1, sizeof(mfxFrameInfo));
    if (!mFrameInfo) return JD_INITIALIZATION_ERROR;

    mFrameAllocator = new JpegVaapiFrameAllocator((VADisplay)mDisplay);
    if (!mFrameAllocator) return JD_INITIALIZATION_ERROR;

    for (int i = 0; i < num ; i++)
    {
        mfxMemId mid;
        mFrameAllocator->LoadSurface((const buffer_handle_t)(targets[i]->handle), true, *mFrameInfo, &mid);
        mMfxSurfaceMap[targets[i]->handle] = i;
    }
    mfxFrameAllocRequest  iorequest;
    memset(&iorequest, 0, sizeof(mfxFrameAllocRequest));
    iorequest.NumFrameMin       = num;
    iorequest.NumFrameSuggested = num;
    iorequest.Info.Width  = width;
    iorequest.Info.Height = height;
    iorequest.Info.CropW  = width;
    iorequest.Info.CropH  = height;
    iorequest.Info.FourCC = MFX_FOURCC_NV12;
    iorequest.Type = JPEG_MEMTYPE_GRALLOC | MFX_MEMTYPE_EXTERNAL_FRAME | MFX_IOPATTERN_OUT_VIDEO_MEMORY | MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET | MFX_MEMTYPE_FROM_DECODE;
    status = mFrameAllocator->Alloc(mFrameAllocator->pthis, &iorequest, mAllocResponse);
    if (MFX_ERR_NONE != status) return JD_INITIALIZATION_ERROR;

    if (mAllocResponse->NumFrameActual != num) return JD_INITIALIZATION_ERROR;

    mSession = new MFXVideoSession();
    if (!mSession) return JD_INITIALIZATION_ERROR;

    mfx_status = mSession->Init(MFX_IMPL_HARDWARE, &mfxVer);
    if (MFX_ERR_NONE != mfx_status) return JD_INITIALIZATION_ERROR;

    mfx_status = mSession->SetFrameAllocator(mFrameAllocator);
    if (MFX_ERR_NONE != mfx_status) return JD_INITIALIZATION_ERROR;

    mfx_status = mSession->SetHandle(static_cast<mfxHandleType>(MFX_HANDLE_VA_DISPLAY), (mfxHDL)mDisplay);
    if (MFX_ERR_NONE != mfx_status) return JD_INITIALIZATION_ERROR;

    mDecoder = new MFXVideoDECODE(*mSession);
    if (!mDecoder) return JD_INITIALIZATION_ERROR;

    mInited = true;
    return JD_SUCCESS;
}


void JpegDecoder::deinit()
{
    if (!mInited) return;
}


JpegDecodeStatus JpegDecoder::parse(JpegInfo &jpginfo)
{
    JpegAutoLock lock(m_mutex);
    mfxBitstream  bitstream;
    mfxStatus     status;
    mfxVideoParam params;
    mfxVideoParam good_params;
    if (!mInited) return JD_UNINITIALIZED;

    if (!jpginfo.buf || (jpginfo.bufsize == 0)) return JD_ERROR_BITSTREAM;


    memset(&bitstream,   0, sizeof(mfxBitstream));
    memset(&params,      0, sizeof(mfxVideoParam));
    memset(&good_params, 0, sizeof(mfxVideoParam));

    // Library needs to know what codec is going to be used
    params.mfx.CodecId = MFX_CODEC_JPEG;

    bitstream.Data = jpginfo.buf;
    bitstream.DataLength    = bitstream.MaxLength = jpginfo.bufsize;
    bitstream.DataOffset    = 0;
    bitstream.EncryptedData = 0;
    status = mDecoder->DecodeHeader(&bitstream, &params);
    if (MFX_ERR_NONE != status) return JD_ERROR_BITSTREAM;

    if (!mDecoderInited)
    {
        memcpy(&good_params, &params, sizeof(mfxVideoParam));

        params.AsyncDepth = 1;
        params.IOPattern  = MFX_MEMTYPE_EXTERNAL_FRAME | MFX_IOPATTERN_OUT_VIDEO_MEMORY | MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET | MFX_MEMTYPE_FROM_DECODE;
        status = mDecoder->Query(&params, &good_params);
        if (MFX_ERR_NONE != status) return JD_SUCCESS;

        memcpy(mFrameInfo, &params.mfx.FrameInfo, sizeof(mfxFrameInfo));

        status = mDecoder->Init(&good_params);
        if (MFX_ERR_NONE != status) return JD_SUCCESS;

        mDecoderInited = true;
    }
    return JD_SUCCESS;
}

JpegDecodeStatus JpegDecoder::decode(JpegInfo &jpginfo, RenderTarget &target)
{
    JpegAutoLock lock(m_mutex);
    mfxBitstream      bitstream;
    mfxStatus         status;
    mfxFrameSurface1 *work_surface;
    mfxFrameSurface1 *output_surface = NULL;
    mfxSyncPoint      sync_point;

    if (!mInited || !mDecoderInited)
        return JD_UNINITIALIZED;

    if (!jpginfo.buf || (jpginfo.bufsize == 0))
        return JD_ERROR_BITSTREAM;

    bitstream.Data = jpginfo.buf;
    bitstream.DataLength    = bitstream.MaxLength = jpginfo.bufsize;
    bitstream.DataOffset    = 0;
    bitstream.EncryptedData = 0;
    work_surface = (mfxFrameSurface1 *)calloc(1, sizeof(mfxFrameSurface1));
    memcpy(&(work_surface->Info), mFrameInfo, sizeof(mfxFrameInfo));
    work_surface->Data.Locked = 0;
    int index = mMfxSurfaceMap[target.handle];
    work_surface->Data.MemId  = mAllocResponse->mids[index];
    status = mDecoder->DecodeFrameAsync(&bitstream, work_surface, &output_surface, &sync_point);
    if (MFX_ERR_NONE != status)
        return JD_SUCCESS;

    while(1)
    {
        status = mSession->SyncOperation(sync_point, 1000);
        if ( MFX_ERR_NONE == status)
        {
            break;
        }
    }

    if (MFX_ERR_NONE != status) return JD_DECODE_FAILURE;

    return JD_SUCCESS;
}


JpegDecodeStatus JpegDecoder::sync(RenderTarget &target)
{
    if (!mInited) return JD_UNINITIALIZED;
    return JD_SUCCESS;
}


bool JpegDecoder::busy(RenderTarget &target) const
{
    if (!mInited) return false;
    return false;
}

JpegDecodeStatus JpegDecoder::createSurfaceDrm(int width, int height, int pixel_format, unsigned long boname, int stride, VASurfaceID *surf_id)
{
    VAStatus                        st;
    VASurfaceAttrib                 attrib_list;
    VASurfaceAttribExternalBuffers  vaSurfaceExternBuf;
    uint32_t fourcc = pixelFormat2Fourcc(pixel_format);
    vaSurfaceExternBuf.pixel_format = fourcc;
    vaSurfaceExternBuf.width        = width;
    vaSurfaceExternBuf.height       = height;
    vaSurfaceExternBuf.pitches[0]   = stride;
    vaSurfaceExternBuf.buffers      = &boname;
    vaSurfaceExternBuf.num_buffers  = 1;
    vaSurfaceExternBuf.flags        = VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM;
    attrib_list.type          = VASurfaceAttribExternalBufferDescriptor;
    attrib_list.flags         = VA_SURFACE_ATTRIB_SETTABLE;
    attrib_list.value.type    = VAGenericValueTypePointer;
    attrib_list.value.value.p = (void *)&vaSurfaceExternBuf;

    st = vaCreateSurfaces(mDisplay,
            fourcc2VaFormat(fourcc),
            width,
            height,
            surf_id,
            1,
            &attrib_list,
            1);
    if (st != VA_STATUS_SUCCESS) return JD_RESOURCE_FAILURE;

    return JD_SUCCESS;
}


JpegDecodeStatus JpegDecoder::blit(RenderTarget &src, RenderTarget &dst)
{
    if (!mInited) return JD_UNINITIALIZED;

    return JD_SUCCESS;
}
