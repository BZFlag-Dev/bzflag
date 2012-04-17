lib_LTLIBRARIES = hiddenAdmin.la

hiddenAdmin_la_SOURCES = hiddenAdmin.cpp
hiddenAdmin_la_CPPFLAGS = -I$(top_srcdir)/include
hiddenAdmin_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	hiddenAdmin.def \
	hiddenAdmin.sln \
	hiddenAdmin.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
