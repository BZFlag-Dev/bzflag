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
  
  /** used by the agressive filter */
  std::set<std::string> suffixes;
  std::set<std::string> prefixes;
  
  std::string filterChars;
  
  /** structure containing the filter words and optional compiled version */
  typedef struct badWord {
    std::string word;
    std::string expression;
    bool compiled;
    regex_t *compiledWord;
  } badWord_t;
  
  /** word expressions to be filtered including compiled regexp versions */
  struct expressionCompare {
    bool operator () (const badWord_t& word1, const badWord_t& word2) const {
      
      
      return strncasecmp(word1.word.c_str(), word2.word.c_str(), 1024) < 0;
    }
  };
  std::set<badWord_t, expressionCompare> badWords;
  
 protected:

  /** This filter does a simple case-insensitive
   * word comparison that compares all filter 
   * words with all alphabetic string sets in the
   * input string. If test is a filter word, then
   * input strings "test", "testy", and "test;"
   * will get filtered to "****", "testy", and 
   * "****;" respectively.
   */
  bool simpleFilter(char *input);
  
  
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
  bool aggressiveFilter(char *input);
  
  /** provides a pointer to a fresh compiled
   * expression for some given expression
   */
  regex_t *getCompiledExpression(const std::string &word);
  
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
  bool filter(char *input, bool simple=false);

  /** dump a list of words in the filter to stdout */
  void outputWords(void);
};


#else
class WordFilter;
#endif
// ex: shiftwidth=2 tabstop=8
