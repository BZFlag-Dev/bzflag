/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __WORDFILTER_H__
#define __WORDFILTER_H__

#ifdef _WIN32
#  define HAVE_REGEX_H 0
#else
#  define HAVE_REGEX_H 1
#endif

#include <string>
#include <vector>
#include <set>

// work around an ugly STL bug in BeOS
// FIXME someone test whether it is still needed
#ifdef __BEOS__
#define private public
#endif
#include <bitset>
#ifdef __BEOS__
#undef private
#endif

#include <fstream>
#include <iostream>

#include <sys/types.h>

#if HAVE_REGEX_H
#  include <regex.h>
#else
#  define regex_t void
#endif

#include "common.h"


/** utility function returns truthfully whether 
 * given character is an alphanumeric
 */
inline bool isAlphanumeric(const char c)
{
  if (  ( c > 96 && c < 123 ) || 
	( c > 64  && c < 91 ) || 
	( c > 47 && c < 58 )) {
    return true;
  }
  return false;
}

/** utility function returns truthfully whether
 * given character is printable (including white
 * space).
 */
inline bool isPrintable(const char c)
{
  /* 9 is tab, 10 is newline, 32 is space */
  if (((c < 32) && (c != 9) && (c != 10)) || ((unsigned char)c == 127)) {
    return false;
  }
  return true;
}


#if defined (UINT8_MAX)
static const unsigned short int MAX_FILTERS = UINT8_MAX;
#else
static const unsigned short int MAX_FILTERS = 2048;  
#endif

static const unsigned short int MAX_WORDS = 256;


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

 private:
  
  /** used by the simple filter */
  std::string alphabet;
  
  /** set of characters used to replace filtered content */
  std::string filterChars;

  /** structure for a single filter word, the expanded
   * corresponding expression, and a compiled regular
   * expression
   */
  typedef struct filter {
    std::string word;
    std::string expression;
    regex_t *compiled;
  } filter_t;
  
  /** word expressions to be filtered including compiled regexp versions */
  struct expressionCompare {
    bool operator () (const filter_t& word1, const filter_t& word2) const {
      return strncasecmp(word1.word.c_str(), word2.word.c_str(), 1024) < 0;
    }
  };


  /** main collection of what to filter.  items are stored into
   * the array indexed by the first character of the filter word.
   * this means a sparse array, but it's a small price for 
   * minimal hashing and rather fast lookups.
   */
  // XXX consider making a numeric hash to avoid array overflows
  std::set<filter_t, expressionCompare> filters[MAX_FILTERS];


  /** used by the agressive filter */
  std::set<filter_t, expressionCompare> suffixes;

  /** used by the agressive filter */
  std::set<filter_t, expressionCompare> prefixes;


  /** utility method that returns the position of the 
   * first printable character from a string
   */
  inline int firstAlphanumeric(const std::string &input) const
  {
    if (input.size() == 0) {
      return -1;
    }

    int i = 0;
    /* range of printable characters, with failsafe */
    while (!isAlphanumeric(input[i])) {
      i++;
    }
    return i;
  }


  /** utility method that returns the position of the 
   * first printable character from a string
   */
  inline int firstNonalphanumeric(const std::string &input) const
  {
    if (input.size() == 0) {
      return -1;
    }

    int i = 0;
    /* range of printable characters, with failsafe */
    while (isAlphanumeric(input[i])) {
      i++;
    }
    return i;
  }
  

  /** utility method that returns the position of the 
   * first printable character from a string
   */
  inline int firstPrintable(const std::string &input) const
  {
    if (input.size() == 0) {
      return -1;
    }

    int i = 0;
    /* range of printable characters, with failsafe */
    while (!isPrintable(input[i]) && (i < MAX_FILTERS)) {
      i++;
    }
    return i;
  }
  
  /** utility method that returns the position of the
   * first non-printable character from a string
   */
  int firstNonprintable(const std::string &input) const
  {
    if (input.size() == 0) {
      return -1;
    }

    int i = 0;
    /* range of non-printable characters, with failsafe */
    while (isPrintable(input[i]) && (i < MAX_FILTERS)) {
      i++;
    }
    return i;
  }

  /** utility method performs an actual replacement of
   * characters in an input character ray within a specified
   * range.  
   */
  int filterCharacters(char *input, unsigned int start, size_t length, bool filterSpaces=false) const
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
    int maxFilterChar = filterChars.size();
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
      } else if (isAlphanumeric(c)) {
	input[start + j] = filterChars[randomCharPos];
	count++;
      } /* else it is non-letters so we can ignore */
      
    }
    return count;
  }
  
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

  /** expands a word into an uncompiled regular
   *  expression.
   */
  std::string expressionFromString(const std::string &word) const;

  
 public:
  
  WordFilter(void);
  ~WordFilter(void);
  
  /** loads a set of bad words from a specified file */
  unsigned int loadFromFile(const std::string &fileName, bool verbose=false);

  /** adds a new filter to the existing filter list, and 
   * optionally recursively adds all combinations of
   * available suffixes and prefixes.
   */
  bool addToFilter(const std::string &word);
  bool addToFilter(const std::string &word, bool append);
  bool addToFilter(const std::string &word, const std::string &expression);
  bool addToFilter(const std::string &word, const std::string &expression, bool append);
  
  /** given an input string, filter the input
   * using either the simple or agressive filter
   */
  bool filter(char *input, const bool simple=false) const;

  /** dump a list of words in the filter to stdout */
  void outputWords(void) const;
  /** dump the filter to stdout (including expressions) */
  void outputFilter(void) const;
  /** retuns a count of how many words are in the filter */
  unsigned long int wordCount(void) const;
};


#else
class WordFilter;
#endif
// ex: shiftwidth=2 tabstop=8
