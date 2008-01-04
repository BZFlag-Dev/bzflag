/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "BZWError.h"

// implementation-specific system headers
#include <string>
#include <iostream>

BZWError::BZWError(std::string _location) :
	  hadError(false), hadWarning(false), location(_location)
{
}

BZWError::~BZWError()
{
}

bool BZWError::fatalError(std::string errorMsg, int line)
{
  // toggle flag
  hadError = true;

  // sanity
  if (line < 0)
    return false;
  if (errorMsg == "")
    errorMsg = "unspecified error";

  // report error
  std::cout << location << ": error";
  if (line != 0)
    std::cout << " (line " << line << ")" ;
  std::cout << ": " << errorMsg << std::endl << std::flush;
  return true;
}

bool BZWError::warning(std::string warningMsg, int line)
{
  // toggle flag
  hadWarning = true;

  // sanity
  if (line < 0)
    return false;
  if (warningMsg == "")
    warningMsg = "unspecified warning";

  // report warning
  std::cout << location << ": warning";
  if (line != 0)
    std::cout << " (line " << line << ")" ;
  std::cout << ": " << warningMsg << std::endl << std::flush;

  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
