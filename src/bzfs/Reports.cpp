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

#include "Reports.h"
#include "bzfsAPI.h"
#include "WorldEventManager.h"
#include "bzfs.h"
#include "time.h"
#include "bzglob.h"

/** initialize the singleton */
template <>
Reports* Singleton<Reports>::_instance = (Reports*)0;

/**
* default constructor, protected because of singleton
*/
Reports::Reports() : Singleton<Reports>() {
}

Reports::~Reports() {

}

bool Reports::file(const std::string& user, const std::string message) {
  time_t now = time(NULL);

  Report report(ctime(&now), user, message);

  if (clOptions->reportFile.size()) {
    std::ofstream ofs(clOptions->reportFile.c_str(), std::ios::out | std::ios::app);
    ofs << report.fileLine() << std::endl;
  }

  if (clOptions->reportPipe.size() > 0) {
    FILE* pipeWrite = popen(clOptions->reportPipe.c_str(), "w");
    if (pipeWrite != NULL) {
      fprintf(pipeWrite, "%s\n\n", report.fileLine().c_str());
    }
    else {
      debugf(1, "Couldn't write report to the pipe\n");
    }
    pclose(pipeWrite);
  }

  std::string temp = std::string("**\"") + user + "\" reports: " + message;
  const std::vector<std::string> words = TextUtils::tokenize(temp, " \t");
  unsigned int cur = 0;
  const unsigned int wordsize = words.size();
  std::string temp2;

  while (cur != wordsize) {
    temp2.clear();
    while (cur != wordsize && (temp2.size() + words[cur].size() + 1) < (unsigned) MessageLen) {
      temp2 += words[cur] + " ";
      ++cur;
    }
    sendMessage(ServerPlayer, AdminPlayers, temp2.c_str());
  }

  debugf(1, "A report from %s has been filed(time: %s).\n", report.from.c_str(), report.time.c_str());

  // Notify plugins of the report filed
  bz_ReportFiledEventData_V1 reportData;
  reportData.from = user;
  reportData.message = message;
  worldEventManager.callEvents(bz_eReportFiledEvent, &reportData);

  return true;
}

size_t Reports::getLines(std::vector<std::string> &lines, const char* p) {
  lines.clear();
  if (clOptions->reportFile.size() > 0) {
    return 0;
  }

  std::ifstream ifs(clOptions->reportFile.c_str(), std::ios::in);
  if (ifs.fail()) {
    return 0;
  }

  std::string pattern = "*";
  if (p) {
    pattern = p;
  }

  // assumes that empty lines separate the reports

  bool done = false;
  while (!done) {
    std::string line;
    done = std::getline(ifs, line) == NULL;
    if (line.size()) {
      Report report(line);

      if (report.match(pattern)) {
        lines.push_back(report.fileLine());
      }
    }
  }
  return lines.size();
}

size_t Reports::count(void) {
  std::ifstream ifs(clOptions->reportFile.c_str(), std::ios::in);
  if (ifs.fail()) {
    return 0;
  }

  size_t s = 0;

  bool done = false;
  while (!done) {
    std::string line;
    done = std::getline(ifs, line) == NULL;
    if (line.size()) {
      s++;
    }
  }
  return s;
}

bool Reports::clear(void) {
  // just blast out the file with a single newline
  if (!clOptions->reportFile.size()) {
    std::ofstream ofs(clOptions->reportFile.c_str(), std::ios::out);
    ofs << std::endl;
    return true;
  }

  return false;
}

bool Reports::clear(size_t index) {
  if (!clOptions->reportFile.size()) {
    return false;
  }

  std::vector<Report> reports;

  std::ifstream ifs(clOptions->reportFile.c_str(), std::ios::in);
  if (ifs.fail()) {
    return false;
  }

  // read em all in
  bool done = false;
  while (!done) {
    std::string line;
    done = std::getline(ifs, line) == NULL;
    if (line.size()) {
      reports.push_back(Report(line));
    }
  }
  ifs.close();

  if (index >= reports.size()) {
    return false;
  }

  reports.erase(reports.begin() + index);

  std::ofstream ofs(clOptions->reportFile.c_str(), std::ios::out);

  std::vector<Report>::iterator itr = reports.begin();
  while (itr != reports.end())
    for (size_t r = 0; r < reports.size(); r++) {
      ofs << (itr++)->fileLine() << std::endl;
    }

  return true;
}

Reports::Report Reports::get(size_t index) {
  if (!clOptions->reportFile.size()) {
    return Report();
  }

  std::list<Report> reports;

  std::ifstream ifs(clOptions->reportFile.c_str(), std::ios::in);
  if (ifs.fail()) {
    return Report();
  }

  size_t s = 0;
  // read em all in
  bool done = false;
  while (!done) {
    std::string line;
    done = std::getline(ifs, line) == NULL;
    if (line.size()) {
      if (s == index) {
        return Report(line);
      }
      s++;
    }
  }
  ifs.close();

  return Report();
}

Reports::Report::Report(const char* t, const std::string& f, const std::string& m) {
  if (t) {
    time = t;
  }

  from = f;
  message = m;
}

bool Reports::Report::fill(const std::string& line) {
  std::vector<std::string> parts = TextUtils::tokenize(line, std::string(":"), 3, false);
  if (parts.size() >= 4) {
    time = parts[0];
    from = parts[2];
    message = parts[3];
  }
  else {
    from = time = message = "";
  }
  return time.size() > 0;
}

std::string Reports::Report::fileLine(void) {
  return time + ":Reported by :" + from + ":" + message;
}

bool Reports::Report::matchName(const std::string pattern) {
  return glob_match(pattern, TextUtils::toupper(from));
}

bool Reports::Report::matchMessage(const std::string pattern) {
  return glob_match(pattern, TextUtils::toupper(message));
}

bool Reports::Report::match(const std::string pattern) {
  return matchMessage(pattern) || matchName(pattern);
}





// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
