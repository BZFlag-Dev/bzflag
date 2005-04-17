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
CURLM                  *cURLManager::multiHandle = NULL;
std::list<cURLManager*> cURLManager::cURLList;

cURLManager::cURLManager()
{
  CURLcode result;

  theData   = NULL;
  theLen    = 0;
  errorCode = CURLE_OK;
  added     = false;

  if (!inited)
    setup();

  easyHandle = curl_easy_init();
  if (!easyHandle) {
    DEBUG1("Something wrong with CURL\n");
    return;
  }
#if LIBCURL_VERSION_NUM >= 0x070a00
  result = curl_easy_setopt(easyHandle, CURLOPT_NOSIGNAL, true);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_NOSIGNAL error: %d\n", result);
  }
#endif

  result = curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION,
			    cURLManager::writeFunction);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_WRITEFUNCTION error: %d\n", result);

  result = curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, this);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_FILE error: %d\n", result);

}

cURLManager::~cURLManager()
{
  if (added)
    removeHandle();
  curl_easy_cleanup(easyHandle);
  free(theData);
}

void cURLManager::setup()
{
  CURLcode result;

  DEBUG1("LIBCURL: %s\n", curl_version());
#if LIBCURL_VERSION_NUM >= 0x070a00
  if ((result = curl_global_init(CURL_GLOBAL_NOTHING)))
    DEBUG1("Unexpected error from libcurl; Error: %d\n", result);
#endif
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
  return len;
}

void cURLManager::setTimeout(long timeout)
{
  CURLcode result;

  result = curl_easy_setopt(easyHandle, CURLOPT_TIMEOUT, timeout);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_TIMEOUT error: %d\n", result);
  }
}

void cURLManager::setNoBody(bool nobody)
{
  CURLcode result;

  result = curl_easy_setopt(easyHandle, CURLOPT_NOBODY, nobody);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_NOBODY error: %d\n", result);
  }
}

void cURLManager::setURL(std::string url)
{
  CURLcode result;

  if (url == "")
    result = curl_easy_setopt(easyHandle, CURLOPT_URL, NULL);
  else
    result = curl_easy_setopt(easyHandle, CURLOPT_URL, url.c_str());
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_URL error: %d\n", result);
  }
}

void cURLManager::addHandle()
{
  CURLMcode result = curl_multi_add_handle(multiHandle, easyHandle);
  if (result != CURLM_OK)
    DEBUG1("Unexpected error from libcurl; Error: %d\n", result);
  cURLList.push_back(this);
  added = true;
}

void cURLManager::removeHandle()
{
  if (!added)
    return;
  for (std::list<cURLManager*>::iterator i = cURLList.begin();
       i != cURLList.end(); i++)
    if (*i == this) {
      cURLList.erase(i);
      break;
    }
  CURLMcode result = curl_multi_remove_handle(multiHandle, easyHandle);
  if (result != CURLM_OK)
    DEBUG1("Unexpected error from libcurl; Error: %d\n", result);
  added = false;
}

void cURLManager::collectData(char* ptr, int &len)
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

int cURLManager::perform()
{
  if (!inited)
    setup();

  int activeTransfers = 0;
  CURLMcode result;
  while (true) {
    result = curl_multi_perform(multiHandle, &activeTransfers);
    if (result != CURLM_CALL_MULTI_PERFORM)
      break;
  }
  if (result != CURLM_OK)
    DEBUG1("Unexpected error from libcurl; Error: %d\n", result);

  int      msgs_in_queue;
  CURLMsg *pendingMsg;
  CURL    *easy;


  while (true) {
    pendingMsg = curl_multi_info_read(multiHandle, &msgs_in_queue);
    if (!pendingMsg)
      break;

    easy        = pendingMsg->easy_handle;

    for (std::list<cURLManager*>::iterator i = cURLList.begin();
	 i != cURLList.end(); i++)
      if ((*i)->easyHandle == easy) {
	(*i)->infoComplete(pendingMsg->data.result);
	break;
      }

    if (msgs_in_queue <= 0)
      break;
  }

  return activeTransfers;
}

void cURLManager::infoComplete(CURLcode result)
{
  if (result != CURLE_OK)
    DEBUG1("Unexpected error from libcurl; Error: %d\n", result);
  finalization((char *)theData, theLen, result == CURLE_OK);
  free(theData);
  removeHandle();
}

bool cURLManager::getFileTime(time_t &t)
{
  long filetime;
  CURLcode result;
  result = curl_easy_getinfo(easyHandle, CURLINFO_FILETIME, &filetime);
  if (result) {
    DEBUG1("CURLINFO_FILETIME error: %d\n", result);
    return false;
  }
  t = (time_t)filetime;
  return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
