/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef UIMAP_H
#define UIMAP_H

/* global interface headers */
#include "common.h"

/* system interface headers */
#include <map>
#include <string>

#include "BZAdminUI.h"
#include "PlayerInfo.h"
#include "Singleton.h"


class BZAdminClient;


/** The function type that creates interface objects. */
typedef BZAdminUI* (*UICreator)(BZAdminClient&);


/** This class maps strings to BZAdmin interfaces (subclasses of
    BZAdminUI). New interface classes should register using the UIAdder
    class. */
class UIMap : public std::map<std::string, UICreator>,
	      public Singleton<UIMap>
{
protected:
  friend class Singleton<UIMap>;
  /** The constructor is hidden, this is a singleton. */
  UIMap();

};


/** A helper class that can be used to add interfaces when the program loads.
    To register the UI class @c MyBZAdminUI, you need a UICreator that returns
    a pointer to a @c MyBZAdminUI (casted to a BZAdminUI *). Then you include
    a static member variable of the type @c UIAdder in @c MyBZAdminUI, and
    initialize it with the constructor call
    <code>UIAdder MyBZAdminUI::myUIAdder("my_ui", &MyBZAdminUI::myCreator)
    </code>. The constructor will then register @c myCreator in the UIMap with
    the name "my_ui". */
class UIAdder {
public:
  UIAdder(const std::string& name, UICreator creator);
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
