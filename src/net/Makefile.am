noinst_LIBRARIES = libNet.a

if BUGGY_MIPSPRO
MIPSPRO_CXXFLAGS = -LANG:std
else
MIPSPRO_CXXFLAGS = @CXXFLAGS@
endif

AM_CXXFLAGS = $(MIPSPRO_CXXFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/include

MAINTAINERCLEANFILES = \
	Makefile.in

libNet_a_SOURCES =			\
	Address.cxx			\
	AdnsHandler.cxx			\
	Pack.cxx			\
	Ping.cxx			\
	multicast.cxx			\
	udpnetwork.cxx			\
	network.cxx

EXTRA_DIST = \
	README