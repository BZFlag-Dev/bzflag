lib_LTLIBRARIES = serverControl.la

serverControl_la_SOURCES = serverControl.cpp
serverControl_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
serverControl_la_LDFLAGS = -module -avoid-version
serverControl_la_LIBADD = $(top_srcdir)/plugins/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	serverControl.def \
	serverControl.sln \
	serverControl.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
