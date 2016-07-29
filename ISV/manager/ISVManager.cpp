/*
 * Copyright (C) 2014 Intel Corporation.  All rights reserved.
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
#include "manager.h"
#include <memory.h>
#include "ISVProfile.h"
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "ISVPolicyManager"

namespace intel {
namespace isv {

using namespace android;


#define VP_FILTER "VP.FILTER"
#define VP_FRC "VP.FRC"
#define DISP_EXTMODE "DISP.EXTMODE"
#define DISP_AUTOHDMI "DISP.AUTOHDMI"
#define WIDI "DISP.WIDI"
#define SFC "SFC"

#define BITS_PER_FEATURE (2)

#define VP_FEATURE_MASK  (((1 << BITS_PER_FEATURE)-1) << (ISV_VP * BITS_PER_FEATURE))
#define FRC_FEATURE_MASK  (((1 << BITS_PER_FEATURE)-1) << (ISV_FRC * BITS_PER_FEATURE))
#define AUTOHDMI_FEATURE_MASK  (((1 << BITS_PER_FEATURE)-1) << (ISV_DISP_AUTOHDMI * BITS_PER_FEATURE))
#define EXTMODE_FEATURE_MASK  (((1 << BITS_PER_FEATURE)-1) << (ISV_DISP_AUTOMIPI * BITS_PER_FEATURE))
#define WIDI_FEATURE_MASK  (((1 << BITS_PER_FEATURE)-1) << (ISV_WIDI * BITS_PER_FEATURE))
#define SFC_FEATURE_MASK  (((1 << BITS_PER_FEATURE)-1) << (ISV_SFC * BITS_PER_FEATURE))

#define SET_FLAG(status, flag) status = (MANAGER_STATUS)((status) | (flag))
#define CLEAR_FLAG(status, flag) status = (MANAGER_STATUS)((status) & (~flag))
#define TEST_FLAGSET(status, flag) (((status) & (flag)) == flag)
#define TEST_FLAG_NOT_SET(status, flag) (((status) & (flag)) != flag)

ISVPolicyManager::ISVPolicyManager()
{
    mStatus = INITIAL;

    mProfile = NULL;
    memset(&mWorkStatus, 0, sizeof(ISVWorkingStatus));
    memset(&mDecision, 0, sizeof(ISVPolicyDecision));

    mNumProfileScene = 0;
    mPredefinedScene = NULL;

    mProfileRule = NULL;
    mNumProfileRule = 0;

    mProfileFRCRule = NULL;
    mNumProfileFRCRule = 0;
}

status_t ISVPolicyManager::init()
{
    status_t retVal = OK;
    uint32_t num = 0;
    status_t status = OK;
    ISVPolicyScene *profileScene = NULL;

    mProfile = ISVProfile::GetInstance();
    if (mProfile == NULL) {
        retVal = BAD_VALUE;
        goto ERROR_EXIT;
    }

    status = mProfile->getISVSetting(ISV_POLICY_SCENE_NUM, &num);
    if (status != OK) {
        retVal = status;
        goto ERROR_EXIT;
    }
    //allocate memory for Scenes Rule and FRCRule
    //manager store profile predefined scene
    profileScene = new ISVPolicyScene[num];
    if (profileScene == NULL) {
        retVal = NO_MEMORY;
        goto ERROR_EXIT;
    }

    memset(profileScene, 0, sizeof(ISVPolicyScene) * num);
    mNumProfileScene = num;
    mPredefinedScene = new ISVScene[num];
    if (mPredefinedScene == NULL) {
        retVal = NO_MEMORY;
        goto ERROR_EXIT;
    }
    memset(mPredefinedScene, 0, sizeof(ISVScene) * num);
    //Initialized scenes here
    for (uint32_t i = 0; i < num; i++) {
        mPredefinedScene[i].init();
    }
    status = mProfile->getPolicyScenes(profileScene, num);
    if (status != OK) {
        retVal = status;
        goto ERROR_EXIT;
    }
    //Parsed profile scene data to fill manager scener structure
    for (uint32_t i = 0; i < mNumProfileScene; i++) {
        mPredefinedScene[i].parseProfileScene(&profileScene[i]);
        uint32_t feature = mPredefinedScene[i].getFeature();
        ALOGV("Scene default before feature is 0x%X ", feature);
        parseCondition((char *)(&(profileScene[i].condition)), &feature);
        ALOGV("Scene parsed feature is 0x%X ", feature);
        mPredefinedScene[i].setFeature(feature);
    }
        ALOGV("Scene parsed end \n\n ");
    if (profileScene != NULL) {
        delete [] profileScene;
        profileScene = NULL;
    }

    status = mProfile->getISVSetting(ISV_POLICY_RULE_NUM, &num);
    if (status != OK) {
        retVal = status;
        goto ERROR_EXIT;
    }
    // malloc memory for Scenes Rule and FRCRule
    mProfileRule = new ISVPolicyRule[num];
    if (mProfileRule == NULL) {
        retVal = NO_MEMORY;
        goto ERROR_EXIT;
    }
    memset(mProfileRule, 0, sizeof(ISVPolicyRule) * num);
    mNumProfileRule = num;

    status = mProfile->getPolicyRules(mProfileRule, num);
    if (status != OK) {
        retVal = status;
        goto ERROR_EXIT;
    }

    if (mStatus == INITIAL) {
        mStatus = INITIALIZED;
    } else {
        ALOGW("Error init status. Current status is %X ", mStatus);
    }
    ALOGI("ISVManager Initialized. Profile config: Scene Num %d,  num Rule %d ", mNumProfileScene, mNumProfileRule);
    dumpData();
    return retVal;

ERROR_EXIT:
    if (mProfile != NULL) {
        //relase(mProfile);
        mProfile = NULL;
    }
    if (profileScene != NULL) {
        delete [] profileScene;
        profileScene = NULL;
    }
    mNumProfileScene = 0;
    if (mPredefinedScene != NULL) {
        delete [] mPredefinedScene;
        mPredefinedScene = NULL;
    }

    if (mProfileRule!= NULL) {
        delete [] mProfileRule;
        mProfileRule= NULL;
    }
    mNumProfileRule = 0;

    if (mProfileFRCRule != NULL) {
        delete [] mProfileFRCRule;
        mProfileFRCRule = NULL;
    }
    mNumProfileFRCRule = 0;

    return retVal;
}

status_t ISVPolicyManager::deinit()
{
    mStatus = INITIAL;
    if (mWorkStatus.dispCapability != NULL) {
        delete [] mWorkStatus.dispCapability;
        mWorkStatus.dispCapability = NULL;
    }
    memset(&mWorkStatus, 0, sizeof(ISVWorkingStatus));
    memset(&mDecision, 0, sizeof(ISVPolicyDecision));

    mProfile = NULL;

    if (mPredefinedScene != NULL) {
        delete [] mPredefinedScene;
        mPredefinedScene = NULL;
    }
    mNumProfileScene = 0;

    if (mProfileRule != NULL) {
        delete [] mProfileRule;
        mProfileRule = NULL;
    }
    mNumProfileRule = 0;

    if (mProfileFRCRule != NULL) {
        delete [] mProfileFRCRule;
        mProfileFRCRule = NULL;
    }
    mNumProfileFRCRule = 0;

    return OK;
}

ISVPolicyManager::~ISVPolicyManager()
{
    deinit();
}

//ISV policy manager input
status_t ISVPolicyManager::setClipInfo(uint32_t width, uint32_t height, uint32_t fps)
{
    if (TEST_FLAGSET(mStatus, INITIALIZED)) {
        mWorkStatus.frameWidth = width;
        mWorkStatus.frameHeight = height;
        mWorkStatus.frameRate = fps;
        SET_FLAG(mStatus, CLIP_INFO_SET);
        CLEAR_FLAG(mStatus, POLICY_RUN);
        ALOGI("%s: width %d, height %d, fps %d",__func__, width, height, fps);
        return OK;
    } else {
        ALOGW("Falied to set Clip Info. Error status %X. Manager Not Initialized ", mStatus);
        return NO_INIT;
    }
}

status_t ISVPolicyManager::setFeatureStatus(ISVFeatureStatus featureStatus[ISV_FEATURE_MAX])
{
    if (!featureStatus) {
        ALOGW("Invalid input %X", featureStatus);
        return BAD_VALUE;
    }
    if (TEST_FLAGSET(mStatus, INITIALIZED)) {
        for (uint32_t i = 0; i < ISV_FEATURE_MAX; i++) {
            switch(featureStatus[i].feature) {
                case ISV_VP:
                    mWorkStatus.VPAvailable = featureStatus[i].status;
                    break;
                case ISV_FRC:
                    mWorkStatus.FRCAvailable = featureStatus[i].status;
                    break;
                case ISV_DISP_AUTOHDMI:
                    mWorkStatus.autoHdmiAvailable = featureStatus[i].status;
                    break;
                case ISV_DISP_AUTOMIPI:
                    mWorkStatus.mipiOffAvailable = featureStatus[i].status;
                    break;
                case ISV_DISP_WIDI:
                    mWorkStatus.widiConnected = featureStatus[i].status;
                    break;
                case ISV_SFC:
                    mWorkStatus.SFCAvailable = featureStatus[i].status;
                    break;
                case ISV_FEATURE_NONE:
                    //not set ignore this
                    break;
                default:
                    ALOGW("Unknown set ISV module status");
                    break;
            }
        }
        SET_FLAG(mStatus, FEATURE_INFO_SET);
        CLEAR_FLAG(mStatus, POLICY_RUN);
        return OK;
    } else {
        ALOGW("Falied to set Feature Info. Error status %X. Manager Not Initialized ", mStatus);
        return NO_INIT;
    }
}

bool ISVScene::init(ISVWorkingStatus *workStatus)
{
    if (!workStatus) {
        ALOGW("Error input data %X", workStatus);
        return false;
    }

    memset(name, 0, MAX_LENGTH_SCENE_NAME + 1);

    //TODO consider the align of resolution, 1080P 1920x1080 or 1920x1088
    // 720 may be align 32 to 736 etc.
    mClipResolution.mLow = workStatus->frameWidth * workStatus->frameHeight;
    mClipResolution.mHigh = workStatus->frameWidth * workStatus->frameHeight;

    mClipFameRate = workStatus->frameRate;

    mDispCurrResolution.mLow = workStatus->currDispInfo.w * workStatus->currDispInfo.h;
    mDispCurrResolution.mHigh = mDispCurrResolution.mLow;

    mNumDisplayFrameRateCount = workStatus->dispCapNum;
    mDisplayFameRate = new uint32_t[mNumDisplayFrameRateCount];
    if (mDisplayFameRate != NULL) {
        for (uint32_t i = 0; i < mNumDisplayFrameRateCount; i++) {
            mDisplayFameRate[i] = workStatus->dispCapability[i].fps;
        }
    }

    mPriority = INVALID_DATA;
    mAppliedRuleIndex = INVALID_DATA;
    mFeatureStatus = 0xFFFFFFFF;

    return true;
}

ISVScene::~ISVScene()
{
    if (mDisplayFameRate != NULL) {
        delete [] mDisplayFameRate;
        mDisplayFameRate = NULL;
        mNumDisplayFrameRateCount = 0;
    }
}

status_t ISVPolicyManager::getSFCStatus(bool* SFCEnabled)
{
    uint32_t sfc = 0;
    status_t ret = ISVProfile::GetInstance()->getISVSetting(ISV_SFC_ENABLED, &sfc);
    if (ret != OK) {
        ALOGE("%s: failed to get ISV settings: ISV_SFC_ENABLED", __func__);
        return UNKNOWN_ERROR;
    }
    *SFCEnabled = ((sfc == 1 ? true : false));
    return OK;
}

/*
 * calculate the policy decision from input data and rules
 */
