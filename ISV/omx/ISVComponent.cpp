/*
 * Copyright (C) 2012 Intel Corporation.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <OMX_Component.h>
#include <media/hardware/HardwareAPI.h>
#include <OMX_IndexExt.h>
#ifndef TARGET_VPP_USE_GEN
#include <OMX_IntelColorFormatExt.h>
#endif
#include "ISVComponent.h"

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "isv-omxil"

namespace intel {
namespace isv {

using namespace android;

/**********************************************************************************
 * component methods & helpers
 */
#define GET_ISVOMX_COMPONENT(hComponent)                                    \
    ISVComponent *pComp = static_cast<ISVComponent*>                        \
        ((static_cast<OMX_COMPONENTTYPE*>(hComponent))->pComponentPrivate); \
    if (!pComp)                                                             \
        return OMX_ErrorBadParameter;

Vector<ISVComponent*> ISVComponent::g_isv_components;

ISVComponent::ISVComponent(
        OMX_PTR pAppData)
    :   mComponent(NULL),
        mpCallBacks(NULL),
        mCore(NULL),
        mpISVCallBacks(NULL),
        mNumISVBuffers(MIN_ISV_BUFFER_NUM),
        mNumDecoderBuffers(0),
        mNumDecoderBuffersBak(0),
        mUseAndroidNativeBufferIndex(0),
        mStoreMetaDataInBuffersIndex(0),
        mHackFormat(0),
        mWidth(0),
        mHeight(0),
        mFrameRate(0),
        mUseAndroidNativeBuffer(false),
        mUseAndroidNativeBuffer2(false),
        mISVEnabled(false),
        mVPPOn(false),
        mExtModeOn(false),
        mVPPFlushing(false),
        mOutputCropChanged(false),
        mInitialized(false),
        mFactory(NULL),
        mProcThreadObserver(NULL),
        mProcessor(NULL),
        mPolicy(NULL),
        mDpy(NULL),
        mIsVideoDecoder(false),
        mFiltersNum(0)
{
    memset(&mBaseComponent, 0, sizeof(OMX_COMPONENTTYPE));
    memset(&mFilterParam[0], 0, sizeof(ISVFilterParameter) * ISVFilterCount);
    memset(&mActiveDpyInfo, 0, sizeof(DisplayInfo));

    for (uint32_t i = 0; i < ISV_FEATURE_MAX; i++) {
        mPreFeatureStatus[i].feature = ISV_FEATURE_NONE;
        mPreFeatureStatus[i].status = UNKNOWN;
    }

    for (uint32_t i = 0; i < ISV_FEATURE_MAX; i++) {
        mFeatureStatus[i].feature = ISV_FEATURE_NONE;
        mFeatureStatus[i].status = UNKNOWN;
    }

    /* handle initialization */
    SetTypeHeader(&mBaseComponent, sizeof(mBaseComponent));
    mBaseComponent.pApplicationPrivate = pAppData;
    mBaseComponent.pComponentPrivate = static_cast<OMX_PTR>(this);

    /* connect handle's functions */
    mBaseComponent.GetComponentVersion = NULL;
    mBaseComponent.SendCommand = SendCommand;
    mBaseComponent.GetParameter = GetParameter;
    mBaseComponent.SetParameter = SetParameter;
    mBaseComponent.GetConfig = GetConfig;
    mBaseComponent.SetConfig = SetConfig;
    mBaseComponent.GetExtensionIndex = GetExtensionIndex;
    mBaseComponent.GetState = GetState;
    mBaseComponent.ComponentTunnelRequest = NULL;
    mBaseComponent.UseBuffer = UseBuffer;
    mBaseComponent.AllocateBuffer = AllocateBuffer;
    mBaseComponent.FreeBuffer = FreeBuffer;
    mBaseComponent.EmptyThisBuffer = EmptyThisBuffer;
    mBaseComponent.FillThisBuffer = FillThisBuffer;
    mBaseComponent.SetCallbacks = SetCallbacks;
    mBaseComponent.ComponentDeInit = NULL;
    mBaseComponent.UseEGLImage = NULL;
    mBaseComponent.ComponentRoleEnum = ComponentRoleEnum;
    g_isv_components.push_back(static_cast<ISVComponent*>(this));
}

ISVComponent::~ISVComponent()
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s", __func__);
    if (mExtModeOn) {
        if (mDpy != NULL && mDpy->mExtControl != NULL)
            mDpy->mExtControl->setVideoState(reinterpret_cast<int64_t>(this), false);
        mDpy = NULL;
    }

    if (mpISVCallBacks) {
        free(mpISVCallBacks);
        mpISVCallBacks = NULL;
    }

    for (OMX_U32 i = 0; i < g_isv_components.size(); i++) {
        if (g_isv_components.itemAt(i) == static_cast<ISVComponent*>(this)) {
            g_isv_components.removeAt(i);
        }
    }

    memset(&mBaseComponent, 0, sizeof(OMX_COMPONENTTYPE));
    mVPPOn = false;
}

