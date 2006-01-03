# Makefile.PL
#
# Copyright (c) 2003 - 2006 Tucker McLean, Tim Riker.
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named COPYING that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
###

use 5.006001;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
  'NAME'		=> 'BZFlag::Info',
  'VERSION_FROM'	=> 'Info.pm', # finds $VERSION
  'PREREQ_PM'		=> {LWP::UserAgent => 0, Socket => 0}, # e.g., Module::Name => 1.1
  ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
    (ABSTRACT_FROM => 'Info.pm', # retrieve abstract from module
     AUTHOR     => 'Tucker McLean <tuckerm@noodleroni.com>') : ()),
);

# Local Variables: ***
# mode: Perl ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
