# bzflag
# Copyright (c) 1993 - 2002 Tim Riker
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named LICENSE that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

#
# install a particular configuation file as the current configuration
#

DEPTH = ..

include $(DEPTH)/configs/config-$(PLATFORM)

# copy config file to config-sys
default:
	@$(ECHO) > $(DEPTH)/config-sys
	@$(CP) config-$(PLATFORM) $(DEPTH)/config-sys

# copy config file to config-sys and append macros to do debug build
debug:
	@$(ECHO) > $(DEPTH)/config-sys
	@$(CP) config-$(PLATFORM) $(DEPTH)/config-sys
	@$(ECHO) "COPTIMIZER   = $(CDEBUG)" >> $(DEPTH)/config-sys
	@$(ECHO) "CXXOPTIMIZER   = $(CXXDEBUG)" >> $(DEPTH)/config-sys
	@$(ECHO) "DEBUG_RENDERING = -DDEBUG_RENDERING" >> $(DEPTH)/config-sys

# copy config file to config-sys and append macros to do non-optimized build
noopt:
	@$(ECHO) > $(DEPTH)/config-sys
	@$(CP) config-$(PLATFORM) $(DEPTH)/config-sys
	@$(ECHO) "COPTIMIZER   = " >> $(DEPTH)/config-sys
	@$(ECHO) "CXXOPTIMIZER   = " >> $(DEPTH)/config-sys
