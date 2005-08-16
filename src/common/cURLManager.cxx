/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// class interface header

#include "cURLManager.h"

// common implementation headers
#include "bzfio.h"

bool                    cURLManager::inited      = false;
bool                    cURLManager::justCalled;
CURLM                  *cURLManager::multiHandle = NULL;
std::map<CURL*,
	 cURLManager*>  cURLManager::cURLMap;
char                    cURLManager::errorBuffer[CURL_ERROR_SIZE];

cURLManager::cURLManager()
{
  CURLcode result;

  theData   = NULL;
  theLen    = 0;
  added     = false;

  formPost  = NULL;
  formLast  = NULL;

  if (!inited)
    setup();

  easyHandle = curl_easy_init();
  if (!easyHandle) {
    DEBUG1("Something wrong with CURL\n");
    return;
  }

  if (debugLevel >= 4) {
    result = curl_easy_setopt(easyHandle, CURLOPT_VERBOSE, (long)1);
    if (result != CURLE_OK) {
      DEBUG1("CURLOPT_VERBOSE error: %d\n", result);
    }
  }

  result = curl_easy_setopt(easyHandle, CURLOPT_ERRORBUFFER, errorBuffer);
  if (result != CURLE_OK) {
    DEBUG1("CURLOPT_ERRORBUFFER error: %d\n", result);
  }

  result = curl_easy_setopt(easyHandle, CURLOPT_NOSIGNAL, true);
  if (result != CURLE_OK) {
    DEBUG1("CURLOPT_NOSIGNAL error %d : %s\n", result, errorBuffer);
  }

  result = curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION,
			    cURLManager::writeFunction);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_WRITEFUNCTION error %d : %s\n", result, errorBuffer);

  result = curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, this);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_WRITEDATA error %d : %s\n", result, errorBuffer);

  cURLMap[easyHandle] = this;
}

cURLManager::~cURLManager()
{
  if (added)
    removeHandle();
  cURLMap.erase(easyHandle);
  curl_easy_cleanup(easyHandle);
  free(theData);
}

void cURLManager::setup()
{
  CURLcode result;

  DEBUG1("LIBCURL: %s\n", curl_version());
  if ((result = curl_global_init(CURL_GLOBAL_NOTHING)))
    DEBUG1("cURL Global init Error: %d\n", result);
  multiHandle = curl_multi_init();
  if (!multiHandle)
    DEBUG1("Unexpected error creating multi handle from libcurl \n");
  inited = true;
}

size_t cURLManager::writeFunction(void *ptr, size_t size, size_t nmemb,
				  void *stream)
{
  int len = size * nmemb;
  ((cURLManager*)stream)->collectData((char*)ptr, len);
  justCalled = true;
  return len;
}

void cURLManager::setTimeout(long timeout)
{
  CURLcode result;

  result = curl_easy_setopt(easyHandle, CURLOPT_TIMEOUT, timeout);
  if (result != CURLE_OK) {
    DEBUG1("CURLOPT_TIMEOUT error %d : %s\n", result, errorBuffer);
  }
}

void cURLManager::setNoBody()
{
  CURLcode result;
  long     nobody = 1;

  result = curl_easy_setopt(easyHandle, CURLOPT_NOBODY, nobody);
  if (result != CURLE_OK) {
    DEBUG1("CURLOPT_NOBODY error %d : %s\n", result, errorBuffer);
  }
}

void cURLManager::setGetMode()
{
  CURLcode result;
  long     get = 1;

  result = curl_easy_setopt(easyHandle, CURLOPT_HTTPGET, get);
  if (result != CURLE_OK) {
    DEBUG1("CURLOPT_GET error %d : %s\n", result, errorBuffer);
  }
}

void cURLManager::setPostMode(std::string _postData)
{
  CURLcode result;

  postData = _postData;

  result = curl_easy_setopt(easyHandle, CURLOPT_POSTFIELDS, postData.c_str());
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_POSTFIELD error %d : %s\n", result, errorBuffer);
  result = curl_easy_setopt(easyHandle, CURLOPT_POSTFIELDSIZE, postData.size());
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_POSTFIELD error %d : %s\n", result, errorBuffer);
  result = curl_easy_setopt(easyHandle, CURLOPT_POST, 1);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_POST error %d : %s\n", result, errorBuffer);
}

void cURLManager::setHTTPPostMode()
{
  CURLcode result;

  result = curl_easy_setopt(easyHandle, CURLOPT_HTTPPOST, formPost);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_HTTPPOST error %d : %s\n", result, errorBuffer);
}

