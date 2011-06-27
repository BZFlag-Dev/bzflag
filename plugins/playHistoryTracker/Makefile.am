lib_LTLIBRARIES = playHistoryTracker.la

playHistoryTracker_la_SOURCES = playHistoryTracker.cpp
playHistoryTracker_la_CPPFLAGS = -I$(top_srcdir)/include
playHistoryTracker_la_LDFLAGS = -module -avoid-version -shared

EXTRA_DIST = \
	playHistoryTracker.def \
	playHistoryTracker.sln \
	playHistoryTracker.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
