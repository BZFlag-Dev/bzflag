lib_LTLIBRARIES = webadmin.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

webadmin_la_SOURCES = \
		webadmin.cpp \
		loops.cpp \
		loops.h \
		logs.cpp \
		actions.cpp\
		actions.h \
		commonItems.cpp \
		commonItems.h

webadmin_la_LDFLAGS = -module -avoid-version
webadmin_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
webadmin_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	templates \
	webadmin.def \
	webadmin.sln \
	webadmin.vcproj

MAINTAINERCLEANFILES = \
	Makefile.in
