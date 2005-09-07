/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_BUNDLE_H
#define BZF_BUNDLE_H

// common header first
#include "common.h"

// system headers
#include <map>
#include <vector>
#include <string>

typedef std::map<std::string, std::string> BundleStringMap;

class Bundle
{
public:
  /** Localize a string */
  std::string getLocalString(const std::string &key) const;
  std::string formatMessage(const std::string &key, const std::vector<std::string> *parms) const;

private:
  typedef enum { tERROR, tCOMMENT, tMSGID, tMSGSTR, tAPPEND } TLineType;

  Bundle(const Bundle *pBundle);
  Bundle(const Bundle &xBundle);
  Bundle& operator=(const Bundle &xBundle);
  void load(const std::string &path);
  TLineType parseLine(const std::string &line, std::string &data) const;
  void ensureNormalText(std::string &msg);
  BundleStringMap mappings;

  friend class BundleMgr;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

