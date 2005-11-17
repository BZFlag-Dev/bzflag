lib_LTLIBRARIES = hiddenAdmin.la

hiddenAdmin_la_SOURCES = hiddenAdmin.cpp
hiddenAdmin_la_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
hiddenAdmin_la_CXXFLAGS = @CONF_CXXFLAGS@
hiddenAdmin_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	hiddenAdmin.def \
	hiddenAdmin.sln \
	hiddenAdmin.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
