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

package com.intel.extmode;

import android.util.Slog;


public class ExtMode {
    private final String TAG = "EXTMODE";
    private final boolean LOG = true;
    public static final int PHONE_STATE_OFF = 0;
    public static final int PHONE_STATE_ON  = 1;
    public static final int DISPLAY_PRIMARY  = 0;
    public static final int DISPLAY_EXTERNAL = 1;
    public static final int DISPLAY_VIRTUAL  = 2;

    private final ExtModeJni mJni;

    public ExtMode() {
        if (LOG) Slog.i(TAG, "Create Video ExtMode instance");
        mJni = new ExtModeJni();
    }

    @Override
    protected void finalize() throws Throwable {
        if (LOG) Slog.i(TAG, "Finalize Video ExtMode instance");
        super.finalize();
    }

    public boolean getDpyState(int dpyID) {
        return mJni.native_getDpyState(dpyID);
    }
    public boolean updatePhoneCallState(boolean phoneState) {
        return mJni.native_updatePhoneCallState(phoneState);
    }

    public boolean updateInputState(boolean inputState) {
        return mJni.native_updateInputState(inputState);
    }

    public boolean updateDpyState(int dpyID, boolean state) {
        return mJni.native_dpyHotplug(dpyID, state);
    }
}