void cURLManager::setURL(const std::string url)
{
  CURLcode result;

  DEBUG4("Tried to fetch URL: %s\n", url.c_str());

  if (url == "") {
    result = curl_easy_setopt(easyHandle, CURLOPT_URL, NULL);
  } else {
    usedUrl = url;
    result = curl_easy_setopt(easyHandle, CURLOPT_URL, usedUrl.c_str());
    DEBUG2("CURLOPT_URL is : %s\n", usedUrl.c_str());
  }
  if (result != CURLE_OK) {
    DEBUG1("CURLOPT_URL error %d : %s\n", result, errorBuffer);
  }
}

void cURLManager::setProgressFunction(curl_progress_callback func, void* data)
{
  CURLcode result;
  if (func != NULL) {
    result = curl_easy_setopt(easyHandle, CURLOPT_PROGRESSFUNCTION, func);
    if (result == CURLE_OK)
      result = curl_easy_setopt(easyHandle, CURLOPT_PROGRESSDATA, data);
    if (result == CURLE_OK)
      result = curl_easy_setopt(easyHandle, CURLOPT_NOPROGRESS, 0);
  } else {
   result = curl_easy_setopt(easyHandle, CURLOPT_NOPROGRESS, 1);
  }
  if (result != CURLE_OK) {
    DEBUG1("CURLOPT_SET_PROGRESS error %d : %s\n", result, errorBuffer);
  }
}

void cURLManager::setRequestFileTime(bool request)
{
  CURLcode result;
  long     requestFileTime = request ? 1 : 0;
  result = curl_easy_setopt(easyHandle, CURLOPT_FILETIME, requestFileTime);
  if (result != CURLE_OK) {
    DEBUG1("CURLOPT_FILETIME error %d : %s\n", result, errorBuffer);
  }
}

void cURLManager::addHandle()
{
  CURLMcode result = curl_multi_add_handle(multiHandle, easyHandle);
  if (result > CURLM_OK)
    DEBUG1("Error while adding easy handle from libcurl %d : %s\n",
	   result, errorBuffer);
  added = true;
}

void cURLManager::removeHandle()
{
  if (!added)
    return;
  CURLMcode result = curl_multi_remove_handle(multiHandle, easyHandle);
  if (result != CURLM_OK)
    DEBUG1("Error while removing easy handle from libcurl %d : %s\n",
	   result, errorBuffer);
  added = false;
}

void cURLManager::finalization(char *, unsigned int, bool)
{
}

void cURLManager::collectData(char* ptr, int len)
{
  unsigned char *newData = (unsigned char *)realloc(theData, theLen + len);
  if (!newData) {
    DEBUG1("memory exhausted\n");
  } else {
    memcpy(newData + theLen, ptr, len);
    theLen += len;
    theData = newData;
  }
}

void cURLManager::performWait()
{
  CURLcode result;
  result = curl_easy_perform(easyHandle);
  infoComplete(result);
}

int cURLManager::fdset(fd_set &read, fd_set &write)
{
  fd_set    exc;
  int       max_fd = -1;
  CURLMcode result;

  FD_ZERO(&exc);

  result = curl_multi_fdset(multiHandle, &read, &write, &exc, &max_fd);
  if (result != CURLM_OK)
    DEBUG1("Error while doing multi_fdset from libcurl %d : %s\n",
	   result, errorBuffer);
  return max_fd;
}

bool cURLManager::perform()
{
  if (!inited)
    setup();

  int activeTransfers = 0;
  CURLMcode result;
    justCalled = false;
    while (true) {
      result = curl_multi_perform(multiHandle, &activeTransfers);
      if (result != CURLM_CALL_MULTI_PERFORM)
	break;
    }
    if (result != CURLM_OK)
      DEBUG1("Error while doing multi_perform from libcurl %d : %s\n",
	     result, errorBuffer);

    int      msgs_in_queue;
    CURLMsg *pendingMsg;
    CURL    *easy;

    while (true) {
      pendingMsg = curl_multi_info_read(multiHandle, &msgs_in_queue);
      if (!pendingMsg)
	break;

      easy        = pendingMsg->easy_handle;

      if (cURLMap.count(easy))
	cURLMap[easy]->infoComplete(pendingMsg->data.result);

      if (msgs_in_queue <= 0)
	break;
    }

  return justCalled;
}

void cURLManager::infoComplete(CURLcode result)
{
  if (result != CURLE_OK)
    DEBUG1("File transfer terminated with error from libcurl %d : %s\n",
	   result, errorBuffer);
  if (formPost) {
    curl_formfree(formPost);
    formPost = NULL;
    formLast = NULL;
  }
  removeHandle();
  finalization((char *)theData, theLen, result == CURLE_OK);
  justCalled = true;
  free(theData);
  theData = NULL;
  theLen  = 0;
}

