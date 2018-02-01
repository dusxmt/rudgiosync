# -- Process this file with automake to generate a `Makefile.in' file. --

ACLOCAL_AMFLAGS = -I m4
SUBDIRS         = src


dist-hook: generate-chlog

generate-chlog:
	if test -d .git; then					\
	  $(top_srcdir)/build-aux/gitlog-to-changelog		\
	    > $(distdir)/ChangeLog.new;				\
	  rm -f $(distdir)/ChangeLog;				\
	  mv $(distdir)/ChangeLog.new $(distdir)/ChangeLog;	\
	fi
