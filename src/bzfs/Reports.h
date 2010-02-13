/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _REPORTS_H
#define _REPORTS_H

#include "common.h"
#include <string>
#include <vector>

#include "Singleton.h"


#define REPORTS Reports::instance()


/**
 * Singleton report generator.  used for logging matches to a file.
 */
class Reports : public Singleton<Reports>
{
public:

  bool file(const std::string &user, const std::string message);
  size_t getLines(std::vector<std::string> &lines, const char* pattern = NULL
);

  size_t count(void);
  bool clear(void);
  bool clear(size_t index);

  class Report
  {
  public:
    Report() {};
    Report(const std::string &line) {
      fill(line);
    }
    Report(const char* t, const std::string &f, const std::string & m);

    std::string from;
    std::string time;
    std::string message;

    bool matchName(const std::string pattern);
    bool matchMessage(const std::string pattern);
    bool match(const std::string pattern);

    bool fill(const std::string &line);

    std::string fileLine(void);
 };

  Report get(size_t index);

protected:
  friend class Singleton<Reports>;

private:
  Reports();
  ~Reports();
};


#endif /* _REPORTS_H */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
