noinst_LTLIBRARIES =	libTank.la

AM_CPPFLAGS = -I$(top_srcdir)/src/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libTank_la_LDFLAGS = -static
libTank_la_SOURCES =	\
	high_barrel.cpp		\
	high_body.cpp		\
	high_ltread.cpp		\
	high_rtread.cpp		\
	high_turret.cpp		\
	low_barrel.cpp		\
	low_body.cpp		\
	low_ltread.cpp		\
	low_rtread.cpp		\
	low_turret.cpp		\
	medium_barrel.cpp	\
	medium_body.cpp		\
	medium_ltread.cpp	\
	medium_rtread.cpp	\
	medium_turret.cpp

MAINTAINERCLEANFILES = \
	Makefile.in