status_t ISVComponent::init()
{
    status_t err = OK;
    if (mInitialized)
        return OK;

    if (mProcThreadObserver == NULL)
        mProcThreadObserver = new ISVProcThreadObserver(&mBaseComponent, mComponent, mpCallBacks);

    if (mFactory == NULL) {
        mFactory = new ISVFactory(mProcThreadObserver);
    }

    if (mProcessor == NULL) {
        mProcessor = mFactory->createVpProcessor();

        if (mProcessor != NULL) {
            mVPPOn = mProcessor->isVPPOn()|| mProcessor->isFRCOn();
            mPreFeatureStatus[ISV_VP].feature = ISV_VP;
            mPreFeatureStatus[ISV_VP].status = mProcessor->isVPPOn() ? ON : OFF;

            mPreFeatureStatus[ISV_FRC].feature = ISV_FRC;
            mPreFeatureStatus[ISV_FRC].status = mProcessor->isFRCOn() ? ON : OFF;
        }
    }

    Vector<DisplayInfo> displayInfos;

    if (mDpy == NULL) {
        bool hasHDMIAutoSet = false;
        bool hasMIPIAutoOff = false;
        bool connected = false;

        mDpy = new ISVDisplay();
        if (OK == mDpy->getDpyState(HWC_DISPLAY_VIRTUAL, &connected)) {
            mPreFeatureStatus[ISV_DISP_WIDI].feature = ISV_DISP_WIDI;
            mPreFeatureStatus[ISV_DISP_WIDI].status = connected ? ON : OFF;
        }

        for (int32_t i = HWC_DISPLAY_EXTERNAL; i >= 0; i--) {
            err = mDpy->getDpyState(i, &connected);
            if (err == OK && connected) {
                displayInfos.clear();
                err = mDpy->getActiveDpyInfo(i, &mActiveDpyInfo);
                if (err != OK) {
                    ALOGE("%s: failed to get active display info, dpyId %d", __func__, i);
                    return err;
                }
                // TODO: Disable ISVManager checking temporary,
                // because ISVDisplay has an issue for query all display configs.
                // Please add the patch "https://android.intel.com/#/c/394347/" before you enable it.
#if 0
                err = mDpy->getDpyInfo(i, &displayInfos);
                if (err != OK) {
                    ALOGE("%s: failed to get display infos, dpyId %d", __func__, i);
                    return err;
                }
                break;
#endif
            }
        }

        if (mDpy->mExtControl != NULL) {
            if (OK == mDpy->mExtControl->getDefaultExtModeState(&hasHDMIAutoSet, &hasMIPIAutoOff)) {
                mPreFeatureStatus[ISV_DISP_AUTOHDMI].feature = ISV_DISP_AUTOHDMI;
                mPreFeatureStatus[ISV_DISP_AUTOHDMI].status = hasHDMIAutoSet ? ON : OFF;

                mPreFeatureStatus[ISV_DISP_AUTOMIPI].feature = ISV_DISP_AUTOMIPI;
                mPreFeatureStatus[ISV_DISP_AUTOMIPI].status = hasMIPIAutoOff ? ON : OFF;
            }
        }
    }

    if (mPolicy == NULL) {
        mPolicy = mFactory->createPolicyManager();
        if (mPolicy != NULL && (OK == mPolicy->init()))
            mPolicy->setDisplayInfo(&mActiveDpyInfo, &displayInfos);
    }
    mInitialized = true;
    return err;
}

void ISVComponent::deinit()
{
    if (mProcessor != NULL) {
        mProcessor->deinit();
        delete mProcessor;
        mProcessor = NULL;
        ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: delete ISV processor ", __func__);
    }

    if (mPolicy != NULL) {
        mPolicy->deinit();
        delete mPolicy;
        mPolicy = NULL;
    }

    if (mFactory != NULL) {
        delete mFactory;
        mFactory = NULL;
    }

    if (mProcThreadObserver != NULL) {
        delete mProcThreadObserver;
        mProcThreadObserver = NULL;
    }
    mInitialized = false;
    mIsVideoDecoder = false;
    return;
}

OMX_CALLBACKTYPE* ISVComponent::getCallBacks(OMX_CALLBACKTYPE* pCallBacks)
{
    //reset component callback functions
    mpCallBacks = pCallBacks;
    if (mpISVCallBacks) {
        free(mpISVCallBacks);
        mpISVCallBacks = NULL;
    }

    mpISVCallBacks = (OMX_CALLBACKTYPE *)calloc(1, sizeof(OMX_CALLBACKTYPE));
    if (!mpISVCallBacks) {
        ALOGE("%s: failed to alloc isv callbacks", __func__);
        return NULL;
    }
    mpISVCallBacks->EventHandler = EventHandler;
    mpISVCallBacks->EmptyBufferDone = pCallBacks->EmptyBufferDone;
    mpISVCallBacks->FillBufferDone = FillBufferDone;
    return mpISVCallBacks;
}

OMX_ERRORTYPE ISVComponent::SendCommand(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_COMMANDTYPE Cmd,
    OMX_IN  OMX_U32 nParam1,
    OMX_IN  OMX_PTR pCmdData)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_SendCommand(Cmd, nParam1, pCmdData);
}

