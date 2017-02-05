lib_LTLIBRARIES = fairCTF.la

fairCTF_la_SOURCES = fairCTF.cpp
fairCTF_la_CPPFLAGS = -I$(top_srcdir)/include
fairCTF_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.fairCTF.txt \
	fairCTF.sln \
	fairCTF.vcxproj	\
	fairCTF.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
