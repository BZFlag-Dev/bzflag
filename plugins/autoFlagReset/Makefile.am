lib_LTLIBRARIES = autoFlagReset.la

autoFlagReset_la_SOURCES = autoFlagReset.cpp
autoFlagReset_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
autoFlagReset_la_LDFLAGS = -module -avoid-version -shared

EXTRA_DIST = \
	README.txt \
	autoFlagReset.def \
	autoFlagReset.sln \
	autoFlagReset.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
