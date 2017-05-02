lib_LTLIBRARIES = HoldTheFlag.la

HoldTheFlag_la_SOURCES = HoldTheFlag.cpp
HoldTheFlag_la_CPPFLAGS = -I$(top_srcdir)/include
HoldTheFlag_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.HoldTheFlag.txt \
	HoldTheFlag.sln \
	HoldTheFlag.vcxproj	\
	HoldTheFlag.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