OMX_ERRORTYPE ISVComponent::ISV_SendCommand(
    OMX_IN  OMX_COMMANDTYPE Cmd,
    OMX_IN  OMX_U32 nParam1,
    OMX_IN  OMX_PTR pCmdData)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: Cmd index 0x%08x, nParam1 %d", __func__, Cmd, nParam1);

    if (mISVEnabled && mVPPOn) {
        if ((Cmd == OMX_CommandFlush && (nParam1 == kPortIndexOutput || nParam1 == OMX_ALL))
                || (Cmd == OMX_CommandStateSet && nParam1 == OMX_StateIdle)
                || (Cmd == OMX_CommandPortDisable && nParam1 == 1)) {
            ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: receive flush command, notify vpp thread to flush(Seek begin)", __func__);
            mVPPFlushing = true;
            mProcessor->notifyFlush();
        }
    }

    return OMX_SendCommand(mComponent, Cmd, nParam1, pCmdData);
}

OMX_ERRORTYPE ISVComponent::GetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR pComponentParameterStructure)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_GetParameter(nParamIndex, pComponentParameterStructure);
}

OMX_ERRORTYPE ISVComponent::ISV_GetParameter(
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR pComponentParameterStructure)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: nIndex 0x%08x", __func__, nParamIndex);

    OMX_ERRORTYPE err = OMX_GetParameter(mComponent, nParamIndex, pComponentParameterStructure);

    if (err == OMX_ErrorNone && mISVEnabled && mVPPOn) {
        OMX_PARAM_PORTDEFINITIONTYPE *def =
            static_cast<OMX_PARAM_PORTDEFINITIONTYPE*>(pComponentParameterStructure);

        if (nParamIndex == OMX_IndexParamPortDefinition
                && def->nPortIndex == kPortIndexOutput) {
            ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: orignal bufferCountActual %d, bufferCountMin %d",  __func__, def->nBufferCountActual, def->nBufferCountMin);
            def->nBufferCountActual += mNumISVBuffers;
            def->nBufferCountMin += mNumISVBuffers;
            OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def->format.video;
#ifndef TARGET_VPP_USE_GEN
            //FIXME: THIS IS A HACK!! Request NV12 buffer for YV12 format
            //because VSP only support NV12 output
            if (video_def->eColorFormat == OMX_INTEL_COLOR_FormatHalYV12) {
                mHackFormat = HAL_PIXEL_FORMAT_YV12;
                video_def->eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_INTEL_COLOR_FormatYUV420PackedSemiPlanar;
            }
#endif
        }
    }

    return err;
}

OMX_ERRORTYPE ISVComponent::SetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentParameterStructure)
{
    GET_ISVOMX_COMPONENT(hComponent);
 
    return pComp->ISV_SetParameter(nIndex, pComponentParameterStructure);
}

