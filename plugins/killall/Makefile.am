lib_LTLIBRARIES = killall.la

killall_la_SOURCES = killall.cpp
killall_la_CPPFLAGS = -I../../include
killall_la_LDFLAGS = -module -avoid-version ${top_builddir}/src/bzfs/libBZFSplugin.la

EXTRA_DIST = \
	killall.def \
	killall.sln \
	killall.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
