noinst_LTLIBRARIES = libplugin_utils.la

AM_CPPFLAGS = -I$(top_srcdir)/include $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

libplugin_utils_la_SOURCES = \
	plugin_utils.cpp \
	plugin_utils.h \
	plugin_files.cpp \
	plugin_files.h \
	plugin_groups.cpp \
	plugin_groups.h \
	plugin_config.cpp \
	plugin_config.h
libplugin_utils_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES =	\
	Makefile.in

EXTRA_DIST = \
	plugin_utils.sln \
	plugin_utils.vcxproj	\
	plugin_utils.vcxproj.filters
