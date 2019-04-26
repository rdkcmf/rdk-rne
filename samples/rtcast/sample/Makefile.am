SUBDIRS =
AM_CXXFLAGS= \
   -DRT_PLATFORM_LINUX \
   -I$(top_srcdir) \
   -I=/usr/include/pxcore \
   -I=/usr/include/glib-2.0 \
   -I=/usr/lib/glib-2.0/include \
   -std=c++11
AM_CFLAGS = $(AM_CXXFLAGS) -Wall
bin_PROGRAMS =
bin_PROGRAMS += LighteningCast
LighteningCast_SOURCES = main.cpp \
rtabstractservice.cpp \
lighteningcastservice.cpp
LighteningCast_LDFLAGS= \
   $(AM_LDFLAGS) \
   -lpthread \
   -lrtCore \
   -lrtRemote \
   -lglib-2.0 \
   -Wl,-undefined -Wl,dynamic_lookup
