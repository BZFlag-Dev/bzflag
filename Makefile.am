SUBDIRS = data debian include misc man package win32 src

package: FORCE
	cd package && $(MAKE) targets

FORCE:

EXTRA_DIST = acconfig.h BUGS bzflag.lsm DEVINFO PORTING $(wildcard README.*) \
	RELNOTES TODO

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	config.guess \
	config.sub \
	configure \
	INSTALL \
	install-sh \
	missing \
	mkinstalldirs \
	Makefile.in
