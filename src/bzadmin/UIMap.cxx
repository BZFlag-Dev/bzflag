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

#include "UIMap.h"


UIMap::UIMap() {

}


UIMap& UIMap::getInstance() {
  static UIMap uiMap;
  return uiMap;
}


UIAdder::UIAdder(const string& name, UICreator creator) {
  UIMap::getInstance()[name] = creator;
}
