lib_LTLIBRARIES = nagware.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

nagware_la_SOURCES = nagware.cpp
nagware_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	CHANGELOG.txt \
	NAGSAMPLE.cfg \
	README.txt \
	nagware.def \
	nagware.sln \
	nagware.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
