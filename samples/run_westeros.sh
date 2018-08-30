#!/bin/sh

export XDG_RUNTIME_DIR=/tmp/
export WAYLAND_DISPLAY=mydisplay
export LD_PRELOAD=libwayland-egl.so.0
westeros --renderer libwesteros_render_nexus.so.0 --framerate 60 --display mydisplay&
