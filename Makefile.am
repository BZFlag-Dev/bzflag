SUBDIRS = src data debian include misc man package win32

package: FORCE
	cd package && $(MAKE) targets

FORCE:

EXTRA_DIST = acconfig.h BUGS bzflag.lsm DEVINFO PORTING $(wildcard README.*) \
	RELNOTES TODO
