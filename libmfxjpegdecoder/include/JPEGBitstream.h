/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#ifndef __JPEG_BITSTREAM_H__
#define __JPEG_BITSTREAM_H__

#include "JPEGCommon.h"

struct mfxBitstream2 : mfxBitstream
{
    mfxU16 DependencyId;//splitter might want to put source information, analog to FrameId in surface
    bool   isNull; // whether to use null pointer instead of this object

};

class JpegBitstreamBuffer
{
public:
    JpegBitstreamBuffer();
    virtual ~JpegBitstreamBuffer();

    virtual mfxStatus Init( mfxU32 nBufferSizeMin
                           , mfxU32 nBufferSizeMax);

    virtual mfxStatus SetMinBuffSize(mfxU32 nBufferSizeMin);
    virtual mfxStatus GetMinBuffSize(mfxU32& nBufferSizeMin);

    virtual mfxStatus Reset();//reset will empties buffer without fully closing it
    virtual mfxStatus Close();

    static mfxStatus ExtendBs(mfxU32 nNewLen, mfxBitstream *src);


protected:

    mfxU32        mBufferSizeMax;
    mfxU32        mBufferSizeMin;
    mfxU32        mMinBufferSize;
    bool          mInited;
    mfxBitstream  mAllocatedBuf;
    bool          mLocked;
    bool          mEos;//eos flag set by upstream block
    mfxU32        m_nMinSize;//buffer wilnot lock untill it reached this size

    mfxBitstream2  m_inputBS;//stored last bitstream
};

#endif
