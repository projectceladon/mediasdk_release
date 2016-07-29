#ifndef __ISVEXTMODEBASEON_HWC_SERVICE_H__
#define __ISVEXTMODEBASEON_HWC_SERVICE_H__

#include <utils/Errors.h>
#include <hardware/hwcomposer_defs.h>
#include <ui/DisplayInfo.h>
#include "ISVDisplay.h"


namespace intel {
namespace isv {

using namespace android;

class ISVExtModeBaseOnHwcService : public ISVDisplay::IISVExtMode {
private:
    bool mHdmiAutoSet;
    bool mMipiAutOff;
    int mState;
    bool mFPSetted;
    status_t informObserver(int64_t videoSessionID, bool isPlaying);

public:
    ISVExtModeBaseOnHwcService();

    ~ISVExtModeBaseOnHwcService();

    status_t getDefaultExtModeState(bool* hasHDMIAutoSet, bool* hasMIPIAutOff);

    status_t setRealExtModeState(bool hasHDMIAutoSet, bool hasMIPIAutOff);

    status_t setInputState(bool isActive);

    status_t setVideoFPS(int64_t videoHandler, int32_t FPS);

    status_t setVideoState(int64_t videoHandler, bool isPlaying);
    status_t setPhoneCallState(bool isCalling);
};

} //namespace intel
} //namespace isv

#endif //__ISVEXTMODEBASEON_HWC_SERVICE_H__
