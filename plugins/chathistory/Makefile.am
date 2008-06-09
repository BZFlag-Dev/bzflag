lib_LTLIBRARIES = chathistory.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

chathistory_la_SOURCES = chathistory.cpp
chathistory_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	chathistory.def \
	chathistory.sln \
	chathistory.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
