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

#ifndef AUTOCOMPLETER_H
#define AUTOCOMPLETER_H

#include <string>
#include <vector>

using namespace std;

/** This class will try to complete strings to registered words. */
class AutoCompleter {
 public:

  /** Use this function to register a new word that strings should be
      checked against and possibly completed to. Empty strings will not
      be registered. */
  void registerWord(const string& str);

  /** Use this function to unregister a word. If the word hasn't been
      registered previously nothing will happen. */
  void unregisterWord(const string& str);

  /** This function will search the list of registered words and see if
      the given string can be completed to any of those words. If the string
      can be completed to several words, their largest common prefix will
      be returned. */
  string complete(const string& str);

 protected:
  vector<string> words;
};

#endif

/* Local Variables: ***
 * mode:C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
