//
//  OkSdk.m
//
//  Created by Vasiliy on 17.04.17.
//
//

#include "OkSdk.hpp"
#include "scripting/js-bindings/manual/cocos2d_specifics.hpp"
#include "scripting/js-bindings/manual/js_manual_conversions.h"
#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#include <sstream>
#include "base/CCDirector.h"
#include "base/CCScheduler.h"
#include "utils/PluginUtils.h"

static void cpp_loginResult(int callbackId, std::string tokenStr);
static void cpp_requestResult(int callbackId, std::string errStr, std::string resultStr);

static void printLog(const char* str) {
    CCLOG("%s", str);
    cocos2d::log("%s", str);

    /*
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/AppActivity", "printLog", "(Ljava/lang/String;)V")) {
        return;
    }
    jstring s = methodInfo.env->NewStringUTF(str);
    methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, s);
    methodInfo.env->DeleteLocalRef(s);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    */
}

static bool invokeMethod(const char* method) {
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/OkPlugin", method, "()Z")) {
        return false;
    }
    bool res = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    return res;
}

static bool invokeMethod(const char* method, int callbackId) {
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/OkPlugin", method, "(I)Z")) {
        return false;
    }
    bool res = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID, callbackId);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    return true;
}

static bool invokeMethod(const char* method, const char* param) {
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/OkPlugin", method, "(Ljava/lang/String;)Z")) {
        return false;
    }
    jstring s = methodInfo.env->NewStringUTF(param);
    bool res = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID, s);
    methodInfo.env->DeleteLocalRef(s);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    return true;
}

static bool invokeMethod(const char* method, std::vector<std::string> &param) {
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/OkPlugin", method, "([Ljava/lang/String;)Z")) {
        return false;
    }
    jobjectArray args = 0;
    args = methodInfo.env->NewObjectArray(param.size(), methodInfo.env->FindClass("java/lang/String"), 0);
    for(int i=0; i<param.size(); i++) {
        jstring s = methodInfo.env->NewStringUTF(param[i].c_str());
        methodInfo.env->SetObjectArrayElement(args, i, s);
    }
    bool res = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID, args);
    methodInfo.env->DeleteLocalRef(args);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    return true;
}


static bool invokeMethod(const char* method, const char* param, int callbackId) {
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/OkPlugin", method, "(Ljava/lang/String;I)Z")) {
        return false;
    }
    jstring s = methodInfo.env->NewStringUTF(param);
    bool res = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID, s, callbackId);
    methodInfo.env->DeleteLocalRef(s);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    return true;
}

static bool invokeMethod(const char* method, std::vector<std::string> &param, int callbackId) {
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/OkPlugin", method, "([Ljava/lang/String;I)Z")) {
        return false;
    }
    jobjectArray args = 0;
    args = methodInfo.env->NewObjectArray(param.size(), methodInfo.env->FindClass("java/lang/String"), 0);
    for(int i=0; i<param.size(); i++) {
        jstring s = methodInfo.env->NewStringUTF(param[i].c_str());
        methodInfo.env->SetObjectArrayElement(args, i, s);
    }
    bool res = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID, args, callbackId);
    methodInfo.env->DeleteLocalRef(args);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    return true;
}

static bool invokeMethod(const char* method, const char* param1, const char* param2) {
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/OkPlugin", method, "(Ljava/lang/String;Ljava/lang/String;)Z")) {
        return false;
    }
    jstring s1 = methodInfo.env->NewStringUTF(param1);
    jstring s2 = methodInfo.env->NewStringUTF(param2);
    bool res = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID, s1, s2);
    methodInfo.env->DeleteLocalRef(s2);
    methodInfo.env->DeleteLocalRef(s1);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    return true;
}

static bool invokeMethod(const char* method, const char* param1, const char* param2, int callbackId) {
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/OkPlugin", method, "(Ljava/lang/String;Ljava/lang/String;I)Z")) {
        return false;
    }
    jstring s1 = methodInfo.env->NewStringUTF(param1);
    jstring s2 = methodInfo.env->NewStringUTF(param2);
    bool res = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID, s1, s2, callbackId);
    methodInfo.env->DeleteLocalRef(s2);
    methodInfo.env->DeleteLocalRef(s1);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    return true;
}

