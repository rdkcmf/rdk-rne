##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2018 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
AUTOMAKE_OPTIONS = subdir-objects
# ACLOCAL_AMFLAGS = -I cfg

SUBDIRS =

AM_CXXFLAGS = -g \
	-Wall \
	-std=c++0x \
	-DGCC4_XXX \
	$(CURL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GST_CFLAGS) \
	$(JANSON_CFLAGS) \
	-I$(top_srcdir) \
	-I=/usr/include/pxcore \
	-I=/usr/include/ \
	-I$(top_srcdir)/common \
	-I$(top_srcdir)/aamp \
	-I=/usr/include/rdk/ds \
	-I=/usr/include/rdk/ds-hal \
	-I=/usr/include/rdk/iarmbus \
	-I=/usr/include/rdk/iarmmgrs


AM_CXXFLAGS += -DRT_PLATFORM_LINUX

if ENABLE_RMF
AM_CXXFLAGS += -I$(top_srcdir)/rmf \
	-I=$(includedir)/directfb
endif

bin_PROGRAMS = rdkmediaplayer

rdkmediaplayer_SOURCES = \
	main.cpp \
	rdkmediaplayer.cpp \
	rdkmediaplayer.h \
	common/rdkmediaplayerimpl.h \
	common/rdkmediaplayerimpl.cpp \
	common/rtevents.h \
	common/rtevents.cpp \
	common/glib_tools.cpp \
	common/glib_tools.h \
	common/logger.cpp \
	common/logger.h \
	common/timer.cpp \
	common/timer.h \
	common/intrect.h \
	aamp/aampplayer.h \
	aamp/aampplayer.cpp 

	
if ENABLE_RMF 
rdkmediaplayer_SOURCES += \
	rmf/mediaplayer.h \
	rmf/rmfplayer.cpp \
	rmf/rmfplayer.h \
	rmf/mediaplayerdlna.cpp \
	rmf/mediaplayerdlna.h \
	rmf/mediaplayergeneric.cpp \
	rmf/mediaplayergeneric.h
endif

rdkmediaplayer_LDFLAGS = \
	-lIARMBus
	
if ENABLE_RMF
rdkmediaplayer_LDFLAGS += \
	-lrmfbase \
	-lhnsource \
	-lmediaplayersink \
	-ldumpfilesink
endif

if ENABLE_COMCAST_BUILD
rdkmediaplayer_SOURCES += \
	common/closedcaptions.h \
	common/closedcaptions.cpp
rdkmediaplayer_LDFLAGS += \
	-lAVEPlayer \
	-lrdkCCReader \
	-lrdkCCManager \
	-lrdkCCGfx \
	-llog4c \
	-ldirectfb
else
AM_CXXFLAGS += -DDISABLE_CLOSEDCAPTIONS
endif

rdkmediaplayer_LDFLAGS += -laamp

# TODO-test is static libs make a difference on performance or memory consumption
#rdkmediaplayer_LDFLAGS += -lrtRemote_s -lrtCore_s -lpthread -luuid -ldl
rdkmediaplayer_LDFLAGS += -lrtRemote -lrtCore

rdkmediaplayer_LDFLAGS += \
	$(SOC_LIBS) \
	$(CURL_LIBS) \
	$(GLIB_LIBS) \
	$(GST_LIBS) \
	$(JANSON_LIBS)

if ENABLE_DIRECT_QAM
AM_CXXFLAGS += -DENABLE_DIRECT_QAM
rdkmediaplayer_LDFLAGS += -lqamsrc
endif

if ENABLE_IPPV_CLIENT
AM_CXXFLAGS += -DIPPV_CLIENT_ENABLED
rdkmediaplayer_LDFLAGS += -lippvsource
endif

if ENABLE_VOD_SRC
AM_CXXFLAGS += -DUSE_VODSRC
rdkmediaplayer_LDFLAGS += -lvodsrc
endif

if ENABLE_RDK_LOGGER
AM_CXXFLAGS += -DUSE_RDK_LOGGER
rdkmediaplayer_LDFLAGS += -lrdkloggers -llog4c
endif

if ENABLE_BREAKPAD
AM_CXXFLAGS += -DENABLE_BREAKPAD $(breakpad_CFLAGS)
rdkmediaplayer_LDFLAGS += $(breakpad_LIBS)
endif