OMX_ERRORTYPE ISVComponent::ISV_SetParameter(
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentParameterStructure)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: nIndex 0x%08x", __func__, nIndex);

    if (nIndex == static_cast<OMX_INDEXTYPE>(OMX_IndexExtSetISVMode)) {
        ISV_MODE* def = static_cast<ISV_MODE*>(pComponentParameterStructure);

        if (*def == ISV_AUTO) {
            mISVEnabled = true;
            ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: mISVEnabled -->true", __func__);
#ifndef TARGET_VPP_USE_GEN
            if (mVPPOn) {
                uint32_t number = MIN_INPUT_NUM + MIN_OUTPUT_NUM;
                OMX_INDEXTYPE index;
                status_t error =
                    OMX_GetExtensionIndex(
                            mComponent,
                            "OMX.Intel.index.vppBufferNum",
                            &index);
                if (error == OK) {
                    error = OMX_SetParameter(mComponent, index, (OMX_PTR)&number);
                } else {
                    // ingore this error
                    ALOGW("Get vpp number index failed");
                }
            }
#endif
        } else if (*def == ISV_DISABLE)
            mISVEnabled = false;
        return OMX_ErrorNone;
    }

    OMX_ERRORTYPE err = OMX_SetParameter(mComponent, nIndex, pComponentParameterStructure);
    if (err != OMX_ErrorNone)
        return err;
    if (mISVEnabled) {
        init();
    }
    if (mISVEnabled && mProcessor != NULL) {
        if (nIndex == OMX_IndexParamPortDefinition) {
            OMX_PARAM_PORTDEFINITIONTYPE *def =
                static_cast<OMX_PARAM_PORTDEFINITIONTYPE*>(pComponentParameterStructure);

            if (def->nPortIndex == kPortIndexOutput) {
                //set the buffer count we should fill to decoder before feed buffer to VPP
                mNumDecoderBuffersBak = mNumDecoderBuffers = def->nBufferCountActual - MIN_OUTPUT_NUM - UNDEQUEUED_NUM;
                OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def->format.video;

                //FIXME: init itself here
                if (mWidth != video_def->nFrameWidth
                        || mHeight != video_def->nFrameHeight) {
                    if (mVPPOn && mProcessor != NULL) {
                        mProcessor->deinit();
                        if (OK != mProcessor->init(video_def->nFrameWidth, video_def->nFrameHeight)) {
                            ALOGE("%s: failed to init ISVProcessor, set VPPEnabled -->false", __func__);
                            mISVEnabled = false;
                            return OMX_ErrorUndefined;
                        }
                    }
                    //FIXME: we don't support scaling yet, so set src region equal to dst region
                    mWidth = video_def->nFrameWidth;
                    mHeight = video_def->nFrameHeight;
                }
                ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: def->nBufferCountActual %d, mNumDecoderBuffersBak %d", __func__,
                        def->nBufferCountActual, mNumDecoderBuffersBak);
                if (mVPPOn && mProcessor != NULL && OK != mProcessor->setBufferCount(def->nBufferCountActual)) {
                    ALOGE("%s: failed to set ISV buffer count, set VPPOn -->false", __func__);
                    mVPPOn = false;
                }
                ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: video frame width %d, height %d, fps %d",  __func__,
                        video_def->nFrameWidth, video_def->nFrameHeight, mFrameRate);
            }

            if (def->nPortIndex == kPortIndexInput) {
                OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def->format.video;
                mFrameRate = (uint32_t)(video_def->xFramerate/65536);
            }
            if (mWidth && mHeight && mPolicy) {
                bool isVPPOn = false, isFRCOn = false, isSFCOn = false, hasHDMIAutoSet = false, hasMIPIAutoOff = false;
                int32_t fps = 0;

                mPolicy->setClipInfo(mWidth, mHeight, mFrameRate);

                // set SFC status
                if (mIsAVCCodec
                        && OK == mPolicy->getSFCStatus(&isSFCOn)
                        && isSFCOn) {
                    mPreFeatureStatus[ISV_SFC].feature = ISV_SFC;
                    mPreFeatureStatus[ISV_SFC].status = ON;
                } else {
                    mPreFeatureStatus[ISV_SFC].feature = ISV_SFC;
                    mPreFeatureStatus[ISV_SFC].status = OFF;
                }
                // clear SFC status
                isSFCOn = false;

                // set all features' status
                mPolicy->setFeatureStatus(&mPreFeatureStatus[0]);
                //FIXME: until now, mPolicy is ready for querying feature status.
                // Update feature status here.
                mPolicy->getFeatureStatus(&mFeatureStatus[0]);
                mPolicy->getDisplayPolicySeting(fps);

                for (uint32_t i = 0; i < ISV_FEATURE_MAX; i++) {
                    switch (mFeatureStatus[i].feature) {
                        case ISV_VP:
                            isVPPOn = (mFeatureStatus[i].status == ON) ? true : false;
                            break;
                        case ISV_FRC:
                            isFRCOn = (mFeatureStatus[i].status == ON) ? true : false;
                            break;
                        case ISV_DISP_AUTOHDMI:
                            hasHDMIAutoSet = (mFeatureStatus[i].status == ON) ? true : false;
                            break;
                        case ISV_DISP_AUTOMIPI:
                            hasMIPIAutoOff = (mFeatureStatus[i].status == ON) ? true : false;
                            break;
                        case ISV_SFC:
                            isSFCOn = (mFeatureStatus[i].status == ON) ? true : false;
                            if (isSFCOn) {
                                OMX_INDEXTYPE index;
                                OMX_ERRORTYPE error = OMX_GetExtensionIndex(
                                        mComponent,
                                        "OMX.intel.android.index.enableSFC",
                                        &index);

                                if (error == OMX_ErrorNone) {
                                    OMX_ConfigureSFCParams params;
                                    SetTypeHeader(&params, sizeof(params));
                                    params.bEnableSFC = OMX_TRUE;
                                    params.nPortIndex = kPortIndexOutput;
                                    params.nOutputWidth = mActiveDpyInfo.w;
                                    params.nOutputHeight = mActiveDpyInfo.h;

                                    error = OMX_SetParameter(mComponent, index, &params);
                                    if (error != OMX_ErrorNone)
                                        ALOGW("%s: failed to set SFC parameters", __func__);
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }

                mExtModeOn = hasHDMIAutoSet || hasMIPIAutoOff;
                ALOGI("%s: isVPPOn %d, isFRCOn %d, mExtendMode %d, hasMIPIAutoOff %d, isSFCOn %d ",
                        __func__, isVPPOn, isFRCOn, mExtModeOn, hasMIPIAutoOff, isSFCOn);
                mVPPOn = isVPPOn || isFRCOn;
                if (mDpy != NULL && mDpy->mExtControl != NULL) {
                    mDpy->mExtControl->setRealExtModeState(hasHDMIAutoSet, hasMIPIAutoOff);
                    if (mExtModeOn) {
                        mDpy->mExtControl->setVideoState(reinterpret_cast<int64_t>(this), true);
                        mDpy->mExtControl->setVideoFPS(reinterpret_cast<int64_t>(this), fps);
                    }
                }
            }
        }

        if (mUseAndroidNativeBuffer
                && nIndex == static_cast<OMX_INDEXTYPE>(mUseAndroidNativeBufferIndex)) {
            UseAndroidNativeBufferParams *def =
                static_cast<UseAndroidNativeBufferParams*>(pComponentParameterStructure);

            if (OK != mProcessor->useBuffer(def->nativeBuffer,
                        (unsigned long)*(def->bufferHeader))) {
                    ALOGE("%s: failed to register graphic buffers to ISV, set mVPPOn -->false", __func__);
                    mVPPOn = false;
            }
        }

        if (nIndex == static_cast<OMX_INDEXTYPE>(mStoreMetaDataInBuffersIndex)) {
            StoreMetaDataInBuffersParams *params = static_cast<StoreMetaDataInBuffersParams*>(pComponentParameterStructure);
            if (params->nPortIndex == kPortIndexOutput) {
                bool bMetaDataMode = params->bStoreMetaData == OMX_TRUE;
                mProcessor->setMetaDataMode(bMetaDataMode);
            }
            ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: receive ISVStoreMetaDataInBuffers mISVWorkMode %d", __func__, (params->bStoreMetaData == OMX_TRUE));
        }
    } else {
        // enable video extmode for video playback if ISV is disabled
        if (nIndex == OMX_IndexParamPortDefinition && mIsVideoDecoder) {
            OMX_PARAM_PORTDEFINITIONTYPE *def =
                static_cast<OMX_PARAM_PORTDEFINITIONTYPE*>(pComponentParameterStructure);
            if (def->nPortIndex == kPortIndexInput) {
                OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def->format.video;
                mFrameRate = (uint32_t)(video_def->xFramerate/65536);
            }
            if (def->nPortIndex == kPortIndexOutput) {
                OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def->format.video;
                if (mWidth != video_def->nFrameWidth
                        || mHeight != video_def->nFrameHeight) {
                    mWidth = video_def->nFrameWidth;
                    mHeight = video_def->nFrameHeight;
                }
            }
            ALOGI("Enable Video ExtMode, %dx%d@%d", mWidth, mHeight, mFrameRate);
            if (!mExtModeOn && mWidth > 0 && mHeight > 0/* && mFrameRate > 0*/) {
                    if (mDpy == NULL)
                        mDpy = new ISVDisplay();
                    if (mDpy != NULL && mDpy->mExtControl != NULL) {
                        mExtModeOn = true;
                        mDpy->mExtControl->setRealExtModeState(true, true);
                        mDpy->mExtControl->setVideoState(reinterpret_cast<int64_t>(this), true);
                        mDpy->mExtControl->setVideoFPS(reinterpret_cast<int64_t>(this), mFrameRate);
                    }
            }
        }
    }
    return err;
}

