lib_LTLIBRARIES = regFlag.la

regFlag_la_SOURCES = regFlag.cpp
regFlag_la_CPPFLAGS = -I$(top_srcdir)/include
regFlag_la_LDFLAGS = -module -avoid-version -shared

EXTRA_DIST = \
	README.txt \
	regFlag.def \
	regFlag.sln \
	regFlag.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
