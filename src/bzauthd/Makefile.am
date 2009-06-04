#
# Build BZAuthd
#

bin_PROGRAMS = bzauthd

AM_CPPFLAGS = \
	-DBUILDING_BZAUTHD \
	-I$(top_srcdir)/src/bzAuthCommon \
	$(LIBGCRYPT_CFLAGS) \
	$(GPG_ERROR_CFLAGS) \
	$(OPENLDAP_CFLAGS) \
	-I$(top_srcdir)/include \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

bzauthd_DFLAGS = -static
bzauthd_SOURCES = \
	ClientHandler.cxx \
	Common.cxx \
	ConfigMgr.cxx \
	ConfigMgr.h \
	EventHandler.cxx \
	EventHandler.h \
	NetHandler.cxx \
	NetHandler.h \
	Serverhandler.cxx \
	TimeKeeper.cxx \
	TimeKeeper.h \
	TokenMgr.cxx \
	TokenMgr.h \
	UserStorage.cxx \
	UserStorage.h

LDADD = \
	../bzAuthCommon/bzAuthCommon.la \
	../common/libCommon.la \
	${LIBLDAP}

MAINTAINERCLEANFILES = \
	Makefile.in
