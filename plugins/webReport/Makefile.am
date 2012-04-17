lib_LTLIBRARIES = webReport.la

webReport_la_SOURCES = \
	webReport.cpp

webReport_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	templates \
	webReport.def \
	webReport.sln \
	webReport.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
