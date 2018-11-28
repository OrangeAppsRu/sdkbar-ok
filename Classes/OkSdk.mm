//
//  OkSdk.m
//
//  Created by Vasiliy on 17.04.17.
//
//

#import <Foundation/Foundation.h>
#import "./OkSdk.h"
#import "../proj.ios_mac/ios/OKSDK.h"
#include "scripting/js-bindings/manual/cocos2d_specifics.hpp"
#import <StoreKit/StoreKit.h>

static NSMutableArray *calls = nil;

// FROM JS VAL

static NSDictionary* jsval_to_dictionary(JSContext* cx, JS::HandleValue v)
{
    if (v.isNullOrUndefined())
    {
        return nil;
    }
    
    JS::RootedObject tmp(cx, v.toObjectOrNull());
    if (!tmp)
    {
        CCLOG("%s", "jsval_to_dictionary: the jsval is not an object.");
        return nil;
    }
    NSMutableDictionary *result = [NSMutableDictionary new];
    
    JS::RootedObject it(cx, JS_NewPropertyIterator(cx, tmp));
    
    while (true)
    {
        JS::RootedId idp(cx);
        JS::RootedValue key(cx);
        if (! JS_NextProperty(cx, it, idp.address()) || ! JS_IdToValue(cx, idp, &key))
        {
            return nil; // error
        }
        
        if (key.isNullOrUndefined())
        {
            break; // end of iteration
        }
        
        if (!key.isString())
        {
            continue; // only take account of string key
        }
        
        JSStringWrapper keyWrapper(key.toString(), cx);
        
        JS::RootedValue value(cx);
        JS_GetPropertyById(cx, tmp, idp, &value);
        NSString *keyString = [NSString stringWithUTF8String:keyWrapper.get()];
        if (value.isString()) {
            JSStringWrapper valueWrapper(value.toString(), cx);
            result[keyString] = [NSString stringWithUTF8String:valueWrapper.get()];
        } else if(value.isBoolean()) {
            result[keyString] = [NSNumber numberWithBool:value.get().toBoolean()];
        } else if(value.isDouble()) {
            result[keyString] = [NSNumber numberWithDouble:value.get().toDouble()];
        } else if(value.isInt32()) {
            result[keyString] = [NSNumber numberWithInteger:value.get().toInt32()];
        } else {
            CCASSERT(false, "jsval_to_dictionary: not supported map type");
        }
    }
    
    return result;
}

// TO JS VAL

static jsval object_to_jsval(JSContext *cx, id object);

static jsval string_to_jsval(JSContext *cx, NSString* string)
{
    return std_string_to_jsval(cx, std::string([string UTF8String]));;
}

static jsval number_to_jsval(JSContext *cx, NSNumber* number)
{
    return DOUBLE_TO_JSVAL(number.doubleValue);
}

static jsval array_to_jsval(JSContext *cx, NSArray* array)
{
    JS::RootedObject jsretArr(cx, JS_NewArrayObject(cx, array.count));
    
    int i = 0;
    for(id val in array) {
        JS::RootedValue arrElement(cx);
        arrElement = object_to_jsval(cx, val);
        if (!JS_SetElement(cx, jsretArr, i, arrElement)) {
            break;
        }
        ++i;
    }
    return OBJECT_TO_JSVAL(jsretArr);
}

static jsval dictionary_to_jsval(JSContext* cx, NSDictionary *dict)
{
    JS::RootedObject proto(cx);
    JS::RootedObject parent(cx);
    JS::RootedObject jsRet(cx, JS_NewObject(cx, NULL, proto, parent));

    for(NSString* key in dict.allKeys) {
        JS::RootedValue element(cx);
        
        id obj = dict[key];
        element = object_to_jsval(cx, obj);
        JS_SetProperty(cx, jsRet, [key UTF8String], element);
    }
    return OBJECT_TO_JSVAL(jsRet);
}

