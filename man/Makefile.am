if HASPOD2MAN
man_MANS = bzadmin.6 bzflag.6 bzfs.6 bzw.5 bzfquery.6
else
man_MANS = bzadmin.6 bzflag.6 bzfs.6 bzw.5
endif

bzfquery.6: $(top_srcdir)/misc/query/bzfquery.pl
	pod2man $(top_srcdir)/misc/query/bzfquery.pl > bzfquery.6

CLEANFILES = bzfquery.6

MAINTAINERCLEANFILES = \
	Makefile.in
