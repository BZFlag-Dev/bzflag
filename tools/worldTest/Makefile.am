bin_PROGRAMS = worldTest

all-am: $(top_builddir)/src/world/libBZW.la

$(top_builddir)/src/world/libBZW.la:
	if test ! -f $(top_builddir)/src/world/libBZW.la ; then \
		cd $(top_builddir)/src/world && $(MAKE) $(AM_MAKEFLAGS) ; \
		cd ../../tools ; \
	fi

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

worldTest_SOURCES =	\
	worldTest.cxx

worldTest_CPPFLAGS = -I$(top_srcdir)/include
worldTest_LDADD = $(top_builddir)/src/world/libBZW.la

EXTRA_DIST =

MAINTAINERCLEANFILES = Makefile.in
