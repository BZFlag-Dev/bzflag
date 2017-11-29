lib_LTLIBRARIES = changeTeam.la

changeTeam_la_SOURCES = changeTeam.cpp
changeTeam_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
changeTeam_la_LDFLAGS = -module -avoid-version -shared
changeTeam_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.changeTeam.txt \
	changeTeam.sln \
	changeTeam.vcxproj	\
	changeTeam.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
