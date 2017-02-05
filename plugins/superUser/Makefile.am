lib_LTLIBRARIES = superUser.la

superUser_la_SOURCES = superUser.cpp
superUser_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
superUser_la_LDFLAGS = -module -avoid-version -shared
superUser_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.superUser.txt \
	superUser.cfg \
	superUser.sln \
	superUser.vcxproj	\
	superUser.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
