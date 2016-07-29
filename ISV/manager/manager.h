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
#ifndef __ISV_MAN_H
#define __ISV_MAN_H

#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <ui/DisplayInfo.h>

#include "ISVManager.h"
#include "ISVProfile.h"

namespace intel {
namespace isv {

using namespace android;

#define MANAGER_MAX_RESOLUTION (8192*4096)

#define INVALID_DATA (0xFFFFFFFF)

class ISVProfile;

typedef uint32_t MANAGER_STATUS;

#define INITIAL  0
#define INITIALIZED  1
#define CLIP_INFO_SET 0x10
#define FEATURE_INFO_SET 0x20
#define DISPLAY_INFO_SET 0x40
//Must initiazlied and set Clip and feature status
#define MUST_SET_INFO ((INITIALIZED) | (CLIP_INFO_SET) | (FEATURE_INFO_SET))
#define POLICY_RUN 0x8000

#define KEY_VALUE_MAX_LEN (255)

/*
 * ISV input status. It's this status formed the ISV scnery.
 * We use this input status to judge which predefined scenery belongs to.
 */
typedef struct _ISVWorkingStatus {
    //Video Clip Infomation
    uint32_t frameWidth;
    uint32_t frameHeight;
    uint32_t frameRate;

    //Is the feature Module enable/avaiable in the applicatoin.
    FEATURE_STATUS VPAvailable;
    FEATURE_STATUS FRCAvailable;

    //Display Information
    FEATURE_STATUS autoHdmiAvailable;
    FEATURE_STATUS mipiOffAvailable;
    FEATURE_STATUS widiConnected;
    FEATURE_STATUS SFCAvailable;

    //int32_t displayWidth;
    //int32_t displayHeight;
    //int32_t freshRate;
    DisplayInfo currDispInfo;
    DisplayInfo *dispCapability;
    uint32_t dispCapNum;
} ISVWorkingStatus;

typedef struct _ISVPolicyDecision {
    //Is the feature Module enable/avaiable in the applicatoin.
    FEATURE_STATUS VPOn;
    FEATURE_STATUS FRCOn;

    ISVFilterParameter filters[ISVFilterCount];
    uint32_t      numFilter;
    ISV_FRC_RATE  frcRate;

    //Display Information
    FEATURE_STATUS autoHdmiOn;
    FEATURE_STATUS extendModeOn;
    uint32_t dispFrameRate;

    FEATURE_STATUS SFCOn;
} ISVPolicyDecision;

typedef struct _RESOLUTION {
    uint32_t mLow;
    uint32_t mHigh;
} RESOLUTION;

class ISVScene {
public:
    ISVScene(){ init(); };
    ~ISVScene();
    /*
     * User may not set some status if they do not care bout it.
     * For example they may set VP only (do NOT set ISV display
     * if they are not interested display.
     * The scenery should be initiazlied to cover above cases.
     * If For uninitizlied This initialization should
     */
    bool init(ISVWorkingStatus *workStatus);
    bool init(){
        memset(name, 0, MAX_LENGTH_SCENE_NAME + 1);
        mClipResolution.mLow = 0;
        mClipResolution.mHigh = MANAGER_MAX_RESOLUTION;
        mClipFameRate = 0;

        mDispCurrResolution.mLow = 0;
        mDispCurrResolution.mHigh = MANAGER_MAX_RESOLUTION;

        mDisplayFameRate = NULL;
        mNumDisplayFrameRateCount = 0;

        mPriority = INVALID_DATA;
        mAppliedRuleIndex = INVALID_DATA;

        //initialized module status as unknown 0b11
        mFeatureStatus = 0xFFFFFFFF;
        return true;
    };

    bool isSubSet(class ISVScene *scene);
    void setFeature(uint32_t feature) {mFeatureStatus = feature; };
    uint32_t getFeature() { return mFeatureStatus; };

    uint32_t getPriority() { return mPriority; };
    void setPriority(uint32_t priority) { mPriority = priority; };
    bool parseProfileScene(ISVPolicyScene *profileScene);