static bool invokeMethod(const char* method, const char* param1, const char* param2, const char* param3, int callbackId) {
    cocos2d::JniMethodInfo methodInfo;

    if (! cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/javascript/OkPlugin", method, "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z")) {
        return false;
    }
    jstring s1 = methodInfo.env->NewStringUTF(param1);
    jstring s2 = methodInfo.env->NewStringUTF(param2);
    jstring s3 = methodInfo.env->NewStringUTF(param3);
    bool res = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID, s1, s2, s3, callbackId);
    methodInfo.env->DeleteLocalRef(s3);
    methodInfo.env->DeleteLocalRef(s2);
    methodInfo.env->DeleteLocalRef(s1);
    methodInfo.env->DeleteLocalRef(methodInfo.classID);
    return true;
}


/*static bool jsb_oksdk_init(JSContext *cx, uint32_t argc, jsval *vp)
{
	printLog("jsb_oksdk_init");
	JSAutoRequest rq(cx);
	JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
	JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
	js_proxy_t *proxy = jsb_get_js_proxy(obj);
	CCLOG("init, param count:%d.\n", argc);
	JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
	bool ok = true;
	if(argc == 2) {
		std::string arg0;
		JS::RootedValue arg0Val(cx, args.get(0));
		ok &= jsval_to_std_string(cx, arg0Val, &arg0);
		CCLOG("arg0: %s\n", arg0.c_str());
		std::string arg1;
		JS::RootedValue arg1Val(cx, args.get(1));
		ok &= jsval_to_std_string(cx, arg1Val, &arg1);
		CCLOG("arg1: %s\n", arg1.c_str());
		if(invokeMethod("init", arg0.c_str(), arg1.c_str())) {
			rec.rval().set(JSVAL_TRUE);
		} else {
			rec.rval().set(JSVAL_FALSE);
		}
		return true;
	} else {
		JS_ReportError(cx, "Invalid number of arguments");
		return false;
	}
}*/

