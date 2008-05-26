lib_LTLIBRARIES = CHATLOG.la

CHATLOG_la_SOURCES = chatlog.cpp
CHATLOG_la_LDFLAGS = -module -avoid-version
CHATLOG_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
CHATLOG_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	chatlog.def \
	chatlog.sln \
	chatlog.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
