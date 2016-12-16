/********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

*********************************************************************************/

#ifndef _JPEG_COMMON_H_
#define _JPEG_COMMON_H_

#include <map>

#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>
#include <ufo/gralloc.h>
#include <ufo/graphics.h>
#include <media/hardware/MetadataBufferType.h>

#include <va/va.h>
#include <va/va_android.h>
#include <va/va_drmcommon.h>

#include <android/log.h>

// MFX headers
#include <mfxdefs.h>
#include <mfxstructures.h>
#include <mfxvideo.h>
#include <mfxvideo++.h>
#include <mfxjpeg.h>

using namespace android;

typedef struct _mfx_pvr_buffer_details_t
{
        int width;
        int height;
        int format;
        int pitch;
} mfx_pvr_buffer_details_t;

enum
{
    JPEG_INTEL_COLOR_FormatVa_NV12_Linear = 0x7FA00E00,
    JPEG_INTEL_COLOR_FormatVa_NV12_Tiled  = 0x7FA00F00,
    JPEG_INTEL_COLOR_FormatRawVa = JPEG_INTEL_COLOR_FormatVa_NV12_Tiled,
    JPEG_PIXEL_FORMAT_NV12_TILED_INTEL = HAL_PIXEL_FORMAT_NV12_TILED_INTEL
};

enum
{
    JPEG_MEMTYPE_GRALLOC = 0x1000
};

#ifndef RENDERTARGET
#define RENDERTARGET
struct RenderTarget {
    enum bufType{
        KERNEL_DRM,
        ANDROID_GRALLOC,
    };

    int width;
    int height;
    int stride;
    bufType type;
    int format;
    int pixel_format;
    int handle;
    VARectangle rect;
};
#endif

#define RENDERTARGET_INTERNAL_BUFFER (RenderTarget::ANDROID_GRALLOC + 1)

// libmix defines
typedef enum
{
    MEM_MODE_MALLOC = 1,
    MEM_MODE_CI = 2,
    MEM_MODE_V4L2 = 4,
    MEM_MODE_SURFACE = 8,
    MEM_MODE_USRPTR = 16,
    MEM_MODE_GFXHANDLE = 32,
    MEM_MODE_KBUFHANDLE = 64,
    MEM_MODE_ION = 128,
    MEM_MODE_NONECACHE_USRPTR = 256,
}MemoryMode;

typedef struct
{
    MemoryMode mode;        //memory type, vasurface/malloc/gfx/ion/v4l2/ci etc
    uint32_t handle;        //handle
    uint32_t size;          //memory size
    uint32_t width;         //picture width
    uint32_t height;        //picture height
    uint32_t lumaStride;    //picture luma stride
    uint32_t chromStride;   //picture chrom stride
    uint32_t format;        //color format
    uint32_t s3dformat;     //S3D format
#ifdef INTEL_VIDEO_XPROC_SHARING
    uint32_t sessionFlag;     //for buffer sharing session
#endif
}BufferInfo;

typedef enum
{
    MfxMetadataBufferTypeCameraSource = android::kMetadataBufferTypeCameraSource,
    MfxMetadataBufferTypeGrallocSource = android::kMetadataBufferTypeGrallocSource,
    MfxMetadataBufferTypeExtension = 0xFF,   //intel extended type
    MfxMetadataBufferTypeEncoder = MfxMetadataBufferTypeExtension,        //for WiDi clone mode
    MfxMetadataBufferTypeUser = MfxMetadataBufferTypeExtension + 1,       //for WiDi user mode
    MfxMetadataBufferTypeLast = MfxMetadataBufferTypeExtension + 2,       //type number
}MfxMetadataBufferType;

struct MetadataBuffer
{
    MfxMetadataBufferType type;
    mfxU8* handle;
    BufferInfo info;
};

typedef uint32_t Display;

#define JPEG_MAX_COMPONENTS 4

#define JPEG_ALIGN_16(sz) ((((sz) + 15) >> 4 ) << 4)
#define JPEG_ALIGN_32(sz) ((((sz) + 31) >> 5 ) << 5)

#define JPEG_MAX(_a,_b)      ((_a)<(_b)?(_b):(_a))

#define SAFE_DELETE_ARRAY(P) {if(P) { delete [] P; P = NULL; } }

#ifndef JPEG_MODULE_NAME
 #define JPEG_MODULE_NAME "unknown"
#endif

#define JPEG_LOG_TAG "JpegDecoder"
#define JPEG_LOG_LEVEL ANDROID_LOG_DEBUG


#define JPEG_CLASS_NO_COPY(_class) \
    _class(const _class&); \
    _class& operator=(const _class&);

#define JPEG_THROW_IF(_if, _what) \
    { if (_if) throw _what; }

#define JPEG_ZERO_MEMORY(_obj) \
    { memset(&(_obj), 0, sizeof(_obj)); }

#endif
