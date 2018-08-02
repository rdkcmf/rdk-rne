RNE SDK version 0.3
===================
- [Introduction](#intro)
- [Prerequisites](#prerequisites)
    - [System Requirements](#system-requirements)
    - [Software Requirements](#software-requirements)
- [Installation](#installation)
    - [Installation of RDK Image on Raspberry Pi](#installation-of-rdk-image-on-raspberry-pi)
    - [Installation of RNE SDK](#installation-of-rne-sdk)
- [Developing applications](#developing-applications)
    - [Troubleshooting](#troubleshooting)
    - [Breakpad support](#breakpad-support)
- [Terminal Access](#terminal-access)
	- [For Raspberry Pi](#for-raspberry-pi)
	- [For Pace xi5](#for-pace-xi5)
- [Installing Sample Applications](#installing-sample-applications)
    - [Preparing USB](#preparing-USB)
	- [Running Sample Applications on Raspberry Pi](#running-sample-applications-raspberry-pi)
	- [Running Sample Applications on Pace xi5](#running-sample-applications-pace-xi5)
	- [App Manager Registry File](#app-manager-registry-file)
	- [Running Sample Applications Standalone on Raspberry Pi](#running-sample-applications-standalone-on-raspberry-pi)
- [Using the App Manager](#using-the-app-manager)
	- [App Manager Logging](#app-manager-logging)
- [Key App Features](#key-app-features)
    - [Graphics With Wayland](#graphics-with-wayland)
    - [Keyboard And Remote Input](#keyboard-and-remote-input)
    - [Handling Resolution Changes](#handling-resolution-changes)
    - [Communication With App Manager](#communication-with-app-manager)
    - [Suspend And Resume](#suspend-and-resume)
    - [Audio And Video Playback With Gstreamer](#audio-and-video-playback-with-gstreamer)
- [Sample Applications](#sample-applications)
    - [Graphics Sample](#graphics-sample)
    - [Player Sample](#player-sample)
    - [Lifecyle Sample](#lifecycle-sample)
    - [MSE Player Sample](#mse-player-sample)

<a name="intro"></a>

Introduction
------------
RDK Native Environment SDK (RNE) is intended to provide a development environment for applications targeted to run in RDK environment.

<a name="prerequisites"></a>

Prerequisites 
-------------
<a name="system-requirements"></a>
### System Requirements ###

Ubuntu 14.4 **64 bit** OS or higher
8GB of RAM

<a name="software-requirements"></a>
### Software Requirements ###

The following software packages needs to be installed
- awk 
- wget 
- git-core 
- diffstat 
- unzip 
- texinfo 
- gcc-multilib 
- build-essential 
- chrpath 
- socat 
- cpio 
- python 
- python3 
- python3-pip 
- python3-pexpect 
- xz-utils 
- debianutils 
- iputils-ping 
- libsdl1.2-dev 
- xterm

This can be installed using the following command

     sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
	 build-essential chrpath socat cpio python python3 python3-pip python3-pexpect \
	 xz-utils debianutils iputils-ping libsdl1.2-dev xterm

<a name="installation"></a>

Installation
------------

<a name="installation-of-rdk-image-on-raspberry-pi"></a>
### Installation of RDK Image on Raspberry Pi ###

Currently the only supported version of the Raspberry pi is **Raspberry Pi 3 Model B**.
With Pi 3 Model B+ support planned for a future release.

As part of RNE package you should receive an rdk sdimg to flash to the raspberry pi sdcard.
Insert the raspberry pi sdcard into your desktop machine and flash the image using the
following commands (**This will overwrite anything on the sdcard**).

>       sudo dd if=/path/to/image/*.rpi-sdimg of=/dev/sdX bs=4M
>       sync 

Replace *if=* path with correct path to sdimg.  
Replace the *X* in *sdX* with the correct device letter of the sdcard when inserted into your machine.  
**Example:**  
>       sudo dd if=/home/user/rne-0.3/RPIMC_default_20171212212142sdy.rootfs.rpi-sdimg of=/dev/sdb bs=4M  


Once flashed insert the sdcard into the pi and make sure it has a valid ethernet and HDMI connection.
Then power up the pi, it should boot up into the app manager screen with the ethernet IP listed at the bottom right.

<a name="installation-of-rne-sdk"></a>
### Installation of RNE SDK ###

To install the sdk, go the directory where it is copied, and run the following commands:

On Raspberry Pi:
>      chmod +x ./raspberrypi-rdk-mc-RNE-SDK-2.0.sh
>      ./raspberrypi-rdk-mc-RNE-SDK-2.0.sh 

On Pace xi5:
>      chmod +x ./pacexi5-daisy-RNE-SDK-2.0.sh
>      ./pacexi5-daisy-RNE-SDK-2.0.sh 

Follow the instructions in the screen regarding installation directory. If you don't want to change the default directory, please press *ENTER* and proceed.
It's easiest to run the above script in an empty directory and type . to install in the current directory.

<a name="developing-applications"></a>

Developing applications
------------

Assuming that the SDK is installed in *sdk_root* directory, issue the following command

>      source  <sdk_root>/environment-setup-<chipset>-rdk-linux-gnueabi

**Example:**
>      source RNE-3.0/environment-setup-cortexa7t2hf-vfp-vfpv4-neon-rdk-linux-gnueabi

Here RNE-3.0 is sdk_root directory

Now all the cross compiler tools are available for development. Four native sample applications are provided as part of build. To build the samples, go to the samples directory and execute the following command

>      ./build_samples.sh

After the samples are built they are contained in a partnerapps directory for copying to the provided device.

<a name="breakpad-support"></a>
### Breadpad support ###
Google breakpad support is available for the applications. For more information about Google breakpad and it's integration, check this link
https://chromium.googlesource.com/breakpad/breakpad/+/master/docs/linux_starter_guide.md
The static library is added as part of SDK. 

The rne player application, included as part of this, has breakpad support.

<a name="troubleshooting"></a>
If a shared library has secondary dependencies which it cannot resolve, the following linker flags should be added to tell the linker to ignore its undefined symbols
-Wl, --allow-shlib-undefined

<a name="terminal-access"></a>

Terminal Access
------------

<a name="for-raspberry-pi"></a>
### For Raspberry Pi ###
Once the pi is booted into the app manager and the pi has a valid ethernet connection the app manager will list the device's ethernet ip on the bottom right.
This can be used to ssh into the box from a machine on the same network.  Login is root, password is empty.

Example:
>      ssh root@192.168.0.103

Then press enter twice to get ssh terminal access to the box.

<a name="for-pace-xi5"></a>
### For Pace xi5 ###
TODO...

<a name="installing-sample-applications"></a>

Installing Sample Applications
----------------------
The following sections will detail how to load and run the provided sample applications on the provided device.

<a name="preparing-USB"></a>
### Preparing USB ###
As of this release, USB storage device should have **single partition and ext3 format**.
To format USB in ext3 

**Below assumes usb device is on device sdb**

- Unmount USB drive. 
>      sudo umount /dev/sdb 

- Format USB drive in ext3 format 
>      sudo mkfs.ext3 /dev/sdb 

<a name="running-sample-applications-raspberry-pi"></a>
### Running Sample Applications on Raspberry Pi ###

Once the sample applications are built with the provided sdk a partnerapps folder will be created at the root of the samples directory.
You can then place this directory at the root of the USB storage device and then insert the USB stick into the pi.
Once inserted, use a USB keyboard and press ctrl-e to reload the app manager.

You can have the app manager launch your own binary by renaming the binary *partnerapp* and placing it the partnerapps folder at the root of the USB storage device.

<a name="running-sample-applications-pace-xi5"></a>
### Running Sample Applications on Pace xi5 ###
TODO...

<a name="app-manager-registry-file"></a>
### App Manager Registry File ###
The appmanagerregistry.conf is a simple json file that the app manager reads to find the apps it can launch.
This file is contained at the root of the partnerapps directory on the USB storage device.
It can easily be modified to launch and interact with new partner applications.

<a name="running-sample-applications-standalone-on-raspberry-pi"></a>
### Running Sample Applications Standalone on Raspberry Pi ###
For debugging purposes you can run your applications standalone without the app manager
Once sshed into the raspberry pi do the following:

- Shutdown the app manager
>      systemctl stop appmanager 
- Start the westeros service (Needed for graphics to be displayed)
>      systemctl start westeros
- Run the provided run_partner_app.sh script provided in the partnerapps folder
>      /usb/partnerapps/run_partner_app.sh

By default the the above shell script runs the rne_triangle sample app.  But it
can easily be modified to run any app of your choosing.

To use the app manager again, just start its service back up.
>      systemctl start appmanager 


<a name="using-the-app-manager"></a>

Using the App Manager
----------------------

A USB keyboard or remote control (on supported systems) can be used on the device to interact with the app manager.  

The following actions are supported:

- Pressing the up and down keyboard arrow keys (or remote arrows)  moves between the apps
- Pressing Enter (or OK button remote)  will launch or switch to a selected app.
- Pressing left or right arrow keys (or remote arrows)  will select one of three options (Launch/Suspend/Stop)  
-- Launch will launch the selected application, if an app is already launched it will be switched to.  
-- Suspend will put an application in suspend state if it supports it (Graphics Lifecycle and MSE sample player support suspend/resume)  
-- If an app is suspended this option will become Resume, which when pressed will resume the application.
- When an application is taking up the whole screen Pressing Ctrl-m (or remote xfinity key)  will bring you back to the app manager.
- To reload the app manager (if the contents of the usb key were changed) press Ctrl-e (or Exit key on the remote).

<a name="app-manager-logging"></a>
### App Manager Logging ###
The app manager application will log all its output to /opt/logs/appmanager.log.

All partner applications that are launched from the app manager will have their stdout/stderr also redirected to this file.

<a name="key-app-features"></a>

Key App Features
------------

<a name="graphics-with-wayland"></a>
### Graphics With Wayland ###

The app manager has the job of running a wayland compositor for all the apps it runs.  Before painting to the screen apps
will connect to this compositor using wayland calls.

The following is a general graphics setup flow.
```
// Connect to compositor
display= wl_display_connect(display_name);  // A NULL display name connects to default WAYLAND_DISPLAY set by app manager

// Use the registry as the hook to connect to the rest of the wayland callbacks
registry= wl_display_get_registry(display); 
wl_registry_add_listener(registry, &registryListener, &ctx);

// Complete communication to compositor for above commands
wl_display_roundtrip(display);

// Setup EGL

// Use the the wayland display to get the egl display
ctx->eglDisplay = eglGetDisplay((NativeDisplayType)display);
// Continue setting up EGL...

// Create a surface with the compositor
// Get compositor reference from wayland registry callback
// Create surface
ctx->surface= wl_compositor_create_surface(ctx->compositor);

// Create egl window from surface
ctx->native= wl_egl_window_create(ctx->surface, ctx->planeWidth, ctx->planeHeight);

// Continue setting up surface with normal egl calls..

// Do Open GL setup for your app

// Run graphics app loop
while (shouldRunMainLoop)
{
  // dispatch queued wayland events on default queue, can also use blocking method wl_display_dispatch
  wl_display_dispatch_pending(display)

  // render your gl scene

  // swap buffers
  eglSwapBuffers(eglDisplay, eglSurfaceWindow);
}
```

<a name="keyboard-and-remote-input"></a>
### Keyboard And Remote Input ###

All keyboard and remote control (for supported systems) for a native app is handled through the standard wayland seat mechanism.
Remote control keys will come in as a standard wayland keyboard seat.  The app manager compositor will route input to the focused
app.

The following is a general way of getting keys
```
// Connect to display and registry callback with wayland registry
display= wl_display_connect(display_name);
registry= wl_display_get_registry(display);
wl_registry_add_listener(registry, &registryListener, &ctx);

// In registry callback add a seat listener
seat= (struct wl_seat*)wl_registry_bind(registry, id, &wl_seat_interface, 4);
wl_seat_add_listener(seat, &seatListener, ctx);

// In seat listener add listener for keyboard
keyboard= wl_seat_get_keyboard( seat );
wl_keyboard_add_listener( keyboard, &keyboardListener, ctx );

// Use the keyboard listener callbacks to get key information:
static const struct wl_keyboard_listener keyboardListener = {
  keyboardHandleKeymap,
  keyboardHandleEnter,
  keyboardHandleLeave,
  keyboardHandleKey,
  keyboardHandleModifiers,
  keyboardHandleRepeatInfo,
};
```

<a name="handling-resolution-changes"></a>
### Handling Resolution Changes ###

Resolution changes can be handled through the wayland output mechanism.
Add a listener on wl_output and then a callback for outputHandleMode.
The outputHandleMode callback will be called with resolution changes.
This will also be called once first registered to get initial output resolution.
```
// Connect to display and registry callback with wayland registry
display= wl_display_connect(display_name);
registry= wl_display_get_registry(display);
wl_registry_add_listener(registry, &registryListener, &ctx);

// In registry callback add an output listener
output= (struct wl_output*)wl_registry_bind(registry, id, &wl_output_interface, 2);
wl_output_add_listener(output, &outputListener, ctx);

// Make sure your outputListener contains an outputMode callback
static const struct wl_output_listener outputListener = {
   outputGeometry,
   outputMode,
   outputDone,
   outputScale
};

//output mode will get passed in a width and height
static void outputMode( void *data, struct wl_output *output, uint32_t flags,
                        int32_t width, int32_t height, int32_t refresh )
{
}
```

<a name="communication-with-app-manager"></a>
### Communication With App Manager ###

Apps will communicate with the app manager through the interprocess communication library rt.
rt enables the app manager to call functions in your application and for your application
to send events and status to the app manager.

Using rt:
```
// Your object to be registered must inherit the rtObject interface
// Here you will declare your object and define functions.
class MyObject : public rtObject {
 public:
  rtDeclareObject(MyObject, rtObject);
  rtMethodNoArgAndNoReturn("suspend", suspend);
  rtMethodNoArgAndNoReturn("resume", resume);

// You then need to initialize rt and register your object.
rtError rc = rtRemoteInit();
const char* objectName = "MY OBJECT";
MyObject* myObject = new MyObject;
rc = rtRemoteRegisterObject(objectName, myObject);

// Then for rt processing to work you must call its process
// iteration in your apps main loop.

// This will register a callback that will get called everytime there is rt messages
// to be processed
rtRemoteRegisterQueueReadyHandler( rtEnvironmentGetGlobal(), rtRemoteCallback, nullptr );

// You then must call rtRemoteProcessSingleItem() to process the message. You can do
// this as part of your apps main loop or use a thread and wait on a condition.
```

<a name="suspend-and-resume"></a>
### Suspend And Resume ###

Apps should support a suspend and resume option if needed so they can start quickly
and stay running in the background.  Apps when suspended should free all graphics
and AV resources while using minimal CPU and RAM while running in the background.
When the app is resumed it should be usable by the user in a few seconds.

The app manager is ready to call suspend/resume on your app through rt.
You will need to initialize and register an object to rt that contains
these functions.
```
// Your object to be registered must inherit the rtObject interface
// Here you will declare your object and define functions.
// Header file:
class MyObject : public rtObject {
 public:
  rtDeclareObject(MyObject, rtObject);
  rtMethodNoArgAndNoReturn("suspend", suspend); //declare suspend function to rt
  rtMethodNoArgAndNoReturn("resume", resume); //declare resume function to rt

  rtError suspend(); //actual suspend c++ function
  rtError resume(); //acutal resume c++ function

//CPP file:
// Define rt object and methods
rtDefineObject (MyObject, rtObject);
rtDefineMethod (MyObject, suspend);
rtDefineMethod (MyObject, resume);

// Implement suspend/resume c++ functions
rtError MyObject::suspend()
{
  // suspend your app from active state
}

rtError MyObject::resume()
{
  // resume your app from suspend state
}
```

<a name="audio-and-video-playback-with-gstreamer"></a>
### Audio And Video Playback With Gstreamer ###

How your app generally uses gstreamer is up to you.  Ideally your app will be able to use a playbin pipeline.
The only requirement is that your app must use gstreamer for its audio and video needs and that it
use the westerossink for its videosink.  westerossink allows your video path to go through wayland
so it can be manipulated by the application manager if necessary.  A simple gstreamer use case
is showcased in the rne-player sample app and a more real world MSE type AV player is showcased in the
mse-player sample application.

<a name="sample-applications"></a>

Sample Applications
------------
The provided sample apps are to help developers get up to speed with using the graphics and video APIs in RNE.
Generally all graphics and keyboard/remote input will be provided through wayland, and audio and video will use gstreamer.

<a name="graphics-sample"></a>
### Graphics Sample ###
rne-triangle sample app demonstrates a simple graphics app that uses opengl es and wayland to render a spinning triangle to the screen.
The following key features are demostrated:

1.  How to use wayland and OpenGL ES to render graphics
2.  How to get keyboard/remote input
3.  How to get screen resolution

<a name="player-sample"></a>
### Player Sample ###
rne-player sample app shows how to build and use a simple gstreamer pipeline that uses westerosink.  It will load a movie from a URL,
buffer it, and play it back.
The following key features are demonstrated:

1.  How to build a simple gstreamer pipeline with westerossink
2.  How to get screen resolution

<a name="lifecycle-sample"></a>
### Lifecycle Sample ###
graphics-lifecycle sample app extends the rne-triangle sample app to support rt and suspend/resume.
The following key features are demonstrated:

1.  How to use wayland and OpenGL ES to render graphics
2.  How to get keyboard/remote input
3.  How to get screen resolution
4.  How to setup and register an object with rt
5.  How to handle suspend/resume

<a name="mse-player-sample"></a>
### MSE Player Sample ###
The MSE (Media Source Extensions) player sample app demos how to put everything together for a more real world example.
The app will show how to build a gstreamer pipeline that can be fed raw H264 and AAC frames asynchronously. The sample content
contains three separate fragments of the same video to show how to simulate a seek by flushing the video pipeline and providing
new samples at different period in time.  The app also uses the essos library which simplifies setting up wayland for graphics
and keyboard/remote input.

The following key features are demonstrated:

1.  How to setup and use a more complicated gstreamer pipeline with a custom source
2.  How to setup and register an object with rt
3.  How to handle suspend/resume