OMX_ERRORTYPE ISVComponent::GetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_GetConfig(nIndex, pComponentConfigStructure);
}

OMX_ERRORTYPE ISVComponent::ISV_GetConfig(
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: nIndex 0x%08x", __func__, nIndex);

    OMX_ERRORTYPE err = OMX_GetConfig(mComponent, nIndex, pComponentConfigStructure);
    if (err == OMX_ErrorNone && mISVEnabled && mVPPOn && mProcessor != NULL) {
        if (nIndex == OMX_IndexConfigCommonOutputCrop) {
            OMX_CONFIG_RECTTYPE *rect = static_cast<OMX_CONFIG_RECTTYPE*>(pComponentConfigStructure);
            if (rect->nPortIndex == kPortIndexOutput &&
                    rect->nWidth < mWidth &&
                    rect->nHeight < mHeight) {
                mProcessor->setBuffersFlag(IISVBuffer::ISV_BUFFERFLAG_DIRTY);
                ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: mark all buffers need clear", __func__);
            }
        }
    }
    return err;
}

OMX_ERRORTYPE ISVComponent::SetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentConfigStructure)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_SetConfig(nIndex, pComponentConfigStructure);
}

OMX_ERRORTYPE ISVComponent::ISV_SetConfig(
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentConfigStructure)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: nIndex 0x%08x", __func__, nIndex);
    if (nIndex == static_cast<OMX_INDEXTYPE>(OMX_IndexExtOtherStartUnused + 1/*OMX_IndexConfigAutoFramerateConversion*/)) {
        OMX_CONFIG_BOOLEANTYPE *config = static_cast<OMX_CONFIG_BOOLEANTYPE*>(pComponentConfigStructure);
        if (config->bEnabled) {
            mISVEnabled = true;
            ALOGI("%s: mISVEnabled=true", __func__);
        } else {
            mISVEnabled = false;
            ALOGI("%s: mISVEnabled=false", __func__);
        }
        return OMX_ErrorNone;
    }
    return OMX_SetConfig(mComponent, nIndex, pComponentConfigStructure);
}

OMX_ERRORTYPE ISVComponent::GetExtensionIndex(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_STRING cParameterName,
    OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_GetExtensionIndex(cParameterName, pIndexType);
}

OMX_ERRORTYPE ISVComponent::ISV_GetExtensionIndex(
    OMX_IN  OMX_STRING cParameterName,
    OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: cParameterName %s", __func__, cParameterName);
    if(!strncmp(cParameterName, "OMX.intel.index.SetISVMode", strlen(cParameterName))) {
        *pIndexType = static_cast<OMX_INDEXTYPE>(OMX_IndexExtSetISVMode);
        return OMX_ErrorNone;
    }

    OMX_ERRORTYPE err = OMX_GetExtensionIndex(mComponent, cParameterName, pIndexType);

    if(err == OMX_ErrorNone &&
            !strncmp(cParameterName, "OMX.google.android.index.useAndroidNativeBuffer2", strlen(cParameterName)))
        mUseAndroidNativeBuffer2 = true;

    if(err == OMX_ErrorNone &&
            !strncmp(cParameterName, "OMX.google.android.index.useAndroidNativeBuffer", strlen(cParameterName))) {
        mUseAndroidNativeBuffer = true;
        mUseAndroidNativeBufferIndex = static_cast<uint32_t>(*pIndexType);
    }

    if(err == OMX_ErrorNone &&
            !strncmp(cParameterName, "OMX.google.android.index.storeMetaDataInBuffers", strlen(cParameterName))) {
        mStoreMetaDataInBuffersIndex = static_cast<uint32_t>(*pIndexType);
        ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: storeMetaDataInBuffersIndex 0x%08x return %d", __func__, mStoreMetaDataInBuffersIndex, err);
    }
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: cParameterName %s, nIndex 0x%08x", __func__,
            cParameterName, *pIndexType);
    return err;
}

