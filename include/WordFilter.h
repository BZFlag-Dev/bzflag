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

#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <regex.h>

#include "common.h"


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
    regex_t compiled;
  } filter_t;
  
  /** word expressions to be filtered including compiled regexp versions */
  struct expressionCompare {
    bool operator () (const filter_t& word1, const filter_t& word2) const {
      
      
      return strncasecmp(word1.word.c_str(), word2.word.c_str(), 1024) < 0;
    }
  };

#if defined (UNIT8_MAX)
  const unsigned short int MAX_FILTERS = UINT8_MAX;
#else
  const unsigned short int MAX_FILTERS = sizeof(unsigned char);  
#endif

  /** main collection of what to filter.  items are stored into
   * the array indexed by the first character of the filter word.
   * this means a sparse array, but it's a small price for 
   * minimal hashing and rather fast lookups.
   */
  std::set<filter_t, expressionCompare> filters[MAX_FILTERS];


  /** used by the agressive filter */
  filter_t *suffixes;

  /** used by the agressive filter */
  filter_t *prefixes;
  
  /** utility method that returns the position of the 
   * first printable character from a string
   */
  int firstPrintable(const std::string &input) const
  {
    if (input == NULL) {
      return -1;
    }

    int i = 0;
    /* range of printable characters, with failsafe */
    while (((input[i] < 33) || (input[i] == 127) || (input[i] == 255)) \
	   && (i < 32768)) {
      i++;
    }
    return i;
  }
  
  /** utility method that returns the position of the
   * first non-printable character from a string
   */
  int firstNonprintable(const std::string &input) const
  {
    if (input == NULL) {
      return -1;
    }

    int i = 0;
    /* range of non-printable characters, with failsafe */
    while (((input[i] > 32) && (input[i] < 255) && (input[i] != 127)) \
	   && (i < 32768)) {
      i++;
    }
    return i;
  }

  int filterCharacters(std::string &input, int start, int end, bool filterSpaces=false) const
  {
    if (input == NULL) {
      return -1;
    }
    int length = end - start;
    if (length <= 0) {
      return -1;
    }

    int count=0;
    for (int j=0; j < length; j++) {
      char c = input[start + j];
      
      // don't repeat random chars
      do {
	randomCharPos = (int)((float)maxFilterChar * (float)bzfrand());
      } while (randomCharPos == previousCharPos);
      previousCharPos = randomCharPos;
      
      /* when filterspaces is truce, we filter everything.
       * otherise the ascii character code ranges for a-z, A-Z, and 0-9 
       * are filtered.
       */
      if (filterSpaces) {
	input[start + j] = filterchars[randomCharPos];
	count++;
      } else  if (  ( c > 96 && c < 123 ) || 
		    ( c > 64  && c < 91 ) || 
		    ( c > 47 && c < 58 )) {
	input[start + j] = filterChars[randomCharPos];
	count++;
      }
      
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
  std::string *expressionFromString(const std::string &word) const;

  
 public:
  
  WordFilter(void);
  ~WordFilter(void);
  
  /** loads a set of bad words from a specified file */
  void loadFromFile(const std::string &fileName);

  /** adds an individual word to the filter list */
  bool addWord(const std::string &word);
  
  /** given an input string, filter the input
   * using either the simple or agressive filter
   */
  bool filter(char *input, const bool simple=false) const;

  /** dump a list of words in the filter to stdout */
  void outputWords(void) const;
};


#else
class WordFilter;
#endif
// ex: shiftwidth=2 tabstop=8
