/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2014 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#include "JPEGUtils.h"

JpegMutex::JpegMutex(void)
{
    int res = pthread_mutex_init(&m_mutex, NULL);
    JPEG_THROW_IF(res, std::bad_alloc());
}

JpegMutex::~JpegMutex(void)
{
    pthread_mutex_destroy(&m_mutex);
}

mfxStatus JpegMutex::Lock(void)
{
    return (pthread_mutex_lock(&m_mutex))? MFX_ERR_UNKNOWN: MFX_ERR_NONE;
}

mfxStatus JpegMutex::Unlock(void)
{
    return (pthread_mutex_unlock(&m_mutex))? MFX_ERR_UNKNOWN: MFX_ERR_NONE;
}

bool JpegMutex::Try(void)
{
    return (pthread_mutex_trylock(&m_mutex))? false: true;
}


JpegAutoLock::JpegAutoLock(JpegMutex& mutex):
    m_rMutex(mutex),
    m_bLocked(false)
{
    JPEG_THROW_IF((MFX_ERR_NONE != Lock()), std::bad_alloc());
}

/*------------------------------------------------------------------------------*/

JpegAutoLock::~JpegAutoLock(void)
{
    Unlock();
}

/*------------------------------------------------------------------------------*/

mfxStatus JpegAutoLock::Lock(void)
{
    mfxStatus sts = MFX_ERR_NONE;
    if (!m_bLocked)
    {
        if (!m_rMutex.Try())
        {
            // add time measurement here to estimate how long you sleep on mutex
            sts = m_rMutex.Lock();
        }
        m_bLocked = true;
    }
    return sts;
}

/*------------------------------------------------------------------------------*/

mfxStatus JpegAutoLock::Unlock(void)
{
    mfxStatus sts = MFX_ERR_NONE;
    if (m_bLocked)
    {
        sts = m_rMutex.Unlock();
        m_bLocked = false;
    }
    return sts;
}



jpeg_trace::jpeg_trace(const char* _modulename, const char* _function, const char* _taskname)
{
    modulename = _modulename;
    function = _function;
    taskname = _taskname;
#if !defined(ANDROID) && defined(jpeg_STDOUT)
    if (g_dbg_file)
    {
        if (taskname)
            fprintf(g_dbg_file, "%s: %s: %s: +\n", modulename, function, taskname);
        else
            fprintf(g_dbg_file, "%s: %s: +\n", modulename, function);
        fflush(g_dbg_file);
    }
#else
    if (taskname)
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s: +", modulename, function, taskname);
    else
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: +", modulename, function);
#endif
}

/*------------------------------------------------------------------------------*/

jpeg_trace::~jpeg_trace(void)
{
#if !defined(ANDROID) && defined(jpeg_STDOUT)
    if (g_dbg_file)
    {
        if (taskname)
            fprintf(g_dbg_file, "%s: %s: %s: -\n", modulename, function, taskname);
        else
            fprintf(g_dbg_file, "%s: %s: -\n", modulename, function);
        fflush(g_dbg_file);
    }
#else
    if (taskname)
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s: -", modulename, function, taskname);
    else
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: -", modulename, function);
#endif
}

/*------------------------------------------------------------------------------*/

void jpeg_trace::printf_i32(const char* name, mfxI32 value)
{
#if !defined(ANDROID) && defined(jpeg_STDOUT)
    if (g_dbg_file)
    {
        if (taskname)
            fprintf(g_dbg_file, "%s: %s: %s: %s = %d\n", modulename, function, taskname, name, value);
        else
            fprintf(g_dbg_file, "%s: %s: %s = %d\n", modulename, function, name, value);
        fflush(g_dbg_file);
    }
#else
    if (taskname)
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s: %s = %d", modulename, function, taskname, name, value);
    else
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s = %d", modulename, function, name, value);
#endif
}

/*------------------------------------------------------------------------------*/

void jpeg_trace::printf_u32(const char* name, mfxU32 value)
{
#if !defined(ANDROID) && defined(jpeg_STDOUT)
    if (g_dbg_file)
    {
        if (taskname)
            fprintf(g_dbg_file, "%s: %s: %s: %s = 0x%x\n", modulename, function, taskname, name, value);
        else
            fprintf(g_dbg_file, "%s: %s: %s = 0x%x\n", modulename, function, name, value);
        fflush(g_dbg_file);
    }
#else
    if (taskname)
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s: %s = 0x%x", modulename, function, taskname, name, value);
    else
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s = 0x%x", modulename, function, name, value);

#endif
}

/*------------------------------------------------------------------------------*/

void jpeg_trace::printf_i64(const char* name, mfxI64 value)
{
#if !defined(ANDROID) && defined(jpeg_STDOUT)
    if (g_dbg_file)
    {
        if (taskname)
            fprintf(g_dbg_file, "%s: %s: %s: %s = %ld\n", modulename, function, taskname, name, value);
        else
            fprintf(g_dbg_file, "%s: %s: %s = %ld\n", modulename, function, name, value);
        fflush(g_dbg_file);
    }
#else
    if (taskname)
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s: %s = %ld", modulename, function, taskname, name, value);
    else
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s = %ld", modulename, function, name, value);

#endif
}

/*------------------------------------------------------------------------------*/

