/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __COUNTRY_H__
#define __COUNTRY_H__

#include "common.h"

// system interface headers
#include <string>

/** Representation of countries, including those described by ISO 3166
 */
class Country
{
 private:

 protected:

  static bool isValid(int country);
  static bool isValid(const std::string& country);

 public:

  Country(std::string);
  ~Country();

  /** returns the ISO 3166 country code */
  int number() const;
  /** returns the ISO 3166 country 2-char abbreviation */
  std::string iso2() const;
  /** returns the ISO 3166 country 3-char abbreviation */
  std::string iso3() const;
  /** returns the ISO 3166 country english name for display */
  std::string englishName() const;
  /** returns the ISO 3166 country french name for display */
  std::string frenchName() const;


  /** returns the ISO 3166 country code */
  static int number(int country);
  static int number(const std::string& country);
  /** returns the ISO 3166 country 2-char abbreviation */
  static std::string iso2(int country);
  static std::string iso2(const std::string& country);
  /** returns the ISO 3166 country 3-char abbreviation */
  static std::string iso3(int country);
  static std::string iso3(const std::string& country);
  /** returns the ISO 3166 country english name for display */
  static std::string englishName(int country);
  static std::string englishName(const std::string& country);
  /** returns the ISO 3166 country french name for display */
  static std::string frenchName(int country);
  static std::string frenchName(const std::string& country);

  /*
  static const Country CANADA = Country("CA");
  static const Country CHINA = Country("ZH");
  static const Country FRANCE = Country("FR");
  static const Country GERMANY = Country("DE");
  static const Country ITALY = Country("IT");
  static const Country JAPAN = Country("JP");
  static const Country KOREA = Country("KO");
  static const Country UK = Country("UK");
  static const Country US = Country("US");
  */

}; /* class Country */


#else
class Country;
#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

