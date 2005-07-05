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
#include <vector>

// revert _WINSOCK2API_ to its previous state
#ifndef _ALREADYHADWS2_
#undef _WINSOCK2API_
#endif

class cURLManager {
public:
  cURLManager();
  virtual ~cURLManager();

  enum timeCondition {
    None,
    ModifiedSince
  };

  void addHandle();
  void removeHandle();

  void setTimeout(long timeout);
  void setNoBody();
  void setGetMode();
  void setHTTPPostMode();
  void setPostMode(std::string postData);
  void setRequestFileTime(bool request);
  void setURL(const std::string url);
  void setProgressFunction(curl_progress_callback func, void* data);
  void setTimeCondition(timeCondition condition, time_t &t);
  void setInterface(const std::string interface);
  void setUserAgent(const std::string userAgent);

  void addFormData(const char *key, const char *value);

  bool getFileTime(time_t &t);

  virtual void collectData(char *ptr, int len);
  virtual void finalization(char *data, unsigned int length, bool good);

  static int fdset(fd_set &read, fd_set &write);
  static int perform();
  void       performWait();

protected:
  void         *theData;
  unsigned int  theLen;
private:

  void          infoComplete(CURLcode result);

  static bool   inited;
  CURL         *easyHandle;
  static CURLM *multiHandle;
  static char   errorBuffer[CURL_ERROR_SIZE];
  bool          added;
  std::string   usedUrl;
  std::string   interface;
  std::string   userAgent;
  std::string   postData;

  struct curl_httppost* formPost;
  struct curl_httppost* formLast;

  static void   setup();

  static size_t writeFunction(void *ptr, size_t size, size_t nmemb,
			      void *stream);

  static std::map<CURL*, cURLManager*> cURLMap;
};


typedef enum
{
	eImage,
	eSound,
	eFont,
	eFile,
	eUnknown
}teResourceType;

typedef struct 
{
	teResourceType	resType;
	std::string		URL;
	std::string		filePath;
	std::string		fileName;
}trResourceItem;

class resourceGeter :  cURLManager
{
public:
	resourceGeter();
	virtual ~resourceGeter();

	void addResource ( trResourceItem &item );
	void flush ( void );

	virtual void finalization(char *data, unsigned int length, bool good);

protected:
	bool itemExists ( trResourceItem &item );
	void getResource ( void );

	std::vector<trResourceItem>	resources;
	bool doingStuff;
};

#endif // CURL_MANAGER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