void jpeg_trace::printf_f64(const char* name, mfxF64 value)
{
#if !defined(ANDROID) && defined(jpeg_STDOUT)
    if (g_dbg_file)
    {
        if (taskname)
            fprintf(g_dbg_file, "%s: %s: %s: %s = %f\n", modulename, function, taskname, name, value);
        else
            fprintf(g_dbg_file, "%s: %s: %s = %f\n", modulename, function, name, value);
        fflush(g_dbg_file);
    }
#else
    if (taskname)
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s: %s = %f", modulename, function, taskname, name, value);
    else
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s = %f", modulename, function, name, value);

#endif
}

/*------------------------------------------------------------------------------*/

void jpeg_trace::printf_p(const char* name, void* value)
{
#if !defined(ANDROID) && defined(jpeg_STDOUT)
    if (g_dbg_file)
    {
        if (taskname)
            fprintf(g_dbg_file, "%s: %s: %s: %s = %p\n", modulename, function, taskname, name, value);
        else
            fprintf(g_dbg_file, "%s: %s: %s = %p\n", modulename, function, name, value);
        fflush(g_dbg_file);
    }
#else
    if (taskname)
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s: %s = %p", modulename, function, taskname, name, value);
    else
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s = %p", modulename, function, name, value);

#endif
}

/*------------------------------------------------------------------------------*/

void jpeg_trace::printf_s(const char* name, const char* value)
{
#if !defined(ANDROID) && defined(jpeg_STDOUT)
    if (g_dbg_file)
    {
        if (taskname)
            fprintf(g_dbg_file, "%s: %s: %s: %s = '%s'\n", modulename, function, taskname, name, value);
        else
            fprintf(g_dbg_file, "%s: %s: %s = '%s'\n", modulename, function, name, value);
        fflush(g_dbg_file);
    }
#else
    if (taskname)
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s: %s = %s", modulename, function, taskname, name, value);
    else
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s = %s", modulename, function, name, value);

#endif
}

/*------------------------------------------------------------------------------*/

void jpeg_trace::printf_msg(const char* msg)
{
#if !defined(ANDROID) && defined(jpeg_STDOUT)
    if (g_dbg_file)
    {
        if (taskname)
            fprintf(g_dbg_file, "%s: %s: %s: %s\n", modulename, function, taskname, msg);
        else
            fprintf(g_dbg_file, "%s: %s: %s\n", modulename, function, msg);
        fflush(g_dbg_file);
    }
#else
    if (taskname)
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s: %s", modulename, function, taskname, msg);
    else
        __android_log_print(JPEG_LOG_LEVEL, JPEG_LOG_TAG, "%s: %s: %s", modulename, function, msg);

#endif
}


mfxStatus jpeg_get_metadatabuffer_info(mfxU8* data, mfxU32 size, MetadataBuffer* pInfo)
{
    JPEG_AUTO_TRACE_FUNC();
    mfxStatus mfx_res = MFX_ERR_NONE;

    JPEG_AUTO_TRACE_I32(size);

    if (!data || !size || !pInfo)
        mfx_res = MFX_ERR_NULL_PTR;

    if (mfx_res == MFX_ERR_NONE)
    {
        memcpy(&pInfo->type, data, sizeof(pInfo->type));
        data += sizeof(pInfo->type);

        memcpy(&pInfo->handle, data, sizeof(pInfo->handle));
        data += sizeof(pInfo->handle);

        mfxU32 extraSize = size - sizeof(pInfo->type) - sizeof(pInfo->handle);

        switch (pInfo->type)
        {
            case MfxMetadataBufferTypeCameraSource:
            case MfxMetadataBufferTypeEncoder:
            case MfxMetadataBufferTypeUser:
            {
                if ((extraSize > 0) && (extraSize < sizeof(BufferInfo)))
                {
                    mfx_res = MFX_ERR_UNDEFINED_BEHAVIOR;
                    break;
                }

                if (extraSize > sizeof(BufferInfo))
                {
                    if ((extraSize - sizeof(BufferInfo)) % sizeof(pInfo->handle) != 0)
                    {
                        mfx_res = MFX_ERR_UNDEFINED_BEHAVIOR;
                        break;
                    }
                }

                if (extraSize > 0)
                {
                    memcpy(&pInfo->info, data, sizeof(BufferInfo));
                }

                break;
            }
            case MfxMetadataBufferTypeGrallocSource:
                if (extraSize > 0)
                {
                    mfx_res = MFX_ERR_UNDEFINED_BEHAVIOR;
                }

                break;
            default:
                mfx_res = MFX_ERR_UNSUPPORTED;
        }
    }

    JPEG_AUTO_TRACE_I32(mfx_res);
    return mfx_res;
}

void jpeg_copy_nv12(mfxFrameSurface1* dst, mfxFrameSurface1* src)
{
    mfxU32 i;

    for (i = 0; i < src->Info.Height/2; ++i)
    {
        // copying Y
        memcpy(dst->Data.Y + i * dst->Data.Pitch,
               src->Data.Y + i * src->Data.Pitch,
               src->Info.Width);
        // copying UV
        memcpy(dst->Data.UV + i * dst->Data.Pitch,
               src->Data.UV + i * src->Data.Pitch,
               src->Info.Width);
    }
    for (i = src->Info.Height/2; i < src->Info.Height; ++i)
    {
        // copying Y (remained data)
        memcpy(dst->Data.Y + i * dst->Data.Pitch,
               src->Data.Y + i * src->Data.Pitch,
               src->Info.Width);
    }
}