status_t ISVPolicyManager::getFeatureStatus(ISVFeatureStatus featureStatus[ISV_FEATURE_MAX])
{
    if (!featureStatus) {
        ALOGW("Error input data %X", featureStatus);
        return false;
    }

    if (TEST_FLAG_NOT_SET(mStatus, MUST_SET_INFO)) {
        return NOT_ENOUGH_DATA;
    }

    if (TEST_FLAG_NOT_SET(mStatus, POLICY_RUN)) {
        status_t retStatus = runPolicyDecision();
        if (retStatus != OK) {
            return retStatus;
        }
    }

    featureStatus[ISV_VP].feature = ISV_VP;
    featureStatus[ISV_VP].status = mDecision.VPOn;

    featureStatus[ISV_FRC].feature = ISV_FRC;
    featureStatus[ISV_FRC].status = mDecision.FRCOn;

    featureStatus[ISV_DISP_AUTOHDMI].feature = ISV_DISP_AUTOHDMI;
    featureStatus[ISV_DISP_AUTOHDMI].status = mDecision.autoHdmiOn;

    featureStatus[ISV_DISP_AUTOMIPI].feature = ISV_DISP_AUTOMIPI;
    featureStatus[ISV_DISP_AUTOMIPI].status = mDecision.extendModeOn;

    featureStatus[ISV_DISP_WIDI].feature = ISV_DISP_WIDI;
    featureStatus[ISV_DISP_WIDI].status = UNKNOWN;

    featureStatus[ISV_SFC].feature = ISV_SFC;
    featureStatus[ISV_SFC].status = mDecision.SFCOn;

    return OK;
}

