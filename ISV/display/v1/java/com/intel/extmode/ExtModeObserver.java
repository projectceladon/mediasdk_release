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
 */

package com.intel.extmode;

import java.util.Vector;
import android.view.Display;
import android.hardware.display.DisplayManager;
import android.hardware.display.DisplayManager.DisplayListener;
import android.hardware.display.WifiDisplayStatus;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.IBinder;
import android.os.ServiceManager;
import android.telephony.TelephonyManager;
import android.view.MotionEvent;
import android.view.WindowManagerPolicy;
import android.view.WindowManagerPolicy.PointerEventListener;
import android.util.Slog;

public class ExtModeObserver extends IExtModeObserver.Stub {
    public  static final String EXTMODE_OBSERVER = "com.intel.extmode.ExtModeObserver";
    public  static final int VIDEO_CLIENT_MSG_LENGTH = 128;
    public  static final String VIDEO_SERVER_SOCKET = "intel_extmode";
    private static final String TAG = "ISVEXTMODE";
    private static final boolean LOG = true;

    private static ExtModeObserver mService;
    private final DisplayManager mDpyM;
    private static int mBuiltInDpyLogicID = 0;
    private static int mExtDpyLogicID = 1;
    private static int mWifiDpyLogicID = 0;

    // indicate HDMI/WIFI Display device connect state
    private boolean mHdmiConnected = false;
    private boolean mWifiConnected = false;

    private Context mContext;
    private WakeLock mWakeLock;  // held while there is a pending route change
    private boolean mHasIncomingCall = false;
    private boolean mInCallScreenFinished = true;
    private ExtMode mExtMode;
    private ExtModeSocket mExtModeSocket;


    //Message need to handle
    private final int MSG_INPUT_TIMEOUT = 1;
    private final int MSG_START_MONITORING_INPUT = 2;
    private final int MSG_STOP_MONITORING_INPUT = 3;
    private final int INPUT_TIMEOUT_MSEC = 5000;

    private class VideoInstance {
        private String pID;
        private boolean pPlaying;

        public VideoInstance(String ID, boolean state) {
            pID = ID;
            pPlaying = state;
        }

        String getID() {
            return pID;
        }

        boolean getState() {
            return pPlaying;
        }
    }

    private Vector<VideoInstance> mVIList;

    private void dumpVideoInstance() {
        if (mVIList.size() <= 0) {
            logv("No valid Video Playback Instance");
            return;
        }
        for (int i = 0;i < mVIList.size(); i++) {
            VideoInstance vi = mVIList.get(i);
            if (vi != null)
                logv("Video Playback Instance[" + i + "] ID:" + vi.getID() + ",Status:" + vi.getState());
            else
                logv("Warning!!! Video Playback Instance[" + i + "] is invalid");
        }
    }

    private boolean hasVideoPlaying() {
        for (int i = 0;i < mVIList.size(); i++) {
            VideoInstance vi = mVIList.get(i);
            if (vi != null && vi.getState())
                return true;
        }
        return false;
    }

    private VideoInstance getVideoInstance(String sessionID) {
        for (int i = 0;i < mVIList.size(); i++) {
            VideoInstance vi = mVIList.get(i);
            if (vi != null && vi.getID().equals(sessionID))
                return vi;
        }
        return null;
    }

    // Broadcast receiver for device connections intent broadcasts
    private final BroadcastReceiver mReceiver = new ExtModeObserverBroadcastReceiver();

    private final class ExtModeObserverTouchEventListener implements PointerEventListener {
        private ExtModeObserver mObserver;

        public ExtModeObserverTouchEventListener(ExtModeObserver observer) {
            mObserver = observer;
        }

        @Override
        public void onPointerEvent(MotionEvent motionEvent) {
            if (motionEvent.getAction() == motionEvent.ACTION_CANCEL) {
                return;
            }
            mObserver.onInputEvent();
        }
    }

    private ExtModeObserverTouchEventListener mTouchEventListener = null;
    private WindowManagerPolicy.WindowManagerFuncs mWindowManagerFuncs;

    public void onInputEvent() {
        //logv("input is active");
        mExtMode.updateInputState(true);
        if (mHandler.hasMessages(MSG_INPUT_TIMEOUT))
            mHandler.removeMessages(MSG_INPUT_TIMEOUT);

        mHandler.sendEmptyMessageDelayed(MSG_INPUT_TIMEOUT, INPUT_TIMEOUT_MSEC);
    }

