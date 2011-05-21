lib_LTLIBRARIES = weaponArena.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

weaponArena_la_SOURCES = weaponArena.cpp
weaponArena_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	README.txt \
	weaponArena.def \
	weaponArena.sln \
	weaponArena.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