status_t ISVPolicyManager::setDisplayInfo(DisplayInfo *ActivDisplayInfo,
                   Vector<DisplayInfo> *displayInfo)
{

    if (TEST_FLAGSET(mStatus, INITIALIZED)) {
        if (ActivDisplayInfo != NULL) {
            mWorkStatus.currDispInfo = *ActivDisplayInfo;
        }
        if (mWorkStatus.dispCapability != NULL) {
            delete mWorkStatus.dispCapability;
            mWorkStatus.dispCapability = NULL;
            mWorkStatus.dispCapNum = 0;
        }
        // TODO: Disable ISVManager checking temporary,
        // because ISVDisplay has an issue for query all display configs.
        // Please add the patch "https://android.intel.com/#/c/394347/" before you enable it.
#if 0
        mWorkStatus.dispCapability = new DisplayInfo[displayInfo->size()];
        if (mWorkStatus.dispCapability != NULL) {
            memset(mWorkStatus.dispCapability, 0, sizeof(DisplayInfo)*displayInfo->size());
            mWorkStatus.dispCapNum = displayInfo->size();
            for (uint32_t i = 0; i < displayInfo->size(); i++) {
                mWorkStatus.dispCapability[i] = displayInfo->itemAt(i);
            }
        } else {
            return NO_MEMORY;
        }
#endif

        SET_FLAG(mStatus, DISPLAY_INFO_SET);
        CLEAR_FLAG(mStatus, POLICY_RUN);
        return OK;
    } else {
        ALOGW("Falied to set Display Info. Error status %d. Manager Not Initialized ", mStatus);
        return NO_INIT;
    }
}

