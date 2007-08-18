SUBDIRS =
DIST_SUBDIRS = linux rpm irix win32
TARGETDIR  = $(top_srcdir)/bin
PACKAGEDIR = $(top_srcdir)/dist

MAINTAINERCLEANFILES = \
	Makefile.in

#
# tools
#
if WIN32
MKDIR	= mkdir.exe
RM	= del.exe /F /Q
RMR	= del.exe /F /Q /S
else
if IRIX
MKDIR	= /usr/bin/mkdir
RM	= /usr/bin/rm -f
CD	= cd
else
if MACOS
MKDIR	= /bin/mkdir
RM	= /bin/rm
else
MKDIR	= mkdir
RM	= rm -f
endif
endif
RMR	= $(RM) -r
endif

if LINUX
if PC
ARCH = i386
else
if PPC
ARCH = ppc
else
ARCH  = `uname -m | sed -e 's/i.86/i386/'`
endif
endif

RPM	= rpm
RPMBASEDIR = tmp
RPMDIR     = $(RPM)/$(RPMBASEDIR)
RPMRC      = rpmrc
SPECFILE   = spec
RPMROOT    = $(RPMDIR)/root
TMPROOT    = /var/tmp/bzflag-$(VERSIONID)-root

LDIRT    = $(RPM)/$(RPMRC)

#
# build RPM package
#
targets: binprep rpmprep
	rpmversion=`rpm --version | sed -e 's/[^0-9]*\([0-9]*\).*/\1/'`; \
	if [ $$rpmversion -ge 3 ]; then \
	  cd $(RPM); rpm -bb --target $(ARCH)-linux \
			--rcfile $(RPMRC) $(RPMBASEDIR)/SPECS/$(SPECFILE); \
	else \
	  cd $(RPM); rpm -bb --buildarch $(ARCH) \
			--rcfile $(RPMRC) $(RPMBASEDIR)/SPECS/$(SPECFILE); \
	fi
	mv $(RPMDIR)/RPMS/$(ARCH)/*.rpm $(PACKAGEDIR)

#
# prepare binaries
#
binprep:
	strip ../src/bzflag/bzflag
	strip ../src/bzfs/bzfs

#
# prepare RPM stuff
#
rpmprep:
# clean up
	-$(RMR) $(RPMDIR) $(TMPROOT)

# make RPM area
	$(MKDIR) $(RPMDIR) \
		$(RPMDIR)/RPMS $(RPMDIR)/RPMS/$(ARCH) \
		$(RPMDIR)/SOURCES $(RPMDIR)/SPECS $(RPMDIR)/SRPMS

# build area just our top level directory
	ln -s `cd $(DEPTH); pwd` $(RPMDIR)/BUILD

# adjust spec file.  replace version number and don't do anything
# in the prep stage.  also tell rpm where the config file says the
# data is supposed to go.
#
# also what version of rpm?  rpm 2.5 appears to need topdir in
# rpmrc instead of defined in the spec file but rpm 3.0 won't
# use that.
	rpmversion=`rpm --version | sed -e 's/[^0-9]*\([0-9]*\).*/\1/'`; \
	( if [ $$rpmversion -ge 3 ]; then \
		echo "%define _topdir $(RPMBASEDIR)"; fi; \
		cat $(RPM)/$(SPECFILE) ) | \
		sed -e 's/%setup -q -c//' \
		-e 's/%define version.*/%define version '$(VERSIONID)/ \
		-e 's@%define bzdatadir.*@%define bzdatadir '$(pkgdatadir)@ > \
						$(RPMDIR)/SPECS/$(SPECFILE)
# make rpmrc file
	$(ECHO) "" > $(RPM)/$(RPMRC)
	rpmversion=`rpm --version | sed -e 's/[^0-9]*\([0-9]*\).*/\1/'`; \
	if [ $$rpmversion -lt 3 ]; then \
	    $(ECHO) "topdir: $(RPMBASEDIR)" >> $(RPM)/$(RPMRC); \
	fi

# make buildroot named in spec file point into our RPM root area
	ln -s `pwd`/$(RPMROOT) $(TMPROOT)

#
# easy way to make the spec file.  note that this isn't used for the
# build.  it's just a handy way to get an up to date spec file for
# use directly with rpm.
#
newspec:
# adjust spec file.  replace version number and tell rpm where
# the config files say the data is supposed to go.
	cat $(RPM)/$(SPECFILE) | \
		sed -e 's/%define version.*/%define version '$(VERSIONID).$(VERSIONBUILD)/ \
		-e 's@%define bzdatadir.*@%define bzdatadir '$(pkgdatadir)@ > \
								newspec

clean: $(COMMONPREF)clean

clobber: $(COMMONPREF)clobber
	$(RMR) $(RPMDIR) $(TMPROOT) newspec
else
if IRIX

SPECFILE = irix/spec-$(CONFIG)
IDBFILE  = irix/idb
TARDIST  = bzflag.tardist
GENDIST = $(TOOLROOT)/usr/sbin/gendist

targets: tardist

dist: $(_FORCE)
	@VERSION=$(VERSION); export VERSION; \
	VERSIONID=$(VERSIONID); export VERSIONID; \
	if test ! -d $(PACKAGEDIR); then $(MKDIR) $(PACKAGEDIR); fi; \
	if test ! -d $(PACKAGEDIR)/irix; then $(MKDIR) $(PACKAGEDIR)/irix; fi; \
	($(CD) $(PACKAGEDIR)/irix; $(RM) *); \
	USEALLIMAGES=`grep "^[	 ]*product" < $(SPECFILE) | \
		awk '{list=list $2 " "}END{print list}'`; \
	$(GENDIST) -verbose -all -root / -source `\pwd`/$(DEPTH) \
		-idb $(IDBFILE) -spec $(SPECFILE) \
		-dist $(PACKAGEDIR)/irix $$USEALLIMAGES

tardist: dist $(_FORCE)
	($(CD) $(PACKAGEDIR)/irix; tar cf ../$(TARDIST) *)


endif
endif
