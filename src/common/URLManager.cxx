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

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#define _WINSOCK2API_
#endif

// class interface header
#include "URLManager.h"

// system headers
#ifdef _WIN32
#include <winsock.h>
#endif
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#include <iostream>

// common implementation headers
#include "bzfio.h"
#include "StateDatabase.h"


template <>
URLManager* Singleton<URLManager>::_instance = (URLManager*)0;

#ifdef HAVE_CURL
static size_t writeFunction(void *ptr, size_t size, size_t nmemb, void *stream);
#endif // HAVE_CURL

bool URLManager::getURL(const std::string URL, std::string &data)
{
  clearInternal();

  if (!beginGet(URL))
    return false;

  char* newData = (char*)malloc(theLen + 1);
  memcpy(newData, theData, theLen);
  newData[theLen] = 0;

  data = newData;
  free(newData);

  return true;
}

bool URLManager::getURL(const std::string URL, void **data, unsigned int& size)
{
  clearInternal();

  if (!beginGet(URL))
    return false;

  *data = malloc(theLen);
  memcpy(*data, theData, theLen);
  size = theLen;
  return true;
}

void URLManager::freeURLData(void *data)
{
  free(data);
}

URLManager::URLManager()
{
  easyHandle = NULL;
  theData = NULL;
  theLen = 0;

#ifdef HAVE_CURL

#if LIBCURL_VERSION_NUM >= 0x070a00
  CURLcode curlResult;
  if ((curlResult = curl_global_init(CURL_GLOBAL_NOTHING)))
    DEBUG1("Unexpected error from libcurl; Error: %d\n", curlResult);
#endif

  easyHandle = curl_easy_init();
  if (!easyHandle) {
    DEBUG1("Something wrong with CURL\n");
    return;
  }

  CURLcode result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_WRITEFUNCTION, writeFunction);
  if (result)
    DEBUG1("Something wrong with CURL; Error: %d\n", result);

  result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_FILE, this);
  if (result)
    DEBUG1("Something wrong with CURL; Error: %d\n", result);
#endif
}

URLManager::~URLManager()
{
  clearInternal();

#ifdef HAVE_CURL
  if (easyHandle)
    curl_easy_cleanup((CURL*)easyHandle);

#if LIBCURL_VERSION_NUM >= 0x070a00
  curl_global_cleanup();
#endif

#endif
}

void URLManager::collectData(char* ptr, int len)
{
  unsigned char	*newData = (unsigned char*)malloc(theLen + len);
  if (theData)
    memcpy(newData, theData, theLen);

  memcpy(&(newData[theLen]), ptr, len);
  theLen += len;

  free(theData);
  theData = newData;
}

void URLManager::clearInternal()
{
  if (theData)
    free (theData);

  theData = NULL;
  theLen = 0;
}

#ifdef HAVE_CURL
bool URLManager::beginGet(const std::string URL)
#else
bool URLManager::beginGet(const std::string)
#endif
{
#ifdef HAVE_CURL
  CURLcode result = CURLE_OK;
  if (!easyHandle) {
    return false;
  }

  float timeout = 15;
  if (BZDB.isSet("httpTimeout"))
    timeout = BZDB.eval("httpTimeout");

#if LIBCURL_VERSION_NUM >= 0x070a00
  result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_NOSIGNAL, true);
  if (result) {
    DEBUG1("Something wrong with CURL; Error: %d\n", result);
  }
#endif

  result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_TIMEOUT, timeout);
  if (result) {
    DEBUG1("Something wrong with CURL; Error: %d\n", result);
  }

  result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_URL, URL.c_str());
  if (result) {
    DEBUG1("Something wrong with CURL; Error: %d\n", result);
  }

  // FIXME: This could block for a _long_ time.
  result = curl_easy_perform((CURL*)easyHandle);
  if (result == (CURLcode)CURLOPT_ERRORBUFFER) {
    DEBUG1("Error: server reported: %d\n", result);
    return false;
  } else if (result) {
    DEBUG1("Something wrong with CURL; Error: %d\n", result);
    return false;
  }

  result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_URL, NULL);
  if (result) {
    DEBUG1("Something wrong with CURL; Error: %d\n", result);
    return false;
  }

  if (!theData)
    return false;

  return true;
#else
  return false;
#endif // HAVE_CURL
}

#ifdef HAVE_CURL
static size_t writeFunction(void *ptr, size_t size, size_t nmemb, void *stream)
{
  int len = size * nmemb;
  ((URLManager*)stream)->collectData((char*)ptr, len);
  return len;
}
#endif // HAVE_CURL

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
