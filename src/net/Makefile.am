noinst_LTLIBRARIES = libNet.la

if BUGGY_MIPSPRO
AM_CXXFLAGS = -LANG:std
endif

MAINTAINERCLEANFILES = \
	Makefile.in

libNet_la_LDFLAGS = -static
libNet_la_SOURCES =			\
	Address.cxx			\
	AresHandler.cxx			\
	Pack.cxx			\
	Ping.cxx			\
	multicast.cxx			\
	network.cxx

EXTRA_DIST = \
	README