    private void startMonitoringInput() {
        if (mTouchEventListener != null) {
            return;
        }
        if (mWindowManagerFuncs == null) {
            logv("Invalid WindowManagerFuncs");
            return;
        }

        logv("Start monitoring input");
        mTouchEventListener =
            new ExtModeObserverTouchEventListener(this);
        if (mTouchEventListener == null) {
           logv("Invalid input event receiver.");
           return;
        }
        mWindowManagerFuncs.registerPointerEventListener(
                mTouchEventListener);

        // start "input idle" count down
        mHandler.sendEmptyMessageDelayed(MSG_INPUT_TIMEOUT, INPUT_TIMEOUT_MSEC);
    }

    private void stopMonitoringInput() {
        if (mWindowManagerFuncs == null
                || mTouchEventListener == null)
            return;

        logv("Stop monitoring input");
        mWindowManagerFuncs.unregisterPointerEventListener(
                mTouchEventListener);
        mTouchEventListener = null;

        if (mHandler.hasMessages(MSG_INPUT_TIMEOUT)) {
            mHandler.removeMessages(MSG_INPUT_TIMEOUT);
        }
    }

    public static ExtModeObserver main(Context cxt,
            WindowManagerPolicy.WindowManagerFuncs funcs) {
        if (mService != null) {
            return mService;
        }
        if (funcs == null) {
            Slog.w(TAG, "Invalid WindowManagerFuns, Video-ExtMode Observer couldn't handle input envent");
        }
        mService = new ExtModeObserver(cxt, funcs);
        try {
            Slog.v(TAG, "Create Video-ExtMode Observer");
            ServiceManager.addService(EXTMODE_OBSERVER, mService);
        } catch (Throwable e) {
            Slog.e(TAG, "Fail to create Video-ExtMode Observer ");
            e.printStackTrace();
        }
        return mService;
    }

    public static IExtModeObserver getService() {
        IBinder b = ServiceManager.getService(EXTMODE_OBSERVER);
        if (b == null)
            return null;
        return IExtModeObserver.Stub.asInterface(b);
    }

    private ExtModeObserver(Context context,
            WindowManagerPolicy.WindowManagerFuncs funcs) {
        mContext = context;
        mDpyM = (DisplayManager)context.getSystemService(Context.DISPLAY_SERVICE);
        mDpyM.registerDisplayListener(mDisplayListener, null);
        mWindowManagerFuncs = funcs;
        mExtMode = new ExtMode();
        IntentFilter intentFilter = new IntentFilter(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        intentFilter.addAction(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED);

        mContext.registerReceiver(mReceiver, intentFilter);
        PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "ExtModeObserver");
        mWakeLock.setReferenceCounted(false);
        mVIList = new Vector<VideoInstance>();
        mVIList.clear();
        mExtModeSocket = new ExtModeSocket();
        mExtModeSocket.setObserver(this);
        mExtModeSocket.start();
        mHdmiConnected = mExtMode.getDpyState(mExtMode.DISPLAY_EXTERNAL);
        logv("External Display is connected: " + mHdmiConnected);
    }

    protected void finalize() throws Throwable {
        mExtModeSocket.stopListen();
        mDpyM.unregisterDisplayListener(mDisplayListener);
        stopMonitoringInput();
        mContext.unregisterReceiver(mReceiver);
        super.finalize();
    }

