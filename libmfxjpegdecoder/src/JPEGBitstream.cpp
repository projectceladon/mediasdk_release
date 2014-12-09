/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#include "JPEGBitstream.h"

JpegBitstreamBuffer::JpegBitstreamBuffer()
{
   mEos = false;
   mInited = false;
}

JpegBitstreamBuffer::~JpegBitstreamBuffer()
{
    Close();
}

mfxStatus JpegBitstreamBuffer::Init(mfxU32 nBufferSizeMin, mfxU32 nBufferSizeMax)
{
    Close();

    if (nBufferSizeMax <= nBufferSizeMin)
    {
        nBufferSizeMax += nBufferSizeMin;//at least min + 1 frame
    }

    mBufferSizeMin  = nBufferSizeMin;
    mBufferSizeMax  = nBufferSizeMax;

    return MFX_ERR_NONE;
}

mfxStatus JpegBitstreamBuffer::SetMinBuffSize(mfxU32 nBufferSizeMin)
{
    if ((nBufferSizeMin <= 4 && nBufferSizeMin > 1) || nBufferSizeMin > mBufferSizeMax)
    {
        return MFX_ERR_UNKNOWN;
    }
    mBufferSizeMin = nBufferSizeMin;
    return MFX_ERR_NONE;
}

mfxStatus JpegBitstreamBuffer::GetMinBuffSize(mfxU32 &nBufferSizeMin)
{
    nBufferSizeMin = mBufferSizeMin;
    return MFX_ERR_NONE;
}

mfxStatus JpegBitstreamBuffer::Reset()
{
    m_inputBS.DataLength = 0;
    return MFX_ERR_NONE;
}

mfxStatus JpegBitstreamBuffer::Close()
{
    mEos = false;

    SAFE_DELETE_ARRAY(m_inputBS.Data);

    return MFX_ERR_NONE;
}

mfxStatus JpegBitstreamBuffer::ExtendBs(mfxU32 nNewLen, mfxBitstream *src)
{
    if (NULL == src) return MFX_ERR_NONE;

    if (nNewLen <= src->MaxLength) return MFX_ERR_NONE;

    nNewLen = JPEG_ALIGN_32(nNewLen);

    mfxU8 * p = new mfxU8[nNewLen];
    if (!p) return MFX_ERR_MEMORY_ALLOC;

    src->MaxLength = nNewLen;
    memcpy(p, src->Data + src->DataOffset, src->DataLength);
    delete [] src->Data;
    src->Data = p;
    src->DataOffset = 0;

    return MFX_ERR_NONE;
}
