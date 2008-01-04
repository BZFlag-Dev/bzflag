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

#ifndef __WORDFILTER_H__
#define __WORDFILTER_H__

#include "common.h"

/* system interface headers */
#include <string>
#include <set>
#include <algorithm>
#include <string.h>

#include <fstream>
#include <iostream>

#include <ctype.h>
#include <sys/types.h>

/* common interface headers */
#include "bzregex.h"
#include "TextUtils.h"



/* words are stored by the index of their first letter of
 * UTF-8 format.  it would be nice to eventually support
 * full indexing of
 */
#define MAX_FILTER_SETS 256


/** WordFilter will load a list of words and phrases from a file or one at
 * a time or manually.
 *
 * Loading from file assumes that words/phrases are listed one per line;
 * comments are possible using the shell-style "#" delimiter.	 Words are
 * matched case-insensitive; punctuation and non-newline white-space are
 * always ignored with the default filter.
 *
 * By default (aggressive filtering), the filter will match many
 * additional combinations to avoid the need to list all potential spelling
 * variations and hacks in the filter file.
 *
 * e.g. "dumb ass" should match "dumbass!", "DUMB a s s", "d_u_m_b! a_S_S", etc
 *
 * Strings should also match repetitive identical letter expansions.
 *
 * e.g. "dumb ass" should match "dumb asssss", "dumbaaaass", "dumb as s", etc
 *
 * Strings should also match l33t-speak. (l=1, o=0, e=3, a=@, s=z, i=l, f=ph)
 *
 * e.g. "ass whipe" should match "@sz wh1p3", etc
 *
 * Strings should also match common word suffixes (at least for English)
 *	 (dom|ity|memt|sion|tion|ness|ance|ence|er|or|ist) for nouns
 *	 (ive|en|ic|al|able|y|ous|ful|less) for adjectives
 *	 (en|ize|ate|ify|fy|ed) for verbs
 *	 (ly) for adverbs
 *	 (a|z|r|ah|io|rs|rz|in|n|ster|meister) for slang
 *	 (s|es) for plurality
 *	 (ing|let) for imperfect verb, and diminutive
 *
 * e.g. "dumb ass" should match "dumb assness", "dumb asses", "dumb assly", etc
 *
 * Strings should also match common word prefixes (at least for English)
 *	 (bz|beze) for bzflag-centric words
 *
 * e.g. "bitch" should also match "bzbitch", "beezzeebitch", etc
 *
 * Since all of the above matchings are done for free with aggressive matching,
 * only root words need to be provided.	 For foreign languages, it may be
 * necessary to list all tenses of certain verbs, unless the rules can be
 * strictly and simply quantified.
 *
 * There is also a simple filter mode which is not as resource intensive and
 * performs a literal match with the filter words (so you have to specify
 * absolutely everything you want to filter and all variations).	It is still
 * case-insensitive and ignores punctuation.
 */
class WordFilter
{
 public:

  /** structure for a single filter word, and a compiled regular expression
   */
  typedef struct filterStruct {
    std::string word;
    regex_t *compiled;
  } filter_t;


 private:

  /** used by the simple filter */
  std::string alphabet;

  /** set of characters used to replace filtered content */
  std::string filterChars;

  /** word expressions to be filtered including compiled regexp versions */
  struct expressionCompare {
    bool operator () (const filter_t& word1, const filter_t& word2) const {
      return (strncasecmp(word1.word.c_str(), word2.word.c_str(), 1024) < 0);
    }
  };

  typedef std::set<filter_t, expressionCompare> ExpCompareSet;

  /** main collection of what to filter.  items are stored into
   * the array indexed by the first character of the filter word.
   * this means a sparse array, but it's a small price for
   * minimal hashing and rather fast lookups.
   */
  // XXX consider making a numeric hash to avoid array overflows
  ExpCompareSet filters[MAX_FILTER_SETS];


  /** used by the agressive filter */
  ExpCompareSet suffixes;

  /** used by the agressive filter */
  ExpCompareSet prefixes;


  /** utility method performs an actual replacement of
   * characters in an input character ray within a specified
   * range.
   */
  inline int filterCharacters(char *input, unsigned int start, size_t length, bool filterSpaces) const;

  /** utility method adds a letter to a string if it is not
   * already in the string
   */
  inline void appendUniqueChar(std::string& string, char c) const;

  /** utility method to add words to the prefix set
   */
  inline void addPrefix(const char *word);

  /** utility method to add words to the suffix set
   */
  inline void addSuffix(const char *word);


 protected:

  /** This filter does a simple case-insensitive
   * word comparison that compares all filter
   * words with all alphabetic string sets in the
   * input string. If test is a filter word, then
   * input strings "test", "testy", and "test;"
   * will get filtered to "****", "testy", and
   * "****;" respectively.
   */
  bool simpleFilter(char *input) const;


  /** This filter will take a filter word and
   * create a rather complex regular expression
   * that catches a variety of variations on a
   * word.  Variations include automatic internal
   * expansion, optional punctuation and
   * whitespace, suffix matching (including
   * plurality), leet-speak conversion and more.
   * See the header above for more details.  This
   * filter should be the default.
   */
  bool aggressiveFilter(char *input) const;

  /** provides a pointer to a fresh compiled
   * expression for some given expression
   */
  regex_t *getCompiledExpression(const std::string &expression) const;

  /** returns a set of characters that represent the
   * given character in "l33t-speak"
   */
  std::string l33tspeakSetFromCharacter(const char c) const;

  /** returns what alphabetic character a given char
   * corresponds to (e.g. 3 => e, | => il)
   */
  std::string alphabeticSetFromCharacter(const char c) const;

  /** expands a word into an uncompiled regular
   *  expression.
   */
  std::string expressionFromString(const std::string &word) const;


 public:

  WordFilter(void);
  WordFilter(const WordFilter& filter);
  ~WordFilter(void);

  /** loads a set of bad words from a specified file */
  unsigned int loadFromFile(const std::string &fileName, bool verbose=false);

  /** adds a new filter to the existing filter list */
  bool addToFilter(const std::string &word, const std::string &expression);

  /** given an input string, filter the input
   * using either the simple or agressive filter
   */
  bool filter(char *input, const bool simple=false) const;
  bool filter(std::string &input, const bool simple=false) const;

  /** dump a list of words in the filter to stdout */
  void outputWords(void) const;
  /** dump the filter to stdout (including expressions) */
  void outputFilter(void) const;
  /** retuns a count of how many words are in the filter */
  unsigned long int wordCount(void) const;
};


/** utility method performs an actual replacement of
 * characters in an input character ray within a specified
 * range.
 */
inline int WordFilter::filterCharacters(char *input, unsigned int start, size_t length, bool filterSpaces=false) const
{
  if (input == NULL) {
    return -1;
  }
  if (length <= 0){
    return -1;
  }
  if (strlen(input) < start) {
    return 0;
  }

  int randomCharPos, previousCharPos = -1;
  int maxFilterChar = (int)filterChars.size();
  int count=0;
  for (unsigned int j=0; j < (unsigned int)length; j++) {
    char c = input[start + j];

    // don't repeat random chars
    do {
      randomCharPos = (int)((float)maxFilterChar * (float)bzfrand());
    } while (randomCharPos == previousCharPos);
    previousCharPos = randomCharPos;

    /* when filterspaces is true, we filter everything.
      * otherise the ascii character code ranges for a-z, A-Z, and 0-9
      * are filtered.
      */
    if (filterSpaces) {
      input[start + j] = filterChars[randomCharPos];
      count++;
    } else if (TextUtils::isAlphanumeric(c)) {
      input[start + j] = filterChars[randomCharPos];
      count++;
    } /* else it is non-letters so we can ignore */

  }
    return count;
}


inline void WordFilter::appendUniqueChar(std::string& string, char c) const
{
#ifdef HAVE_STD__COUNT
// ISO standard std::count
  if (std::count(string.begin(), string.end(), c) == 0) {
    string += c;
  }
#else
// old HP-style std::count (SunPRO for instance)
  int n = 0;
  std::count(string.begin(), string.end(), c, n);
  if (n == 0) {
    string += c;
  }
#endif
}


inline void WordFilter::addPrefix(const char *word)
{
  filter_t fix;
  std::pair<ExpCompareSet::iterator, bool> result;
  fix.word = std::string(word);
  fix.compiled = getCompiledExpression(expressionFromString(fix.word));
  result = prefixes.insert(fix);
  if (!result.second && fix.compiled) {
    regfree(fix.compiled);
    free(fix.compiled);
  }
}

inline void WordFilter::addSuffix(const char *word)
{
  filter_t fix;
  std::pair<ExpCompareSet::iterator, bool> result;
  fix.word = std::string(word);
  fix.compiled = getCompiledExpression(expressionFromString(fix.word));
  result = suffixes.insert(fix);
  if (!result.second && fix.compiled) {
    regfree(fix.compiled);
    free(fix.compiled);
  }
}


#else
class WordFilter;
#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
