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

#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include "common.h"

// system interface headers
#include <string>
#include <vector>


/** ISO 639 language representation */
class Language
{
 private:
  int _number;
  std::string _iso2;
  std::string _iso3;
  std::string _english;
  std::string _french;

  static std::vector<Language> _language;

 protected:

  Language(int numberCode, std::string iso2Code, std::string iso3Code="", std::string english="", std::string french="");
  ~Language();

  static bool addLanguage(Language& language);

 public:

  /** loads entries from a file and return the count added */
  static unsigned int loadFromFile(std::string filename, bool verbose=false);

  /** returns the language code */
  int number() const;
  /** returns the 2-char language abbreviation */
  std::string iso2() const;
  /** returns the 3-char language abbreviation */
  std::string iso3() const;
  /** returns the english name for display */
  std::string englishName() const;
  /** returns the french name for display */
  std::string frenchName() const;


  /** returns the language code */
  static int number(int code);
  static int number(std::string language);
  /** returns the 2-char language abbreviation */
  static std::string iso2(int code);
  static std::string iso2(std::string language);
  /** returns the 3-char language abbreviation */
  static std::string iso3(int code);
  static std::string iso3(std::string language);
  /** returns the english name for display */
  static std::string englishName(int code);
  static std::string englishName(std::string language);
  /** returns the french name for display */
  static std::string frenchName(int code);
  static std::string frenchName(std::string language);

  /*
  static const Language CHINESE = Language("zh");
  static const Language ENGLISH = Language("en");
  static const Language FRENCH = Language("fr");
  static const Language GERMAN = Language("de");
  static const Language ITALIAN = Language("it");
  static const Language JAPANESE = Language("jp");
  static const Language KOREAN = Language("ko");
  static const Language SIMPLIFIED_CHINESE = Language("zh");
  static const Language TRADITIONAL_CHINESE = Language("zh");
  */

}; /* class Language */


#else
class Language;
#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
