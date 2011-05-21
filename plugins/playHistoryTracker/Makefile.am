lib_LTLIBRARIES = playHistoryTracker.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

playHistoryTracker_la_SOURCES = playHistoryTracker.cpp
playHistoryTracker_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	playHistoryTracker.def \
	playHistoryTracker.sln \
	playHistoryTracker.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
