noinst_LTLIBRARIES = libcares.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CFLAGS = @CONF_CFLAGS@


libcares_la_LDFLAGS = -static
libcares_la_SOURCES =		\
	ares_cancel.c		\
	ares__close_sockets.c	\
	ares_destroy.c		\
	ares_dns.h		\
	ares_expand_name.c	\
	ares_expand_string.c	\
	ares_fds.c		\
	ares_free_hostent.c	\
	ares_free_string.c	\
	ares_gethostbyaddr.c	\
	ares_gethostbyname.c	\
	ares__get_hostent.c	\
	ares_init.c		\
	ares_mkquery.c		\
	ares_parse_a_reply.c	\
	ares_parse_ptr_reply.c	\
	ares_private.h		\
	ares_process.c		\
	ares_query.c		\
	ares__read_line.c	\
	ares_search.c		\
	ares_send.c		\
	ares_strerror.c		\
	ares_timeout.c		\
	ares_version.c		\
	ares_version.h		\
	nameser.h		\
	setup.h 		\
	windows_port.c

EXTRA_DIST = \
	ares.h			\
	CHANGES			\
	FILES			\
	NEWS			\
	README			\
	README.cares		\
	vc/adig/adig.dep	\
	vc/adig/adig.dsp	\
	vc/adig/adig.mak	\
	vc/ahost/ahost.dep	\
	vc/ahost/ahost.dsp	\
	vc/ahost/ahost.mak	\
	vc/areslib/areslib.dep	\
	vc/areslib/areslib.dsp	\
	vc/areslib/areslib.mak	\
	vc/vc.dsw
