# -- Process this file with automake to generate a `Makefile.in' file. --

# The main program recipe.
bin_PROGRAMS          = rudgiosync

# Source code files:
rudgiosync_SOURCES    = boiler.h        \
                        main.c          \
                                        \
                        operations.c    \
                        operations.h    \
                                        \
                        descriptions.c  \
                        descriptions.h  \
                                        \
                        checksum.c      \
                        checksum.h      \
                                        \
                        errors.c        \
                        errors.h


# Fundamental dependency: glib
rudgiosync_CPPFLAGS   = @glib_CFLAGS@
rudgiosync_LDADD      = @glib_LIBS@


# Optional dependency: nettle
if RUDGIOSYNC_CHECKSUM_ENABLED
rudgiosync_CPPFLAGS  += -DRUDGIOSYNC_CHECKSUM_ENABLED
rudgiosync_LDADD     += -lnettle
endif
