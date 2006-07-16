/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

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



DefaultCompleter::DefaultCompleter()
{
  setDefaults();
}

void DefaultCompleter::setDefaults()
{
  words.clear();
  registerWord("/ban ");
  registerWord("/banlist");
  registerWord("/checkip ");
  registerWord("/countdown");
  registerWord("/clientquery");
  registerWord("/date");
  registerWord("/dumpvars");
  registerWord("/flag ");
  registerWord("reset");
  registerWord("up");
  registerWord("show");
  registerWord("/flaghistory");
  registerWord("/gameover");
  registerWord("/ghost ");
  registerWord("/grouplist");
  registerWord("/groupperms");
  registerWord("/help");
  registerWord("/highlight ");
  registerWord("/hostban ");
  registerWord("/hostunban ");
  registerWord("/hostbanlist");
  registerWord("/idban ");
  registerWord("/idunban ");
  registerWord("/idbanlist");
  registerWord("/idlist");
  registerWord("/idlestats");
  registerWord("/jitterdrop");
  registerWord("/jitterwarn");
  registerWord("/kick ");
  registerWord("/kill ");
  registerWord("/lagdrop");
  registerWord("/lagstats");
  registerWord("/lagwarn ");
  registerWord("/localset ");
  registerWord("/mute ");
  registerWord("/password ");
  registerWord("/playerlist");
  registerWord("/poll ");
  registerWord("ban");
  registerWord("kick");
  registerWord("kill");
  registerWord("/quit");
  registerWord("/record");
  registerWord("start");
  registerWord("stop");
  registerWord("size");
  registerWord("rate");
  registerWord("stats");
  registerWord("file");
  registerWord("save");
  registerWord("/reload");
  registerWord("/masterban"); // also uses list
  registerWord("reload");
  registerWord("flush");
  registerWord("/removegroup ");
  registerWord("/replay ");
  registerWord("list");
  registerWord("load");
  registerWord("play");
  registerWord("skip");
  registerWord("/report ");
  registerWord("/reset");
  registerWord("/retexture");
  registerWord("/roampos ");
  registerWord("/saveworld ");
  registerWord("/serverquery");
  registerWord("/set");
  registerWord("/setgroup ");
  registerWord("/setpass ");
  registerWord("/showgroup ");
  registerWord("/showperms ");
  registerWord("/shutdownserver");
  registerWord("/silence ");
  registerWord("/unsilence ");
  registerWord("/superkill");
  registerWord("/time");
  registerWord("/unban ");
  registerWord("/unmute ");
  registerWord("/uptime");
  registerWord("/veto");
  registerWord("/viewreports");
  registerWord("/vote");
  registerWord("/loadplugin");
  registerWord("/listplugins");
  registerWord("/unloadplugin");
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