OMX_ERRORTYPE ISVComponent::GetState(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_STATETYPE* pState)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_GetState(pState);
}

OMX_ERRORTYPE ISVComponent::ISV_GetState(
    OMX_OUT OMX_STATETYPE* pState)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s", __func__);

    return OMX_GetState(mComponent, pState);
}

OMX_ERRORTYPE ISVComponent::UseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8 *pBuffer)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_UseBuffer(ppBufferHdr, nPortIndex,
                                 pAppPrivate, nSizeBytes, pBuffer);
}

OMX_ERRORTYPE ISVComponent::ISV_UseBuffer(
    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8 *pBuffer)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s", __func__);

    OMX_ERRORTYPE err = OMX_UseBuffer(mComponent, ppBufferHdr, nPortIndex,
            pAppPrivate, nSizeBytes, pBuffer);
#ifndef USE_IVP
    if(err == OMX_ErrorNone
            && mISVEnabled
            && mVPPOn
            && nPortIndex == kPortIndexOutput
            && mProcessor != NULL
            /*&& mUseAndroidNativeBuffer2*/) {
        if (OK != mProcessor->useBuffer(reinterpret_cast<unsigned long>(pBuffer), (unsigned long)(*ppBufferHdr))) {
            ALOGE("%s: failed to register graphic buffers to ISV, set mVPPOn -->false", __func__);
            mVPPOn = false;
        }
    }
#endif
    return err;
}

OMX_ERRORTYPE ISVComponent::AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_AllocateBuffer(ppBuffer, nPortIndex,
                                      pAppPrivate, nSizeBytes);
}

OMX_ERRORTYPE ISVComponent::ISV_AllocateBuffer(
    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s", __func__);

    return OMX_AllocateBuffer(mComponent, ppBuffer, nPortIndex,
                                      pAppPrivate, nSizeBytes);
}

OMX_ERRORTYPE ISVComponent::FreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE *pBuffer)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_FreeBuffer(nPortIndex, pBuffer);
}

OMX_ERRORTYPE ISVComponent::ISV_FreeBuffer(
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE *pBuffer)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: pBuffer %p", __func__, pBuffer);

    if(mISVEnabled
            && mVPPOn
            && nPortIndex == kPortIndexOutput
            && mProcessor != NULL) {
        if (OK != mProcessor->freeBuffer(reinterpret_cast<unsigned long>(pBuffer->pBuffer)))
            ALOGW("%s: pBuffer %p has not been registered into ISV", __func__, pBuffer);
    }
    return OMX_FreeBuffer(mComponent, nPortIndex, pBuffer);
}

OMX_ERRORTYPE ISVComponent::EmptyThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_EmptyThisBuffer(pBuffer);
}

OMX_ERRORTYPE ISVComponent::ISV_EmptyThisBuffer(
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: pBuffer %p", __func__, pBuffer);

    return OMX_EmptyThisBuffer(mComponent, pBuffer);
}

OMX_ERRORTYPE ISVComponent::FillThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE *pBuffer)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: API entry.", __func__);
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_FillThisBuffer(pBuffer);
}

OMX_ERRORTYPE ISVComponent::ISV_FillThisBuffer(
    OMX_IN  OMX_BUFFERHEADERTYPE *pBuffer)
{
    if(!mISVEnabled || !mVPPOn)
        return OMX_FillThisBuffer(mComponent, pBuffer);

    if (mProcessor != NULL) {
        IISVBuffer* outputBuffer = mProcessor->mapBuffer(reinterpret_cast<unsigned long>(pBuffer->pBuffer));
        if (outputBuffer == NULL) {
            ALOGE("%s: failed to map ISVBuffer, set mVPPOn -->false", __func__);
            mVPPOn = false;
            return OMX_FillThisBuffer(mComponent, pBuffer);
        }

        if (OK != outputBuffer->initBufferInfo(mHackFormat)) {
            ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: outputBuffer %p failed to initBufferInfo", __func__, outputBuffer);
            mVPPOn = false;
            return OMX_FillThisBuffer(mComponent, pBuffer);
        }

        if (mNumDecoderBuffers > 0) {
            mNumDecoderBuffers--;
            ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: fill pBuffer %p to the decoder, decoder still need extra %d buffers", __func__,
                    pBuffer, mNumDecoderBuffers);

            outputBuffer->clearIfNeed();
            return OMX_FillThisBuffer(mComponent, pBuffer);
        }
        mProcessor->addOutput(outputBuffer);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE ISVComponent::FillBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: API entry. ISV component num %d, component handle %p on index 0", __func__,
            g_isv_components.size(),
            g_isv_components.itemAt(0));
    for (OMX_U32 i = 0; i < g_isv_components.size(); i++) {
        if (static_cast<OMX_HANDLETYPE>(g_isv_components.itemAt(i)->mComponent) == hComponent)
            return g_isv_components.itemAt(i)->ISV_FillBufferDone(hComponent, pAppData, pBuffer);
    }
    return OMX_ErrorUndefined;
}

