dnl -- Process this file with autoconf to generate a `configure' script. --

# Ensure that our tools are new enough.
AC_PREREQ([2.69])
LT_PREREQ([2.4.2])

# Project initialization
AC_INIT([rudgiosync], [0.0.0-dev], [dusxmt@gmx.com])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

AM_INIT_AUTOMAKE([1.11.6 -Wall -Werror])


# Minimal versions of used libraries.
MIN_GLIB_VER=2.32.4


# Check for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AR


# Check for libraries.
PKG_CHECK_MODULES([glib], [
  glib-2.0      >= $MIN_GLIB_VER,
  gobject-2.0   >= $MIN_GLIB_VER,
  gio-2.0       >= $MIN_GLIB_VER
  ],  [], [
  AC_MSG_ERROR([Unable to find glib >= $MIN_GLIB_VER. Please ensure that both the runtime libraries and development files of glib are installed and of the correct version.])])

AC_SUBST(glib_CFLAGS)
AC_SUBST(glib_LIBS)


# Check for headers.
AC_CHECK_HEADERS([errno.h])


AC_OUTPUT
