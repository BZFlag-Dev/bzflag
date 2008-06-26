noinst_LTLIBRARIES = libplugin_utils.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libplugin_utils_la_SOURCES = \
	plugin_utils.cpp \
	plugin_utils.h \
	plugin_config.cpp \
	plugin_config.h \
	plugin_HTTPVDIR.cpp \
	plugin_HTTPVDIR.h \
	plugin_files.cpp \
	plugin_files.h	\
	plugin_groups.cpp \
	plugin_groups.h

libplugin_utils_la_LDFLAGS = -avoid-version

DIST_SUBDIRS = VC8

EXTRA_DIST = \
	templates_readme.txt

MAINTAINERCLEANFILES =	\
	Makefile.in
