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

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.os.Bundle;
import android.util.Log;

public class ExtModeSocket extends Thread {
    private final String TAG = "ISVEXTMODE";
    private boolean mRun = true;
    private LocalServerSocket mSSocket;
    private ExtModeObserver mExt;

    public void setObserver(ExtModeObserver ext) {
        mExt = ext;
    }

    public void stopListen() {
        mRun = false;
    }

    public void run() {
        try {
            mSSocket = new LocalServerSocket(mExt.VIDEO_SERVER_SOCKET);
        } catch (IOException e) {
            Log.e(TAG, "Fail to create EXTMode local socket");
            e.printStackTrace();
            mRun = false;
        }

        while (mRun) {
            try {
                Log.i(TAG, "Wait extmode client to connect");
                LocalSocket tmp = mSSocket.accept();
                if(mRun) {
                    Log.i(TAG, "A new extmode client is connected");
                    new InteractSocket(tmp, mExt).start();
                }
            } catch (IOException e) {
                Log.e(TAG, "Fail to run EXTMode local socket");
                e.printStackTrace();
                mRun = false;
            }
        }
        if (mSSocket != null) {
            try {
                mSSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private class InteractSocket extends Thread {
        private LocalSocket pSocket;
        private ExtModeObserver pExt;

        public InteractSocket(LocalSocket socket, ExtModeObserver ext) {
            this.pSocket = socket;
            this.pExt = ext;
        }

        @Override
        public void run() {
            StringBuilder recvStr = new StringBuilder();
            InputStream input = null;
            try {
                input = pSocket.getInputStream();
                InputStreamReader inputReader = new InputStreamReader(input);
                char[] msg = new char[pExt.VIDEO_CLIENT_MSG_LENGTH];
                int readCnt = -1;
                while ((readCnt = inputReader.read(msg)) != -1) {
                    String tmpS = new String(msg, 0, readCnt);
                    recvStr.append(tmpS);
                }
                if (recvStr != null) {
                    Log.i(TAG, "Recv a extmode msg " + recvStr.toString());
                    pExt.handleVideoEvent(recvStr);
                }
            } catch (IOException e) {
                Log.e(TAG, "Read socket data error...");
                e.printStackTrace();
            }

            finally {
                if (input != null) {
                    try {
                        input.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }
}
