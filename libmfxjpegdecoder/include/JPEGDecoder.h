/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#ifndef _LIBMFXJPEGDEC_
#define _LIBMFXJPEGDEC_

#include "JPEGCommon.h"
#include "JPEGBaseAllocator.h"
#include "JPEGAllocator.h"
#include "JPEGUtils.h"
#include <fcntl.h>
#include <stdio.h>

struct JpegInfo
{
    // in
    uint8_t *buf;
    size_t bufsize;
    // out
    uint32_t image_width;
    uint32_t image_height;
    uint32_t image_color_fourcc;
    int      image_pixel_format;
    VAPictureParameterBufferJPEGBaseline picture_param_buf;
    VASliceParameterBufferJPEGBaseline slice_param_buf[JPEG_MAX_COMPONENTS];
    VAIQMatrixBufferJPEGBaseline qmatrix_buf;
    VAHuffmanTableBufferJPEGBaseline hufman_table_buf;
    uint32_t dht_byte_offset[4];
    uint32_t dqt_byte_offset[4];
    uint32_t huffman_tables_num;
    uint32_t quant_tables_num;
    uint32_t soi_offset;
    uint32_t eoi_offset;
    uint32_t scan_ctrl_count;
};

enum JpegDecodeStatus
{
    JD_SUCCESS,
    JD_UNINITIALIZED,
    JD_ALREADY_INITIALIZED,
    JD_RENDER_TARGET_TYPE_UNSUPPORTED,
    JD_INPUT_FORMAT_UNSUPPORTED,
    JD_OUTPUT_FORMAT_UNSUPPORTED,
    JD_INVALID_RENDER_TARGET,
    JD_RENDER_TARGET_NOT_INITIALIZED,
    JD_CODEC_UNSUPPORTED,
    JD_INITIALIZATION_ERROR,
    JD_RESOURCE_FAILURE,
    JD_DECODE_FAILURE,
    JD_BLIT_FAILURE,
    JD_ERROR_BITSTREAM,
    JD_RENDER_TARGET_BUSY,
};



class JpegDecoder {
public:
    JpegDecoder();
    virtual ~JpegDecoder();
    virtual JpegDecodeStatus init(int width, int height, RenderTarget **targets, int num);
    virtual void deinit();
    virtual JpegDecodeStatus parse(JpegInfo &jpginfo);
    virtual JpegDecodeStatus decode(JpegInfo &jpginfo, RenderTarget &target);
    virtual JpegDecodeStatus sync(RenderTarget &target);
    virtual bool busy(RenderTarget &target) const;
    virtual JpegDecodeStatus blit(RenderTarget &src, RenderTarget &dst);

protected:
    virtual JpegDecodeStatus createSurfaceDrm(int width, int height, int pixel_format, unsigned long boname, int stride, VASurfaceID *surf_id);

private:
    VADisplay mDisplay;
    bool      mInited;
    bool      mDecoderInited;
    JpegMutex m_mutex;
    int frame;
    /* Target Render -> VA surf ID */
    std::map<unsigned long, VASurfaceID> mVaSurfaceMap;

    /* Target Render -> VA surf ID */
    std::map<unsigned long, int>         mMfxSurfaceMap;
    mfxFrameAllocResponse *mAllocResponse;
    mfxFrameInfo          *mFrameInfo;
    MFXVideoSession       *mSession;
    MFXVideoDECODE        *mDecoder;
    JpegVaapiFrameAllocator    *mFrameAllocator;
};

struct MapHandle
{
friend class JpegDecoder;
public:
    bool valid;
private:
    VAImage *img;
};

#endif
