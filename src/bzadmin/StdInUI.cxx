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

#include <iostream>

#include "StdInUI.h"

using namespace std;


// add this UI to the map
UIAdder StdInUI::uiAdder("stdin", &StdInUI::creator);


bool StdInUI::checkCommand(string& str) {
  if (cin.eof()) {
    str = "/quit";
    return true;
  }
  getline(cin, str);
  if (str == "")
    return false;
  return true;
}


BZAdminUI* StdInUI::creator(const map<PlayerId, string>&, PlayerId) {
  return new StdInUI();
}
