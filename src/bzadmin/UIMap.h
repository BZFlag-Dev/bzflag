/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef UIMAP_H
#define UIMAP_H

#include <map>
#include <string>

#include "BZAdminUI.h"

using namespace std;


/** The function type that creates interface objects. */
typedef BZAdminUI* (*UICreator)(const map<PlayerId, string>& players,
				PlayerId me);


/** This class maps strings to BZAdmin interfaces (subclasses of
    BZAdminUI). New interface classes should register using the UIAdder
    class. */
class UIMap : public map<string, UICreator> {
private:
  /** The constructor is private, this is a singleton. */
  UIMap();

public:

  /** This function returns the single instance of this class. */
  static UIMap& getInstance();

};


/** A helper class that can be used to add interfaces when the program loads.*/
class UIAdder {
public:
  UIAdder(const string& name, UICreator creator);
};

#endif

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
