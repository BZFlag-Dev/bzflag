lib_LTLIBRARIES = chathistory.la

chathistory_la_SOURCES = chathistory.cpp
chathistory_la_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
chathistory_la_CXXFLAGS = @CONF_CXXFLAGS@
chathistory_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	chathistory.def \
	chathistory.sln \
	chathistory.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
