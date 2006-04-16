/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include "Country.h"

// system headers
#include <string>

/* private */

/* protected */
bool Country::isValid(int)
{
  return false;
}
bool Country::isValid(const std::string &)
{
  return false;
}

/* public: */

Country::Country(std::string)
{
}
Country::~Country()
{
}

int Country::number() const
{
  return 0;
}
std::string Country::iso2() const
{
  return "";
}
std::string Country::iso3() const
{
  return "";
}
std::string Country::englishName() const
{
  return "";
}
std::string Country::frenchName() const
{
  return "";
}


int Country::number(int country)
{
  // XXX - validate number
  return country;
}
int Country::number(const std::string& )
{
  return 0;
}
std::string Country::iso2(int)
{
  return "";
}
std::string Country::iso2(const std::string &)
{
  return "";
}
std::string Country::iso3(int)
{
  return "";
}
std::string Country::iso3(const std::string& )
{
  return "";
}
std::string Country::englishName(int)
{
  return "";
}
std::string Country::englishName(const std::string& )
{
  return "";
}
std::string Country::frenchName(int)
{
  return "";
}
std::string Country::frenchName(const std::string& )
{
  return "";
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

