/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef CURL_MANAGER_H
#define CURL_MANAGER_H

// even if we don't have winsock2 already included, don't include it.
// otherwise some systems have a fit.
#ifndef _WINSOCK2API_
#define _WINSOCK2API_
#else
#define _ALREADYHADWS2_
#endif

// bzflag common header
#include "common.h"

// system headers
#include <curl/curl.h>
#include <string>
#include <map>

// revert _WINSOCK2API_ to its previous state
#ifndef _ALREADYHADWS2_
#undef _WINSOCK2API_
#endif

class cURLManager {
public:
  cURLManager();
  virtual ~cURLManager();

  void addHandle();
  void removeHandle();

  void setTimeout(long timeout);
  void setNoBody();
  void setGetMode();
  void setRequestFileTime(bool request);
  void setURL(const std::string url);
  void setProgressFunction(curl_progress_callback func, void* data);

  bool getFileTime(time_t &t);

  virtual void collectData(char *ptr, int len);
  virtual void finalization(char *data, unsigned int length, bool good);

  static int perform();

protected:
  void         *theData;
  unsigned int  theLen;
private:

  void          infoComplete(CURLcode result);

  static bool   inited;
  int           errorCode;
  CURL         *easyHandle;
  static CURLM *multiHandle;
  bool          added;
  std::string   usedUrl;

  static void   setup();

  static size_t writeFunction(void *ptr, size_t size, size_t nmemb,
			      void *stream);

  static std::map<CURL*, cURLManager*> cURLMap;
};

#endif // CURL_MANAGER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
