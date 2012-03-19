noinst_LTLIBRARIES = libNet.la

if BUGGY_MIPSPRO
AM_CXXFLAGS = -LANG:std
endif

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

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
