dnl -- Process this file with autoconf to generate a `configure' script. --

AC_PREREQ([2.69])
AC_INIT([rudgiosync], [1.0.0], [dusxmt@gmx.com])

AC_CONFIG_SRCDIR([src/descriptions.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

AM_INIT_AUTOMAKE([1.11.6 -Wall -Werror])


# Options for enabling/disabling optional features.
AC_ARG_ENABLE([checksum],
              [AS_HELP_STRING([--enable-checksum],
               [enable support for checksum-based file comparison, requires nettle [default=auto]])],
              [enable_checksum=$enableval], [enable_checksum=auto])

# Minimal versions of glib.
MIN_GLIB_VER=2.32.4


# Check for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AR


# Check for glib and friends.
PKG_CHECK_MODULES([glib], [
  glib-2.0      >= $MIN_GLIB_VER,
  gobject-2.0   >= $MIN_GLIB_VER,
  gio-2.0       >= $MIN_GLIB_VER
  ],  [], [
  AC_MSG_ERROR([Unable to find glib >= $MIN_GLIB_VER. Please ensure that both the runtime libraries and development files of glib are installed and of the correct version.])])

AC_SUBST(glib_CFLAGS)
AC_SUBST(glib_LIBS)


# Check for system headers.
AC_CHECK_HEADERS([errno.h])


# Check for checksum support.
have_checksum=no

if test x"$enable_checksum" != x"no"; then
  have_nettle_sha2=no
  AC_CHECK_HEADER([nettle/sha2.h], [have_nettle_sha2=yes])

  if test x"$enable_checksum" = x"yes"; then
    if test x"$have_nettle_sha2" != x"yes"; then
      AC_MSG_ERROR([Could not find the nettle/sha2.h header file, which is required for checksum support.])
    fi
  fi

  if test x"$have_nettle_sha2" = x"yes"; then
    have_nettle_lib=no

    AC_CHECK_LIB([nettle], [nettle_sha256_init], [have_nettle_lib=yes])
    if test x"$enable_checksum" = x"yes"; then
      if test x"$have_nettle_lib" != x"yes"; then
        AC_MSG_ERROR([Could not find the nettle library file, which is required for checksum support.])
      fi
    fi
    if test x"$have_nettle_lib" = x"yes"; then
      have_checksum=yes
    fi
  fi
fi

AM_CONDITIONAL([RUDGIOSYNC_CHECKSUM_ENABLED], [test x"$have_checksum" = x"yes"])


AC_OUTPUT

echo ""
echo ""
echo "Configuration summary for rudgiosync:"
echo ""
echo "Checksum support: $have_checksum"
