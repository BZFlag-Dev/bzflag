# $Id: Makefile.am 234 2007-07-31 09:07:21Z brlcad $

include_HEADERS = \
	IRCClient.h \
	IRCEvents.h \
	IRCNumerics.h \
	IRCServer.h \
	IRCTextUtils.h	\
	IRCUserManager.h \
	Singleton.h \
	TCPConnection.h \
	ircCommands.h \
	libIRC.h \
	net.h

noinst_HEADERS = \
	config.h \
	libirc_version.h

EXTRA_DIST = \
	config.h.in \
	version

BUILT_SOURCES = \
	libirc_version.h \
	version

DISTCLEANFILES = \
	$(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

libirc_version.h:
	@sh $(top_srcdir)/misc/vers.sh libirc_version "libIRC" > $@

# make sure that the build date/version is updated every time make is run
all-local:
	@sh $(top_srcdir)/misc/vers.sh libirc_version "libIRC" > libirc_version.h

include $(top_srcdir)/misc/Makefile.defs
