check_PROGRAMS = unittests

unittests_SOURCES = \
        tests.cxx \
	bans.cxx \
	../src/bzfs/AccessControlList.cxx

unittests_CXXFLAGS = -lCppUTest

EXTRA_DIST = 

MAINTAINERCLEANFILES = \
        Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/bzfs \
	$(CONF_CPPFLAGS) \
	-I$(top_srcdir)/include \
	-DCPPUTEST
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

LDADD = \
	../src/common/libCommon.la \
	$(LIBREGEX)
