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

// bzflag common header
#include "common.h"

// system headers
#include <curl/curl.h>
#include <string>
#include <map>

class cURLManager {
public:
  cURLManager();
  virtual ~cURLManager();

  void addHandle();
  void removeHandle();

  void setTimeout(long timeout);
  void setNoBody(bool nobody);
  void setURL(std::string url);
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
