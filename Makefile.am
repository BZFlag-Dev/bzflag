SUBDIRS = src data debian include misc man package win32

package: FORCE
	cd package && $(MAKE) targets

FORCE:

EXTRA_DIST = BUGS bzflag.lsm DEVINFO LICENSE PORTING $(wildcard README.*) \
	RELNOTES TODO
