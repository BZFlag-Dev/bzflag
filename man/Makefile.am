if HASPOD2MAN
man_MANS = bzadmin.6 bzflag.6 bzfs.6 bzw.5 bzfquery.6
else
man_MANS = bzadmin.6 bzflag.6 bzfs.6 bzw.5
endif

bzfquery.6: $(top_srcdir)/misc/bzfquery.pl
	pod2man $(top_srcdir)/misc/bzfquery.pl > bzfquery.6

DISTCLEANFILES = bzfquery.6

EXTRA_DIST = makeHTML.bat

MAINTAINERCLEANFILES = \
	Makefile.in
