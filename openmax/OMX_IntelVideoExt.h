/*
 * Copyright (c) 2010 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/** OMX_VideoExt.h - OpenMax IL version 1.1.2
 * The OMX_VideoExt header file contains extensions to the
 * definitions used by both the application and the component to
 * access video items.
 */

#ifndef OMX_IntelVideoExt_h
#define OMX_IntelVideoExt_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each OMX header shall include all required header files to allow the
 * header to compile without errors.  The includes below are required
 * for this header file to compile successfully
 */
#include <OMX_Core.h>
/** NALU Formats */
typedef enum OMX_INTEL_NALUFORMATSTYPE {
    OMX_NaluFormatZeroByteInterleaveLength = 32,
    OMX_NaluFormatStartCodesSeparateFirstHeader = 64,
    OMX_NaluFormatLengthPrefixedSeparateFirstHeader = 128,
} OMX_INTEL_NALUFORMATSTYPE;


typedef struct OMX_VIDEO_PARAM_BYTESTREAMTYPE {
     OMX_U32 nSize;                 // Size of the structure
     OMX_VERSIONTYPE nVersion;      // OMX specification version
     OMX_U32 nPortIndex;            // Port that this structure applies to
     OMX_BOOL bBytestream;          // Enable/disable bytestream support
} OMX_VIDEO_PARAM_BYTESTREAMTYPE;

typedef struct OMX_VIDEO_CONFIG_INTEL_BITRATETYPE {
     OMX_U32 nSize;
     OMX_VERSIONTYPE nVersion;
     OMX_U32 nPortIndex;
     OMX_U32 nMaxEncodeBitrate;    // Maximum bitrate
     OMX_U32 nTargetPercentage;    // Target bitrate as percentage of maximum bitrate; e.g. 95 is 95%
     OMX_U32 nWindowSize;          // Window size in milliseconds allowed for bitrate to reach target
     OMX_U32 nInitialQP;           // Initial QP for I frames
     OMX_U32 nMinQP;
     OMX_U32 nMaxQP;
     OMX_U32 nFrameRate;
     OMX_U32 nTemporalID;
} OMX_VIDEO_CONFIG_INTEL_BITRATETYPE;

typedef enum  {
    OMX_Video_Intel_ControlRateVideoConferencingMode = OMX_Video_ControlRateVendorStartUnused + 1
};

typedef struct OMX_VIDEO_PARAM_INTEL_AVC_DECODE_SETTINGS {
     OMX_U32 nSize;                       // Size of the structure
     OMX_VERSIONTYPE nVersion;            // OMX specification version
     OMX_U32 nPortIndex;                  // Port that this structure applies to
     OMX_U32 nMaxNumberOfReferenceFrame;  // Maximum number of reference frames
     OMX_U32 nMaxWidth;                   // Maximum width of video
     OMX_U32 nMaxHeight;                  // Maximum height of video
} OMX_VIDEO_PARAM_INTEL_AVC_DECODE_SETTINGS;


typedef struct OMX_VIDEO_CONFIG_INTEL_SLICE_NUMBERS {
     OMX_U32 nSize;                       // Size of the structure
     OMX_VERSIONTYPE nVersion;            // OMX specification version
     OMX_U32 nPortIndex;                  // Port that this structure applies to
     OMX_U32 nISliceNumber;               // I frame slice number
     OMX_U32 nPSliceNumber;               // P frame slice number
} OMX_VIDEO_CONFIG_INTEL_SLICE_NUMBERS;


typedef struct OMX_VIDEO_CONFIG_INTEL_AIR {
     OMX_U32 nSize;                       // Size of the structure
     OMX_VERSIONTYPE nVersion;            // OMX specification version
     OMX_U32 nPortIndex;                  // Port that this structure applies to
     OMX_BOOL bAirEnable;                 // Enable AIR
     OMX_BOOL bAirAuto;                   // AIR auto
     OMX_U32 nAirMBs;                     // Number of AIR MBs
     OMX_U32 nAirThreshold;               // AIR Threshold

} OMX_VIDEO_CONFIG_INTEL_AIR;

typedef struct OMX_VIDEO_PARAM_INTEL_AVCVUI {
     OMX_U32 nSize;                       // Size of the structure
     OMX_VERSIONTYPE nVersion;            // OMX specification version
     OMX_U32 nPortIndex;                  // Port that this structure applies to
     OMX_BOOL  bVuiGeneration;            // Enable/disable VUI generation

} OMX_VIDEO_PARAM_INTEL_AVCVUI;