    uint32_t getRuleIndex() { return mAppliedRuleIndex;};

    void dump();

private:
    char name[MAX_LENGTH_SCENE_NAME + 1];
    RESOLUTION mClipResolution;
    //TODO to be a class of set {}
    uint32_t mClipFameRate;

    RESOLUTION mDispCurrResolution;
    //TODO to be a class of set {}
    uint32_t *mDisplayFameRate;
    uint32_t mNumDisplayFrameRateCount;
    uint32_t mPriority;
    uint32_t mAppliedRuleIndex;

    /*
      Module on/off status
      every two bit to represent a module enable disable conditoin.

      |        5        |          4        |    3   |    2    |   1   |   0    |
      |DISP_AUTOHDMI_ON | DISP_AUTOHDMI_OFF | FRC_ON | FRC_OFF | VP_ON | VP_OFF |

      | bit xxxxxxx  |   11   |   10    |       9      |       8       |         7       |       6          |
      | reserved bit | SFC_ON | SFC_OFF | DISP_WIDI_ON | DISP_WIDI_OFF | DISP_EXTMODE_ON | DISP_EXTMODE_OFF |

       To define the scenery:
       if we want the scenery apply to the case VP_ON only, then feature[1:0] = b10.
       if we want the scenery apply to the case VP_OFF only, then feature[1:0] = b01.
       if we want the scenery apply whatever the case VP stat is feature[1:0] = b11.
       What does 00 mean ?
    */
    uint32_t mFeatureStatus;
};

class ISVPolicyManager : public IISVPolicyManager {
public:
    //sp <class ISVProfile> *mProfile;
    class ISVProfile *mProfile;
    ISVPolicyManager();
    virtual ~ISVPolicyManager();
    status_t init();
    status_t deinit();

    //ISV policy manager input
    status_t setClipInfo(uint32_t width, uint32_t height, uint32_t fps);
    status_t setFeatureStatus(ISVFeatureStatus featureStatus[ISV_FEATURE_MAX]);
    status_t setDisplayInfo(DisplayInfo *ActivDisplayInfo,
                   Vector <DisplayInfo> *DisplayInfo);

    //ISV policy manager output
    status_t getSFCStatus(bool* SFCEnabled);
    status_t getFeatureStatus(ISVFeatureStatus featureStatus[ISV_FEATURE_MAX]);
    /*
      get the policy decision of VP result.
      caller get the VP filter setting of array ISVPolicyUsedFilter
     */
    status_t getVpPolicySetting(ISVFilterParameter* filters, uint32_t &filterCount);
    /*
      get policy decision of Display
      hdmiAutoSet: indicate hdmiAtuoSet (dymic setting) feature recommend status
      MipAutoOff: indicate MipiAutoOff feature recommend status
      hdmiFreshRate: are fresh rate policy manger recommened.
        If hdmiAtuoSet is false, it must be the same as hdmi active refresh rate
        in getDisplayInfo. Otherwise it could be any one in displayInfo.
        -1 if getDisplayInfo is not set
    */
    status_t getDisplayPolicySeting (int32_t &hdmiFreshRate);

private:
    //ISV Manager status
    MANAGER_STATUS mStatus;

    ISVWorkingStatus mWorkStatus;

    //ISV internal structure to present predefined scenery
    class ISVScene *mPredefinedScene;
    uint32_t mNumProfileScene;

    //manager store profile predefined policy rule
    ISVPolicyRule *mProfileRule;
    uint32_t mNumProfileRule;

    //manager store profile frc rule
    ISVPolicyFRCRule *mProfileFRCRule;
    uint32_t mNumProfileFRCRule;

    ISVPolicyDecision mDecision;

    bool parseCondition(char *condition, uint32_t *feature);
    /* If the input is enough, try to run policy decision.
     * Run it in manager private to simplify the calling of public API.
     */
    status_t runPolicyDecision();
    void dumpData();
};

} //namespace isv
} //namespace intel
#endif //__ISV_MAN_H
