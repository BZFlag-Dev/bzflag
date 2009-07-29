noinst_LTLIBRARIES = libbzAuthCommon.la

AM_CFLAGS = -D_XOPEN_SOURCE @CONF_CFLAGS@

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(LIBGCRYPT_CFLAGS) \
	$(LIBGPG_ERROR_CLAGS) \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libbzAuthCommon_la_LDFLAGS = -static

libbzAuthCommon_la_LIBADD = \
	$(GPG_ERROR_LIBS) \
	$(LIBGCRYPT_LIBS)

libbzAuthCommon_la_SOURCES  = \
	../../plugins/plugin_utils/plugin_config.cpp \
	../../plugins/plugin_utils/plugin_config.h \
	AuthProtocol.cxx \
	AuthProtocol.h \
	base64.cpp \
	base64.h \
	Common.cxx \
	Log.cxx \
	Log.h \
	mongoose.cxx \
	mongoose.h \
	net.cpp \
	net.h \
	RSA.cxx \
	RSA.h \
	Socket.cxx \
	Socket.h \
	TCPConnection.cpp \
	TCPConnection.h


MAINTAINERCLEANFILES = \
	Makefile.in
