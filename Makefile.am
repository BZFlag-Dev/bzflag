SUBDIRS = src man package data bzwtransform debian misc win32 doc include

package: FORCE
	cd package && $(MAKE) targets

FORCE:

EXTRA_DIST = BUGS bzflag.lsm DEVINFO LICENSE PORTING $(wildcard README.*) \
	RELNOTES TODO