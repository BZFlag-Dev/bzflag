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


/******************************************************************************/
#ifdef HAVE_CURL
/******************************************************************************/


static size_t writeFunction(void *ptr, size_t size, size_t nmemb, void *stream);


bool URLManager::getURL(const std::string& URL, std::string &data)
{
  clearInternal();

  if (!beginGet(URL)) {
    return false;
  }

  char* newData = (char*)malloc(theLen + 1);
  memcpy(newData, theData, theLen);
  newData[theLen] = 0;

  data = newData;
  free(newData);

  return true;
}


bool URLManager::getURL(const std::string& URL, void **data, unsigned int& size)
{
  clearInternal();

  if (!beginGet(URL)) {
    return false;
  }

  *data = malloc(theLen);
  memcpy(*data, theData, theLen);
  size = theLen;
  return true;
}


bool URLManager::getFileTime(time_t& t)
{
  t = 0;
  if (errorCode != CURLE_OK) {
    return false;
  }

  long filetime;
  CURLcode result;
  result = curl_easy_getinfo((CURL*)easyHandle, CURLINFO_FILETIME, &filetime);
  if (result) {
    DEBUG1("CURLINFO_FILETIME error: %d\n", result);
    return false;
  }
  t = (time_t)filetime;

  return true;
}


bool URLManager::getURLHeader(const std::string& URL)
{
  CURL* curl = (CURL*)easyHandle;
  if (curl == NULL) {
    return false;
  }

  CURLcode result;  
  errorCode = CURLE_OK;
  
  float timeout = 15;
  if (BZDB.isSet("httpTimeout")) {
    timeout = BZDB.eval("httpTimeout");
  }

#if LIBCURL_VERSION_NUM >= 0x070a00
  result = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, true);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_NOSIGNAL error: %d\n", result);
  }
#endif

  result = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_TIMEOUT error: %d\n", result);
  }

  result = curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_NOBODY error: %d\n", result);
  }

  result = curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_URL error: %d\n", result);
  }

  // FIXME: This could block for a _long_ time.
  result = curl_easy_perform(curl);
  if (result == (CURLcode)CURLOPT_ERRORBUFFER) {
    errorCode = result;
    DEBUG1("curl_easy_perform() error: server reported: %d\n", result);
  } else if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("curl_easy_perform() error: %d\n", result);
  }

  result = curl_easy_setopt(curl, CURLOPT_URL, NULL);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_URL error: %d\n", result);
  }

  result = curl_easy_setopt(curl, CURLOPT_NOBODY, 0);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_NOBODY error: %d\n", result);
  }

  result = curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_HTTPGET error: %d\n", result);
  }

  return (errorCode == CURLE_OK);
}


void URLManager::freeURLData(void *data)
{
  free(data);
}


URLManager::URLManager()
{
  CURLcode result;
  
  easyHandle = NULL;
  theData = NULL;
  theLen = 0;
  errorCode = CURLE_OK;

  DEBUG1("LIBCURL: %s\n", curl_version());
  
#if LIBCURL_VERSION_NUM >= 0x070a00
  if ((result = curl_global_init(CURL_GLOBAL_NOTHING)))
    DEBUG1("Unexpected error from libcurl; Error: %d\n", result);
#endif

  easyHandle = curl_easy_init();
  if (!easyHandle) {
    DEBUG1("Something wrong with CURL\n");
    return;
  }
  CURL* curl = easyHandle;
  
  result = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_WRITEFUNCTION error: %d\n", result);

  result = curl_easy_setopt(curl, CURLOPT_FILE, this);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_FILE error: %d\n", result);

  result = curl_easy_setopt(curl, CURLOPT_FILETIME, 1);
  if (result != CURLE_OK)
    DEBUG1("CURLOPT_FILETIME error: %d\n", result);
}


URLManager::~URLManager()
{
  clearInternal();

  if (easyHandle)
    curl_easy_cleanup((CURL*)easyHandle);

#if LIBCURL_VERSION_NUM >= 0x070a00
  curl_global_cleanup();
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
  errorCode = CURLE_OK;
  
  return;
}