typedef struct OMX_VIDEO_PARAM_INTEL_ADAPTIVE_SLICE_CONTROL {
     OMX_U32 nSize;                       // Size of the structure
     OMX_VERSIONTYPE nVersion;            // OMX specification version
     OMX_U32 nPortIndex;                  // Port that this structure applies to
     OMX_BOOL bEnable;                    // enable adaptive slice control
     OMX_U32 nMinPSliceNumber;            // minimum number of P slices
     OMX_U32 nNumPFramesToSkip;           // number of P frames after an I frame to skip before kicking in adaptive slice control
     OMX_U32 nSliceSizeThreshold;         // Slice size threshold for adaptive slice control to start a new slice
     OMX_U32 nSliceSizeSkipThreshold;     // Slice size skip threshold for adaptive slice control to start a new slice
} OMX_VIDEO_PARAM_INTEL_ADAPTIVE_SLICE_CONTROL;

/**
 * Vendor Private Configs
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nCapacity  : Specifies the private unit size
 *  nHolder    : Pointer to private unit address
 */
typedef struct OMX_VIDEO_CONFIG_PRI_INFOTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nCapacity;
    OMX_PTR nHolder;
} OMX_VIDEO_CONFIG_PRI_INFOTYPE;

// Error reporting data structure
typedef struct OMX_VIDEO_CONFIG_INTEL_ERROR_REPORT {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bEnable;
} OMX_VIDEO_CONFIG_INTEL_ERROR_REPORT;

#define MAX_ERR_NUM 10

typedef enum
{
    OMX_Decode_HeaderError   = 0,
    OMX_Decode_MBError       = 1,
    OMX_Decode_SliceMissing  = 2,
    OMX_Decode_RefMissing    = 3,
} OMX_VIDEO_DECODE_ERRORTYPE;

typedef struct OMX_VIDEO_ERROR_INFO {
    OMX_VIDEO_DECODE_ERRORTYPE type;
    OMX_U32 num_mbs;
    union {
        struct {OMX_U32 start_mb; OMX_U32 end_mb;} mb_pos;
    } error_data;
};

typedef struct OMX_VIDEO_ERROR_BUFFER {
    OMX_U32 errorNumber;   // Error number should be no more than MAX_ERR_NUM
    OMX_S64 timeStamp;      // presentation time stamp
    OMX_VIDEO_ERROR_INFO errorArray[MAX_ERR_NUM];
};

typedef struct OMX_VIDEO_OUTPUT_ERROR_BUFFERS {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nErrorBufIndex;
    OMX_VIDEO_ERROR_BUFFER errorBuffers;
};

typedef struct OMX_VIDEO_INTEL_VP8REFERENCEFRAMETYPE {
    OMX_BOOL bPreviousFrameRefresh;
    OMX_U32 nGoldenFrameRefresh;
    OMX_U32 nAlternateFrameRefresh;
    OMX_BOOL bUsePreviousFrame;
    OMX_BOOL bUseGoldenFrame;
    OMX_BOOL bUseAlternateFrame;
    OMX_BOOL bUpdateEntropy;
} OMX_VIDEO_INTEL_VP8REFERENCEFRAMETYPE;


// max frame size for VP8 encode during WebRTC feature
typedef struct OMX_VIDEO_CONFIG_INTEL_VP8_MAX_FRAME_SIZE_RATIO {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nMaxFrameSizeRatio;
} OMX_VIDEO_CONFIG_INTEL_VP8_MAX_FRAME_SIZE_RATIO;

// temporal layer for Sand
typedef struct OMX_VIDEO_PARAM_INTEL_TEMPORAL_LAYER {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nNumberOfTemporalLayer;
    OMX_U32 nPeriodicity;
    OMX_U32 nLayerID[32];
    OMX_VIDEO_INTEL_VP8REFERENCEFRAMETYPE nVP8RefFrameType[32];
} OMX_VIDEO_PARAM_INTEL_TEMPORAL_LAYER;

// hrd buffer parameter minQP manQP for Sand VP8 encode
typedef struct OMX_VIDEO_INTEL_VP8_PARAM{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nHrdbuf_fullness;
    OMX_U32 nHrdbuf_initial;
    OMX_U32 nHrdbuf_optimal;
    OMX_U32 nMin_qp;
    OMX_U32 nMax_qp;
    OMX_U32 nMaxNumOfConsecutiveDropFrames;
} OMX_VIDEO_INTEL_VP8_PARAM;