OMX_ERRORTYPE ISVComponent::ISV_FillBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    ISV_UNUSED(hComponent);
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: %p <== buffer_handle_t %p. mISVEnabled %d, mVPPOn %d", __func__,
            pBuffer, pBuffer->pBuffer, mISVEnabled, mVPPOn);
    if (!mpCallBacks) {
        ALOGE("%s: no call back functions were registered.", __func__);
        return OMX_ErrorUndefined;
    }

    if(!mISVEnabled || !mVPPOn || mVPPFlushing || pBuffer->nFilledLen == 0) {
        ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: FillBufferDone pBuffer %p, timeStamp %.2f ms", __func__, pBuffer, pBuffer->nTimeStamp/1E3);
        return mpCallBacks->FillBufferDone(&mBaseComponent, pAppData, pBuffer);
    }

    if (mProcessor != NULL) {
        IISVBuffer* inputBuffer = mProcessor->mapBuffer(reinterpret_cast<unsigned long>(pBuffer->pBuffer));
        if (inputBuffer == NULL) {
            ALOGE("%s: failed to map ISVBuffer, set mVPPOn -->false", __func__);
            mVPPOn = false;
            return mpCallBacks->FillBufferDone(&mBaseComponent, pAppData, pBuffer);
        }

        inputBuffer->setTimeStamp(pBuffer->nTimeStamp);
        if (pBuffer->nFlags & OMX_BUFFERFLAG_EOS)
            inputBuffer->setFlag(IISVBuffer::ISV_BUFFERFLAG_EOS);

        if (mOutputCropChanged) {
            inputBuffer->setFlag(IISVBuffer::ISV_BUFFERFLAG_CROP_CHANGED);
            mOutputCropChanged = false;
        }

        status_t ret = OK;
        if (mPolicy != NULL) {
            ret = mPolicy->getVpPolicySetting(mFilterParam, mFiltersNum);
            if (ret != OK) {
                ALOGE("%s: failed to getVpPolicySetting, set mVPPOn -->false", __func__);
                mVPPOn = false;
                return mpCallBacks->FillBufferDone(&mBaseComponent, pAppData, pBuffer);
            }
        }

        ret = mProcessor->configFilters(mFilterParam, mFiltersNum, pBuffer->nTimeStamp, pBuffer->nFlags);
        if (ret == NOT_ENOUGH_DATA) {
            return OMX_FillThisBuffer(mComponent, pBuffer);
        } else if (ret == UNKNOWN_ERROR) {
            ALOGI("%s: no filter configure, set mVPPOn -->false", __func__);
            mVPPOn = false;
            return mpCallBacks->FillBufferDone(&mBaseComponent, pAppData, pBuffer);
        } else if (ret == OK)
            mProcessor->addInput(inputBuffer);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE ISVComponent::EventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: API entry. ISV component num %d, component handle %p on index 0", __func__,
            g_isv_components.size(),
            g_isv_components.itemAt(0));
    for (OMX_U32 i = 0; i < g_isv_components.size(); i++) {
        if (static_cast<OMX_HANDLETYPE>(g_isv_components.itemAt(i)->mComponent) == hComponent)
            return g_isv_components.itemAt(i)->ISV_EventHandler(hComponent, pAppData, eEvent, nData1, nData2, pEventData);
    }
    return OMX_ErrorUndefined;
}

OMX_ERRORTYPE ISVComponent::ISV_EventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
{
    ISV_UNUSED(hComponent);
    if (!mpCallBacks) {
        ALOGE("%s: no call back functions were registered.", __func__);
        return OMX_ErrorUndefined;
    }

    if(!mISVEnabled || !mVPPOn)
        return mpCallBacks->EventHandler(&mBaseComponent, pAppData, eEvent, nData1, nData2, pEventData);

    switch (eEvent) {
        case OMX_EventCmdComplete:
        {
            ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: OMX_EventCmdComplete Cmd type 0x%08x, data2 %d", __func__,
                    nData1, nData2);
            if (((OMX_COMMANDTYPE)nData1 == OMX_CommandFlush && (nData2 == kPortIndexOutput || nData2 == OMX_ALL))
                || ((OMX_COMMANDTYPE)nData1 == OMX_CommandStateSet && nData2 == OMX_StateIdle)
                || ((OMX_COMMANDTYPE)nData1 == OMX_CommandPortDisable && nData2 == 1)) {
                mProcessor->waitFlushFinished();
                mVPPFlushing = false;
                mNumDecoderBuffers = mNumDecoderBuffersBak;
            }
            break;
        }

        case OMX_EventError:
        {
            //do we need do anything here?
            ALOGE("%s: ERROR(0x%08x, %d)", __func__, nData1, nData2);
            //mProcessor->flush();
            break;
        }

        case OMX_EventPortSettingsChanged:
        {
            if (nData1 == kPortIndexOutput && nData2 == OMX_IndexConfigCommonOutputCrop) {
                ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: output crop changed", __func__);
                mOutputCropChanged = true;
                return OMX_ErrorNone;
            }
            break;
        }

        default:
        {
            ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: EVENT(%d, %ld, %ld)", __func__, eEvent, nData1, nData2);
            break;
        }
    }
    return mpCallBacks->EventHandler(&mBaseComponent, pAppData, eEvent, nData1, nData2, pEventData);
}

