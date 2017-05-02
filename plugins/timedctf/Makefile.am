lib_LTLIBRARIES = timedctf.la

timedctf_la_SOURCES = timedctf.cpp
timedctf_la_CPPFLAGS = -I$(top_srcdir)/include
timedctf_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.timedctf.txt \
	help_timedctf_admin.txt \
	help_timedctf_rules.txt \
	timedctf.sln \
	timedctf.vcxproj	\
	timedctf.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