static jsval object_to_jsval(JSContext *cx, id object)
{
    if([object isKindOfClass:NSString.class]) {
        return string_to_jsval(cx, object);
    } else if([object isKindOfClass:NSDictionary.class]) {
        return dictionary_to_jsval(cx, object);
    } else if([object isKindOfClass:NSArray.class]) {
        return array_to_jsval(cx, object);
    } else if([object isKindOfClass:NSNumber.class]) {
        return number_to_jsval(cx, object);
    } else if([object isKindOfClass:NSNull.class]) {
        return JSVAL_NULL;
    } else {
        NSLog(@"Error: unknown value class %@", object);
        return JSVAL_NULL;
    }
}

/******************** OKCall ********************/

@interface OKCall: NSObject {
    NSString *method;
    NSDictionary *params;
    BOOL sdkCall;
@public
    JSContext *context;
    mozilla::Maybe<JS::PersistentRootedObject> contextObject;
    mozilla::Maybe<JS::PersistentRootedValue> callback;
    mozilla::Maybe<JS::PersistentRootedValue> thisObject;
}

-(id) initWithMethod:(NSString*)method params:(NSDictionary*)params;
-(id) initWithMethod:(NSString*)method params:(NSDictionary*)params sdk:(BOOL)sdk;
-(void) start;
-(void) callWithResult:(id)result error:(NSError*)error;

@end

@implementation OKCall

-(id)initWithMethod:(NSString *)_method params:(NSDictionary *)_params
{
    if((self = [super init])) {
        method = _method;
        params = _params;
        sdkCall = NO;
    }
    return self;
}

-(id)initWithMethod:(NSString *)_method params:(NSDictionary *)_params sdk:(BOOL)_sdk
{
    if((self = [super init])) {
        method = _method;
        params = _params;
        sdkCall = _sdk;
    }
    return self;
}

-(void) start
{
    void(^success)(id data) = ^(id data) {
        [self callWithResult:data error:nil];
    };
    void(^error)(NSError*) = ^(NSError* error) {
        [self callWithResult:nil error:error];
    };
    if(sdkCall)
        [OKSDK invokeSdkMethod:method arguments:params success:success error:error];
    else
        [OKSDK invokeMethod:method arguments:params success:success error:error];
}

-(void)callWithResult:(id)result error:(NSError *)error
{
    dispatch_async(dispatch_get_main_queue(), ^{
        JSAutoRequest rq(context);
        JSAutoCompartment ac(context, contextObject.ref());
        JS::RootedValue retVal(context);
        JS::AutoValueVector valArr(context);
        if(error) {
            NSLog(@"OK Error: %@", error);
            valArr.append(std_string_to_jsval(context, [error.description UTF8String]));
            valArr.append( JSVAL_NULL);
        } else {
            NSLog(@"OK Result: %@", result);
            valArr.append(JSVAL_NULL);
            valArr.append(object_to_jsval(context, result));
        }
        JS::HandleValueArray funcArgs = JS::HandleValueArray::fromMarkedLocation(2, valArr.begin());
        JS::RootedObject thisObj(context, thisObject.ref().get().toObjectOrNull());
        JS_CallFunctionValue(context, thisObj, callback.ref(), funcArgs, &retVal);
        [calls removeObject:self];
    });
}

-(void)callWithStatus:(BOOL)status result:(id)result
{
    dispatch_async(dispatch_get_main_queue(), ^{
        JSAutoRequest rq(context);
        JSAutoCompartment ac(context, contextObject.ref());
        JS::RootedValue retVal(context);
        JS::AutoValueVector valArr(context);
        NSLog(@"OK Status: %d result: %@", status, result);
        valArr.append(BOOLEAN_TO_JSVAL(status));
        valArr.append(object_to_jsval(context, result));
        JS::HandleValueArray funcArgs = JS::HandleValueArray::fromMarkedLocation(2, valArr.begin());
        JS::RootedObject thisObj(context, thisObject.ref().get().toObjectOrNull());
        JS_CallFunctionValue(context, thisObj, callback.ref(), funcArgs, &retVal);
        [calls removeObject:self];
    });
}

@end