// If the input is enough, try to run policy decision.
status_t ISVPolicyManager::runPolicyDecision()
{
    if (TEST_FLAG_NOT_SET(mStatus, MUST_SET_INFO)) {
        return NOT_ENOUGH_DATA;
    }

    class ISVScene currScene;
    currScene.init(&mWorkStatus);

    //clear all feature 0, featuren unknown status
    uint32_t feature  = ((mWorkStatus.VPAvailable) << (ISV_VP * BITS_PER_FEATURE)) |
            (mWorkStatus.FRCAvailable) << (ISV_FRC * BITS_PER_FEATURE) |
            (mWorkStatus.autoHdmiAvailable) << (ISV_DISP_AUTOHDMI * BITS_PER_FEATURE) |
            (mWorkStatus.mipiOffAvailable) << (ISV_DISP_AUTOMIPI * BITS_PER_FEATURE) |
            (mWorkStatus.widiConnected) << (ISV_DISP_WIDI * BITS_PER_FEATURE) |
            (mWorkStatus.SFCAvailable) << (ISV_SFC * BITS_PER_FEATURE);
    currScene.setFeature(feature);

    uint32_t matchedPriority = INVALID_DATA;
    uint32_t matchedSceneIdx = INVALID_DATA;
    for (uint32_t i = 0; i < mNumProfileScene; i++) {
        if (currScene.isSubSet(&mPredefinedScene[i])) {
            if (matchedPriority > mPredefinedScene[i].getPriority()) {
                matchedPriority = mPredefinedScene[i].getPriority();
                matchedSceneIdx = i;
                ALOGV("Feature 0x%x matched scened idx %d", feature, matchedSceneIdx);
            }
        }
    }

    //set the rule of matched scene
    uint32_t ruleIndex = INVALID_DATA;
    if ((matchedSceneIdx != INVALID_DATA) && (mNumProfileRule > 0)) {
        ruleIndex = mPredefinedScene[matchedSceneIdx].getRuleIndex();
        ALOGI("matched scened idx %d, rule idx %d", matchedSceneIdx, ruleIndex);
        if (ruleIndex >= mNumProfileRule)
        {
            ALOGW("Bad profile scene & rule config. matchedSceneIdx %d, rule index %d ", matchedSceneIdx, ruleIndex);
            return BAD_VALUE;
        }
        ISVPolicyRule *rule = &mProfileRule[ruleIndex];
        if ((rule != NULL) && rule->numFilter <= ISVFilterCount) {
            memcpy(mDecision.filters, rule->filters, sizeof(ISVFilterParameter) * rule->numFilter);
            mDecision.numFilter = rule->numFilter;
        }

        uint32_t feature = 0;
        //ParseConditon change the input, copy the rule to a temporary result
        {
            uint32_t len = strlen(rule->result);
            char *temp = new char[len+1];
            if (temp != NULL) {
                memset(temp,0, len + 1);
                memcpy(temp,(char *)(rule->result), len);
                parseCondition(temp, &feature);
                ALOGV("mDecision parse feature 0x%x ",feature);
                delete [] temp;
            }
        }
        mDecision.VPOn = (FEATURE_STATUS)((feature & VP_FEATURE_MASK) >> (ISV_VP * BITS_PER_FEATURE));
        mDecision.FRCOn = (FEATURE_STATUS)((feature & FRC_FEATURE_MASK) >> (ISV_FRC * BITS_PER_FEATURE));
        mDecision.autoHdmiOn = (FEATURE_STATUS)((feature & AUTOHDMI_FEATURE_MASK) >> (ISV_DISP_AUTOHDMI* BITS_PER_FEATURE));
        mDecision.extendModeOn = (FEATURE_STATUS)((feature & EXTMODE_FEATURE_MASK) >> (ISV_DISP_AUTOMIPI* BITS_PER_FEATURE));
        mDecision.SFCOn = (FEATURE_STATUS)((feature & SFC_FEATURE_MASK) >> (ISV_SFC* BITS_PER_FEATURE));

        ALOGV("mDecision vp 0x%x, frc 0x%x, hdmi 0x%x, ext 0x%x, sfc 0x%x",
           mDecision.VPOn, mDecision.FRCOn,  mDecision.autoHdmiOn, mDecision.extendModeOn, mDecision.SFCOn);
    } else {
        //If not found matched scene, what's the default parameters for the feature VPP/FRC?
        //Used first rule VPP/FRC parameters here
        if (mNumProfileRule > 0) {
            ISVPolicyRule *rule = &mProfileRule[0];
            if (rule->numFilter <= ISVFilterCount) {
                memcpy(mDecision.filters, rule->filters, sizeof(ISVFilterParameter) * rule->numFilter);
                mDecision.numFilter = rule->numFilter;
            } else {
                ALOGW("Bad profile scene & rule config. rule 0, num filters parameters %d ", rule->numFilter);
                return BAD_VALUE;
            }
        }

        mDecision.VPOn = mWorkStatus.VPAvailable;
        mDecision.FRCOn = mWorkStatus.FRCAvailable;
        mDecision.autoHdmiOn = mWorkStatus.autoHdmiAvailable;
        mDecision.extendModeOn = mWorkStatus.mipiOffAvailable;
        mDecision.SFCOn = mWorkStatus.SFCAvailable;
    }

    SET_FLAG(mStatus, POLICY_RUN);

    return OK;
}

