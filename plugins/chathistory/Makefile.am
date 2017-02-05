lib_LTLIBRARIES = chathistory.la

chathistory_la_SOURCES = chathistory.cpp
chathistory_la_CPPFLAGS = -I$(top_srcdir)/include
chathistory_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.chathistory.txt \
	chathistory.sln \
	chathistory.vcxproj	\
	chathistory.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
