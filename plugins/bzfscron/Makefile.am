lib_LTLIBRARIES = bzfscron.la

bzfscron_la_SOURCES = \
	bzfscron.cpp \
	CronJob.cpp \
	CronJob.h
bzfscron_la_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
bzfscron_la_LDFLAGS = -module -avoid-version -shared
bzfscron_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.bzfscron.txt \
	bzfscron.cfg \
	bzfscron.sln \
	bzfscron.vcxproj	\
	bzfscron.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
