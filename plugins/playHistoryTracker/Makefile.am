lib_LTLIBRARIES = playHistoryTracker.la

playHistoryTracker_la_SOURCES = playHistoryTracker.cpp
playHistoryTracker_la_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
playHistoryTracker_la_CXXFLAGS = @CONF_CXXFLAGS@
playHistoryTracker_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	playHistoryTracker.def \
	playHistoryTracker.sln \
	playHistoryTracker.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
