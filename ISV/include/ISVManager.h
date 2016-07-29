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
#ifndef __ISV_MANAGER_H
#define __ISV_MANAGER_H

#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <ui/DisplayInfo.h>

#include "ISVType.h"

namespace intel {
namespace isv {

using namespace android;

typedef enum {
    ISV_VP = 0,
    ISV_FRC,
    ISV_DISP_AUTOHDMI,
    ISV_DISP_AUTOMIPI,
    ISV_DISP_WIDI,
    ISV_SFC,
    ISV_FEATURE_MAX,
    ISV_FEATURE_NONE = 0xFFFFFFFF,
} ISV_FEATURE;

//initialized clear to 0 as UNKNOWN status
typedef enum {
    UNKNOWN = 0,
    ON,
    OFF,
} FEATURE_STATUS;

typedef struct {
    ISV_FEATURE feature;
    FEATURE_STATUS status;
} ISVFeatureStatus;

class IISVPolicyManager {

public:
    IISVPolicyManager(){};
    virtual ~IISVPolicyManager(){};

    virtual status_t init() = 0;
    virtual status_t deinit() = 0;

    // ISV policy manager input
    virtual status_t setClipInfo(uint32_t width, uint32_t height, uint32_t fps) = 0;
    virtual status_t setFeatureStatus(ISVFeatureStatus featureStatus[ISV_FEATURE_MAX]) = 0;

    virtual status_t setDisplayInfo(DisplayInfo *ActiveDisplayInfo,
                   Vector<DisplayInfo> *DisplayInfo) = 0;

    // ISV policy manager output
    // get SFC profile capability
    virtual status_t getSFCStatus(bool* SFCEnabled) = 0;

    // get the policy decision of features' status
    virtual status_t getFeatureStatus(ISVFeatureStatus featureStatus[ISV_FEATURE_MAX]) = 0;

    /*
      get the policy decision of VP result.
      caller get the VP filter setting of array ISVPolicyUsedFilter
     */
    virtual status_t getVpPolicySetting(ISVFilterParameter* filters, uint32_t &filterCount) = 0;
    /*
      get policy decision of Display.
      hdmiAutoSet: indicate hdmiAtuoSet (dymic setting) feature recommend status
      MipAutoOff: indicate MipiAutoOff feature recommend status
      hdmiFreshRate: are fresh rate policy manger recommened.
        If hdmiAtuoSet is false, it must be the same as hdmi active refresh rate
        in getDisplayInfo. Otherwise it could be any one in displayInfo.
        -1 if getDisplayInfo is not set
    */
    virtual status_t getDisplayPolicySeting(int32_t &hdmiFreshRate) = 0;
};

}; // namespace isv
}; //namespace intel

#endif //__ISV_MANAGER_H