static bool jsb_oksdk_login(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_login");
    JSAutoRequest rq(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    CCLOG("init, param count:%d.\n", argc);
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 3) {
        // permissions, callback, this
        CallbackFrame *loginCallback = new CallbackFrame(cx, obj, args.get(2), args.get(1));
        std::vector<std::string> arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_vector_string(cx, arg0Val, &arg0);
        if(invokeMethod("login", arg0, loginCallback->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        CCLOG("permissions size: %d\n", (int)arg0.size());
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
    return true;
}

static bool jsb_oksdk_logout(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_logout");
    JSAutoRequest rq(cx);
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    if(argc == 0) {
        if(invokeMethod("logout")) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_loggedin(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_loggedin");
    JSAutoRequest rq(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    CCLOG("init, param count:%d.\n", argc);
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    if(argc == 2) {
        // callback, this
        CallbackFrame *loginCallback = new CallbackFrame(cx, obj, args.get(1), args.get(0));

        // Access to the callee must occur before accessing/setting
        // the return value.
        JSObject &callee = rec.callee();
        rec.rval().set(JS::ObjectValue(callee));

        if(invokeMethod("isLoggedIn", loginCallback->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_users_get(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_users_get");
    JSAutoRequest rq(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 4) {
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(3), args.get(2));
        std::string arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &arg0);
        std::string arg1;
        JS::RootedValue arg1Val(cx, args.get(1));
        ok &= jsval_to_std_string(cx, arg1Val, &arg1);
        if(invokeMethod("usersGetInfo", arg0.c_str(), arg1.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_friends_get(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_friends_get");
    JSAutoRequest rq(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 4) {
        // params, callback function & this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(3), args.get(2));
        std::string arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &arg0);
        std::string arg1;
        JS::RootedValue arg1Val(cx, args.get(1));
        ok &= jsval_to_std_string(cx, arg1Val, &arg1);
        if(invokeMethod("friendsGet", arg0.c_str(), arg1.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_friends_get_online(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_friends_get_online");
    JSAutoRequest rq(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 4) {
        // params, callback function & this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(3), args.get(2));
        std::string arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &arg0);
        std::string arg1;
        JS::RootedValue arg1Val(cx, args.get(1));
        ok &= jsval_to_std_string(cx, arg1Val, &arg1);
        if(invokeMethod("friendsGetOnline", arg0.c_str(), arg1.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_share(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_share");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 4) {
        // param1, param2, callback function & this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(3), args.get(2));
        std::string arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &arg0);
        std::string arg1;
        JS::RootedValue arg1Val(cx, args.get(1));
        ok &= jsval_to_std_string(cx, arg1Val, &arg1);
        if(invokeMethod("share", arg0.c_str(), arg1.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_report_payment(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_report_payment");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 5) {
        // trx_id, amount, currency, callback, this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(4), args.get(3));
        std::string arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &arg0);
        std::string arg1;
        JS::RootedValue arg1Val(cx, args.get(1));
        ok &= jsval_to_std_string(cx, arg1Val, &arg1);
        std::string arg2;
        JS::RootedValue arg2Val(cx, args.get(2));
        ok &= jsval_to_std_string(cx, arg2Val, &arg2);
        if(invokeMethod("reportPayment", arg0.c_str(), arg1.c_str(), arg2.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_get_install_source(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_get_install_source");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 2) {
        // callback, this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(1), args.get(0));
        if(invokeMethod("getInstallSource", cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_is_ok_app_installed(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_is_ok_app_installed");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 0) {
        if(invokeMethod("isOkAppInstalled")) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_perform_posting(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_perform_posting");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 3) {
        // attachment, callback, this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(2), args.get(1));
        std::string arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &arg0);
        if(invokeMethod("performPosting", arg0.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_perform_suggest(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_perform_suggest");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 3) {
        // params, callback, this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(2), args.get(1));
        JS::RootedValue arg0Val(cx, args.get(0));
        std::string res = Stringify(cx, arg0Val);
        if(invokeMethod("performSuggest", res.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_report_stats(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_report_stats");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 3) {
        // params, callback, this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(2), args.get(1));
        JS::RootedValue arg0Val(cx, args.get(0));
        std::string res = Stringify(cx, arg0Val);
        if(invokeMethod("reportStats", res.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_perform_invite(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_perform_invite");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    bool ok = true;
    if(argc == 3) {
        // params, callback, this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(2), args.get(1));
        JS::RootedValue arg0Val(cx, args.get(0));
        std::string res = Stringify(cx, arg0Val);
        if(invokeMethod("performInvite", res.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_call_api(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_call_api");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    if(argc == 4) {
        // method, params, callback, this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(3), args.get(2));
        bool ok = true;
        std::string method;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &method);

        JS::RootedValue arg1Val(cx, args.get(1));
        std::string params = Stringify(cx, arg1Val);
        if(invokeMethod("callApi", method.c_str(), params.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_get_api(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_oksdk_get_api");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    if(argc == 4) {
        // method, params, callback, this
        CallbackFrame *cb = new CallbackFrame(cx, obj, args.get(3), args.get(2));
        bool ok = true;
        std::string method;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &method);

        JS::RootedValue arg1Val(cx, args.get(1));
        std::string params = Stringify(cx, arg1Val);
        if(invokeMethod("getApi", method.c_str(), params.c_str(), cb->callbackId)) {
            rec.rval().set(JSVAL_TRUE);
        } else {
            rec.rval().set(JSVAL_FALSE);
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

void register_all_oksdk_framework(JSContext* cx, JS::HandleObject obj)
{
    printLog("register_all_oksdk_framework");
    JS::RootedObject ns(cx);
    get_or_create_js_obj(cx, obj, "oksdk", &ns);

	// JS_DefineFunction(cx, ns, "init", jsb_oksdk_init, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "login", jsb_oksdk_login, 3, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "logout", jsb_oksdk_logout, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "loggedin", jsb_oksdk_loggedin, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "users_get", jsb_oksdk_users_get, 3, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "friends_get", jsb_oksdk_friends_get, 3, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "friends_get_online", jsb_oksdk_friends_get_online, 3, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "share", jsb_oksdk_share, 4, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "report_payment", jsb_oksdk_report_payment, 4, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "get_install_source", jsb_oksdk_get_install_source, 1, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "is_ok_app_installed", jsb_oksdk_is_ok_app_installed, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "perform_posting", jsb_oksdk_perform_posting, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "perform_suggest", jsb_oksdk_perform_suggest, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "report_stats", jsb_oksdk_report_stats, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "perform_invite", jsb_oksdk_perform_invite, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "call_api", jsb_oksdk_call_api, 4, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "get_api", jsb_oksdk_get_api, 3, JSPROP_ENUMERATE | JSPROP_PERMANENT);
}

static void cpp_loginResult(int callbackId, std::string tokenStr)
{
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([callbackId, tokenStr] {
            CallbackFrame *loginCallback = CallbackFrame::getById(callbackId);
            if(!loginCallback) {
                printLog("requestResult: callbackId not found!");
                return;
            }
            JSAutoRequest rq(loginCallback->cx);
            JSAutoCompartment ac(loginCallback->cx, loginCallback->_ctxObject.ref());

            JS::AutoValueVector valArr(loginCallback->cx);
            jsval tkn = std_string_to_jsval(loginCallback->cx, tokenStr);
            valArr.append(tkn);
            JS::HandleValueArray funcArgs = JS::HandleValueArray::fromMarkedLocation(1, valArr.begin());
            loginCallback->call(funcArgs);

            delete loginCallback;
        });
}

void Java_org_cocos2dx_javascript_OkPlugin_loginResult(JNIEnv* env, jobject thiz, jint callbackId, jstring token)
{
    printLog("Get loginResult");
    if(token == NULL) {
        cpp_loginResult(callbackId, "");
    } else {
        const char* tokenStr = env->GetStringUTFChars(token, NULL);
        cpp_loginResult(callbackId, tokenStr);
        env->ReleaseStringUTFChars(token, tokenStr);
    }
}

static void cpp_requestResult(int callbackId, std::string errStr, std::string resultStr)
{
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([callbackId, errStr, resultStr] {
            CallbackFrame *cb = CallbackFrame::getById(callbackId);
            if(!cb) {
                printLog("requestResult: callbackId not found!");
                return;
            }

            JSAutoRequest rq(cb->cx);
            JSAutoCompartment ac(cb->cx, cb->_ctxObject.ref());

            JS::AutoValueVector valArr(cb->cx);
            if(resultStr.size() > 0) {
                valArr.append(JSVAL_NULL);
                Status err;
                JS::RootedValue rval(cb->cx);
                std::wstring attrsW = wstring_from_utf8(std::string(resultStr), &err);
                utf16string string(attrsW.begin(), attrsW.end());
                if(!JS_ParseJSON(cb->cx, reinterpret_cast<const char16_t*>(string.c_str()), (uint32_t)string.size(), &rval))
                    printLog("JSON Error");
                valArr.append(rval);
            } else {
                valArr.append(std_string_to_jsval(cb->cx, errStr));
                valArr.append(JSVAL_NULL);
            };
            JS::HandleValueArray funcArgs = JS::HandleValueArray::fromMarkedLocation(2, valArr.begin());
            cb->call(funcArgs);
            printLog("requestResult finished");
            delete cb;
        });
}

void Java_org_cocos2dx_javascript_OkPlugin_requestResult(JNIEnv* env, jobject thiz, jint callbackId, jstring err, jstring result)
{
    printLog("Get requestResult");
    std::string s_err;
    std::string s_res;
    if(result != NULL) {
        const char* ch = env->GetStringUTFChars(result, NULL);
        s_res = ch;
        env->ReleaseStringUTFChars(result, ch);
    }
    if(err != NULL) {
        const char* ch = env->GetStringUTFChars(err, NULL);
        s_err = ch;
        env->ReleaseStringUTFChars(err, ch);
    }

    cpp_requestResult(callbackId, s_err, s_res);
}