bool URLManager::beginGet(const std::string URL)
{
  CURL* curl = (CURL*)easyHandle;
  if (curl == NULL) {
    return false;
  }

  CURLcode result;  
  errorCode = CURLE_OK;
  
  float timeout = 15;
  if (BZDB.isSet("httpTimeout"))
    timeout = BZDB.eval("httpTimeout");

#if LIBCURL_VERSION_NUM >= 0x070a00
  result = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, true);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_NOSIGNAL error: %d\n", result);
  }
#endif

  result = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_TIMEOUT error: %d\n", result);
  }

  result = curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_URL error: %d\n", result);
  }

  // FIXME: This could block for a _long_ time.
  result = curl_easy_perform(curl);
  if (result == (CURLcode)CURLOPT_ERRORBUFFER) {
    errorCode = result;
    DEBUG1("curl_easy_perform() error: server reported: %d\n", result);
  } else if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("curl_easy_perform() error: %d\n", result);
  }

  result = curl_easy_setopt(curl, CURLOPT_URL, NULL);
  if (result != CURLE_OK) {
    errorCode = result;
    DEBUG1("CURLOPT_URL error: %d\n", result);
  }

  if (!theData) {
    return false;
  }

  return (errorCode == CURLE_OK);
}


void URLManager::setProgressFunc(int (*func)(void* clientp,
					     double dltotal, double dlnow,
					     double ultotal, double ulnow),
					     void* data)
{
  CURL* curl = (CURL*)easyHandle;
  if (curl == NULL) {
    return;
  }
  CURLcode code;
  if (func != NULL) {
    code = curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, func);
    code = curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, data);
    code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
  } else {
    code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
  }
  return;
}


static size_t writeFunction(void *ptr, size_t size, size_t nmemb, void *stream)
{
  int len = size * nmemb;
  ((URLManager*)stream)->collectData((char*)ptr, len);
  return len;
}


