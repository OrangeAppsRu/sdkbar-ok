//
//  OkSdk.h
//
//  Created by Vasiliy on 17.04.17.
//
//

#ifndef OkSdk_h
#define OkSdk_h

#include "base/ccConfig.h"
#include "jsapi.h"
#include "jsfriendapi.h"
#include "platform/android/jni/JniHelper.h"
#include <jni.h>

void register_all_oksdk_framework(JSContext* cx, JS::HandleObject obj);

extern "C"
{
    void Java_org_cocos2dx_javascript_OkPlugin_loginResult(JNIEnv* env, jobject thiz, jint callbackId, jstring token);

	void Java_org_cocos2dx_javascript_OkPlugin_requestResult(JNIEnv* env, jobject thiz, jint callbackId, jstring err, jstring result);
};

#endif /* OkSdk_h */
