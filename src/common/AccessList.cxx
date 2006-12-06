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

#include "common.h"

// implementation header
#include "AccessList.h"

// system headers
#include <stdio.h>
#include <ctype.h>

// common headers
#include "bzfio.h"
#include "bzglob.h"
#include "TextUtils.h"
#include "FileManager.h"
#include "DirectoryNames.h"


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


AccessList::AccessList(const std::string& _filename, const char* content)
{
  filename = getConfigDirName();
  filename += _filename;

  alwaysAuth = false;

  bool fileExists = reload();

  if (!fileExists && (content != NULL)) {
    makeContent(content);
  }

  return;
}


AccessList::~AccessList()
{
  return;
}


const std::string& AccessList::getFileName() const
{
  return filename;
}


bool AccessList::reload()
{
  patterns.clear();
  alwaysAuth = false;

  FILE* file = fopen(filename.c_str(), "r");
  if (file == NULL) {
    return false;
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
      type = deny_regex;
      c = c + 10;
    }
    else if (strncasecmp(c, "deny", 4) == 0) {
      type = deny;
      c = c + 4;
    }
    else {
      logDebugMessage(1,"%s: malformed line (%s)\n", filename.c_str(), buf);
      continue; // ignore this line
    }

    c = eatWhite(c);

    if (*c == '\0') {
      logDebugMessage(1,"%s: missing pattern (%s)\n", filename.c_str(), buf);
      continue; // ignore this line
    }

    // terminate the pattern on the first non-white
    char* end = eatNonWhite(c);
    *end = '\0';

    AccessPattern pattern;
    pattern.type = type;
    pattern.pattern = c;
    patterns.push_back(pattern);

    logDebugMessage(4,"AccessList(%s):  added  (%i: %s)\n", filename.c_str(), type, c);
  }

  fclose(file);

  alwaysAuth = computeAlwaysAuth();

  return true;
}


bool AccessList::computeAlwaysAuth() const
{
  for (unsigned int i = 0; i < patterns.size(); i++) {
    const AccessPattern& p = patterns[i];
    if ((p.type == deny) || (p.type == deny_regex)) {
      return false;
    }
    if ((p.type == allow) && (p.pattern == "*")) {
      return true;
    }
    if ((p.type == allow_regex) && (p.pattern == ".*")) {
      return true;
    }
  }
  return true;
}


bool AccessList::authorized(const std::vector<std::string>& strings) const
{
  for (unsigned int i = 0; i < patterns.size(); i++) {
    const AccessPattern& p = patterns[i];
    if ((p.type == allow) || (p.type == deny)) {
      // simple globbing
      for (unsigned int s = 0; s < strings.size(); s++) {
        const std::string upperString = TextUtils::toupper(strings[s]);
        const std::string upperPattern = TextUtils::toupper(p.pattern);
        if (glob_match(upperPattern, upperString)) {
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
      regfree(&re);
    }
  }

  return true;
}


void AccessList::makeContent(const char* content) const
{
  FILE* file = fopen(filename.c_str(), "w");
  if (file == NULL) {
    return;
  }
  fputs(content, file);
  fclose(file);
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
