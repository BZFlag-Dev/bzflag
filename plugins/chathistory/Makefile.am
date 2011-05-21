lib_LTLIBRARIES = chathistory.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

chathistory_la_SOURCES = chathistory.cpp
chathistory_la_LDFLAGS = -module -avoid-version
chathistory_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
chathistory_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	chathistory.def \
	chathistory.sln \
	chathistory.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
