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

#include <iostream>
#include <pthread.h>
#include <glib.h>
#include "lighteningcastservice.h"


rtRemoteEnvironment* env;
pthread_t lcast_thread;
static GMainLoop *gLoop = nullptr;
rtLighteningCastRemoteObject *lCastObj;



static gboolean pumpRemoteObjectQueue(gpointer data)
{
//    printf("### %s  :  %s  :  %d   ### \n",__FILE__,__func__,__LINE__);
    rtError err;
    GSource *source = (GSource *)data;
    do {
        g_source_set_ready_time(source, -1);
        err = rtRemoteProcessSingleItem();
    } while (err == RT_OK);
    if (err != RT_OK && err != RT_ERROR_QUEUE_EMPTY) {
        //printf("rtRemoteProcessSingleItem() returned %s", rtStrError(err));
        return G_SOURCE_CONTINUE;
    }
    return G_SOURCE_CONTINUE;
}

static GSource *attachRtRemoteSource()
{
//    printf("### %s  :  %s  :  %d   ###\n",__FILE__,__func__,__LINE__);
    static GSourceFuncs g_sourceFuncs =
        {
            nullptr, // prepare
            nullptr, // check
            [](GSource *source, GSourceFunc callback, gpointer data) -> gboolean // dispatch
            {
                if (g_source_get_ready_time(source) != -1) {
                    g_source_set_ready_time(source, -1);
                    return callback(data);
                }
                return G_SOURCE_CONTINUE;
            },
            nullptr, // finalize
            nullptr, // closure_callback
            nullptr, // closure_marshall
        };
    GSource *source = g_source_new(&g_sourceFuncs, sizeof(GSource));
    g_source_set_name(source, "RT Remote Event dispatcher");
    g_source_set_can_recurse(source, TRUE);
    g_source_set_callback(source, pumpRemoteObjectQueue, source, nullptr);
    g_source_set_priority(source, G_PRIORITY_HIGH);

    rtError e = rtRemoteRegisterQueueReadyHandler(env, [](void *data) -> void {
        GSource *source = (GSource *)data;
        g_source_set_ready_time(source, 0);
    }, source);

    if (e != RT_OK)
    {
        printf("Failed to register queue handler: %d", e);
        g_source_destroy(source);
        return nullptr;
    }
    g_source_attach(source, NULL);
    return source;
}

void * lcastPump(void*) {

    gLoop = g_main_loop_new( NULL , FALSE);
    GSource *remoteSource = attachRtRemoteSource();

    if (!remoteSource)
       printf("Failed to attach rt remote source");

    g_main_loop_run(gLoop);
    g_source_unref(remoteSource);
}

void lcastExit() {
    printf("LighteningCastService Exit!!!\n");
    g_main_loop_quit(gLoop);
    pthread_join( lcast_thread, NULL );
    rtError e = rtRemoteShutdown();
    if (e != RT_OK){
        printf("rtRemoteShutdown failed: %s \n", rtStrError(e));
    }
}


int main() {
    rtError err;
    char* objName;

    env = rtEnvironmentGetGlobal();
    err = rtRemoteInit(env);

    printf("LighteningCastService!!!  Func: %s\n",__func__);

    if (err != RT_OK){
        printf("rtRemoteinit Failed\n");
        return 1;
    }

    pthread_create(&lcast_thread, NULL, lcastPump, NULL);

    objName =  getenv("PX_WAYLAND_CLIENT_REMOTE_OBJECT_NAME");
    if(!objName) objName = "com.comcast.lighteningcast";

    lCastObj = new rtLighteningCastRemoteObject("com.comcast.lighteningcast");
    err = rtRemoteRegisterObject(env, objName, lCastObj);
    if (err != RT_OK){
        printf("rtRemoteRegisterObject for %s failed! error:%s !\n", objName, rtStrError(err));
        return 1;
    }

    printf("LIGHTENINGCAST>>>>\n");
    std::string appName;
    std::string args;
    int options;
    bool exit_lcast = false;

    while(!exit_lcast) {
       printf("Select Options:\n");
       printf("1) launch Application\n2) stop Application\n3) query Application status\n4) Exit\n");
       options = 0;
       scanf(" %d",&options);
       while(getchar() != '\n');

        switch (options)
        {
            case 1:
              printf("\nLaunch Application\nApplication Name : ");
              std::getline(std::cin,appName);
              printf("Launch Params:\n");
              std::getline(std::cin,args);
              lCastObj->launchApplication(appName.c_str(),args.c_str());
              break;

            case 2:
              printf("\nStop Application\nApplication Name : ");
              std::getline(std::cin,appName);
              lCastObj->stopApplication(appName.c_str(),NULL);
              break;

            case 3:
              printf("\nQuery Application Status\nApplication Name : ");
              std::getline(std::cin,appName);
              lCastObj->getApplicationState(appName.c_str(),NULL);
              break;

            case 4:
              printf("\nEXIT\n");
              lcastExit();
              exit_lcast = true;
              break;

            default:
              printf("Invalid Option!!!\n");
              break;
        }
    }

    return 0;
}
