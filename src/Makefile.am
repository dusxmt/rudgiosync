# -- Process this file with automake to generate a `Makefile.in' file. --

# Dependencies.
DEPEND_LIBS = @glib_LIBS@
AM_CPPFLAGS = @glib_CFLAGS@

# The main program recipe.
bin_PROGRAMS = rudgiosync

rudgiosync_SOURCES = boiler.h descriptions.c descriptions.h errors.c errors.h main.c operations.c operations.h
rudgiosync_LDADD   = $(DEPEND_LIBS)
