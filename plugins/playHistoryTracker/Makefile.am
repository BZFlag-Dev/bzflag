lib_LTLIBRARIES = playHistoryTracker.la

playHistoryTracker_la_SOURCES = playHistoryTracker.cpp
playHistoryTracker_la_CPPFLAGS = -I$(top_srcdir)/include
playHistoryTracker_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	playHistoryTracker.def \
	playHistoryTracker.sln \
	playHistoryTracker.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
