/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

#include <algorithm>

#include "AutoCompleter.h"


void AutoCompleter::registerWord(const std::string& str) {
  words.insert(std::lower_bound(words.begin(), words.end(), str), str);
}


void AutoCompleter::unregisterWord(const std::string& str) {
  std::vector<std::string>::iterator iter = std::lower_bound(words.begin(), words.end(), str);
  if (iter != words.end() && *iter == str)
    words.erase(iter);
}


std::string AutoCompleter::complete(const std::string& str) {

  if (str.size() == 0)
    return str;

  // find the first and last word with the prefix str
  std::vector<std::string>::iterator first, last;
  first = std::lower_bound(words.begin(), words.end(), str);
  if (first == words.end() || first->substr(0, str.size()) != str)
    return str;
  std::string tmp = str;
  tmp[tmp.size() - 1]++;
  last = std::lower_bound(first, words.end(), tmp) - 1;

  // return the largest common prefix
  unsigned int i;
  for (i = 0; i < first->size() && i < last->size(); ++i)
    if ((*first)[i] != (*last)[i])
      break;
  return first->substr(0, i);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
