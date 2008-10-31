/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// implementation header
#include "AutoCompleter.h"

// system headers
#include <ctype.h>
#include <string.h>
#include <algorithm>


AutoCompleter::WordRecord::WordRecord(const std::string& w, bool q)
{
  word = w;
  quoteString = q;
}

bool AutoCompleter::WordRecord::operator<(const WordRecord& w) const
{
  return (word < w.word);
}

bool AutoCompleter::WordRecord::operator==(const WordRecord& w) const
{
  return (word == w.word);
}

bool AutoCompleter::WordRecord::operator!=(const WordRecord& w) const
{
  return (word != w.word);
}


void AutoCompleter::registerWord(const std::string& str, bool quoteString)
{
  // only use 'quoteString' if it applies
  if (quoteString) {
    quoteString = false;
    for (int i = 0; i < (int)str.size(); i++) {
      if (isspace(str[i])) {
	quoteString = true;
	break;
      }
    }
  }
  WordRecord rec(str, quoteString);
  words.insert(std::lower_bound(words.begin(), words.end(), rec), rec);
}


void AutoCompleter::unregisterWord(const std::string& str)
{
  WordRecord rec(str, false);
  while (true) {
    std::vector<WordRecord>::iterator iter =
      std::lower_bound(words.begin(), words.end(), rec);
    if (iter != words.end() && *iter == rec) {
      words.erase(iter);
    } else {
      return;
    }
  }
}


std::string AutoCompleter::complete(const std::string& str, std::string* matches)
{
  if (str.size() == 0) {
    return str;
  }

  // from the last space
  const int lastSpace = str.find_last_of(" \t");
  const std::string tail = str.substr(lastSpace + 1);
  if (tail.size() == 0) {
    return str;
  }
  const std::string head = str.substr(0, lastSpace + 1);

  // find the first and last word with the prefix str
  std::vector<WordRecord>::iterator first, last;
  WordRecord rec(tail, false);
  first = std::lower_bound(words.begin(), words.end(), rec);
  if ((first == words.end()) ||
      (first->word.substr(0, tail.size()) != tail)) {
    return str; // no match
  }
  std::string tmp = tail;
  tmp[tmp.size() - 1]++;
  last = std::lower_bound(first, words.end(), WordRecord(tmp, false)) - 1;

  // get a list of partial matches
  if (matches != NULL) {
    *matches = "";
    if (first != last) {
      std::vector<WordRecord>::iterator it = first;
      for (it = first; it != (last + 1); it++) {
	std::string tmp2 = it->word;
	// strip the trailing whitespace
	while ((tmp2.size() > 0) && isspace(tmp2[tmp2.size() - 1])) {
	  tmp2.resize(tmp2.size() - 1);
	}
	if (tmp2.size() > 0) {
	  if (it->quoteString) {
	    *matches += "\"" + tmp2 + "\" ";
	  } else {
	    *matches += tmp2 + " ";
	  }
	}
      }
    }
  }

  const bool noQuotes = (lastSpace == -1);

  // return the largest common prefix without any spaces
  const int minLen = first->word.size() < last->word.size() ?
		     first->word.size() : last->word.size();
  int i;
  for (i = 0; i < minLen; ++i) {
    if ((!noQuotes && isspace(first->word[i])) ||
	(first->word[i] != last->word[i])) {
      break;
    }
  }

  if (!noQuotes && first->quoteString && (first == last)) {
    const std::string quoted = "\"" + first->word + "\"";
    return (head + quoted);
  } else {
    return (head + first->word.substr(0, i));
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
