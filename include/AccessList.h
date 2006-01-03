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

#ifndef ACCESS_LIST_H
#define ACCESS_LIST_H

#include <stdio.h>
#include <string>
#include <vector>

class AccessList {
  public:
    AccessList(const std::string& filename, const char* content);
    ~AccessList();

    bool reload();

    bool alwaysAuthorized() const;
    bool authorized(const std::vector<std::string>& strings) const;

    const std::string& getFileName() const;

  private:
    bool computeAlwaysAuth() const;
    void makeContent(const char* content) const;

  private:
    std::string filename;
    bool alwaysAuth;

    enum AccessType {
      invalid,
      allow,		// simple globbing
      deny,
      allow_regex,	// regular expressions
      deny_regex
    };
    typedef struct {
      AccessType type;
      std::string pattern;
    } AccessPattern;

    std::vector<AccessPattern> patterns;
};

inline bool AccessList::alwaysAuthorized() const
{
  return alwaysAuth;
}


#endif

/*
 * Local Variables: ***
 * mode:C ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
