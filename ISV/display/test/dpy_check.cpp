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

#include <stdio.h>
#include <ISVDisplay.h>

using namespace android;
using namespace intel::isv;

void dumpDpyInfo(DisplayInfo* info) {
    if (info == NULL)
        return;
    printf("Info: [%dx%d@%dHz]--[%d:%d:%d:%d:%d]\n",
            info->w, info->h, (int32_t)(info->fps),
            (int32_t)(info->xdpi), (int32_t)(info->ydpi),
            (int32_t)(info->density),
            info->secure, info->orientation);
}

int main(int argc, char** argv)
{
    sp<ISVDisplay> dpy = new ISVDisplay();
    for (int i = 0; i < HWC_NUM_DISPLAY_TYPES; i++) {
        bool connected = false;
        // check display connected state
        dpy->getDpyState(i, &connected);
        if (!connected) {
            printf("++++++++++\n");
            printf("Display[%d] is NOT connected\n", i);
            printf("++++++++++\n");
        } else {
            DisplayInfo active;
            memset(&active, 0, sizeof(active));
            // check display actvie config
            dpy->getActiveDpyInfo(i, &active);
            printf("****Dump active config begin******\n");
            printf("Display[%d] is connected, active config is:\n", i);
            dumpDpyInfo(&active);
            printf("****Dump active config end******\n\n");
            Vector<DisplayInfo> info;
            // check disply all configs
            dpy->getDpyInfo(i, &info);
            printf("=====Dump all configs begin=====\n");
            printf("Display[%d] has %d configs:\n", i, info.size());
            for (int j = 0; j < info.size(); j++) {
                DisplayInfo tmp = info.itemAt(j);
                dumpDpyInfo(&tmp);
            }
            printf("=====Dump all configs end=====\n\n");
        }
    }
    dpy = NULL;
    return 0;
}
