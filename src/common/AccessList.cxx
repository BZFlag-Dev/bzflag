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

#include "common.h"

// implementation header
#include "AccessList.h"

// system headers
#include <stdio.h>
#include <ctype.h>

// common headers
#include "bzfio.h"
#include "bzglob.h"
#include "regex.h"
#include "DirectoryNames.h"


AccessList ServerAccessList("server_list.txt");
AccessList DownloadAccessList("download_list.txt");


static inline char* eatWhite(char* c)
{
  while ((*c != '\0') && isspace(*c)) {
    c++;
  }
  return c;
}

static inline char* eatNonWhite(char* c)
{
  while ((*c != '\0') && !isspace(*c)) {
    c++;
  }
  return c;
}


AccessList::AccessList(const std::string& _filename)
{
  filename = getConfigDirName();
  filename += _filename;
  reload();
  return;
}


AccessList::~AccessList()
{
  return;
}


void AccessList::reload()
{
  patterns.clear();
  
  FILE* file = fopen(filename.c_str(), "r");
  if (file == NULL) {
    return;
  }
  
  char buf[256];
  while (fgets (buf, 256, file) != NULL) {

    char* c = eatWhite(buf);
    
    // clip any trailing any CR or NL
    char* tmp = c;
    while (*tmp != '\0') {
      if ((*tmp == '\r') || (*tmp == '\n')) {
        *tmp = '\0';
      }
      tmp++;
    }

    // skip comments and blank lines    
    if ((*c == '\0') || (*c == '#')) {
      continue;
    }

    // get the permission type
    AccessType type = invalid;
    if (strncasecmp(c, "allow_regex", 11) == 0) {
      type = allow_regex;
      c = c + 11;
    }
    else if (strncasecmp(c, "allow", 5) == 0) {
      type = allow;
      c = c + 5;
    }
    else if (strncasecmp(c, "deny_regex", 10) == 0) {
      type = deny;
      c = c + 10;
    }
    else if (strncasecmp(c, "deny", 4) == 0) {
      type = deny;
      c = c + 4;
    }
    else {
      DEBUG1("%s: malformed line (%s)\n", filename.c_str(), buf);
      continue; // ignore this line
    }

    c = eatWhite(c);
    
    if (*c == '\0') {
      DEBUG1("%s: missing pattern (%s)\n", filename.c_str(), buf);
      continue; // ignore this line
    }
    
    // terminate the pattern on the first non-white
    char* end = eatNonWhite(c);
    *end = '\0';

    AccessPattern pattern;
    pattern.type = type;
    pattern.pattern = c;
    patterns.push_back(pattern);
    
    DEBUG4("AccessList(%s):  added  (%i: %s)\n", filename.c_str(), type, c);
  }
  
  fclose(file);
  return;
}
    
    
bool AccessList::authorized(const std::vector<std::string>& strings) const
{
  for (unsigned int i = 0; i < patterns.size(); i++) {
    const AccessPattern& p = patterns[i];
    if ((p.type == allow) || (p.type == deny)) {
      // simple globbing
      for (unsigned int s = 0; s < strings.size(); s++) {
        if (glob_match(p.pattern, strings[s])) {
          if (p.type == allow) {
            return true;
          }
          else if (p.type == deny) {
            return false;
          }
        }
      }
    } else {
      // regular expression
      regex_t re;
      if (regcomp(&re, p.pattern.c_str(), REG_EXTENDED | REG_ICASE) != 0) {
        continue;
      }
      for (unsigned int s = 0; s < strings.size(); s++) {
        if (regexec(&re, strings[s].c_str(), 0, NULL, 0) == 0) {
          if (p.type == allow_regex) {
            return true;
          }
          else if (p.type == deny_regex) {
            return false;
          }
        }
      }
    }
  }
  
  return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
