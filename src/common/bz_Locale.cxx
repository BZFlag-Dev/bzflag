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

#include "bz_Locale.h"

// implementation-specific headers
#include <string>
#include "Language.h"
#include "Country.h"


/* private */

/* protected */

/* public: */

Locale::Locale(std::string _language_, std::string _country_)
{
  setLanguage(_language_);
  setCountry(_country_);
  return;
}

Locale::~Locale(void)
{
  return;
}

void Locale::setLanguage(std::string _language_)
{
  _language = Language::number(_language_);
}

void Locale::setCountry(std::string _country_)
{
  _country = Country::number(_country_);
}

std::string Locale::language() const
{
  return Language::iso2(_language);
}

std::string Locale::language3() const
{
  return Language::iso3(_language);
}

std::string Locale::languageName() const
{
  return Language::englishName(_language);
}


std::string Locale::country() const
{
  return Language::iso2(_country);
}

std::string Locale::country3() const
{
  return Language::iso3(_country);
}

std::string Locale::countryName() const
{
  return Country::englishName(_country);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
