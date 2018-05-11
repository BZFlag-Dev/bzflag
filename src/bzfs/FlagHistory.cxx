/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "FlagHistory.h"

/* system headers */
#include <vector>
#include <string>

#define MAX_FLAG_HISTORY (10)

void FlagHistory::clear() {
  flagHistory.clear();
}

std::string FlagHistory::getStr() {
  std::string flagList;
  std::vector<FlagType*>::iterator fhIt = flagHistory.begin();

  while (fhIt != flagHistory.end()) {
    flagList += " (";
    FlagType * fDesc = (FlagType*)(*fhIt);
    if (fDesc->endurance == FlagNormal) {
      flagList += '*';
      flagList += fDesc->flagName.c_str()[0];
    } else
      flagList += fDesc->flagAbbv;
    flagList += ")";
    ++fhIt;
  }
  return flagList;
}

void FlagHistory::add(FlagType* type) {
  if (flagHistory.size() >= MAX_FLAG_HISTORY)
    flagHistory.erase(flagHistory.begin());
  flagHistory.push_back(type);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
