lib_LTLIBRARIES = webReport.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

webReport_la_SOURCES = \
	webReport.cpp

webReport_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	README.txt \
	templates \
	webReport.def \
	webReport.sln \
	webReport.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
