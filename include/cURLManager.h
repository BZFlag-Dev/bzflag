/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef CURL_MANAGER_H
#define CURL_MANAGER_H

// bzflag common header
#include "common.h"

#include "network.h"

// system headers
#include <curl/curl.h>
#include <string>
#include <map>
#include <vector>


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
  void setInterface(const std::string interfaceIP);
  void setUserAgent(const std::string userAgent);
  void setDNSCachingTime(int time);

  void addFormData(const char *key, const char *value);

  bool getFileTime(time_t &t);
  bool getFileSize(double &size);

  virtual void collectData(char *ptr, int len);
  virtual void finalization(char *data, unsigned int length, bool good);

  static int	fdset(fd_set &read, fd_set &write);
  static bool	perform();
  void		performWait();

protected:
  void	       *theData;
  unsigned int  theLen;
private:

  void		infoComplete(CURLcode result);

  static bool   inited;
  static bool   justCalled;
  CURL	       *easyHandle;
  static CURLM *multiHandle;
  static char   errorBuffer[CURL_ERROR_SIZE];
  bool		added;
  std::string   usedUrl;
  std::string   interfaceIP;
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

class ResourceGetter :  cURLManager
{
public:
	ResourceGetter();
	virtual ~ResourceGetter();

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
