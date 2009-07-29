#
# Build BZAuthd
#

bin_PROGRAMS = bzauthd

AM_CPPFLAGS = \
	-DBUILDING_BZAUTHD \
	-I$(top_srcdir)/src/bzAuthCommon \
	-I$(top_srcdir)/plugins/plugin_utils \
	-include common.h \
	$(LIBGCRYPT_CFLAGS) \
	$(GPG_ERROR_CFLAGS) \
	$(OPENLDAP_CFLAGS) \
	-I$(top_srcdir)/include \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

bzauthd_DFLAGS = -static
bzauthd_SOURCES = \
	bzauthd.cxx \
	ClientHandler.cxx \
	Common.cxx \
	ConfigMgr.cxx \
	ConfigMgr.h \
	EventHandler.cxx \
	EventHandler.h \
	HttpHandler.cxx \
	HttpHandler.h \
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
	../bzAuthCommon/libbzAuthCommon.la 	\
	../common/libCommon.la 			\
	$(OPENLDAP_LIBS)

MAINTAINERCLEANFILES = \
	Makefile.in
