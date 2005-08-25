noinst_LTLIBRARIES = libNet.la

if BUGGY_MIPSPRO
MIPSPRO_CXXFLAGS = -LANG:std
else
MIPSPRO_CXXFLAGS = @CXXFLAGS@
endif

AM_CXXFLAGS = $(MIPSPRO_CXXFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/include

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