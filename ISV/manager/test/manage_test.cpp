#include <stdlib.h>
#include <stdio.h>
#include <utils/Vector.h>
#include "ISVManager.h"
#include "manager.h"
#include "ISVProfile.h"

using namespace intel::isv;
using namespace android;


char feature_name[7][64] = {
    "ISV_VP",
    "ISV_FRC",
    "ISV_DISP_AUTOHDMI",
    "ISV_DISP_EXTMODE",
    "ISV_DISP_WIDI",
    "ISV_SFC",
    "ISV_FEATURE_MAX"
};

char cStrStatus[3][64] = {"unknown", "on", "off" };

int main(int argc, char** argv)
{
	uint32_t value = 0xFFFF;

    class IISVPolicyManager *isvManager = NULL;

    isvManager = (IISVPolicyManager *) new ISVPolicyManager; 
    if (isvManager != NULL) {
        isvManager->init();
    }

    status_t status = OK;
    status = isvManager->setClipInfo(1280, 720, 25);
    if (status == BAD_VALUE)
        ALOGW("Error\n");

    ISVFeatureStatus inputFeatureStatus[ISV_FEATURE_MAX];
    ISVFeatureStatus outputFeatureStatus[ISV_FEATURE_MAX];

    //clear feature status as default. It's safe to set default as unknown. If you do not set it
    // It maybe on, maybe off. 
    //clear all feature
    //memset(inputFeatureStatus, 0,  sizeof(ISVFeatureStatus) * ISV_MAX_COUNT);
    for (int32_t i = 0; i < ISV_FEATURE_MAX; i++) {
        inputFeatureStatus[i].feature = ISV_FEATURE_NONE;
        inputFeatureStatus[i].status = UNKNOWN;
    }

    memset(outputFeatureStatus, 0,  sizeof(ISVFeatureStatus) * ISV_FEATURE_MAX);
    inputFeatureStatus[0].feature = ISV_VP;
    inputFeatureStatus[0].status = OFF;

    inputFeatureStatus[1].feature = ISV_DISP_WIDI;
    inputFeatureStatus[1].status = ON;
    
    //Just to keep input and output interface the same parameters.
    status = isvManager->setFeatureStatus(inputFeatureStatus);
    if (status == BAD_VALUE)
        ALOGW("Error line %d\n", __LINE__);

    //Vector <DisplayInfo> *DisplayInfo;
    DisplayInfo ActivDisplayInfo;
    Vector<DisplayInfo> DispInfo;
    //DisplayInfo *DispInfo = NULL;

    memset(&ActivDisplayInfo, 0, sizeof(DisplayInfo));
    DispInfo.clear();
    ActivDisplayInfo.w = 1280;
    ActivDisplayInfo.h = 720;
    ActivDisplayInfo.fps = 25;

    //read displayInf here ...
    //Set display Info to Manager 
    status = isvManager->setDisplayInfo(&ActivDisplayInfo, &DispInfo);
    if (status == BAD_VALUE)
        ALOGW("Error line %d\n", __LINE__);

    //Get Feature status VPP on/off, FRC on/off etc.
    isvManager->getFeatureStatus(outputFeatureStatus);

    for (int32_t i = 0; i < ISV_FEATURE_MAX; i++) {
        if (outputFeatureStatus[i].feature >= 0 && outputFeatureStatus[i].feature < 7
             && outputFeatureStatus[i].status >= 0 && outputFeatureStatus[i].status <= 2) {
            ALOGI("policy get feature feature %d %s, status %s \n",
                    outputFeatureStatus[i].feature, feature_name[outputFeatureStatus[i].feature], cStrStatus[outputFeatureStatus[i].status]);
        } else {
            ALOGI("policy get feature feature %d, status %d \n",
                    outputFeatureStatus[i].feature, outputFeatureStatus[i].status);
        }
    }

    ISVFilterParameter filters[ISVFilterCount];
    uint32_t filterCountUsed;
    //Get VP filter setting
    status = isvManager->getVpPolicySetting(filters, filterCountUsed);
    if (status == BAD_VALUE)
        ALOGW("Error line %d\n", __LINE__);

    ALOGI("Filters setting  filterCountUsed %d", filterCountUsed);
    for (int32_t i = 0; i < filterCountUsed; i++) {
        for (int32_t k = 0; k < MAX_FILTER_PARAMETERS; k++) {
            ALOGI("Filters setting  %.2f", filters[i].values[k]);
        }
    }

    //Get autoHdmi refresh rate set in manager
    int32_t hdmiRefereshRate;
    status = isvManager->getDisplayPolicySeting(hdmiRefereshRate);
    if (status == BAD_VALUE)
        ALOGW("Error line %d\n", __LINE__);

    isvManager->deinit();

	return 0;
}

