/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#ifndef __JPEG_UTILS_H__
#define __JPEG_UTILS_H__

#include "JPEGCommon.h"

class jpeg_trace
{
public:
    jpeg_trace(const char* _modulename, const char* _function, const char* _taskname);
    ~jpeg_trace(void);
    void printf_msg(const char* msg);
    void printf_i32(const char* name, mfxI32 value);
    void printf_u32(const char* name, mfxU32 value);
    void printf_i64(const char* name, mfxI64 value);
    void printf_f64(const char* name, mfxF64 value);
    void printf_p(const char* name, void* value);
    void printf_s(const char* name, const char* value);

protected:
    const char* modulename;
    const char* function;
    const char* taskname;
private:
    JPEG_CLASS_NO_COPY(jpeg_trace)
};

class JpegMutex
{
public:
    JpegMutex(void);
    ~JpegMutex(void);

    mfxStatus Lock(void);
    mfxStatus Unlock(void);
    bool Try(void);

private: // variables
    bool m_bInitialized;
    pthread_mutex_t m_mutex;

private: // functions
    JPEG_CLASS_NO_COPY(JpegMutex)
};

class JpegAutoLock
{
public:
    JpegAutoLock(JpegMutex& mutex);
    ~JpegAutoLock(void);

    mfxStatus Lock(void);
    mfxStatus Unlock(void);

private: // variables
    JpegMutex& m_rMutex;
    bool m_bLocked;

private: // functions
    JPEG_CLASS_NO_COPY(JpegAutoLock)
};

#define JPEG_AUTO_TRACE(_task_name) \
        jpeg_trace _jpeg_trace(JPEG_MODULE_NAME, __FUNCTION__, _task_name)

#define JPEG_AUTO_TRACE_FUNC() \
        JPEG_AUTO_TRACE(NULL)

#define JPEG_AUTO_TRACE_MSG(_arg) \
        _jpeg_trace.printf_msg(_arg)

#define JPEG_AUTO_TRACE_I32(_arg) \
        _jpeg_trace.printf_i32(#_arg, (mfxI32)_arg)

#define JPEG_AUTO_TRACE_U32(_arg) \
        _jpeg_trace.printf_u32(#_arg, (mfxU32)_arg)

#define JPEG_AUTO_TRACE_I64(_arg) \
        _jpeg_trace.printf_i64(#_arg, (mfxI64)_arg)

#define JPEG_AUTO_TRACE_F64(_arg) \
        _jpeg_trace.printf_f64(#_arg, (mfxF64)_arg)

#define JPEG_AUTO_TRACE_P(_arg) \
        _jpeg_trace.printf_p(#_arg, (void*)_arg)

#define JPEG_AUTO_TRACE_S(_arg) \
        _jpeg_trace.printf_s(#_arg, _arg)

mfxStatus jpeg_get_metadatabuffer_info(mfxU8* data, mfxU32 size, MetadataBuffer* pInfo);
void jpeg_copy_nv12(mfxFrameSurface1* dst, mfxFrameSurface1* src);

#endif // __JPEG_UTILS_H__
