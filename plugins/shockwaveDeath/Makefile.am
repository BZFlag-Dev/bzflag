lib_LTLIBRARIES = shockwaveDeath.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

shockwaveDeath_la_SOURCES = shockwaveDeath.cpp
shockwaveDeath_la_LDFLAGS = -module -avoid-version

EXTRA_DIST =			\
	README.txt		\
	shockwaveDeath.def	\
	shockwaveDeath.sln	\
	shockwaveDeath.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
