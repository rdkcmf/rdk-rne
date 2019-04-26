/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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

#ifndef LIGHTENING_CAST_H
#define LIGHTENING_CAST_H

#include <string>
#include <unistd.h>

#include "rtcast.hpp"


class rtLighteningCastRemoteObject : public rtCastRemoteObject {

public:

     rtLighteningCastRemoteObject(rtString SERVICE_NAME): rtCastRemoteObject(SERVICE_NAME) {printf("rtLighteningCastRemoteObject(): Service %s\n",SERVICE_NAME.cString());}
     ~rtLighteningCastRemoteObject() {} 

     rtError applicationStateChanged(const rtObjectRef& params);
     rtCastError launchApplication(const char* appName, const char* args);
     rtCastError stopApplication(const char* appName, const char* appID);
     rtCastError getApplicationState(const char* appName, const char* appID);

};

#endif //LIGHTENING_CAST_H
