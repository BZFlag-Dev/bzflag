noinst_LIBRARIES = libNet.a

if BUGGY_MIPSPRO
MIPSPRO_CXXFLAGS = -LANG:std
else
MIPSPRO_CXXFLAGS = @CXXFLAGS@
endif

CXXFLAGS = $(MIPSPRO_CXXFLAGS)

MAINTAINERCLEANFILES = \
	Makefile.in

libNet_a_SOURCES =			\
	Address.cxx			\
	Pack.cxx			\
	Ping.cxx			\
	multicast.cxx			\
	udpnetwork.cxx			\
	network.cxx
