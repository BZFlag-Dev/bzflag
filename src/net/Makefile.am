noinst_LTLIBRARIES = libNet.la

if BUGGY_MIPSPRO
MIPSPRO_CXXFLAGS = -LANG:std
endif

AM_CXXFLAGS = $(CONF_CXXFLAGS) $(MIPSPRO_CXXFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/include $(CONF_CPPFLAGS)

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
