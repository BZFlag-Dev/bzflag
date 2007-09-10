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

/* class interface header */
#include "Language.h"

/* implementation-specific headers */
#include <fstream>
#include <string>
#include "FileManager.h"

/* private */

/* protected */

Language::Language(int numberCode, std::string iso2Code, std::string iso3Code, std::string english, std::string french)
  : _number(numberCode),
    _iso2(iso2Code),
    _iso3(iso3Code),
    _english(english),
    _french(french)
{
}
Language::~Language()
{
}

bool Language::addLanguage(Language&)
{

  return false;
}


/* public: */

unsigned int Language::loadFromFile(std::string filename, bool verbose)
{
  unsigned int totalAdded = 0;
  char buffer[2048];
  std::istream* stream = FILEMGR.createDataInStream(filename);
  if (stream == NULL) {
    if (verbose) {
      std::cerr << "Warning: '" << filename << "' language file not found" << std::endl;
    }
    return 0;
  }
  std::cout << "loading from " << filename << std::endl;
  while (!stream->eof()) {
    stream->getline(buffer, 2048);
    std::string languageLine = buffer;

    int position = languageLine.find_first_not_of("\r\n\t ");

    // trim leading whitespace
    if (position > 0) {
      languageLine = languageLine.substr(position);
    }

    position = languageLine.find_first_of("#\r\n");

    // trim trailing comments
    if ((position >=0) && (position < (int)languageLine.length())) {
      languageLine = languageLine.substr(0, position);
    }

    position = languageLine.find_last_not_of("\r\n\t ");
    position += 1;

    // trim trailing whitespace
    if ((position >= 0) && (position < (int)languageLine.length())) {
      languageLine = languageLine.substr(0, position);
    }

    // make sure there is something left to add
    if (languageLine.length() == 0) {
      continue;
    }

    if (verbose) {
      std::cout << ".";
    }

    // parse out the keywords
    int num = -1;
    std::string iso2 = "";
    std::string iso3 = "";
    std::string english = "";
    std::string french = "";

    // need at least the number and the iso2
    if ((num == -1) || (iso2.length() == 0)) {
      continue;
    }

    Language lang = Language(num, iso2, iso3, english, french);

    if (addLanguage(lang) && verbose) {
      std::cout << std::endl << "Language is already added: " << iso2;
    } else {
      totalAdded++;
    }

  } // end iteration over lines in input file

  if (verbose) {
    std::cout << std::endl;
  }

  return totalAdded;
} // end loadFromFile


int Language::number() const
{
  return _number;
}
std::string Language::iso2() const
{
  return _iso2;
}
std::string Language::iso3() const
{
  return _iso3;
}
std::string Language::englishName() const
{
  return _english;
}
std::string Language::frenchName() const
{
  return _french;
}


int Language::number(int country)
{
  // XXX - validate number
  return country;
}
int Language::number(std::string)
{
  return 0;
}
std::string Language::iso2(int)
{
  return "";
}
std::string Language::iso2(std::string)
{
  return "";
}
std::string Language::iso3(int)
{
  return "";
}
std::string Language::iso3(std::string)
{
  return "";
}
std::string Language::englishName(int)
{
  return "";
}
std::string Language::englishName(std::string)
{
  return "";
}
std::string Language::frenchName(int)
{
  return "";
}
std::string Language::frenchName(std::string)
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

