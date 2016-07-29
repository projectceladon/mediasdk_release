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

#ifndef __ISVDISPLAY_H__
#define __ISVDISPLAY_H__

#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <hardware/hwcomposer_defs.h>
#include <ui/DisplayInfo.h>
#include "ISVType.h"


namespace intel {
namespace isv {

using namespace android;

class ISVDisplay : public RefBase {
public:
    class IISVExtMode {
    public:
        virtual ~IISVExtMode() { };
        /**
         * @brief get default video ext mode state from ISV profile
         * @param
         *      bool& hasHDMIAutoSet: "true" means feature
         *          "HDMI refresh rate Auto settting" is enabled
         *      bool& hasMIPIAutOff: "true" means feature
         *          "MIPI backlight Auto on/off" is enabled
         * @return "OK" indicates it work fine, @see status_t in <utils/Errors.h>
         */
        virtual status_t getDefaultExtModeState(bool* hasHDMIAutoSet, bool* hasMIPIAutOff) = 0;
        /**
         * @brief set real video ext mode state
         * @param
         *      bool hasHDMIAutoSet: "true" means feature
         *          "HDMI refresh rate Auto settting" is enabled
         *      bool hasMIPIAutOff: "true" means feature
         *          "MIPI backlight Auto on/off" is enabled
         * @return "OK" indicates it work fine, @see status_t in <utils/Errors.h>
         */
        virtual status_t setRealExtModeState(bool hasHDMIAutoSet, bool hasMIPIAutOff) = 0;

        /**
         * @brief set input event state
         * @param
         *      bool isActive: "true" means there is a touch event
         * @return "OK" indicates it work fine, @see status_t in <utils/Errors.h>
         */
        virtual status_t setInputState(bool isActive)  = 0;
        /**
         * @brief set video playback FPS
         * @param
         *      int32_t videoHandler: an unique ID of current video playback instance
         *      int32_t FPS: video clip's fps
         * @return "OK" indicates it work fine, @see status_t in <utils/Errors.h>
         */
        virtual status_t setVideoFPS(int64_t videoHandler, int32_t FPS) = 0;
        /**
         * @brief set video playback state
         * @param
         *      int32_t videoHandler: an unique ID of current video playback
         *      bool isPlaying: "true" means video is playing
         * @return "OK" indicates it work fine, @see status_t in <utils/Errors.h>
         */
        virtual status_t setVideoState(int64_t videoHandler, bool isPlaying) = 0;
        /**
         * @brief set incoming/outgoing call state
         * @param
         *      bool isCalling: a call is processing
         * @return "OK" indicates it work fine, @see status_t in <utils/Errors.h>
         */
        virtual status_t setPhoneCallState(bool isCalling) = 0;
    };

public:
    class ISVExtModeFactory {
    public:
        static IISVExtMode* getExtMode();
    };

public:
    ISVDisplay();

    ~ISVDisplay();

    IISVExtMode* mExtControl;

    /**
     * @brief get display device connected state
     * @param
     *      uint32_t dpyID: display device ID, refer <hardware/hwcomposer_def.h>
     *      bool& connetced: "true" means device is connected
     * @return @see status_t in <utils/Errors.h>
     */
    status_t getDpyState(uint32_t dpyID, bool* connected);
    /**
     * @brief get display device info which is used now
     * @param
     *      uint32_t dpyID: display device ID, refer <hardware/hwcomposer_def.h>
     *      DisplayInfo* info: refer <ui/DisplayInfo.h>
     * @return "OK" indicates it work fine, @see status_t in <utils/Errors.h>
     */
    status_t getActiveDpyInfo(uint32_t dpyID, DisplayInfo* info);
    /**
     * @brief get display info list which is supported by display device
     * @param
     *      int32_t dpyID: display device ID, refer <hardware/hwcomposer_def.h>
     *      Vector<DisplayInfo>* info: refer <ui/DisplayInfo.h>
     * @return "OK" indicates it work fine, @see status_t in <utils/Errors.h>
     */
    status_t getDpyInfo(uint32_t dpyID, Vector<DisplayInfo>* info);
private:
    void dumpDpyInfo(DisplayInfo* info);
};

} //namespace intel
} //namespace isv

#endif //__ISVDISPLAY_H__
