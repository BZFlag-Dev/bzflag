# BZFlag
# Copyright (c) 1993 - 2002 Tim Riker
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named LICENSE that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

DEPTH = ..
include $(DEPTH)/Make-common

.SUFFIXES: .1s .2s .3s .4s .5s .6s .7s .8s .1 .2 .3 .4 .5 .6 .7 .8 .z .gz

LDIRT   = *.[1-8].t
TARGETS = *.[1-8] *.z *.gz

#
# have to parse the man page names to generate the target names using
# sed since it's too much for make's limited capabilities.  this macro
# lists the source files, one per line.
#
PRINTSRC = find . \( -name '*.[1-8]s' \) -print |		\
	sed -e 's/^\.\///' -e '/\//d'

targets: sourceform packed gzipped

# generate nroff source
sourceform:
	$(PRINTSRC) |						\
	sed -e 's/\.\([1-8]\)s$$/.\1/' |			\
	xargs -l10 $(MAKE) $(MFLAGS)

# generate packed compiled nroff
packed: sourceform
	$(PRINTSRC) |						\
	sed -e 's/\.[1-8]s$$/\.z/' |				\
	xargs -l10 $(MAKE) $(MFLAGS)

# generate gzipped compiled nroff
gzipped: sourceform
	$(PRINTSRC) |						\
	sed -e 's/\.[1-8]s$$/\.gz/' |				\
	xargs -l10 $(MAKE) $(MFLAGS)

# replace tokens in source to make nroff source
.1s.1 .2s.2 .3s.3 .4s.4 .5s.5 .6s.6 .7s.7 .8s.8:
	cat $< | 						\
	sed -e "s@INSTALL_DATA_DIR@$(INSTALL_DATA_DIR)@g" >$@

# compile and pack
.1.z .2.z .3.z .4.z .5.z .6.z .7.z .8.z:
	$(RM) $@
	-$(NROFF) -man $< > $<.t && pack -f $<.t && mv $<.t.z $@

# compile and gzip
.1.gz .2.gz .3.gz .4.gz .5.gz .6.gz .7.gz .8.gz:
	$(RM) $@
	--$(NROFF) -man $< > $<.t && gzip $<.t && mv $<.t.gz $@
