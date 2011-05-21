noinst_LTLIBRARIES = libNet.la

if BUGGY_MIPSPRO
MIPSPRO_CXXFLAGS = -LANG:std
else
MIPSPRO_CXXFLAGS = @CXXFLAGS@
endif

AM_CXXFLAGS = $(MIPSPRO_CXXFLAGS) @CONF_CXXFLAGS@

AM_CPPFLAGS = -I$(top_srcdir)/src/include @CONF_CPPFLAGS@

MAINTAINERCLEANFILES = \
	Makefile.in

libNet_la_LDFLAGS = -static
libNet_la_SOURCES =			\
	Address.cpp			\
	AresHandler.cpp			\
	Pack.cpp			\
	Ping.cpp			\
	htond.cpp			\
	multicast.cpp			\
	network.cpp

EXTRA_DIST = \
	README
