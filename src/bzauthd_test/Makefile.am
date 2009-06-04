#
# Build BZAuthd
#

bin_PROGRAMS = bzauthd_test

AM_CPPFLAGS = \
	-DBUILDING_BZAUTHD_TEST \
	-I$(top_srcdir)/src/bzAuthCommon \
	$(LIBGCRYPT_CFLAGS) \
	$(LIBGPG_ERROR_CLAGS) \
	$(OPENLDAP_CFLAGS) \
	-I$(top_srcdir)/include \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

bzauthd_test_LDFLAGS = -static
bzauthd_test_SOURCES = \
	gle-column \
	common.cpp \
	common.h \
	gcrypt.cpp \
	ldap.cpp \
	Makefile.am \
	tcp-net.cpp \
	test.cpp

LDADD = \
	../bzAuthCommon/bzAuthCommon.la \
	../common/libCommon.la \
	${LIBLDAP} 

MAINTAINERCLEANFILES = \
	Makefile.in
