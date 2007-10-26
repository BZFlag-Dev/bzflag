/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CRONJOB_H__
#define __CRONJOB_H__

#include <vector>
#include <string>

class CronJob
{
public:
  CronJob();
  CronJob(std::string job);
  ~CronJob();

  void setJob(std::string job);

  bool matches(int n, int h, int d, int m, int w) const;

  std::string getCommand() const {return command;};

  std::string displayJob() const {return inputJob;};

private:
  static bool isInVector(const std::vector<int> &iv, const int x);
  static std::vector<int> parseTimeList(const std::string times, const int min, const int max);

  std::vector<int> minutes;
  std::vector<int> hours;
  std::vector<int> days;
  std::vector<int> months;
  std::vector<int> weekdays;
  std::string command;

  std::string inputJob;
};

#endif //__CRONJOB_H__

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

