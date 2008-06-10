lib_LTLIBRARIES = recordmatch.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

recordmatch_la_SOURCES = recordmatch.cpp
recordmatch_la_CPPFLAGS = -I$(top_srcdir)/include
recordmatch_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	README.txt \
	recordmatch.def \
	recordmatch.sln \
	recordmatch.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