const char* URLManager::getErrorString() const
{
  if (easyHandle == NULL) {
    return "libcurl initialization error";
  }
  
  // LIBCURL didn't get curl_easy_strerror() until version 7.12.0.
  // BZFlag isn't currently limiting the libcurl version, so here's
  // a local function to do the same.

#define STRING_CASE(x)  \
  case CURLE_##x: return #x
  
  switch (errorCode) {
    case CURLE_OK: {
      return "Nah problems mon";
    }
    STRING_CASE(UNSUPPORTED_PROTOCOL);
    STRING_CASE(FAILED_INIT);
    STRING_CASE(URL_MALFORMAT);
    STRING_CASE(URL_MALFORMAT_USER);
    STRING_CASE(COULDNT_RESOLVE_PROXY);
    case CURLE_COULDNT_RESOLVE_HOST: {
      return "could not resolve hostname";
    }
    case CURLE_COULDNT_CONNECT: {
      return "could not connect";
    }
    STRING_CASE(FTP_WEIRD_SERVER_REPLY);
    STRING_CASE(FTP_ACCESS_DENIED);
    STRING_CASE(FTP_USER_PASSWORD_INCORRECT);
    STRING_CASE(FTP_WEIRD_PASS_REPLY);
    STRING_CASE(FTP_WEIRD_USER_REPLY);
    STRING_CASE(FTP_WEIRD_PASV_REPLY);
    STRING_CASE(FTP_WEIRD_227_FORMAT);
    STRING_CASE(FTP_CANT_GET_HOST);
    STRING_CASE(FTP_CANT_RECONNECT);
    STRING_CASE(FTP_COULDNT_SET_BINARY);
    STRING_CASE(PARTIAL_FILE);
    STRING_CASE(FTP_COULDNT_RETR_FILE);
    STRING_CASE(FTP_WRITE_ERROR);
    STRING_CASE(FTP_QUOTE_ERROR);
    STRING_CASE(HTTP_NOT_FOUND); // a.k.a. HTTP_RETURNED_ERROR
    STRING_CASE(WRITE_ERROR);
    STRING_CASE(MALFORMAT_USER);
    STRING_CASE(FTP_COULDNT_STOR_FILE);
    STRING_CASE(READ_ERROR);
    STRING_CASE(OUT_OF_MEMORY);
    STRING_CASE(OPERATION_TIMEOUTED);
    STRING_CASE(FTP_COULDNT_SET_ASCII);
    STRING_CASE(FTP_PORT_FAILED);
    STRING_CASE(FTP_COULDNT_USE_REST);
    STRING_CASE(FTP_COULDNT_GET_SIZE);
    STRING_CASE(HTTP_RANGE_ERROR);
    STRING_CASE(HTTP_POST_ERROR);
    STRING_CASE(SSL_CONNECT_ERROR);
    STRING_CASE(FTP_BAD_DOWNLOAD_RESUME); // a.k.a. BAD_DOWNLOAD_RESUME
    STRING_CASE(FILE_COULDNT_READ_FILE);
    STRING_CASE(LDAP_CANNOT_BIND);
    STRING_CASE(LDAP_SEARCH_FAILED);
    STRING_CASE(LIBRARY_NOT_FOUND);
    STRING_CASE(FUNCTION_NOT_FOUND);
    case CURLE_ABORTED_BY_CALLBACK: {
      return "aborted by user";
    }
    STRING_CASE(BAD_FUNCTION_ARGUMENT);
    STRING_CASE(BAD_CALLING_ORDER);
    STRING_CASE(HTTP_PORT_FAILED);
    STRING_CASE(BAD_PASSWORD_ENTERED);
    STRING_CASE(TOO_MANY_REDIRECTS );
    STRING_CASE(UNKNOWN_TELNET_OPTION);
    STRING_CASE(TELNET_OPTION_SYNTAX );
    STRING_CASE(OBSOLETE);
    STRING_CASE(SSL_PEER_CERTIFICATE);
    STRING_CASE(GOT_NOTHING);
    STRING_CASE(SSL_ENGINE_NOTFOUND);
#if LIBCURL_VERSION_NUM >= 0x070C03
    STRING_CASE(SSL_ENGINE_SETFAILED);
    STRING_CASE(SEND_ERROR);
    STRING_CASE(RECV_ERROR);
    STRING_CASE(SHARE_IN_USE);
    STRING_CASE(SSL_CERTPROBLEM);
    STRING_CASE(SSL_CIPHER);
    STRING_CASE(SSL_CACERT);
    STRING_CASE(BAD_CONTENT_ENCODING);
    STRING_CASE(LDAP_INVALID_URL);
    STRING_CASE(FILESIZE_EXCEEDED);
    STRING_CASE(FTP_SSL_FAILED);
#endif    
    default: {
      static char buffer[256];
      sprintf(buffer, "unknown LIBCURL error, code = %i\n", errorCode);
      return buffer;
    }
  }
}


/******************************************************************************/
#else // ! HAVE_CURL
/******************************************************************************/


// stub the functions
URLManager::URLManager()
{
}
URLManager::~URLManager()
{
}
bool URLManager::getURL(const std::string&, std::string&)
{
  return false;
}
bool URLManager::getURL(const std::string&, void **, unsigned int&)
{
  return false;
}
bool URLManager::getFileTime(time_t&)
{
  return false;
}
const char* URLManager::getErrorString() const
{
  return "this client does not have libcurl support";
}    
void URLManager::freeURLData(void*)
{
  return;
}
void URLManager::collectData(char*, int)
{
  return;
}
void URLManager::clearInternal()
{
  return;
}
bool URLManager::beginGet(const std::string)
{
  return false;
}
void URLManager::setProgressFunc(int (*)(void*, double, double,
						double, double), void*)
{
  return;
}


/******************************************************************************/
#endif // HAVE_CURL
/******************************************************************************/


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
