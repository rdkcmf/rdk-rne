/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
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

#ifndef _GRAPHICS_LIFECYCLE_H_
#define _GRAPHICS_LIFECYCLE_H_

/**
 * @defgroup FIREBOLT Firebolt
 *
 * RDK Firebolt SDK (previously called RNE) is intended to provide a development environment for applications targeted to run on RDK.
 * Firebolt SDK can be used by application developers to develop HTML5, Spark & Native applications for RDK.
 * The SDK comes with an RDK image which can be loaded on the target platform on which applications can be developed and executed.
 *
 * @defgroup Graphics Sample Graphics Sample
 * 
 * rne-triangle sample app demonstrates a simple graphics app that uses opengl es and wayland to render a spinning triangle to the screen * The following key features are demostrated:
 * - How to use wayland and OpenGL ES to render graphics
 * - How to get keyboard/remote input
 * - How to get screen resolution
 * @ingroup FIREBOLT
 * 
 * @defgroup Graphics-lifecycle Graphics-lifecycle
 * 
 * Graphics-lifecycle sample app extends the rne-triangle sample app to support rt and suspend/resume.
 * The following key features are demonstrated:
 * - How to use wayland and OpenGL ES to render graphics
 * - How to get keyboard/remote input
 * - How to get screen resolution
 * - How to setup and register an object with rt
 * - How to handle suspend/resume 
 * @ingroup FIREBOLT
 *
 * @defgroup MSE Player Sample MSE Player Sample
 *
 * The MSE (Media Source Extensions) player sample app demos how to put everything together for a more real world example.
 * The app will show how to build a gstreamer pipeline that can be fed raw H264 and AAC frames asynchronously. 
 * The sample content contains three separate fragments of the same video to show how to simulate a seek by flushing the video pipeline
 * and providing new samples at different period in time.
 * The app also uses the essos library which simplifies setting up wayland for graphics and keyboard/remote input.
 * The following key features are demonstrated:
 * - How to setup and use a more complicated gstreamer pipeline with a custom source
 * - How to setup and register an object with rt
 * - How to handle suspend/resume
 * @ingroup FIREBOLT
 *
 * @defgroup Player Sample Player Sample
 * 
 * rne-player sample app shows how to build and use a simple gstreamer pipeline that uses westerosink.
 * It will load a movie from a URL, buffer it, and play it back. The following key features are demonstrated:
 * - How to build a simple gstreamer pipeline with westerossink
 * - How to get screen resolution 
 * @ingroup FIREBOLT
 */

#include <rtRemote.h>
#include <rtError.h>

#include "RtUtils.h"

/**
 * @addtogroup Graphics-lifecycle
 * @{
 */

class GraphicsLifeCycle : public rtObject {
public:
    rtDeclareObject(GraphicsLifeCycle, rtObject);
    rtMethodNoArgAndNoReturn("suspend", suspend);
    rtMethodNoArgAndNoReturn("resume", resume);

    GraphicsLifeCycle();
    virtual ~GraphicsLifeCycle();

	/**
	 *  @brief This API is called by the app manager to suspend the currently running application.
     *
     *  Apps when suspended should free all graphics and AV resources.
     *  Uses  minimal CPU and RAM while running in the background.
     *  The app manager invokes suspend call from the app through rt.
	 *
	 *  @return Returns RT_OK on success, appropriate error code otherwise.
	 */
    rtError suspend();

	/**
	 *  @brief This API is used to resume the application suspended by the app manager.
	 *
	 *  @return Returns RT_OK on success, appropriate error code otherwise.
	 */
    rtError resume();

    /**
	 *  @brief This initializes the rt object, sets the event listeners and also set the remote ready flag.
	 *
	 *  @param[in]  rtUtils   rt object.
	 *
	 *  @return Returns RMF_SUCCESS on success, appropriate error code otherwise.
	 */
    void setRtUtils(RtUtils* rtUtils);

    struct Callbacks {
       void (*onResume)(const char* p);
       void (*onSuspend)();
    };

	/**
	 *  @brief This API sets the callback functions.
     *
     *  Apps should support a suspend and resume option if needed 
     *  so they can start quickly and stay running in the background. 
	 *
	 *  @param[in] cb   Callback functions to set.
	 *
	 *  @return Returns RMF_SUCCESS on success, appropiate error code otherwise.
	 */
    void setCallbacks(const Callbacks &cb);
private:
    RtUtils* mRtUtils;
    Callbacks mCb;
};

#endif
/**
 * @}
 */

