lib_LTLIBRARIES = HoldTheFlag.la

HoldTheFlag_la_SOURCES = HoldTheFlag.cpp
HoldTheFlag_la_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
HoldTheFlag_la_CXXFLAGS = -I$(top_srcdir)/include @CONF_CXXFLAGS@
HoldTheFlag_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	README.txt \
	HoldTheFlag.def \
	HoldTheFlag.sln \
	HoldTheFlag.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
