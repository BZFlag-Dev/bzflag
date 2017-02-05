lib_LTLIBRARIES = thiefControl.la

thiefControl_la_SOURCES = thiefControl.cpp
thiefControl_la_CPPFLAGS = -I$(top_srcdir)/include
thiefControl_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.thiefControl.txt \
	thiefControl.sln \
	thiefControl.vcxproj	\
	thiefControl.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
