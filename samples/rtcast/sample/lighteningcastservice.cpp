/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2017 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "lighteningcastservice.h"


rtError rtLighteningCastRemoteObject::applicationStateChanged(const rtObjectRef& params) {
    printf("rtLighteningCastRemoteObject::applicationStateChanged \n");
    rtObjectRef AppObj = new rtMapObject;
    AppObj = params;
    rtString app, id, state, error;
    AppObj.get("applicationName",app);
//    AppObj.get("applicationId",id);
    AppObj.get("state",state);
//    AppObj.get("error",error);
    printf("AppName : %s\nState : %s\n",app.cString(),state.cString());
    return RT_OK;
}


rtCastError rtLighteningCastRemoteObject::launchApplication(const char* appName, const char* args) {
    printf("rtLighteningCastRemoteObject::launchApplication App:%s  args:%s\n",appName,args);
    rtObjectRef AppObj = new rtMapObject;
    AppObj.set("applicationName",appName);
    AppObj.set("parameters",args);

    rtCastError error(RT_OK,CAST_ERROR_NONE);
    RTCAST_ERROR_RT(error) = notify("onApplicationLaunchRequest",AppObj);
    return error;
}

rtCastError rtLighteningCastRemoteObject::stopApplication(const char* appName, const char* appID) {
    printf("rtLighteningCastRemoteObject::stopApplication App:%s  ID:%s\n",appName,appID);
    rtObjectRef AppObj = new rtMapObject;
    AppObj.set("applicationName",appName);
    if(appID != NULL)
        AppObj.set("applicationId",appID);

    rtCastError error(RT_OK,CAST_ERROR_NONE);
    RTCAST_ERROR_RT(error) = notify("onApplicationStopRequest",AppObj);
    return error;
}

rtCastError rtLighteningCastRemoteObject::getApplicationState(const char* appName, const char* appID) {
    printf("rtLighteningCastRemoteObject::getApplicationState App:%s  ID:%s\n",appName,appID);
    rtObjectRef AppObj = new rtMapObject;
    AppObj.set("applicationName",appName);
    if(appID != NULL)
        AppObj.set("applicationId",appID);

    rtCastError error(RT_OK,CAST_ERROR_NONE);
    RTCAST_ERROR_RT(error) = notify("onApplicationStateRequest",AppObj);
    return error;
}



rtDefineObject(rtCastRemoteObject, rtAbstractService);
rtDefineMethod(rtCastRemoteObject, applicationStateChanged);