/*
 * get the policy decision of VP result.
 * caller get the VP filter setting of array ISVPolicyUsedFilter
 */
status_t ISVPolicyManager::getVpPolicySetting(ISVFilterParameter* filters, uint32_t &filterCount)
{
    //It OK to get feature after initialized, set Clip and Feature info
    if (TEST_FLAG_NOT_SET(mStatus, MUST_SET_INFO)) {
        return NOT_ENOUGH_DATA;
    }
    if (TEST_FLAG_NOT_SET(mStatus, POLICY_RUN)) {
        status_t retStatus = runPolicyDecision();
        if (retStatus != OK) {
            return retStatus;
        }
    }

    memcpy(filters, mDecision.filters, sizeof(ISVFilterParameter) * mDecision.numFilter);
    filterCount = mDecision.numFilter;

    return OK;
}

/*
 * get policy decision of Display
 * hdmiAutoSet: indicate hdmiAtuoSet (dymic setting) feature recommend status
 * MipAutoOff: indicate MipiAutoOff feature recommend status
 * hdmiFreshRate: are fresh rate policy manger recommened.
 * If hdmiAtuoSet is false, it must be the same as hdmi active refresh rate
 * in setDisplayInfo. Otherwise it could be any one in displayInfo.
 * -1 if setDisplayInfo is not set
 */
