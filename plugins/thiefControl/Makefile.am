lib_LTLIBRARIES = thiefControl.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

thiefControl_la_SOURCES = thiefControl.cpp
thiefControl_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	README.txt \
	thiefControl.def \
	thiefControl.sln \
	thiefControl.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
