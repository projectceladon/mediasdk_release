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


//#define LOG_NDEBUG 0
#include "JNIHelp.h"
#include "jni.h"
#include <android_runtime/AndroidRuntime.h>

#include <utils/Log.h>
#include <ISVDisplay.h>

using namespace android;
using namespace intel::isv;

#define CLASS_PATH_NAME  "com/intel/extmode/ExtModeJni"

static sp<ISVDisplay> pdpy = NULL;

static jboolean ExtMode_getDpyState(
        JNIEnv* env, jobject obj, jint dpyID)
{
    ISV_UNUSED(env);
    ISV_UNUSED(obj);
    if (pdpy == NULL)
        pdpy = new ISVDisplay();
    if (pdpy == NULL)
        return false;
    bool connected = false;
    if (pdpy->getDpyState(dpyID, &connected) != NO_ERROR) {
        return false;
    }
    return connected;
}

static jboolean ExtMode_updatePhoneCallState(
        JNIEnv* env, jobject obj, jboolean state)
{
    ISV_UNUSED(env);
    ISV_UNUSED(obj);
    if (pdpy == NULL)
        pdpy = new ISVDisplay();
    if (pdpy == NULL || pdpy->mExtControl == NULL)
        return false;
    if (pdpy->mExtControl->setPhoneCallState(state) != NO_ERROR) {
        return false;
    }
    return true;
}

static jboolean ExtMode_updateInputState(
        JNIEnv* env, jobject obj, jboolean state)
{
    ISV_UNUSED(env);
    ISV_UNUSED(obj);
    if (pdpy == NULL)
        pdpy = new ISVDisplay();
    if (pdpy == NULL || pdpy->mExtControl == NULL)
        return false;
    if (pdpy->mExtControl->setInputState(state) != NO_ERROR) {
        return false;
    }
    return true;
}

static jboolean ExtMode_dpyHotplug(
        JNIEnv* env, jobject obj, jint dpyID, jboolean state)
{
    ISV_UNUSED(env);
    ISV_UNUSED(obj);
    ISV_UNUSED(dpyID);
    ISV_UNUSED(state);
    return true;
}

static JNINativeMethod gMethods[] = {
    /* name, signature, funcPtr */
    {"native_getDpyState", "(I)Z", (void*)ExtMode_getDpyState},
    {"native_updatePhoneCallState", "(Z)Z", (void*)ExtMode_updatePhoneCallState},
    {"native_updateInputState", "(Z)Z", (void*)ExtMode_updateInputState},
    {"native_dpyHotplug", "(IZ)Z", (void*)ExtMode_dpyHotplug},
};

int JNI_OnLoad(JavaVM* vm, void* reserved)
{
    ISV_UNUSED(reserved);
    pdpy = NULL;
    JNIEnv *env = NULL;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }

    jclass cls = env->FindClass(CLASS_PATH_NAME);
    if (env->RegisterNatives(cls, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) < 0) {
        return -1;
    }
    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    ISV_UNUSED(reserved);
    pdpy = NULL;
    JNIEnv *env = NULL;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return;
    }
    return;
}
