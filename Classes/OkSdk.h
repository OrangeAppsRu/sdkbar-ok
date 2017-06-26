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


void register_all_oksdk_framework(JSContext* cx, JS::HandleObject obj);


#endif /* OkSdk_h */
