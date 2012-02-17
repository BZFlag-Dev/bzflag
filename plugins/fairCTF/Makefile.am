lib_LTLIBRARIES = fairCTF.la

fairCTF_la_SOURCES = fairCTF.cpp
fairCTF_la_CPPFLAGS = -I$(top_srcdir)/include
fairCTF_la_LDFLAGS = -module -avoid-version -shared

EXTRA_DIST = \
	README.txt \
	fairCTF.def \
	fairCTF.sln \
	fairCTF.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
