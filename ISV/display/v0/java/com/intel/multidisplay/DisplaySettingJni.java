
package com.intel.multidisplay;

public class DisplaySettingJni {

    static {
        System.loadLibrary("multidisplayjni");
    }

    native boolean native_InitMDSClient(DisplaySetting thiz);
    native boolean native_DeInitMDSClient();
    native int     native_getMode();
    native int     native_getHdmiTiming(int width[],
                             int height[], int refresh[],
                             int interlace[], int ratio[]);
    native boolean native_setHdmiTiming(
             int width, int height,
             int refresh, int interlace, int ratio);
    native int     native_getHdmiInfoCount();
    native boolean native_setHdmiScaleType(int Type);
    native boolean native_setHdmiOverscan(int h, int v);
    native int     native_updatePhoneCallState(boolean state);
    native int     native_updateInputState(boolean state);
}
