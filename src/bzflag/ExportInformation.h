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

#ifndef  _EXPORTINFORMATION_H_
#define  _EXPORTINFORMATION_H_

// bzflag global common header
#include "common.h"

// system headers
#include <string>
#include <map>

// bzflag common headers
#include "Singleton.h"

/* ExportInformation: encapsulates information made available for export to
 * other programs.
 * Each program to which we export information should have an appropriately
 * compile-time protected member method implementing the data serialization
 * and transmission protocols for said program.  Initialization may be done
 * in this class' c'tor, and cleanup in its d'tor. */

class ExportInformation : public Singleton<ExportInformation> {
public:
  ExportInformation();
  ~ExportInformation();

  typedef enum {
    eitServerInfo,	  // info about the server
    eitPlayerInfo,	  // info about the player (callsign etc)
    eitPlayerStatistics,  // stats (score etc)
    eitCount
  } eiType;
  typedef enum {
    eipStandard, // ordinary information - share with anybody (ex: server, map?)
    eipPrivate,  // private information - share only with friends and only if allowed (ex: callsign, scores, client version?)
    eipSecret,   // secret information - not to be shared with untrusted sources (ex: ip, userid?)
    eipCount
  } eiPrivacy;

  void setInformation(const std::string key, const std::string value, eiType type, eiPrivacy privacy);

  void sendPulse();

protected:
  friend class Singleton<ExportInformation>;

private:
  struct eiData {
    eiType type;	// what sort of information is it
    eiPrivacy privacy;	// how private is it
    std::string value;  // what is its value
  };

  std::map<std::string, eiData> dataMap;

#ifdef USE_XFIRE
  void sendXfirePulse();
#endif

  void sendTextOutputPulse();
};

#endif // _EXPORTINFORMATION_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
