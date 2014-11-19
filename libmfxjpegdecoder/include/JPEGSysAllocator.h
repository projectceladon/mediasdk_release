/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#ifndef __JPEGSYSALLOCATOR_H__
#define __JPEGSYSALLOCATOR_H__

#include "JPEGCommon.h"
#include "JPEGUtils.h"
#include "JPEGBaseAllocator.h"
#include <functional>

template <class T>
class safe_array
{
public:
    safe_array(T *ptr = 0):m_ptr(ptr)
    { // construct from object pointer
    };
    ~safe_array()
    {
        reset(0);
    }
    T* get()
    { // return wrapped pointer
        return m_ptr;
    }
    T* release()
    { // return wrapped pointer and give up ownership
        T* ptr = m_ptr;
        m_ptr = 0;
        return ptr;
    }
    void reset(T* ptr)
    { // destroy designated object and store new pointer
        if (m_ptr)
        {
            delete[] m_ptr;
        }
        m_ptr = ptr;
    }
protected:
    T* m_ptr; // the wrapped object pointer
};

struct sBuffer
{
    mfxU32      id;
    mfxU32      nbytes;
    mfxU16      type;
};

struct sFrame
{
    mfxU32          id;
    mfxFrameInfo    info;
};


class BaseBufferAllocator : public mfxBufferAllocator
{
public:
    BaseBufferAllocator();
    virtual ~BaseBufferAllocator();

    virtual mfxStatus AllocBuffer(mfxU32 nbytes, mfxU16 type, mfxMemId *mid) = 0;
    virtual mfxStatus LockBuffer(mfxMemId mid, mfxU8 **ptr) = 0;
    virtual mfxStatus UnlockBuffer(mfxMemId mid) = 0;
    virtual mfxStatus FreeBuffer(mfxMemId mid) = 0;

private:
    static mfxStatus MFX_CDECL  Alloc_(mfxHDL pthis, mfxU32 nbytes, mfxU16 type, mfxMemId *mid);
    static mfxStatus MFX_CDECL  Lock_(mfxHDL pthis, mfxMemId mid, mfxU8 **ptr);
    static mfxStatus MFX_CDECL  Unlock_(mfxHDL pthis, mfxMemId mid);
    static mfxStatus MFX_CDECL  Free_(mfxHDL pthis, mfxMemId mid);
};

class JpegSysBufferAllocator : public BaseBufferAllocator
{
public:
    JpegSysBufferAllocator();
    virtual ~JpegSysBufferAllocator();
    virtual mfxStatus AllocBuffer(mfxU32 nbytes, mfxU16 type, mfxMemId *mid);
    virtual mfxStatus LockBuffer(mfxMemId mid, mfxU8 **ptr);
    virtual mfxStatus UnlockBuffer(mfxMemId mid);
    virtual mfxStatus FreeBuffer(mfxMemId mid);
};

struct mfxAllocatorParams
{
    virtual ~mfxAllocatorParams(){};
};


struct JpegSysAllocatorParams : mfxAllocatorParams
{
    JpegSysAllocatorParams()
        : mfxAllocatorParams() { }
    BaseBufferAllocator *pBufferAllocator;
};

class JpegSysFrameAllocator: public JpegFrameAllocator
{
public:
    JpegSysFrameAllocator(mfxAllocatorParams *pParams);
    virtual ~JpegSysFrameAllocator();

    virtual mfxStatus Init(mfxAllocatorParams *pParams);
    virtual mfxStatus Close();
    virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData *ptr);
    virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData *ptr);
    virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL *handle);

protected:
    virtual mfxStatus CheckRequestType(mfxFrameAllocRequest *request);
    virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse *response);
    virtual mfxStatus AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);

    BaseBufferAllocator *m_pBufferAllocator;
    bool m_bOwnBufferAllocator;
    JpegMutex m_mutex;
    JPEG_CLASS_NO_COPY(JpegSysFrameAllocator)
};


#endif // __MFX_OMX_VAAPI_ALLOCATOR_H__
