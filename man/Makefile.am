SUFFIXES = .6s .6

if HASPOD2MAN
man_MANS = bzadmin.6 bzflag.6 bzfls.6 bzfrelay.6 bzfs.6 bzfquery.6
else
man_MANS = bzadmin.6 bzflag.6 bzfls.6 bzfrelay.6 bzfs.6
endif

bzfquery.6: $(top_srcdir)/misc/bzfquery.pl
	pod2man $(top_srcdir)/misc/bzfquery.pl > bzfquery.6

BUILD_VERSION=$(shell echo $(VERSION) | sed -e "s/\(.*\)\..*/\1/g")
BUILD_DATE=$(shell echo $(VERSION) | sed -e "s/.*\.\([0-9][0-9][0-9][0-9]\)\([0-9][0-9]\)\([0-9][0-9]\)/\1-\2-\3/g")

# replace tokens in source to make nroff source
.1s.1 .2s.2 .3s.3 .4s.4 .5s.5 .6s.6 .7s.7 .8s.8:
	cat $< 							| \
	sed -e "s'INSTALL_DATA_DIR'$(INSTALL_DATA_DIR)'g" 	| \
	sed -e "s'BUILD_DATE'$(BUILD_DATE)'g"			| \
	sed -e "s'BUILD_VERSION'$(BUILD_VERSION)'g" > $@

EXTRA_DIST = bzflag.6s bzfrelay.6s bzfs.6s bzfls.6s bzadmin.6s

DISTCLEANFILES = bzflag.6 bzfrelay.6 bzfs.6 bzfls.6 bzadmin.6 bzfquery.6

MAINTAINERCLEANFILES = \
	Makefile.in
