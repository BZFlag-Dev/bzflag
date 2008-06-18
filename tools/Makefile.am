SUBDIRS = \
	modeltool \
	tankview  \
	worldTest

.PHONY : modeltool worldTest

worldTest:
	@cd $@ && $(MAKE) $(AM_MAKEFLAGS)

modeltool:
	@cd $@ && $(MAKE) $(AM_MAKEFLAGS)

EXTRA_DIST = BZWTestLauncher

MAINTAINERCLEANFILES = \
	Makefile.in
