#ifndef __ISVEXTMODEBASEON_PRIVATE_SERVICE_H__
#define __ISVEXTMODEBASEON_PRIVATE_SERVICE_H__

#include <display/MultiDisplayService.h>
#include <display/MultiDisplayType.h>
#include <display/IMultiDisplayVideoControl.h>

#include "ISVDisplay.h"

namespace intel {
namespace isv {

using namespace android;
using namespace android::intel;

class ISVExtModeBaseOnPrivateService : public ISVDisplay::IISVExtMode {
private:
    int  mSessionId;
    bool mState;
    bool mFPSetted;
    sp<IMultiDisplayVideoControl> mVideo;
    sp<IMDService> getService();
    void close();

public:
    ISVExtModeBaseOnPrivateService();

    ~ISVExtModeBaseOnPrivateService();

    status_t getDefaultExtModeState(bool* hasHDMIAutoSet, bool* hasMIPIAutOff) {
            *hasHDMIAutoSet = true;
            *hasMIPIAutOff  = true;
            return NO_ERROR;
    }

    status_t setRealExtModeState(bool hasHDMIAutoSet, bool hasMIPIAutOff) {
            ISV_UNUSED(hasHDMIAutoSet);
            ISV_UNUSED(hasMIPIAutOff);
            return NO_ERROR;
    };

    status_t setInputState(bool isActive) {
        ISV_UNUSED(isActive);
        return UNKNOWN_ERROR;
    }

    status_t setVideoFPS(int64_t videoHandler, int32_t FPS);

    status_t setVideoState(int64_t videoHandler, bool isPlaying);

    status_t setPhoneCallState(bool isCalling) {
        ISV_UNUSED(isCalling);
        return UNKNOWN_ERROR;
    }
};

} //namespace intel
} //namespace isv

#endif //__ISVEXTMODEBASEON_PRIVATE_SERVICE_H__
