lib_LTLIBRARIES = rabbitTimer.la

rabbitTimer_la_SOURCES = rabbitTimer.cpp
rabbitTimer_la_CPPFLAGS = -I$(top_srcdir)/include
rabbitTimer_la_LDFLAGS = -module -avoid-version -shared

EXTRA_DIST = \
	README.txt \
	rabbitTimer.def \
	rabbitTimer.sln \
	rabbitTimer.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
