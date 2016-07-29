/*
 * Copyright (c) 2012-2013, Intel Corporation. All rights reserved.
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
 * Author: tianyang.zhu@intel.com
 */

package com.intel.multidisplay;

import java.io.IOException;
import android.util.Slog;


public class DisplaySetting {
    private final String TAG = "ISVDPY";
    private final boolean LOG = true;

    // mode setting policy
    public static final int HDMI_ON_NOT_ALLOWED  = 0;
    public static final int HDMI_ON_ALLOWED = 1;

    /// Don't change the enum, pls align with MultiDisplayType.h

    /// Phone call state
    public static final int PHONE_STATE_OFF = 0;
    public static final int PHONE_STATE_ON  = 1;

    /// MDS display mode
    public static final int DVI_CONNECTED_BIT   = 1;
    public static final int HDMI_CONNECTED_BIT  = 1 << 1;
    public static final int WIDI_CONNECTED_BIT  = 1 << 2;
    public static final int VIDEO_ON_BIT        = 1 << 3;

    /// MDS message type
    public static final int MDS_MSG_MODE_CHANGE = 1 << 1;
    public static final int MDS_MSG_HOT_PLUG    = 1 << 2;

    /// MDS display capability
    public static final int DISPLAY_PRIMARY  = 0;
    public static final int DISPLAY_EXTERNAL = 1;
    public static final int DISPLAY_VIRTUAL  = 2;

    /// External display device type
    public static final int EDP_HDMI    = 1;
    public static final int EDP_DVI     = 2;

    public static final String MDS_EDP_HOTPLUG         = "android.intel.mds.EXTERNAL_DP_HOTPLUG";
    public static final String MDS_GET_HDMI_INFO       = "android.intel.mds.GET.HDMI_INFO";
    public static final String MDS_SET_HDMI_MODE       = "android.intel.mds.SET.HDMI_MODE";
    public static final String MDS_HDMI_INFO           = "android.intel.mds.HDMI_INFO";
    public static final String MDS_SET_HDMI_SCALE      = "android.intel.mds.SET.HDMI_SCALE";
    public static final String MDS_SET_HDMI_STEP_SCALE = "android.intel.mds.SET.HDMI_STEP_SCALE";
    public static final String MDS_GET_BOOT_STATUS     = "android.intel.mds.GET.BOOT_STATUS";
    public static final String MDS_BOOT_STATUS         = "android.intel.mds.BOOT_STATUS";
    public static final String MDS_ALLOW_MODE_SET      = "android.intel.mds.ALLOW_MODE_SET";

    private onMdsMessageListener mListener = null;

    private final DisplaySettingJni mJni;

    public DisplaySetting() {
        if (LOG) Slog.i(TAG, "Create a new DisplaySetting");
        mJni = new DisplaySettingJni();
        mJni.native_InitMDSClient(this);
    }

    @Override
    protected void finalize() {
        if (mJni != null) {
            mJni.native_DeInitMDSClient();
        }
    }

    public int getMode() {
        return mJni.native_getMode();
    }

    public void onMdsMessage(int event, int value) {
        if (mListener != null) {
            mListener.onMdsMessage(event, value);
        }
    }

    public void setMdsMessageListener(onMdsMessageListener listener) {
        if (mListener == null) {
            mListener = listener;
        }
    }

    public int getHdmiTiming(int width[], int height[],
                             int refresh[], int interlace[], int ratio[]) {
        return mJni.native_getHdmiTiming(width, height,
                                    refresh, interlace, ratio);
    }

    public boolean setHdmiTiming(int width, int height,
                        int refresh, int interlace, int ratio) {
        return mJni.native_setHdmiTiming(width, height,
                            refresh, interlace, ratio);
    }

    public interface onMdsMessageListener {
        boolean onMdsMessage(int event, int value);
    }

    public int getHdmiInfoCount() {
        return mJni.native_getHdmiInfoCount();
    }

    public boolean setHdmiScaleType(int Type) {
        return mJni.native_setHdmiScaleType(Type);
    }

    public boolean setHdmiOverscan(int hValue, int vValue) {
        return mJni.native_setHdmiOverscan(hValue, vValue);
    }

    public int updatePhoneCallState(boolean phoneState) {
        return mJni.native_updatePhoneCallState(phoneState);
    }

    public int updateInputState(boolean inputState) {
        return mJni.native_updateInputState(inputState);
    }

}