// Request OMX to allocate a black frame to video mute feature
typedef struct OMX_VIDEO_INTEL_REQUEST_BALCK_FRAME_POINTER {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_PTR nFramePointer;
} OMX_VIDEO_INTEL_REQUEST_BALCK_FRAME_POINTER;

typedef struct OMX_VIDEO_CONFIG_INTEL_DECODER_BUFFER_HANDLE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nIndex;
    OMX_U8* pNativeHandle;
} OMX_VIDEO_CONFIG_INTEL_DECODER_BUFFER_HANDLE;

// The structure is used to set HRD parameters on the encoder during
// initialization/reset, with the following parameter structure
typedef struct OMX_VIDEO_PARAM_HRD_PARAM
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nHRDBufferSize;      // nHRDBufferSize is used to configure buffer size for HRD model (in kilo-bytes).
    OMX_U32 nHRDInitialFullness; // nHRDInitialFullness is used to configure initial fullness for HRD model (in kilo-bytes).
    OMX_BOOL bWriteHRDSyntax;    // bWriteHRDSyntax is used to enable HRD syntax writing to bitstream
} OMX_VIDEO_PARAM_HRD_PARAM;

// The structure is used to set maximum picture size parameter on
// the encoder during initialization/reset, with the following parameter structure
typedef struct OMX_VIDEO_PARAM_MAX_PICTURE_SIZE
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nMaxPictureSize; // nMaxPictureSize is used to configure maximum frame size (in bytes).
} OMX_VIDEO_PARAM_MAX_PICTURE_SIZE;

// The structure is used to configure target usage (quility vs speed)
typedef struct OMX_VIDEO_PARAM_TARGET_USAGE
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nTargetUsage; // nTargetUsage is used to configure target usage (quility vs speed) parameter. Values range is [1...7].
                          //1 means the best quality. 7 means the best speed
} OMX_VIDEO_PARAM_TARGET_USAGE;

// The structure is used to add user date to the next encoding
// AVC frame SEI Nalu during encoding with the following parameter structure
typedef struct OMX_VIDEO_CONFIG_USERDATA
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nUserDataSize; // nUserDataSize is used to inform the size of the user data (in bytes)
    OMX_U8 *pUserData;     // pUserData is a pointer to the user data needed to be attached to the next encoding frame.
} OMX_VIDEO_CONFIG_USERDATA;

// The structure is used to set encoder cropping values
typedef struct OMX_VIDEO_PARAM_ENCODE_FRAME_CROPPING_PARAM
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nCropX;
    OMX_U32 nCropY;
    OMX_U32 nCropW;
    OMX_U32 nCropH;
} OMX_VIDEO_PARAM_ENCODE_FRAME_CROPPING_PARAM;

typedef struct OMX_VIDEO_PARAM_ENCODE_VUI_CONTROL_PARAM
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U16 nAspectRatioW;
    OMX_U16 nAspectRatioH;
    OMX_U16 nVideoFormat;
    OMX_U16 nColourDescriptionPresent;
    OMX_U16 nColourPrimaries;
    OMX_U16 nFixedFrameRate;
    OMX_U16 nPicStructPresent;
    OMX_U16 nLowDelayHRD;
    OMX_BOOL bVideoFullRange;
    OMX_U32 nReserved[4];
}
OMX_VIDEO_PARAM_ENCODE_VUI_CONTROL_PARAM;

// The structure is used to configure encoding dirty rect
typedef struct OMX_VIDEO_CONFIG_DIRTY_RECT {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nNumRectangles;
    OMX_CONFIG_RECTTYPE* pRectangles;
} OMX_VIDEO_CONFIG_CONFIG_DIRTY_RECT;

// The structure is used to make encoder to release all buffered surfaces
typedef struct OMX_VIDEO_CONFIG_INTEL_ENCODER_BUFFER_FREE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
} OMX_VIDEO_CONFIG_INTEL_ENCODER_BUFFER_FREE;

// Set temporal layer count
typedef struct OMX_VIDEO_CONFIG_INTEL_TEMPORALLAYERCOUNT {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nTemproalLayerCount;
} OMX_VIDEO_CONFIG_INTEL_TEMPORALLAYERCOUNT;

#define OMX_BUFFERFLAG_TFF 0x00010000
#define OMX_BUFFERFLAG_BFF 0x00020000

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_VideoExt_h */
/* File EOF */
