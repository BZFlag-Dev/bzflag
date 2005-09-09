lib_LTLIBRARIES = playHistoryTracker.la

playHistoryTracker_la_SOURCES = playHistoryTracker.cpp
playHistoryTracker_la_CPPFLAGS = -I../../include
playHistoryTracker_la_LDFLAGS = -module -avoid-version ${top_builddir}/src/bzfs/libBZFSplugin.la

EXTRA_DIST = \
	playHistoryTracker.def \
	playHistoryTracker.sln \
	playHistoryTracker.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
