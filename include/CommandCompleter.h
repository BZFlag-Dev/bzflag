/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef COMMANDCOMPLETER_H
#define COMMANDCOMPLETER_H

/* interface header */
#include "AutoCompleter.h"


/**
 * This class will try to complete strings to registered words.
 * It starts with a bunch of common /-commands.
 */
class CommandCompleter : public AutoCompleter {

 public:

    /** ctor sets default words */
    CommandCompleter();

    /**
     * This function sets the list of registered words to a default
     * which consists of some /-commands; possible other words are
     * removed.
     */
    void reset();
};

#endif /* COMMANDCOMPLETER_H */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
