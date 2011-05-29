lib_LTLIBRARIES = rabbitTimer.la

rabbitTimer_la_SOURCES = rabbitTimer.cpp
rabbitTimer_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
rabbitTimer_la_LDFLAGS = -module -avoid-version
rabbitTimer_la_LIBADD = $(top_srcdir)/plugins/plugin_utils/libplugin_utils.la


EXTRA_DIST = \
	README.txt \
	rabbitTimer.def \
	rabbitTimer.sln \
	rabbitTimer.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
