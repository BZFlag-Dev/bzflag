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

#include <BZAdminUI.h>

using namespace std;


/** This class maps strings to BZAdmin interfaces. */
class UIMap {
private:
  /** The constructor is private, this is a singleton. */
  UIMap();
  
public:
  
  /** The function type that creates interface objects. */
  typedef BZAdminUI* (*UICreator)(const map<PlayerId, string>& players,
				  PlayerId me);
  /** The type of map that is used internally. */
  typedef map<string, UICreator> map_t;
  
  /** Use this function to add a new UICreator and map it to a string. */
  void addUI(const string& name, UICreator creator);
  /** Use this function to get a reference to the internal map. */
  const map_t& getMap() const;
  
  /** This function returns the single instance of this class. */
  static UIMap& getInstance();

protected:
  
  map_t interfaces;
};


/** A helper class that can be used to add interfaces when the program loads.*/
class UIAdder {
public:
  UIAdder(const string& name, UIMap::UICreator creator);
};

#endif
