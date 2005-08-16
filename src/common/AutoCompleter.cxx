/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

#include "common.h"

#include <algorithm>

#include "AutoCompleter.h"


void AutoCompleter::registerWord(const std::string& str) {
  words.insert(std::lower_bound(words.begin(), words.end(), str), str);
}


void AutoCompleter::unregisterWord(const std::string& str) {
  std::vector<std::string>::iterator iter = std::lower_bound(words.begin(), words.end(), str);
  if (iter != words.end() && *iter == str)
    words.erase(iter);
}


std::string AutoCompleter::complete(const std::string& str) {

  if (str.size() == 0)
    return str;

  // find the first and last word with the prefix str
  std::vector<std::string>::iterator first, last;
  first = std::lower_bound(words.begin(), words.end(), str);
  if (first == words.end() || first->substr(0, str.size()) != str)
    return str;
  std::string tmp = str;
  tmp[tmp.size() - 1]++;
  last = std::lower_bound(first, words.end(), tmp) - 1;

  // return the largest common prefix
  unsigned int i;
  for (i = 0; i < first->size() && i < last->size(); ++i)
    if ((*first)[i] != (*last)[i])
      break;
  return first->substr(0, i);
}




DefaultCompleter::DefaultCompleter() {
  setDefaults();
}

void DefaultCompleter::setDefaults() {
  words.clear();
  registerWord("/ban ");
  registerWord("/banlist");
  registerWord("/countdown");
  registerWord("/clientquery");
  registerWord("/date");
  registerWord("/deregister");
  registerWord("/dumpvars");
  registerWord("/flag ");
  registerWord("reset");
  registerWord("up");
  registerWord("show");
  registerWord("/flaghistory");
  registerWord("/gameover");
  registerWord("/ghost ");
  registerWord("/groupperms");
  registerWord("/help");
  registerWord("/highlight ");
  registerWord("/identify ");
  registerWord("/idlestats");
  registerWord("/kick ");
  registerWord("/kill ");
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
  registerWord("/register ");
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
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
