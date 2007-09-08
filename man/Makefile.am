if HASPOD2MAN
man_MANS = bzadmin.6 bzflag.6 bzfs.6 bzw.5 bzfquery.6
else
man_MANS = bzadmin.6 bzflag.6 bzfs.6 bzw.5
endif

EXTRA_DIST = bzadmin.6.in bzflag.6.in bzfs.6.in bzw.5.in

bzfquery.6: $(top_srcdir)/misc/bzfquery.pl
	pod2man $(top_srcdir)/misc/bzfquery.pl > bzfquery.6

edit = sed \
	-e 's|@BZFLAG_DATE[@]|$(BZFLAG_DATE)|g' \
	-e 's|@BZFLAG_VERSION[@]|$(BZFLAG_VERSION)|g' \
	-e 's|@INSTALL_DATA_DIR[@]|$(pkgdatadir)|g'

bzflag.6 bzadmin.6 bzfs.6 bzw.5 : Makefile
	$(edit) '$(srcdir)/$@.in' >$@
bzflag.6: $(srcdir)/bzflag.6.in
bzadmin.6: $(srcdir)/bzadmin.6.in
bzfs.6: $(srcdir)/bzfs.6.in
bzw.5: $(srcdir)/bzw.5.in

CLEANFILES = bzadmin.6 bzflag.6 bzfquery.6 bzfs.6 bzw.5

MAINTAINERCLEANFILES = \
	Makefile.in
