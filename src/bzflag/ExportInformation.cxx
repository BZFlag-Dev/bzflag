/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag global common header
#include "common.h"

// system headers
#include <string>
#include <map>
#include <fstream>
#include <time.h>

// external headers for specific export clients
#ifdef USE_XFIRE
#include "XfireGameClient.h"
#endif

// bzflag common headers
#include "TimeKeeper.h"
#include "StateDatabase.h"

// local headers
#include "ExportInformation.h"

// initialize the singleton
template <>
ExportInformation* Singleton<ExportInformation>::_instance = (ExportInformation*)0;


// Initialization for export recipients may be done here
ExportInformation::ExportInformation()
{
  // Nothing needed here yet
}

// Cleanup for export recipients may be done here
ExportInformation::~ExportInformation()
{
  // Nothing needed here yet
}

// BZFlag will call this function when necessary to keep the
// information contained in our private map up to date
void ExportInformation::setInformation(const std::string key, const std::string value, const eiType type, const eiPrivacy privacy)
{
  if (value == "")
    dataMap.erase(key);
  else
  {
    eiData newData;
    newData.privacy = privacy;
    newData.type = type;
    newData.value = value;
    dataMap[key] = newData;
  }
}

/* Each program to which we export should have a member function here.
 * The member function will be called every frame.
 * It should keep a static TimeKeeper around and export no more frequently
 * than is necessary, especially if exporting is expensive.
 * It should serialize and transmit whatever data the author feels is necessary.
 * Please be considerate of the privacy values. */

// This function will be called by BZFlag every frame.  It should simply call
// whatever additional member functions for export exist.
void ExportInformation::sendPulse()
{
#ifdef USE_XFIRE
  sendXfirePulse();
#endif
  sendTextOutputPulse();
}

// Xfire output function
#ifdef USE_XFIRE
void ExportInformation::sendXfirePulse()
{
  // maximum of once per minute
  static TimeKeeper xftk = TimeKeeper::getCurrent();
  if ((TimeKeeper::getCurrent() - xftk) < 60)
    return;
  else
    xftk = TimeKeeper::getCurrent();

  // make sure we're enabled
  if ((BZDB.evalInt("xfireCommunicationLevel") <= 0)
      || !XfireIsLoaded())
    return;

  const char* keys[100] = { NULL };
  const char* values[100] = { NULL };

  // privacy
  eiPrivacy reqPrivacy;
  if (BZDB.evalInt("xfireCommunicationLevel") >= 2)
    reqPrivacy = eipPrivate;
  else
    reqPrivacy = eipStandard;

  // load the key/value arrays for xfire
  std::map<std::string, eiData>::const_iterator itr;
  int i = 0;
  for (itr = dataMap.begin(); (i < 100) && (itr != dataMap.end()); ++itr, ++i) {
    if (itr->second.privacy <= reqPrivacy) {
      keys[i] = itr->first.c_str();
      values[i] = itr->second.value.c_str();
    }
  }

  XfireSetCustomGameData(100, keys, values);
}
#endif

void ExportInformation::sendTextOutputPulse()
{
  int pulseTime = BZDB.evalInt("statsOutputFrequency");
  if (pulseTime < 1) return;

  // no more frequently than specified
  static TimeKeeper xftk = TimeKeeper::getCurrent();
  if ((TimeKeeper::getCurrent() - xftk) < pulseTime)
    return;
  else
    xftk = TimeKeeper::getCurrent();

  // must be connected
  if (dataMap.find("Server") == dataMap.end())
    return;

  // dump output
  std::map<std::string, eiData>::const_iterator itr;
  std::ofstream out(BZDB.get("statsOutputFilename").c_str(), std::ios::app | std::ios::out);
  if (!out.good()) return;
  out << "*****" << std::endl;
  time_t now = time(NULL);
  char* timeStr = ctime(&now);
  timeStr[24] = '\0';
  out << "Time: " << timeStr << std::endl;
  out << "Server: " << dataMap["Server"].value << std::endl;
  for (itr = dataMap.begin(); (itr != dataMap.end()); ++itr) {
    if (itr->second.type == eitPlayerStatistics)
      out << itr->first << ": " << itr->second.value << std::endl;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