status_t ISVPolicyManager::getDisplayPolicySeting (int32_t &hdmiFreshRate)
{
    //get displayinf is allowed after set MUST INFO and displayinfo
    if (TEST_FLAG_NOT_SET(mStatus, MUST_SET_INFO)) {
        return NOT_ENOUGH_DATA;
    }

    if (TEST_FLAG_NOT_SET(mStatus, DISPLAY_INFO_SET)) {
        return NOT_ENOUGH_DATA;
    }

    if (TEST_FLAG_NOT_SET(mStatus, POLICY_RUN)) {
        status_t retStatus = runPolicyDecision();
        if (retStatus != OK) {
            return retStatus;
        }
    }
    // TODO: Disable ISVManager checking temporary,
    // because ISVDisplay has an issue for query all display configs.
    // Please add the patch "https://android.intel.com/#/c/394347/" before you enable it.
#if 0
    /* dynamic setting is ON and we know the display capability
     * Try to get the matched fps
     */
    hdmiFreshRate = -1;
    if (mDecision.autoHdmiOn && (mWorkStatus.dispCapNum > 0)) {
        for (uint32_t i = 0; i < mWorkStatus.dispCapNum; i++) {
            uint32_t tmp = (uint32_t)(mWorkStatus.dispCapability[i].fps);
            if ( (fabs(mWorkStatus.frameRate  - tmp) < 1)/* &&
                abs(mWorkStatus.frameWidth - mWorkStatus.dispCapability[i].w) < 32 &&
                abs(mWorkStatus.frameHeight - mWorkStatus.dispCapability[i].h) < 32*/)
                hdmiFreshRate = tmp;
        }
    }
#else
    hdmiFreshRate = mWorkStatus.frameRate;
#endif
    ALOGI("ISVManager autoHdmiOn decision %d fps %d ", mDecision.autoHdmiOn, hdmiFreshRate);

    return OK;
}