OMX_ERRORTYPE ISVComponent::SetCallbacks(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
    OMX_IN  OMX_PTR pAppData)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_SetCallbacks(pCallbacks, pAppData);
}

OMX_ERRORTYPE ISVComponent::ISV_SetCallbacks(
    OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
    OMX_IN  OMX_PTR pAppData)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s", __func__);

    if (mISVEnabled && mVPPOn) {
        if (mpISVCallBacks == NULL) {
            mpISVCallBacks = (OMX_CALLBACKTYPE *)calloc(1, sizeof(OMX_CALLBACKTYPE));
            if (!mpISVCallBacks) {
                ALOGE("%s: failed to alloc isv callbacks", __func__);
                return OMX_ErrorUndefined;
            }
        }
        mpISVCallBacks->EventHandler = EventHandler;
        mpISVCallBacks->EmptyBufferDone = pCallbacks->EmptyBufferDone;
        mpISVCallBacks->FillBufferDone = FillBufferDone;
        mpCallBacks = pCallbacks;
        return mComponent->SetCallbacks(mComponent, mpISVCallBacks, pAppData);
    }
    return mComponent->SetCallbacks(mComponent, pCallbacks, pAppData);
}

OMX_ERRORTYPE ISVComponent::ComponentRoleEnum(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_U8 *cRole,
    OMX_IN OMX_U32 nIndex)
{
    GET_ISVOMX_COMPONENT(hComponent);

    return pComp->ISV_ComponentRoleEnum(cRole, nIndex);
}

OMX_ERRORTYPE ISVComponent::ISV_ComponentRoleEnum(
    OMX_OUT OMX_U8 *cRole,
    OMX_IN OMX_U32 nIndex)
{
    ALOGD_IF(ISV_COMPONENT_DEBUG, "%s", __func__);

    return mComponent->ComponentRoleEnum(mComponent, cRole, nIndex);
}


void ISVComponent::SetTypeHeader(OMX_PTR type, OMX_U32 size)
{
    OMX_U32 *nsize;
    OMX_VERSIONTYPE *nversion;

    if (!type)
        return;

    nsize = (OMX_U32 *)type;
    nversion = (OMX_VERSIONTYPE *)((OMX_U8 *)type + sizeof(OMX_U32));

    *nsize = size;
    nversion->nVersion = OMX_SPEC_VERSION;
}


ISVProcThreadObserver::ISVProcThreadObserver(
        OMX_COMPONENTTYPE *pBaseComponent,
        OMX_COMPONENTTYPE *pComponent,
        OMX_CALLBACKTYPE *pCallBacks)
    :   mBaseComponent(pBaseComponent),
        mComponent(pComponent),
        mpCallBacks(pCallBacks)
{
    ALOGV("VPPProcThreadObserver!");
}

ISVProcThreadObserver::~ISVProcThreadObserver()
{
    ALOGV("~VPPProcThreadObserver!");
    mBaseComponent = NULL;
    mComponent = NULL;
    mpCallBacks = NULL;
}

status_t ISVProcThreadObserver::releaseBuffer(PORT_INDEX index, IISVBuffer* pISVBuffer, bool bFLush)
{
    if (!mBaseComponent || !mComponent || !mpCallBacks)
        return UNKNOWN_ERROR;

    unsigned long omxHandle = pISVBuffer->getOMXHandle();
    OMX_BUFFERHEADERTYPE* pBuffer = reinterpret_cast<OMX_BUFFERHEADERTYPE*>(omxHandle);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if (bFLush) {
        pBuffer->nFilledLen = 0;
        pBuffer->nOffset = 0;
        err = mpCallBacks->FillBufferDone(&mBaseComponent, mBaseComponent->pApplicationPrivate, pBuffer);
        ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: flush pBuffer %p", __func__, pBuffer);
        return (err == OMX_ErrorNone) ? OK : UNKNOWN_ERROR;
    }

    if (index == kPortIndexInput) {
        pBuffer->nFilledLen = 0;
        pBuffer->nOffset = 0;
        pBuffer->nFlags = 0;

        err = OMX_FillThisBuffer(mComponent, pBuffer);
        ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: FillBuffer pBuffer %p", __func__, pBuffer);
    } else {
        pBuffer->nTimeStamp = pISVBuffer->getTimeStamp();
        err = mpCallBacks->FillBufferDone(&mBaseComponent, mBaseComponent->pApplicationPrivate, pBuffer);
        ALOGD_IF(ISV_COMPONENT_DEBUG, "%s: FillBufferDone pBuffer %p, timeStamp %.2f ms", __func__, pBuffer, pBuffer->nTimeStamp/1E3);
    }

    return (err == OMX_ErrorNone) ? OK : UNKNOWN_ERROR;
}

status_t ISVProcThreadObserver::reportOutputCrop()
{
    if (!mBaseComponent || !mComponent || !mpCallBacks)
        return UNKNOWN_ERROR;

    OMX_ERRORTYPE err = OMX_ErrorNone;
    err = mpCallBacks->EventHandler(&mBaseComponent, mBaseComponent->pApplicationPrivate,
                                    OMX_EventPortSettingsChanged,
                                    kPortIndexOutput, OMX_IndexConfigCommonOutputCrop, NULL);
    return (err == OMX_ErrorNone) ? OK : UNKNOWN_ERROR;
}

} // namespace intel
} // namespace isv
