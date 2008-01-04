/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef AUTOCOMPLETER_H
#define AUTOCOMPLETER_H

#include <string>
#include <vector>


/** This class will try to complete strings to registered words. */
class AutoCompleter {
  public:
    /** Use this function to register a new word that strings should be
	checked against and possibly completed to. Empty strings will not
	be registered. */
    void registerWord(const std::string& str, bool quoteString = false);

    /** Use this function to unregister a word. If the word hasn't been
	registered previously nothing will happen. */
    void unregisterWord(const std::string& str);

    /** This function will search the list of registered words and see if
	the given string can be completed to any of those words. If the string
	can be completed to several words, their largest common prefix will
	be returned. */
    std::string complete(const std::string& str, std::string* matches = NULL);

  protected:
    class WordRecord {
      public:
	WordRecord(const std::string& str, bool quoteString);
	bool operator<(const WordRecord&) const;
	bool operator==(const WordRecord&) const;
	bool operator!=(const WordRecord&) const;
      public:
	std::string word;
	bool quoteString;
    };
    std::vector<WordRecord> words;
};


/** This class will try to complete strings to registered words.
    It starts with a bunch of common /-commands */
class DefaultCompleter : public AutoCompleter {
  public:
    /** ctor sets default words */
    DefaultCompleter();

    /** This function sets the list of registered words to a default which
	consists of some /-commands; possible other words are removed */
    void setDefaults();
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
