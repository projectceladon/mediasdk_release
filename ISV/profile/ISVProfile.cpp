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

//#define LOG_NDEBUG 0
#include <expat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <utils/Log.h>
#include "ISVProfile.h"
#include <cutils/properties.h>

namespace intel {
namespace isv {

#undef LOG_TAG
#define LOG_TAG "ISVProfile"

#define QCIF_AREA (176 * 144)
#define MAX_BUF_SIZE (32 * 1024)
#define MAX_SUPPORTED_RESOLUTION (4096 * 2160)
#define DEFAULT_XML_FILE "/etc/video_isv_profile.xml"

using namespace android;

ISVProfile* ISVProfile::m_pInstance = NULL;

ISVProfile* ISVProfile::GetInstance()
{
    if (m_pInstance == NULL) {
        m_pInstance = new ISVProfile();
    }

    return m_pInstance;
}

ISVProfile::ISVProfile()
{
    int i;

    mNumEnabledFilter = 0;
    mNumPolicyFRCRule = 0;
    mNumPolicyScene = 0;
    mNumPolicyRule = 0;
    mCurrentFilter = 0;
    mCurPolicyFRCRule = 0;
    mCurPolicyRule = 0;
    mCurPolicyScene = 0;
    mCurrentComponent = 0;
    mDefaultVPPStatus = 0;
    mDefaultFRCStatus = 0;
    mGlobalStatus = OK;
    mMaxSupportResolution = 0;

    mPolicyFRCRules = NULL;
    mPolicyScenes = NULL;
    mPolicyRules = NULL;

    //mReleaseProfile = new ISVProfile::ReleaseProfile();

    memset(mFilterConfigs, 0, sizeof(ISVFilterCapability) * ISVFilterCount);
    for (i = 0; i < ISVFilterCount;i++)
        mFilterConfigs[i].maxResolution = MAX_SUPPORTED_RESOLUTION;

    memset(mSettings, 0, sizeof(uint32_t) * ISV_SETTING_COUNT);

    /* load the config data from XML file */
    getDataFromXmlFile();

    /* dump data for debug */
    dumpConfigData();
}

ISVProfile::~ISVProfile()
{

    if (m_pInstance != NULL) {
        if (mPolicyRules) {
            free(mPolicyRules);
            mPolicyRules = NULL;
        }

        if (mPolicyScenes) {
            free(mPolicyScenes);
            mPolicyScenes = NULL;
        }

        if (mPolicyFRCRules) {
            free(mPolicyFRCRules);
            mPolicyFRCRules = NULL;
        }
    }
}

status_t ISVProfile::getFilterCapability(ISVFilterCapability *configs, uint32_t *count)
{
    status_t result = OK;
    uint32_t i, j, filter_count = 0;

    if (!configs || !count || *count < ISVFilterCount) {
        ALOGE("%s: the parameters are invalid!\n", __func__);
        return BAD_VALUE;
    }

    for (i = 0; i < ISVFilterCount; i++) {
        if (mFilterConfigs[i].enabled == true) {
            configs[filter_count].type = mFilterConfigs[i].type;
            configs[filter_count].enabled = mFilterConfigs[i].enabled;
            configs[filter_count].minResolution = mFilterConfigs[i].minResolution;
            configs[filter_count].maxResolution = mFilterConfigs[i].maxResolution;

            configs[filter_count].algorithm.min_algo = mFilterConfigs[i].algorithm.min_algo;
            configs[filter_count].algorithm.max_algo = mFilterConfigs[i].algorithm.max_algo;
            configs[filter_count].algorithm.default_algo = mFilterConfigs[i].algorithm.default_algo;
            configs[filter_count].algorithm.step = mFilterConfigs[i].algorithm.step;

            for(j = 0; j < mFilterConfigs[i].numberParameter; j++) {
                configs[filter_count].parameters[j].min_value = mFilterConfigs[i].parameters[j].min_value;
                configs[filter_count].parameters[j].max_value = mFilterConfigs[i].parameters[j].max_value;
                configs[filter_count].parameters[j].default_value = mFilterConfigs[i].parameters[j].default_value;
                configs[filter_count].parameters[j].step = mFilterConfigs[i].parameters[j].step;
            }
            configs[filter_count].numberParameter = mFilterConfigs[i].numberParameter;

            filter_count++;
        }
    }

    *count = filter_count;

    return result;
}

status_t ISVProfile::getISVSetting(const ISV_SETTING setting, uint32_t *value)
{
   uint32_t result = OK;

    if (!value || setting <= ISV_SETTING_NONE || setting >= ISV_SETTING_COUNT) {
        ALOGE("%s: the parameters are invalid: displaySetting=%d\n", __func__, setting);
        return BAD_VALUE;
    }

    *value = mSettings[setting];

    return result;

}
status_t ISVProfile::getPolicyScenes(ISVPolicyScene *scenes, uint32_t count)
{
    uint32_t result = OK, i;

    if (!scenes || count < mNumPolicyScene) {
        ALOGE("%s: the parameters are invalid: count=%d\n", __func__, count);
        return BAD_VALUE;
    }

    for (i = 0; i < mNumPolicyScene; i++) {
        strncpy(scenes[i].name, mPolicyScenes[i].name, MAX_LENGTH_SCENE_NAME);
        strncpy(scenes[i].condition, mPolicyScenes[i].condition, MAX_LENGTH_SCENE_CONDITION);

        scenes[i].priority =             mPolicyScenes[i].priority;
        scenes[i].minClipResolution  =   mPolicyScenes[i].minClipResolution;
        scenes[i].maxClipResolution =    mPolicyScenes[i].maxClipResolution;
        scenes[i].minDisplayResolution = mPolicyScenes[i].minDisplayResolution;
        scenes[i].maxDisplayResolution = mPolicyScenes[i].maxDisplayResolution;
        scenes[i].ruleIndex =            mPolicyScenes[i].ruleIndex;
    }

    return result;
}

status_t ISVProfile::getPolicyRules(ISVPolicyRule *rules, uint32_t count)
{
    uint32_t result = OK, i, j, k;

    if (!rules || count < mNumPolicyRule) {
        ALOGE("%s: the parameters are invalid: count=%d\n", __func__, count);
        return BAD_VALUE;
    }

    for (i = 0; i < mNumPolicyRule; i++) {
        rules[i].index = mPolicyRules[i].index;
        strncpy(rules[i].name, mPolicyRules[i].name, MAX_LENGTH_RULE_NAME);
        strncpy(rules[i].result, mPolicyRules[i].result, MAX_LENGTH_RULE_RESULT);
        rules[i].numFilter = mPolicyRules[i].numFilter;
        for (j = 0; j < rules[i].numFilter; j++) {
            rules[i].filters[j].filter = mPolicyRules[i].filters[j].filter;
            rules[i].filters[j].algorithm = mPolicyRules[i].filters[j].algorithm;
            for (k = 0; k < MAX_FILTER_PARAMETERS; k++)
                rules[i].filters[j].values[k] = mPolicyRules[i].filters[j].values[k];
        }
    }

    return result;
}

status_t ISVProfile::getPolicyFRCRules(ISVPolicyFRCRule *frcRules, uint32_t count)
{
    uint32_t result = OK, i;

    if (!frcRules || count < mNumPolicyFRCRule) {
        ALOGE("%s: the parameters are invalid: count=%d\n", __func__, count);
        return BAD_VALUE;
    }

    for (i = 0; i < mNumPolicyFRCRule; i++) {
         frcRules[i].input_fps = mPolicyFRCRules[i].input_fps;
         frcRules[i].output_fps = mPolicyFRCRules[i].output_fps;
         frcRules[i].rate = mPolicyFRCRules[i].rate;
    }

    return result;
}

bool ISVProfile::isVPPOn()
{
    unsigned long status = getGlobalStatus();
    return (status != 0) ? (((status & VPP_COMMON_ON) != 0) ? true : false) : false;
}

bool ISVProfile::isFRCOn()
{
    unsigned long status = getGlobalStatus();
    return (status != 0) ? (((status & VPP_FRC_ON) != 0) ? true : false) : false;
}

ISVFilterType ISVProfile::getFilterID(const char * name)
{
    ISVFilterType index = ISVFilterNone;

    /* Convert the Filter's name to index */
    if (strcmp(name, "ProcFilterNoiseReduction") == 0)
        index = ISVFilterNoiseReduction;
    else if (strcmp(name, "ProcFilterDeinterlacing") == 0)
        index = ISVFilterDeinterlacing;
    else if (strcmp(name, "ProcFilterSharpening") == 0)
        index = ISVFilterSharpening;
    else if (strcmp(name, "ProcFilterColorBalance") == 0)
        index = ISVFilterColorBalance;
    else if (strcmp(name, "ProcFilterDeblocking") == 0)
        index = ISVFilterDeblocking;
    else if (strcmp(name, "ProcFilterFrameRateConversion") == 0)
        index = ISVFilterFrameRateConversion;
    else if (strcmp(name, "ProcFilterSkinToneEnhancement") == 0)
        index = ISVFilterSkinToneEnhancement;
    else if (strcmp(name, "ProcFilterTotalColorCorrection") == 0)
        index = ISVFilterTotalColorCorrection;
    else if (strcmp(name, "ProcFilterNonLinearAnamorphicScaling") == 0)
        index = ISVFilterNonLinearAnamorphicScaling;
    else if (strcmp(name, "ProcFilterImageStabilization") == 0)
        index = ISVFilterImageStabilization;

    return index;
}

void ISVProfile::handleCommonSettings(const char *name, const char **atts)
{
    int attIndex = 0;

    /* There are some error while parsing XML file */
    if (mGlobalStatus != OK)
        return;

    if (strcmp(name, "ISVEnabled") == 0) {
        if (strcmp(atts[attIndex], "value") == 0)
            mSettings[ISV_COMMON_ENABLED] = atoi(atts[1]);
        else
            ALOGE("%s:Couldn't handle the \"%s\" in \"%s\" element\n",
                __func__, atts[attIndex], name);

    } else if (strcmp(name, "DefaultVPPStatus") == 0) {
        if (strcmp(atts[attIndex], "value") == 0)
            mSettings[ISV_COMMON_DEFAULT_VPP] = atoi(atts[1]);
        else
            ALOGE("%s:Couldn't handle the \"%s\" in \"%s\" element\n",
                __func__, atts[attIndex], name);

    } else if (strcmp(name, "DefaultFRCStatus") == 0) {
        if (strcmp(atts[attIndex], "value") == 0)
            mSettings[ISV_COMMON_DEFAULT_FRC] = atoi(atts[1]);
        else
            ALOGE("%s:Couldn't handle the \"%s\" in \"%s\" element\n",
                __func__, atts[attIndex], name);

    } else if (strcmp(name, "SupportedMaxResolution") == 0) {
        mMaxSupportResolution = atoi(atts[1]);
        mSettings[ISV_COMMON_MAX_RESOLUTION] = mMaxSupportResolution;

    } else if (strcmp(name, "CommonSettings") == 0) {
        return;

    } else {
        ALOGE("%s:Couldn't handle the \"%s\" element!\n", __func__, name);
    }

}

void ISVProfile::handlePolicySettings(const char *name, const char **atts)
{
    uint32_t num_value, index, length;

    /* There are some error while parsing XML file */
    if (mGlobalStatus != OK)
        return;

    if (strcmp(name, "PolicyFRCRuleNumber") == 0) {
        /* malloc the memory for FRC Rule table */
        num_value = atoi(atts[1]);
        if (num_value > MAX_NUM_FRC_RULE) {
            ALOGE("%s: the defined frc rule number(%d) is bigger than MAX_NUM(%d)\n",
                __func__, num_value, MAX_NUM_FRC_RULE);
            return;
        }

        mPolicyFRCRules = (ISVPolicyFRCRule *)malloc(sizeof(ISVPolicyFRCRule) * num_value);
        if (!mPolicyFRCRules) {
            ALOGE("%s: failed to malloc memory for mPolicyFRCRules! errno=%d(%s)\n",
                __func__, errno, strerror(errno));
            mGlobalStatus = NO_MEMORY;
            return;
        }

        memset(mPolicyFRCRules, 0 , sizeof(ISVPolicyFRCRule) * num_value);
        mNumPolicyFRCRule = num_value;
        mSettings[ISV_POLICY_FRC_RULE_NUM] = num_value;

    } else if (strcmp(name, "PolicySceneNumber") == 0) {
        uint32_t i, max;

        /* malloc the memory for Scene table */
        num_value = atoi(atts[1]);
        if (num_value > MAX_NUM_SCENE) {
            ALOGE("%s: the defined policy scene number(%d) is bigger than MAX_NUM(%d)\n",
                __func__, num_value, MAX_NUM_SCENE);
            return;
        }

        mPolicyScenes = (ISVPolicyScene *)malloc(sizeof(ISVPolicyScene) * num_value);
        if (!mPolicyScenes) {
            ALOGE("%s: failed to malloc memory for ISVPolicyScene! errno=%d(%s)\n",
                __func__, errno, strerror(errno));
            mGlobalStatus = NO_MEMORY;
            return;
        }

        memset(mPolicyScenes, 0, sizeof(ISVPolicyScene) * num_value);
        /* set the resolution to the max */
        if (mMaxSupportResolution == 0)
            max = MAX_SUPPORTED_RESOLUTION;
        else
            max = mMaxSupportResolution;

        for (i = 0; i < num_value; i++) {
            mPolicyScenes[i].maxClipResolution = max;
            mPolicyScenes[i].maxDisplayResolution = max;
        }
        mNumPolicyScene = num_value;
        mSettings[ISV_POLICY_SCENE_NUM] = num_value;

    }else if (strcmp(name, "PolicyRuleNumber") == 0) {
        /* malloc the memory for Rule table */
        num_value = atoi(atts[1]);
        if (num_value > MAX_NUM_RULE) {
            ALOGE("%s: the defined policy rule number(%d) is bigger than MAX_NUM(%d)\n",
                __func__, num_value, MAX_NUM_RULE);
            return;
        }

        mPolicyRules = (ISVPolicyRule *)malloc(sizeof(ISVPolicyRule) * num_value);
        if (!mPolicyRules) {
            ALOGE("%s: failed to malloc memory for ISVPolicyRule! errno=%d(%s)\n",
                __func__, errno, strerror(errno));
            mGlobalStatus = NO_MEMORY;
            return;
        }
        memset(mPolicyRules, 0, sizeof(ISVPolicyRule) * num_value);

        mNumPolicyRule = num_value;
        mSettings[ISV_POLICY_RULE_NUM] = num_value;

    } else if (strcmp(name, "PolicyFRCRule") == 0) {
        /* fill the mPolicyFRCRules tables */
        index = atoi(atts[1]);

        if (index < mNumPolicyFRCRule) {
            mPolicyFRCRules[index].input_fps = atoi(atts[3]);
            mPolicyFRCRules[index].output_fps = atoi(atts[5]);

            if (!strcmp(atts[7], "2"))
                mPolicyFRCRules[index].rate = FRC_RATE_2X;
            else if (!strcmp(atts[7], "2.5"))
                mPolicyFRCRules[index].rate = FRC_RATE_2_5X;
            else if (!strcmp(atts[7], "4"))
                mPolicyFRCRules[index].rate = FRC_RATE_4X;
            else
                mPolicyFRCRules[index].rate = FRC_RATE_1X;

        } else
            ALOGE("%s: invalid index number for %s!\n", __func__, name);

    } else if (strcmp(name, "PolicyScene") == 0) {
        /* get the current Scene index */
        index = atoi(atts[1]);
        length = strlen(atts[3]);

        if(index < mNumPolicyScene) {
            if (length >= MAX_LENGTH_SCENE_NAME)
               length = MAX_LENGTH_SCENE_NAME;
            mCurPolicyScene = index;
            strncpy(mPolicyScenes[mCurPolicyScene].name, atts[3], length);
            mPolicyScenes[mCurPolicyScene].name[length] = '\0';
        } else
            ALOGE("%s: invalid index(%d) or length(%d) for %s!\n", __func__, index, length, name);

    } else if (strcmp(name, "ScenePriority") == 0) {
        mPolicyScenes[mCurPolicyScene].priority = atoi(atts[1]);

    } else if (strcmp(name, "SceneCondition") == 0) {
        /* the Scene condition is save as string which parsed by caller */
        length = strlen(atts[1]);
        if (length >= MAX_LENGTH_SCENE_CONDITION)
            length = MAX_LENGTH_SCENE_CONDITION;

        strncpy(mPolicyScenes[mCurPolicyScene].condition, atts[1], length);
        mPolicyScenes[mCurPolicyScene].condition[length] = '\0';

    } else if (strcmp(name, "SceneClipResolution") == 0) {
        /* clip resolution range */
        mPolicyScenes[mCurPolicyScene].minClipResolution = atoi(atts[1]);
        mPolicyScenes[mCurPolicyScene].maxClipResolution = atoi(atts[3]);

    } else if (strcmp(name, "SceneDisplayResolution") == 0) {
        /* display resolution range */
        mPolicyScenes[mCurPolicyScene].minDisplayResolution = atoi(atts[1]);
        mPolicyScenes[mCurPolicyScene].minDisplayResolution = atoi(atts[3]);

    } else if (strcmp(name, "SceneRuleIndex") == 0) {
        mPolicyScenes[mCurPolicyScene].ruleIndex = atoi(atts[1]);

    } else if (strcmp(name, "PolicyRule") == 0) {
        uint32_t name_len;

        /* read index and name */
        index = atoi(atts[1]);
        name_len = strlen(atts[3]);
        if (name_len >= MAX_LENGTH_RULE_NAME)
            name_len = MAX_LENGTH_RULE_NAME;

        /* the Rule result is save as string which parsed by caller */
        length = strlen(atts[5]);
        if (length >= MAX_LENGTH_RULE_RESULT)
            length = MAX_LENGTH_RULE_RESULT;

        if(index < mNumPolicyRule) {
            mCurPolicyRule = index;
            mPolicyRules[mCurPolicyRule].index = index;

            strncpy(mPolicyRules[mCurPolicyRule].name, atts[3], name_len);
            mPolicyRules[mCurPolicyRule].name[name_len] = '\0';

            strncpy(mPolicyRules[mCurPolicyRule].result, atts[5], length);
            mPolicyRules[mCurPolicyRule].result[length] = '\0';

            mPolicyRules[mCurPolicyRule].numFilter = 0;
        } else
            ALOGE("%s: invalid index(%d) for %s!\n", __func__, index, name);

    } else if (strcmp(name, "UsedFilter") == 0) {
        ISVFilterType filter;
        int str_len = 0, count = 0;
        float value = 0.0;
        int max_len = 256;
        char name[max_len], *p, *str = name;

        /* get the filter's index */
        filter = getFilterID(atts[1]);
        index = mPolicyRules[mCurPolicyRule].numFilter;

        if (strcmp(atts[2], "value") == 0) {
            value = (float)atof(atts[3]);
            mPolicyRules[mCurPolicyRule].filters[index].values[0] = value;

        } else if (strcmp(atts[2], "values") == 0) {
            /* parse "1:2:3: ... :N" string to float array */
            str_len = strlen(atts[3]);
            if (str_len > max_len - 1)
                str_len = max_len - 1;
            strncpy(name, atts[3], str_len);
            name[str_len] = '\0';

            do {
                p = strtok(str, ":");
                if (p) {
                    value = (float)atof(p);
                    mPolicyRules[mCurPolicyRule].filters[index].values[count] = value;
                }

                str = NULL;
                count++;
            } while(p && count < MAX_FILTER_PARAMETERS);

        } else if (strcmp(atts[2], "algorithm") == 0){
            mPolicyRules[mCurPolicyRule].filters[index].algorithm = atoi(atts[3]);
        }

        mPolicyRules[mCurPolicyRule].filters[index].filter = filter;
        /* update the used filter number */
        mPolicyRules[mCurPolicyRule].numFilter = index + 1;

    } else if (strcmp(name, "PolicySettings") == 0) {
        return;
    } else {
        ALOGE("%s:Couldn't handle the \"%s\" element!\n", __func__, name);
    }

}

void ISVProfile::handleSFCSettings(const char *name, const char **atts)
{
    int attIndex = 0;

    /* There are some error while parsing XML file */
    if (mGlobalStatus != OK)
        return;

    if (strcmp(name, "SFCEnabled") == 0) {
        if (strcmp(atts[attIndex], "value") == 0)
            mSettings[ISV_SFC_ENABLED] = atoi(atts[1]);
        else
            ALOGE("%s:Couldn't handle the \"%s\" in \"%s\" element\n",
                __func__, atts[attIndex], name);
    } else if (strcmp(name, "SFCSettings") == 0) {
        return;
    } else {
        ALOGE("%s:Couldn't handle the \"%s\" element!\n", __func__, name);
    }
}

void ISVProfile::handleDisplaySettings(const char *name, const char **atts)
{
    int attIndex = 0;

    /* There are some error while parsing XML file */
    if (mGlobalStatus != OK)
        return;

    if (strcmp(name, "HdmiAutoSetting") == 0) {
        if (strcmp(atts[attIndex], "value") == 0)
            mSettings[ISV_DISPLAY_HDMI_AUTO] = atoi(atts[1]);
        else
            ALOGE("%s:Couldn't handle the \"%s\" in \"%s\" element\n",
                __func__, atts[attIndex], name);

    } else if (strcmp(name, "MipiAutoSetting") == 0) {
        if (strcmp(atts[attIndex], "value") == 0)
            mSettings[ISV_DISPLAY_MIPI_AUTO] = atoi(atts[1]);
        else
            ALOGE("%s:Couldn't handle the \"%s\" in \"%s\" element\n",
                __func__, atts[attIndex], name);

    } else if (strcmp(name, "DisplaySettings") == 0) {
        return;

    } else {
        ALOGE("%s:Couldn't handle the \"%s\" element!\n", __func__, name);
    }
}

void ISVProfile::handleFilterSettings(const char *name, const char **atts)
{
    int attIndex = 0, value;
    bool enabled;
    ISVFilterType index;

    /* There are some error while parsing XML file */
    if (mGlobalStatus != OK)
        return;

    if (strcmp(name, "Filter") == 0) {

        /* get Index and Enabled info */
        index = getFilterID(atts[1]);
        if (!strcmp(atts[3], "true"))
            enabled = true;
        else
            enabled = false;

        if (!index) {
            ALOGE("%s: couldn't parse this filter name %s\n", __func__, atts[1]);
            mCurrentFilter = 0;
            return;
        }

        mCurrentFilter = index;
        mFilterConfigs[mCurrentFilter].type = index;
        mFilterConfigs[mCurrentFilter].enabled = enabled;

    } else if (strcmp(name, "SupportResolution") == 0) {
        uint32_t min, max;

        /* get supported min/max resolution */
        min = atoi(atts[1]);
        if (!strcmp(atts[3], "FFFFFFFF") || !strcmp(atts[3], "MAX")) {
            if (mMaxSupportResolution == 0)
                max = MAX_SUPPORTED_RESOLUTION;
            else
                max = mMaxSupportResolution;
        } else
            max = atoi(atts[3]);

        if (mCurrentFilter) {
            mFilterConfigs[mCurrentFilter].minResolution = min;
            mFilterConfigs[mCurrentFilter].maxResolution = max;
        }

    } else if (strcmp(name, "FilterParameter") == 0) {
        float min, max, default_value, step;
        uint32_t index;

        /* get the Parameter info */
        index = atoi(atts[1]);
        min = (float)atof(atts[3]);
        max = (float)atof(atts[5]);
        default_value = (float)atof(atts[7]);
        step = (float)atof(atts[9]);

         if (mCurrentFilter && index < MAX_FILTER_PARAMETERS) {
             mFilterConfigs[mCurrentFilter].parameters[index].min_value = min;
             mFilterConfigs[mCurrentFilter].parameters[index].max_value = max;
             mFilterConfigs[mCurrentFilter].parameters[index].default_value = default_value;
             mFilterConfigs[mCurrentFilter].parameters[index].step = step;

             mFilterConfigs[mCurrentFilter].numberParameter += 1;
         }

    } else if (strcmp(name, "FilterAlgorithm") == 0) {
	int min, max, default_algo, step;

        min = atoi(atts[1]);
        max = atoi(atts[3]);
        default_algo = atoi(atts[5]);
        step = atoi(atts[7]);

        mFilterConfigs[mCurrentFilter].algorithm.min_algo = min;
        mFilterConfigs[mCurrentFilter].algorithm.max_algo = max;
        mFilterConfigs[mCurrentFilter].algorithm.default_algo = default_algo;
        mFilterConfigs[mCurrentFilter].algorithm.step = step;

    } else if (strcmp(name, "FilterSettings") == 0) {
        return;

    } else {
        ALOGE("%s:Couldn't handle this element %s!\n", __func__, name);
        mCurrentFilter = 0;
    }
}

void ISVProfile::startElement(void *userData, const char *name, const char **atts)
{
    ISVProfile *profile = (ISVProfile *)userData;

    /* indetify the component */
    if (strcmp(name, "IntelSmartVideoSettings") == 0) {
        profile->mCurrentComponent = PROFILE_COMPONENT_START;
        return;
    } else if (strcmp(name, "CommonSettings") == 0) {
        profile->mCurrentComponent = PROFILE_COMPONENT_COMMON;
    } else if (strcmp(name, "PolicySettings") == 0) {
        profile->mCurrentComponent = PROFILE_COMPONENT_POLICY;
    } else if (strcmp(name, "DisplaySettings") == 0) {
        profile->mCurrentComponent = PROFILE_COMPONENT_DISPLAY;
    } else if (strcmp(name, "FilterSettings") == 0) {
        profile->mCurrentComponent = PROFILE_COMPONENT_FILTER;
    } else if (strcmp(name, "SFCSettings") == 0) {
        profile->mCurrentComponent = PROFILE_COMPONENT_SFC;
    }

    /* handle the element in different component */
    if (profile->mCurrentComponent == PROFILE_COMPONENT_COMMON)
        profile->handleCommonSettings(name, atts);

    else if (profile->mCurrentComponent == PROFILE_COMPONENT_POLICY)
        profile->handlePolicySettings(name, atts);

    else if (profile->mCurrentComponent == PROFILE_COMPONENT_DISPLAY)
        profile->handleDisplaySettings(name, atts);

    else if (profile->mCurrentComponent == PROFILE_COMPONENT_FILTER)
        profile->handleFilterSettings(name, atts);

    else if (profile->mCurrentComponent == PROFILE_COMPONENT_SFC)
        profile->handleSFCSettings(name, atts);

    else
        ALOGE("%s:Could NOT handle the element\"%s\"\n", __func__, name);
}

void ISVProfile::endElement(void *userData, const char *name)
{
    ISVProfile *profile = (ISVProfile *)userData;

    if (!strcmp(name, "IntelSmartVideoSettings") ||
        !strcmp(name, "CommonSettings") ||
        !strcmp(name, "PolicySettings") ||
        !strcmp(name, "DisplaySettings") ||
        !strcmp(name, "FilterSettings"))
        profile->mCurrentComponent = PROFILE_COMPONENT_START;
}

void ISVProfile::getDataFromXmlFile()
{
    int done;
    void *pBuf = NULL;
    FILE *fp = NULL;

    fp = ::fopen(DEFAULT_XML_FILE, "r");
    if (NULL == fp) {
        ALOGE("@%s, line:%d, couldn't open profile %s\n", __func__, __LINE__, DEFAULT_XML_FILE);
        return;
    }

    XML_Parser parser = ::XML_ParserCreate(NULL);
    if (NULL == parser) {
        ALOGE("@%s, line:%d, parser is NULL", __func__, __LINE__);
        goto exit;
    }
    ::XML_SetUserData(parser, this);
    ::XML_SetElementHandler(parser, startElement, endElement);

    pBuf = malloc(MAX_BUF_SIZE);
    if (NULL == pBuf) {
        ALOGE("@%s, line:%d, failed to malloc buffer", __func__, __LINE__);
        goto exit;
    }

    do {
        int len = (int)::fread(pBuf, 1, MAX_BUF_SIZE, fp);
        if (!len) {
            if (ferror(fp)) {
                clearerr(fp);
                goto exit;
            }
        }
        done = len < MAX_BUF_SIZE;
        if (XML_Parse(parser, (const char *)pBuf, len, done) == XML_STATUS_ERROR) {
            ALOGE("@%s, line:%d, XML_Parse error", __func__, __LINE__);
            goto exit;
        }
    } while (!done);

exit:
    if (parser)
        ::XML_ParserFree(parser);
    if (pBuf)
        free(pBuf);
    if (fp)
    ::fclose(fp);
}

unsigned long ISVProfile::getGlobalStatus()
{
    char *endptr;
    char buf[PROPERTY_VALUE_MAX];

    property_get("persist.sys.isv_status", buf, 0);
    unsigned long status = strtoul(buf, &endptr, 16);

    // Per the manpage if buf is not '\0' and
    // endptr is '\0' then the whole string is
    // valid
    if (!(*buf != '\0' && *endptr == '\0')) {
        return 0;
    }
    return status;
}

void ISVProfile::dumpConfigData()
{
    uint32_t i, j, k;
    char filterNames[][50] = {
        "ProcFilterNone",
        "ProcFilterNoiseReduction",
        "ProcFilterDeinterlacing",
        "ProcFilterSharpening",
        "ProcFilterColorBalance",
        "ProcFilterDeblocking",
        "ProcFilterFrameRateConversion",
        "ProcFilterSkinToneEnhancement",
        "ProcFilterTotalColorCorrection",
        "ProcFilterNonLinearAnamorphicScaling",
        "ProcFilterImageStabilization"
    };
    char settingsNames[][50] = {
        "ISV_SETTING_NONE",
        "ISV_COMMON_ENABLED",
        "ISV_COMMON_DEFAULT_VPP",
        "ISV_COMMON_DEFAULT_FRC",
        "ISV_COMMON_MAX_RESOLUTION",
        "ISV_DISPLAY_HDMI_AUTO",
        "ISV_DISPLAY_MIPI_AUTO",
        "ISV_SFC_ENABLED",
        "ISV_POLICY_SCENE_NUM",
        "ISV_POLICY_RULE_NUM",
        "ISV_POLICY_FRC_RULE_NUM"
    };
    char rateNames[][20] = {
        "1X",
        "2X",
        "2.5X",
        "0X",
        "4X",
    };

    ALOGV("========== ISV settings:===========\n");
    for (i = 0; i < ISV_SETTING_COUNT; i++) {
        ALOGV("%s = %d\n", settingsNames[i], mSettings[i]);
    }

    ALOGV("========== Filter capability:==========\n");
    for (i = 1; i < ISVFilterCount; i++) {
        ALOGV("name=%s, enabled=%s\n",
            filterNames[i],
            (mFilterConfigs[i].enabled == true) ? "true" : "false");
        ALOGV("    minResolution=%d, maxResolution=%d\n",
            mFilterConfigs[i].minResolution,
            mFilterConfigs[i].maxResolution);
        ALOGV("    algorithm: min=%d, max=%d, default=%d, step=%d\n",
            mFilterConfigs[i].algorithm.min_algo,
            mFilterConfigs[i].algorithm.max_algo,
            mFilterConfigs[i].algorithm.default_algo,
            mFilterConfigs[i].algorithm.step);
        for (j = 0; j < mFilterConfigs[i].numberParameter; j++)
            ALOGV("    parameter:min=%f,max=%f,default=%f,step=%f\n",
                mFilterConfigs[i].parameters[j].min_value,
                mFilterConfigs[i].parameters[j].max_value,
                mFilterConfigs[i].parameters[j].default_value,
                mFilterConfigs[i].parameters[j].step);
        ALOGV("----------------------------------------------------------\n");
    }

    ALOGV("========== Policy scene:==========\n");
    for (i = 0; i < mNumPolicyScene; i++) {
        ALOGV("name=%s, priority=%d\n",
            mPolicyScenes[i].name,
            mPolicyScenes[i].priority);
        ALOGV("condition=\"%s\"\n", mPolicyScenes[i].condition);
        ALOGV("clip_min=%d, clip_max=%d\n",
            mPolicyScenes[i].minClipResolution, mPolicyScenes[i].maxClipResolution);
        ALOGV("display_min=%d, display_max=%d\n",
            mPolicyScenes[i].minDisplayResolution, mPolicyScenes[i].maxDisplayResolution);
        ALOGV("rule=%d\n", mPolicyScenes[i].ruleIndex);

        ALOGV("----------------------------------------------------------\n");
    }

    ALOGV("========== Policy rule:==========\n");
    for (i = 0; i < mNumPolicyRule; i++) {
        ALOGV("index=%d, name=\"%s\" result=\"%s\"\n",
            mPolicyRules[i].index,
	    mPolicyRules[i].name,
            mPolicyRules[i].result
            );
        for (j = 0; j < mPolicyRules[i].numFilter; j++) {
            ALOGV("    filter_name=%s, algorithm=%d\n",
                filterNames[mPolicyRules[i].filters[j].filter],
                mPolicyRules[i].filters[j].algorithm
                );
            ALOGV("    values=");

            if (mPolicyRules[i].filters[j].filter == ISVFilterColorBalance) {
                for (k = 0; k < 7; k++)
                    ALOGV("%f : ", mPolicyRules[i].filters[j].values[k]);

            } else if (mPolicyRules[i].filters[j].filter == ISVFilterTotalColorCorrection) {
                for (k = 0; k < 6; k++)
                    ALOGV("%f : ", mPolicyRules[i].filters[j].values[k]);

            } else {
                ALOGV("%f : ", mPolicyRules[i].filters[j].values[0]);
            }

            ALOGV("\n");
        }
    }
    ALOGV("========== Policy frc rule:==========\n");
    for (i = 0; i < mNumPolicyFRCRule; i++) {
        ALOGV("input_fps=%d, output_fps=%d, frc_rate=%s\n",
            mPolicyFRCRules[i].input_fps,
            mPolicyFRCRules[i].output_fps,
            rateNames[mPolicyFRCRules[i].rate]);
    }
}

} //namespace isv
} //namespace intel
