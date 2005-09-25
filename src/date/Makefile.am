noinst_LTLIBRARIES = libDate.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = README

libDate_la_LDFLAGS = -static
libDate_la_SOURCES = \
	buildDate.cxx


# reliably force the library to rebuild every time so that the
# timestamp is updated in the resultant object file.

.PHONY: force
force:
	$(LTCXXCOMPILE) -c -o buildDate.lo buildDate.cxx

buildDate.lo: force
	$(LTCXXCOMPILE) -c -o $@ $<

