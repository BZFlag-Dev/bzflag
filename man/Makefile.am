man_MANS = bzadmin.6 bzflag.6 bzfs.6 bzw.5
if HASPOD2MAN
noinst_MANS = bzfquery.6
endif

EXTRA_DIST = bzadmin.6.in bzflag.6.in bzfs.6.in bzw.5.in

bzfquery.6: $(top_srcdir)/misc/bzfquery.pl
	pod2man $(top_srcdir)/misc/bzfquery.pl > $@

edit = sed \
	-e 's|@BUILD_DATE[@]|$(BUILD_DATE)|g' \
	-e 's|@PACKAGE_STRING[@]|$(PACKAGE_STRING)|g' \
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
