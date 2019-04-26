#!/bin/sh

export XDG_RUNTIME_DIR=/run/user/
export LD_PRELOAD=/usr/lib/libwayland-client.so.0:/usr/lib/libwayland-egl.so.0
#export WAYLAND_DISPLAY=lcast
export WAYLAND_APPS_CONFIG=/home/root/waylandregistryreceiver.conf

/home/root/Spark /usr/share/xdial/lighteningCast.js
