#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

dnl AC_PREREQ([2.65])
AC_INIT(nwvidtex, 1.0.0, jeff_wannamaker@cable.comcast.com,nwvidtex)
AC_CONFIG_SRCDIR([])
AC_CONFIG_HEADERS([cfg/config.h])
AC_CONFIG_MACRO_DIR([cfg])
AC_CONFIG_AUX_DIR([cfg])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-bzip2 1.9])
LT_INIT

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_CXX

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_HEADER_STDBOOL

EGL_DETECTED=" "
GLESV2_DETECTED=" "

IARM_CFLAGS=" "

# Checks for library functions.
#Add the subdirectories to be considered for building.
SUBDIRS=" "
#to setup m4 macros
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],
[AM_DEFAULT_VERBOSITY=1
AC_SUBST(AM_DEFAULT_VERBOSITY)])

GST_MAJORMINOR=1.0
PKG_CHECK_MODULES([GST], [gstreamer-1.0 >= 1.0], have_gst1="yes", have_gst1="no")
PKG_CHECK_MODULES([EGL],[egl >= 0.0],[EGL_DETECTED=true],[EGL_DETECTED=false])
PKG_CHECK_MODULES([GLESV2],[glesv2 >= 0.0],[GLESV2_DETECTED=true],[GLESV2_DETECTED=false])

AS_IF([test "x$have_gst1" = "xyes"], [
  PKG_CHECK_MODULES([GSTBASE], [gstreamer-base-1.0 >= 1.0])
  AC_DEFINE(USE_GST1, 1, [Build with GStreamer 1.x])
], [])

AS_IF([test "x$enable_gstreamer0" != "xyes" -a "x$have_gst1" != "xyes"], [
   AC_MSG_ERROR([Could not find GStreamer 1.x dependencies:

$GST_PKG_ERRORS])], [])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

