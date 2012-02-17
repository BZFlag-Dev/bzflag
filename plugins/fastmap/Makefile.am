lib_LTLIBRARIES = fastmap.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

fastmap_la_SOURCES = fastmap.cpp
fastmap_la_LDFLAGS = -module -avoid-version -shared

EXTRA_DIST = \
	README.txt \
	fastmap.def \
	fastmap.sln \
	fastmap.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
