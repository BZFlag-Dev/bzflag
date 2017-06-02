lib_LTLIBRARIES = killall.la

killall_la_SOURCES = killall.cpp
killall_la_CPPFLAGS = -I$(top_srcdir)/include
killall_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	killall.sln \
	killall.vcxproj \
	killall.vcxproj.filters	\
	README.killall.txt

MAINTAINERCLEANFILES =	\
	Makefile.in
