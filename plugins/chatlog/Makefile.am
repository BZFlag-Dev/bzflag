lib_LTLIBRARIES = chatlog.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

chatlog_la_SOURCES = chatlog.cpp
chatlog_la_LDFLAGS = -module -avoid-version
chatlog_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
chatlog_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	chatlog.def \
	chatlog.sln \
	chatlog.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