bool cURLManager::getFileTime(time_t &t)
{
  long filetime;
  CURLcode result;
  result = curl_easy_getinfo(easyHandle, CURLINFO_FILETIME, &filetime);
  if (result) {
    DEBUG1("CURLINFO_FILETIME error %d : %s\n", result, errorBuffer);
    return false;
  }
  t = (time_t)filetime;
  return true;
}

void cURLManager::setTimeCondition(timeCondition condition, time_t &t)
{
  CURLcode result;

  switch (condition) {
  case None:
    result = curl_easy_setopt(easyHandle,
			      CURLOPT_TIMECONDITION,
			      CURL_TIMECOND_NONE);
    if (result != CURLE_OK) {
      DEBUG1("CURLOPT_TIMECONDITION error %d : %s\n", result, errorBuffer);
    }
    break;
  case ModifiedSince:
    result = curl_easy_setopt(easyHandle,
			      CURLOPT_TIMECONDITION,
			      CURL_TIMECOND_IFMODSINCE);
    if (result != CURLE_OK) {
      DEBUG1("CURLOPT_TIMECONDITION error %d : %s\n", result, errorBuffer);
    }
    result = curl_easy_setopt(easyHandle,
			      CURLOPT_TIMEVALUE,
			      (long)t);
    if (result != CURLE_OK) {
      DEBUG1("CURLOPT_TIMEVALUE error %d : %s\n", result, errorBuffer);
    }
    break;
  default:
    break;
  }
}

void cURLManager::setInterface(const std::string _interfaceIP)
{
  interfaceIP = _interfaceIP;

  CURLcode result;

  result = curl_easy_setopt(easyHandle,
			    CURLOPT_INTERFACE,
			    interfaceIP.c_str());
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_SET_INTERFACE error %d : %s\n", result, errorBuffer);
}

void cURLManager::setUserAgent(const std::string _userAgent)
{
  userAgent = _userAgent;

  CURLcode result;

  result = curl_easy_setopt(easyHandle,
			    CURLOPT_USERAGENT,
			    userAgent.c_str());
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_SET_USERAGENT error %d : %s\n", result, errorBuffer);
}

void cURLManager::addFormData(const char *key, const char *value)
{
  CURLFORMcode result;
  result = curl_formadd(&formPost, &formLast, 
			CURLFORM_COPYNAME, key,
			CURLFORM_COPYCONTENTS, value,
			CURLFORM_CONTENTSLENGTH, strlen(value),
			CURLFORM_END); 
  if (result != CURL_FORMADD_OK)
    DEBUG1("addFormData error %d : %s\n", result, errorBuffer);
}

void cURLManager::setDNSCachingTime(int time)
{
  CURLcode result;

  result = curl_easy_setopt(easyHandle,
			    CURLOPT_DNS_CACHE_TIMEOUT,
			    (long)time);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_SET_DNS_CACHE_TIMEOUT error %d : %s\n",
	   result, errorBuffer);
}

//**************************ResourceGetter*************************

ResourceGetter::ResourceGetter() : cURLManager()
{
	doingStuff = false;
}

ResourceGetter::~ResourceGetter()
{

}

void ResourceGetter::addResource ( trResourceItem &item )
{
	resources.push_back(item);

	if (!doingStuff)
		getResource();
}

void ResourceGetter::flush ( void )
{
	resources.clear();
	doingStuff = false;
}

void ResourceGetter::finalization(char *data, unsigned int length, bool good)
{
	if (!resources.size() || !doingStuff)
		return;	// we are suposed to be done

	// this is who we are suposed to be geting
	trResourceItem item = resources[0]; 
	resources.erase(resources.begin());
	if (good)
	{
		// save the thing
		FILE *fp = fopen(item.filePath.c_str(),"wb");
		if (fp)
		{
			fwrite(data,length,1,fp);
			fclose(fp);
		}

		// maybe send a message here saying we did it?
	}
	
	// do the next one if we must
	getResource();
}

bool ResourceGetter::itemExists ( trResourceItem &item )
{
	// save the thing
	FILE *fp = fopen(item.filePath.c_str(),"rb");
	if (fp)
	{
		fclose(fp);
		return true;
	}
	return false;
}

void ResourceGetter::getResource ( void )
{
	while ( resources.size() || itemExists(resources[0]) )
		resources.erase(resources.begin());

	if ( !resources.size() )
		doingStuff = false;
	else
	{
		trResourceItem item = resources[0]; 

		doingStuff = true;
		setURL(item.URL);
		addHandle();
	}
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
