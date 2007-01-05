/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __LOCALE_H__
#define __LOCALE_H__

#include "common.h"

// system interface headers
#include <string>


/** Locale will help control the output of strings so that they are
 *  localized to the specified localization. ;)
 */
class Locale
{
 private:
  int _language;
  int _country;

 protected:

 public:
  Locale(std::string language="en", std::string country="US");
  ~Locale();

  void setLanguage(std::string language);
  void setCountry(std::string country);

  /** returns a 2-char language code */
  std::string language() const;
  /** returns a 3-char language code */
  std::string language3() const;
  /** returns an English name for the locale language */
  std::string languageName() const;

  /** returns a 2-char country code */
  std::string country() const;
  /** returns a 3-char country code */
  std::string country3() const;
  /** returns an English name for the locale country */
  std::string countryName() const;

}; /* class Locale */


#else
class Locale;
#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

