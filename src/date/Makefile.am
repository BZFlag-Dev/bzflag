noinst_LIBRARIES = libDate.a

AM_CXXFLAGS = -DBZ_BUILD_DATE=$$(date +%Y%m%d)

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	README

libDate_a_SOURCES  =			\
	buildDate.cxx

all-local:
	-rm buildDate.o
