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

#ifndef __ISV_PROFILE_H
#define __ISV_PROFILE_H

#include <utils/RefBase.h>
#include "ISVType.h"
#include "utils/Errors.h"

namespace intel {
namespace isv {

using namespace android;

#define MAX_LENGTH_SCENE_CONDITION (256)
#define MAX_LENGTH_RULE_RESULT     (256)
#define MAX_LENGTH_SCENE_NAME (64)
#define MAX_LENGTH_RULE_NAME (64)
#define MAX_LENGTH_VALUE_STRING (64)
#define MAX_NUM_SCENE (128)
#define MAX_NUM_RULE (128)
#define MAX_NUM_FRC_RULE (64)

    /** \brief the ISV settings */
    typedef enum _ISV_SETTING {
        ISV_SETTING_NONE = 0,
        /* Common */
        ISV_COMMON_ENABLED,
        ISV_COMMON_DEFAULT_VPP,
        ISV_COMMON_DEFAULT_FRC,
        ISV_COMMON_MAX_RESOLUTION,
        /* Display */
        ISV_DISPLAY_HDMI_AUTO,
        ISV_DISPLAY_MIPI_AUTO,
        /* SFC */
        ISV_SFC_ENABLED,
        /* Policy */
        ISV_POLICY_SCENE_NUM,
        ISV_POLICY_RULE_NUM,
        ISV_POLICY_FRC_RULE_NUM,

        ISV_SETTING_COUNT
    } ISV_SETTING;

    typedef struct _ISVPolicyScene {
        /** \brief name of Scene */
        char     name[MAX_LENGTH_SCENE_NAME + 1];
        /** \brief priority */
        uint32_t priority;
        /** \brief condition string */
        char     condition[MAX_LENGTH_SCENE_CONDITION + 1];
        /** \brief min clip resolution */
        uint32_t minClipResolution;
        /** \brief max clip resolution */
        uint32_t maxClipResolution;
        /** \brief min display resolution */
        uint32_t minDisplayResolution;
        /** \brief man display resolution */
        uint32_t maxDisplayResolution;
        /** \brief the index of Rule */
        uint32_t ruleIndex;
    } ISVPolicyScene;

    typedef struct _ISVPolicyRule {
        /** \brief name of Rule */
        char               name[MAX_LENGTH_RULE_NAME + 1];
        /** \brief index of Rule */
        uint32_t           index;
        /** \brief result string */
        char               result[MAX_LENGTH_RULE_RESULT + 1];
        /** \brief the used filters' array */
        ISVFilterParameter filters[ISVFilterCount];
        /** \brief the number of filters' array */
        uint32_t           numFilter;
    } ISVPolicyRule;

    typedef struct _ISVPolicyFRCRule {
        /** \brief FPS of input */
        uint32_t      input_fps;
        /** \brief FPS of output */
        uint32_t      output_fps;
        /** \brief FRC rate */
        ISV_FRC_RATE  rate;
    } ISVPolicyFRCRule;

class ISVProfile : public RefBase
{
public:

    static ISVProfile * GetInstance();

    /* get the Setting data */
    status_t getISVSetting(const ISV_SETTING setting, uint32_t *value);

    /* get the Filter's capability */
    status_t getFilterCapability(ISVFilterCapability *caps, uint32_t *count);


    /* get the Policy's data */
    status_t getPolicyScenes(ISVPolicyScene *scenes, uint32_t count);
    status_t getPolicyRules(ISVPolicyRule *rules, uint32_t count);
    status_t getPolicyFRCRules(ISVPolicyFRCRule *frcRules, uint32_t count);

    /* the global setting for VPP */
    static bool isVPPOn();

    /* the global setting for FRC */
    static bool isFRCOn();

private:
    ISVProfile();
    ~ISVProfile();

    /* Read the global setting for ISV */
    static unsigned long getGlobalStatus();

    /* Get the config data from XML file */
    void getDataFromXmlFile(void);

    /* handle the XML file */
    static void startElement(void *userData, const char *name, const char **atts);
    static void endElement(void *userData, const char *name);
    void handleCommonSettings(const char *name, const char **atts);
    void handlePolicySettings(const char *name, const char **atts);
    void handleDisplaySettings(const char *name, const char **atts);
    void handleFilterSettings(const char *name, const char **atts);
    void handleSFCSettings(const char *name, const char **atts);

    /* get the filter's ID according the filter's name */
    ISVFilterType getFilterID(const char * name);

    /* dump the config data */
    void dumpConfigData();

private:
    enum _PROFILE_HANDLE_STATUS {
        PROFILE_COMPONENT_START = 0,
        PROFILE_COMPONENT_COMMON,
        PROFILE_COMPONENT_POLICY,
        PROFILE_COMPONENT_DISPLAY,
        PROFILE_COMPONENT_SFC,
        PROFILE_COMPONENT_FILTER,
        PROFILE_COMPONENT_MAX
    };

    class ReleaseProfile {
        public:
            ~ReleaseProfile()
            {
                if (ISVProfile::m_pInstance) {
                    delete ISVProfile::m_pInstance;
                    ISVProfile::m_pInstance = NULL;
                }
            }
    };

private:
    static ISVProfile * m_pInstance;

    /* it's used to release Profile resource */
    static ReleaseProfile mReleaseProfile;

    /* save the filters' capability */
    ISVFilterCapability mFilterConfigs[ISVFilterCount];
    uint32_t mNumEnabledFilter;

    /* save policy's data */
    ISVPolicyFRCRule *mPolicyFRCRules;
    ISVPolicyScene *mPolicyScenes;
    ISVPolicyRule *mPolicyRules;
    uint32_t mNumPolicyFRCRule;
    uint32_t mNumPolicyScene;
    uint32_t mNumPolicyRule;

    /* used to parse xml file */
    uint32_t mCurrentFilter;
    uint32_t mCurPolicyFRCRule;
    uint32_t mCurPolicyRule;
    uint32_t mCurPolicyScene;
    /* indetify common/policy/display/filter */
    uint32_t mCurrentComponent;

    /* save the setting values */
    uint32_t mSettings[ISV_SETTING_COUNT];

    /* common values */
    uint32_t mDefaultVPPStatus;
    uint32_t mDefaultFRCStatus;
    uint32_t mMaxSupportResolution;

    status_t mGlobalStatus;
};

} //namespace isv
} //namespace intel

#endif /* __ISV_PROFILE_H */
