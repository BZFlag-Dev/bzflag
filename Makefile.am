SUBDIRS = src man package data misc include

package: FORCE
	cd package && $(MAKE) targets

FORCE:

EXTRA_DIST = BUGS bzflag.lsm DEVINFO LICENSE PORTING $(wildcard README.*) \
	RELNOTES TODO