static bool jsb_oksdk_init(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    js_proxy_t *proxy = jsb_get_js_proxy(obj);
    CCLOG("init, param count:%d.\n", argc);
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

        calls = [NSMutableArray new];
        OKSDKInitSettings *settings = [OKSDKInitSettings new];
        settings.appId = [NSString stringWithUTF8String:arg0.c_str()];
        settings.appKey = [NSString stringWithUTF8String:arg1.c_str()];
        settings.controllerHandler = ^UIViewController*() {
            return [UIApplication.sharedApplication.keyWindow rootViewController];
        };
        [OKSDK initWithSettings:settings];
        [OKSDK sdkInit:^(id data) {
            NSLog(@"OK init: %@", data);
        } error:^(NSError *error) {
            NSLog(@"OK init error: %@", error);
        }];
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_login(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    CCLOG("init, param count:%d.\n", argc);
    bool ok = true;
    NSMutableArray *permissions = [NSMutableArray new];
    if(argc == 3) {
        // permissions, callback, this
        std::vector<std::string> arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_vector_string(cx, arg0Val, &arg0);
        if(arg0.size() > 0) {
            for(int i=0; i<arg0.size(); ++i) {
                [permissions addObject:[NSString stringWithUTF8String:arg0[i].c_str()]];
            }
        }
        __block OKCall *call = [OKCall new];
        call->context = cx;
        call->callback.construct(cx, args.get(1));
        call->thisObject.construct(cx, args.get(2));
        call->contextObject.construct(cx, obj);
        
        if([OKSDK currentAccessToken] && [OKSDK currentAccessTokenSecretKey]) {
            [call callWithResult:@[[OKSDK currentAccessToken], [OKSDK currentAccessTokenSecretKey]] error:nil];
        } else {
            [OKSDK authorizeWithPermissions:permissions success:^(id data) {
                [call callWithStatus:YES result:data];
            } error:^(NSError *error) {
                [call callWithStatus:NO result:error];
            }];
        }
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
    return true;
}

static bool jsb_oksdk_logout(JSContext *cx, uint32_t argc, jsval *vp)
{
    if(argc == 0) {
        [OKSDK clearAuth];
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_loggedin(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    if(argc == 2) {
        __block OKCall *call = [OKCall new];
        call->context = cx;
        call->callback.construct(cx, args.get(0));
        call->thisObject.construct(cx, args.get(1));
        call->contextObject.construct(cx, obj);
        
        if([OKSDK currentAccessToken] && [OKSDK currentAccessTokenSecretKey]) {
            [call callWithStatus:YES result:[OKSDK currentAccessToken]];
        } else {
            [call callWithStatus:NO result:[NSError errorWithDomain:NSNetServicesErrorDomain code:403 userInfo:@{}]];
        }
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_users_get(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    if(argc == 3) {
        JS::RootedValue arg0Val(cx, args.get(0));
        OKCall *call = [[OKCall alloc] initWithMethod:@"users.getInfo" params:jsval_to_dictionary(cx, arg0Val)];
        [calls addObject:call];
        call->context = cx;
        call->contextObject.construct(cx, obj);
        call->callback.construct(cx, args.get(1));
        call->thisObject.construct(cx, args.get(2));
        [call start];
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_friends_get(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    if(argc == 2) {
        // callback function & this
        OKCall *call = [[OKCall alloc] initWithMethod:@"friends.get" params:@{}];
        [calls addObject:call];
        call->context = cx;
        call->contextObject.construct(cx, obj);
        call->callback.construct(cx, args.get(0));
        call->thisObject.construct(cx, args.get(1));
        [call start];
        return true;
    } else if(argc == 3) {
        // params, callback function & this
        JS::RootedValue arg0Val(cx, args.get(0));
        NSDictionary *friendsParams = jsval_to_dictionary(cx, arg0Val);
        OKCall *call = [[OKCall alloc] initWithMethod:@"friends.get" params:friendsParams];
        [calls addObject:call];
        call->context = cx;
        call->contextObject.construct(cx, obj);
        call->callback.construct(cx, args.get(1));
        call->thisObject.construct(cx, args.get(2));
        [call start];
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_friends_get_online(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    if(argc == 2) {
        // callback function & this
        OKCall *call = [[OKCall alloc] initWithMethod:@"friends.getOnline" params:@{}];
        [calls addObject:call];
        call->context = cx;
        call->contextObject.construct(cx, obj);
        call->callback.construct(cx, args.get(0));
        call->thisObject.construct(cx, args.get(1));
        [call start];
        return true;
    } else if(argc == 3) {
        // params, callback function & this
        JS::RootedValue arg0Val(cx, args.get(0));
        NSDictionary *friendsParams = jsval_to_dictionary(cx, arg0Val);
        OKCall *call = [[OKCall alloc] initWithMethod:@"friends.getOnline" params:friendsParams];
        [calls addObject:call];
        call->context = cx;
        call->contextObject.construct(cx, obj);
        call->callback.construct(cx, args.get(1));
        call->thisObject.construct(cx, args.get(2));
        [call start];
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_share(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    if(argc == 3) {
        // param1, param2, callback function & this
        bool ok = true;
        std::string param1;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &param1);
        std::string param2;
        JS::RootedValue arg1Val(cx, args.get(1));
        ok &= jsval_to_std_string(cx, arg1Val, &param2);
        
        OKCall *call = [[OKCall alloc] initWithMethod:@"share.addLink" params:@{@"linkUrl": [NSString stringWithUTF8String:param1.c_str()], @"comment": [NSString stringWithUTF8String:param2.c_str()]}];
        [calls addObject:call];
        call->context = cx;
        call->contextObject.construct(cx, obj);
        call->callback.construct(cx, args.get(1));
        call->thisObject.construct(cx, args.get(2));
        [call start];
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_report_payment(JSContext *cx, uint32_t argc, jsval *vp)
{
    // TODO
    return false;
}

static bool jsb_oksdk_get_install_source(JSContext *cx, uint32_t argc, jsval *vp)
{
    // TODO
    return false;
}

static bool jsb_oksdk_is_ok_app_installed(JSContext *cx, uint32_t argc, jsval *vp)
{
    // TODO
    return false;
}

static bool jsb_oksdk_perform_posting(JSContext *cx, uint32_t argc, jsval *vp)
{
    // TODO
    return false;
}

static bool jsb_oksdk_perform_suggest(JSContext *cx, uint32_t argc, jsval *vp)
{
    // TODO
    return false;
}

static bool jsb_oksdk_report_stats(JSContext *cx, uint32_t argc, jsval *vp)
{
    // TODO
    return false;
}

static bool jsb_oksdk_perform_invite(JSContext *cx, uint32_t argc, jsval *vp)
{
    // TODO
    return false;
}

static bool jsb_oksdk_call_api(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    if(argc == 4) {
        bool ok = true;
        std::string arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &arg0);
        NSString *method = [NSString stringWithUTF8String:arg0.c_str()];

        std::map<std::string, std::string> params;
        JS::RootedValue arg1Val(cx, args.get(1));
        NSDictionary *methodParams = jsval_to_dictionary(cx, arg1Val);

        OKCall *call = [[OKCall alloc] initWithMethod:method params:methodParams];
        [calls addObject:call];
        call->context = cx;
        call->contextObject.construct(cx, obj);
        call->callback.construct(cx, args.get(2));
        call->thisObject.construct(cx, args.get(3));
        [call start];
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_oksdk_get_api(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    if(argc == 4) {
        bool ok = true;
        std::string arg0;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &arg0);
        NSString *method = [NSString stringWithUTF8String:arg0.c_str()];
        
        std::map<std::string, std::string> params;
        JS::RootedValue arg1Val(cx, args.get(1));
        NSDictionary *methodParams = jsval_to_dictionary(cx, arg1Val);
        
        OKCall *call = [[OKCall alloc] initWithMethod:method params:methodParams];
        [calls addObject:call];
        call->context = cx;
        call->contextObject.construct(cx, obj);
        call->callback.construct(cx, args.get(2));
        call->thisObject.construct(cx, args.get(3));
        [call start];
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

void register_all_oksdk_framework(JSContext* cx, JS::HandleObject obj)
{
    JS::RootedObject ns(cx);
    get_or_create_js_obj(cx, obj, "oksdk", &ns);
    
    JS_DefineFunction(cx, ns, "init", jsb_oksdk_init, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT);
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
