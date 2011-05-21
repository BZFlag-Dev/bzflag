lib_LTLIBRARIES = soundTest.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

soundTest_la_SOURCES = soundTest.cpp
soundTest_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	README.txt \
	soundTest.def \
	soundTest.sln \
	soundTest.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