    private void logv(String s) {
        if (LOG) {
            Slog.i(TAG, s);
        }
    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            //logv("Handle ExtMode Input message " + msg.what);
            switch(msg.what) {
            case MSG_INPUT_TIMEOUT:
                //logv("Input is idle, stop Input monitoring");
                mExtMode.updateInputState(false);
                break;
            case MSG_START_MONITORING_INPUT:
                startMonitoringInput();
                break;
            case MSG_STOP_MONITORING_INPUT:
                stopMonitoringInput();
                break;
            }
        }
    };

    private class ExtModeObserverBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null)
                return;
            if (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                if (TelephonyManager.EXTRA_STATE == null
                        || TelephonyManager.EXTRA_STATE_RINGING == null)
                    return;
                String extras = intent.getStringExtra(TelephonyManager.EXTRA_STATE);
                if (extras == null)
                    return;
                if (extras.equals(TelephonyManager.EXTRA_STATE_RINGING)) {
                    mHasIncomingCall = true;
                    mInCallScreenFinished = false;
                    //logv("Incoming call is initiated");
                    if (!hasVideoPlaying())
                        return;
                    mExtMode.updatePhoneCallState(true);
                } else if (extras.equals(TelephonyManager.EXTRA_STATE_IDLE)) {
                    mHasIncomingCall = false;
                    mInCallScreenFinished = true;
                    //logv("Phonecall is terminated and Incallscreen disappeared");
                    if (!hasVideoPlaying())
                        return;
                    mExtMode.updatePhoneCallState(false);
                }
            } else if(action.equals(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED)) {
                boolean connected = false;
                WifiDisplayStatus status = (WifiDisplayStatus)intent.getParcelableExtra(
                        DisplayManager.EXTRA_WIFI_DISPLAY_STATUS);
                if (status == null)
                    return;
                if (status.getActiveDisplayState()
                        == WifiDisplayStatus.DISPLAY_STATE_CONNECTED) {
                    connected = true;
                }
                if (mWifiConnected == connected)
                    return;
                //logv("Update WIFI Display status " + connected);
                mExtMode.updateDpyState(mExtMode.DISPLAY_VIRTUAL, connected);
                mWifiConnected = connected;
            }
        }
    }

    public void handleVideoEvent(StringBuilder msg) {
        String seperator = new String(":");
        int seperatorIndex = msg.indexOf(seperator);
        //logv("The Sepaerator Index is " + seperatorIndex);
        String sessionID = msg.substring(0, seperatorIndex);
        String playStatus = msg.substring(seperatorIndex + 1, seperatorIndex + 2);
        String playFlag = new String("1");
        //logv("Video Session ID is " + sessionID + ", Status is " + playStatus);
        VideoInstance vi = getVideoInstance(sessionID);
        boolean playing = playStatus.equals(playFlag);
        if (vi != null && vi.getState() == playing)
            return;
        if (playing) {
            VideoInstance tmp = new VideoInstance(sessionID, playing);
            mVIList.add(tmp);
            if (mHdmiConnected || mWifiConnected)
                mHandler.sendEmptyMessage(MSG_START_MONITORING_INPUT);
        } else {
            VideoInstance tmp = getVideoInstance(sessionID);
            if (tmp != null)
                mVIList.remove(tmp);
            if (mVIList.size() == 0) {
                mExtMode.updateInputState(true);
                mHandler.sendEmptyMessage(MSG_STOP_MONITORING_INPUT);
            }
        }
        dumpVideoInstance();
    }

    private final DisplayListener mDisplayListener = new DisplayListener() {
        //@override
        public void onDisplayAdded(int displayId) {
            //logv("new added Display ID is " + displayId);
            boolean resetInputState = false;
            Display dsp = mDpyM.getDisplay(displayId);
            if (dsp == null)
                return;
            //logv(dsp.toString());
            int type = dsp.getType();
            //logv("new added Display Type is " + type);
            if (type == Display.TYPE_HDMI) {
                //logv("HDMI Display is added");
                mExtDpyLogicID = displayId;
                mHdmiConnected = true;
                resetInputState = true;
            } else if (type == Display.TYPE_WIFI || type == Display.TYPE_VIRTUAL) {
                //logv("WIFI Display is added");
                mWifiDpyLogicID = displayId;
                mWifiConnected = true;
                resetInputState = true;
            }
            if (resetInputState && mVIList.size() > 0) {
                mExtMode.updateInputState(true);
                mHandler.sendEmptyMessage(MSG_START_MONITORING_INPUT);
            }
        }
        //@override
        public void onDisplayChanged(int displayId) {
            ;
        }
        //@override
        public void onDisplayRemoved(int displayId) {
            //logv("Removed Display ID is " + displayId);
            boolean resetInputState = false;
            if ((displayId == mWifiDpyLogicID) && mWifiConnected) {
                mWifiConnected = false;
                resetInputState = true;
                //logv("WIFI Display is removed");
            } else if ((displayId == mExtDpyLogicID) && mHdmiConnected) {
                mHdmiConnected = false;
                resetInputState = true;
                //logv("HDMI Display is removed");
            }
            if (resetInputState) {
                mExtMode.updateInputState(true);
                mHandler.sendEmptyMessage(MSG_STOP_MONITORING_INPUT);
            }
        }
    };
}