bool ISVScene::parseProfileScene(ISVPolicyScene *profileScene)
{
    if (profileScene == NULL) {
        return false;
    }

    strncpy(name, profileScene->name,strlen(profileScene->name));
    mClipResolution.mLow = profileScene->minClipResolution;
    mClipResolution.mHigh = profileScene->maxClipResolution;
    //mPredefinedScene[i].mClipFameRate = INVALID_DATA;
    mDispCurrResolution.mLow = profileScene->minDisplayResolution;
    mDispCurrResolution.mHigh = profileScene->maxDisplayResolution;
    //mPredefinedScene[i].mNumDisplayFrameRateCount,
    mPriority = profileScene->priority;
    mAppliedRuleIndex = profileScene->ruleIndex;

    return true;
}

static bool getKeyValuePair(char *keyValuePair, char *key, uint32_t *value, const char keyValueDelimit)
{
    if ((!keyValuePair) || (!key) || (!value) || (!keyValueDelimit))
        return false;

    char *pt = strchr(keyValuePair, keyValueDelimit);
    char tmp[KEY_VALUE_MAX_LEN];
    memset(tmp, 0, KEY_VALUE_MAX_LEN);

    if (pt != NULL) {
        if((pt-keyValuePair) > KEY_VALUE_MAX_LEN){
            return false;
        }
        strncpy(key, keyValuePair, pt-keyValuePair);
        key[pt-keyValuePair] = '\0';
        for (uint32_t i = 0; i < strlen(key); i++)
            key[i] = toupper(key[i]);
        int str_len = strlen(keyValuePair) - (pt-keyValuePair+1);
        if (str_len > KEY_VALUE_MAX_LEN - 1)
            str_len = KEY_VALUE_MAX_LEN - 1;
        if (str_len > 0) {
            strncpy(tmp, pt+1, str_len);
            *value = atoi(tmp);
        }
        return true;
    } else {
        return false;
    }
}

static inline uint32_t setModuleFeature(uint32_t featureSet, uint32_t featureIdx, uint32_t value)
{
    //cleare the specific feature
    uint32_t ret = featureSet & (~(0x3 << (featureIdx * BITS_PER_FEATURE)));

    //set the feature
    /* Conver the value in Scene and profile defined:
        profile 0 -> off, scene present off 0b10
        profile 1 -> on, scene presetn on 0b01
        profile 2 -> on, scene unkonw 0b00
    */
    if (value == 0) {
        value = OFF;
    } else if (value == 1) {
        value = ON;
    } else {
        value = UNKNOWN;
    }
    ret  = ret | ((value & 0x3) << (featureIdx * BITS_PER_FEATURE));

    return ret;
}

bool ISVPolicyManager::parseCondition(char *condition, uint32_t *featureSet)
{
    if (condition == NULL) {
        return false;
    }

    //Space to delimit the condition
    const char delimit[2] = " ";
    const char keyValueDelimit = '=';

    char *token = NULL;
    char key[KEY_VALUE_MAX_LEN];
    uint32_t value;
    bool bGot = false;

    memset(key, 0, KEY_VALUE_MAX_LEN);

    ALOGV("*****************************parseCondition %s *************************\n", condition);

    token = strtok(condition, delimit);
    while (token != NULL) {
        if (strlen(token) > KEY_VALUE_MAX_LEN) {
            ALOGE("configuration item too long, ignored it %s", token);
            continue;
        }
        bGot = getKeyValuePair(token, key, &value, keyValueDelimit);
        if (bGot) {
            ALOGV("%s, key:%s, value: %d 0x%x\n",token, key, value, *featureSet);
            if (!strcmp(key, VP_FILTER)) {
                *featureSet = setModuleFeature(*featureSet, ISV_VP, value);
            } else if (!strcmp(key, VP_FRC)) {
                *featureSet = setModuleFeature(*featureSet, ISV_FRC, value);
            } else if (!strcmp(key, DISP_AUTOHDMI)) {
                *featureSet = setModuleFeature(*featureSet, ISV_DISP_AUTOHDMI, value);
            } else if (!strcmp(key, DISP_EXTMODE)) {
                *featureSet = setModuleFeature(*featureSet, ISV_DISP_AUTOMIPI, value);
            } else if (!strcmp(key, WIDI)) {
                *featureSet = setModuleFeature(*featureSet, ISV_DISP_WIDI, value);
            } else if (!strcmp(key, SFC)) {
                *featureSet = setModuleFeature(*featureSet, ISV_SFC, value);
            } else {
                ALOGW("unknonw/invalid conditoin set item: %s \n",token);
            }
            ALOGV("%s, key:%s, value: %d 0x%x\n",token, key, value, *featureSet);
            memset(key, 0, KEY_VALUE_MAX_LEN);
        } else {
            ALOGW("@@@@@@@@@@@@@@@@@  invalid settings %s @@@@@@@@@@@@@\n",token);
        }

        token = strtok(NULL, delimit);
    }

    return true;
}

void ISVScene::dump()
{
    ALOGV("name %s: \n clip Resolution: %d ~ %d @ %d, Disp Res: %d ~ %d, disp Frame rate %d, Feature 0x%08X\n ",
            name,
            mClipResolution.mLow,
            mClipResolution.mHigh,
            mClipFameRate,
            mDispCurrResolution.mLow,
            mDispCurrResolution.mHigh,
            mNumDisplayFrameRateCount,
            mFeatureStatus);
    for (uint32_t j = 0; j < mNumDisplayFrameRateCount; j++) {
        ALOGV(" disp Frame rate %d", mDisplayFameRate[j]);
    }
}

void ISVPolicyManager::dumpData()
{
    ALOGV("Working Status %d x %d @ %d,  vp %d frc %d, autoHdmi %d, extMode %d, WidiOn %d, Sfc %d\n",
            mWorkStatus.frameWidth,
            mWorkStatus.frameHeight,
            mWorkStatus.frameRate,
            mWorkStatus.VPAvailable,
            mWorkStatus.FRCAvailable,
            mWorkStatus.autoHdmiAvailable,
            mWorkStatus.mipiOffAvailable,
            mWorkStatus.widiConnected,
            mWorkStatus.FRCAvailable
            );

    ALOGV("********** predefined scenery data *******************\n");
    ALOGV("Manager Status: %d\n", mStatus);
    for (uint32_t i = 0; i < mNumProfileScene; i++) {
        mPredefinedScene[i].dump();
    }

    ALOGV("********** policy rules data *******************\n");
    ALOGV("rule data: %d \n", mNumProfileRule);
    for (uint32_t i = 0; i < mNumProfileRule; i++) {
        ALOGV("result %s, filters num %d:\n", mProfileRule[i].result, mProfileRule[i].numFilter);
        for (uint32_t j = 0; j < mProfileRule[i].numFilter; j++) {
            ALOGV("\t Filter [%02d]: FilterType %d, algorithm %d",
            j,
            mProfileRule[i].filters[j].filter,
            mProfileRule[i].filters[j].algorithm);
            for (int k = 0; k < MAX_FILTER_PARAMETERS; k++)
                ALOGV("value %.3f, ", mProfileRule[i].filters[j].values[k]);
        }
    }

    ALOGV("********** end of policy manage dump data *******************\n");
}

bool ISVScene::isSubSet(class ISVScene *scene)
{
    if ((mClipResolution.mLow < scene->mClipResolution.mLow) || (mClipResolution.mHigh > scene->mClipResolution.mHigh)) {
        ALOGV("resolution does not match clip(%d ~ %d), Scene(%d ~ %d)", mClipResolution.mLow, mClipResolution.mHigh, scene->mClipResolution.mLow, scene->mClipResolution.mHigh);
        return false;
    }

    if ((mFeatureStatus & scene->mFeatureStatus) != mFeatureStatus) {
        ALOGV("feature does not match %x, scene %x", mFeatureStatus, scene->mFeatureStatus);
        return false;
    }
#if 0
    //TODO add frame rate check for FRC
    bool bFpsMatch = false;
    for (int i = 0; i < mNumDisplayFrameRateCount; i++) {
        if (mClipFameRate == mDisplayFameRate[i]) {
            bFpsMatch = true;
        }
    }

    if (bFpsMatch)
        return true;
    else
        return false;
#endif

    return true;
}

}
}
