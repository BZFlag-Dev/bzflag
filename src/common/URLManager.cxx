/* bzflag
* Copyright (c) 1993 - 2004 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// bzflag common header


#include "common.h"

#include "URLManager.h"
#include <iostream>

#include "bzfio.h"

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#define _WINSOCK2API_
#endif
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif

template <>
URLManager* Singleton<URLManager>::_instance = (URLManager*)0;

#ifdef HAVE_CURL
static size_t writeFunction(void *ptr, size_t size, size_t nmemb, void *stream);
#endif // HAVE_CURL

#ifdef HAVE_CURL
bool URLManager::getURL ( const std::string URL, std::string &data )
#else
bool URLManager::getURL ( const std::string, std::string&)
#endif // HAVE_CURL
{
	if (theData)
		free (theData);

	theData = NULL;
	theLen = 0;

#ifdef HAVE_CURL
	CURLcode result;
	if (!easyHandle) {
		return false;
	}

	result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_URL, URL.c_str());
	if (result) {
		DEBUG1("Something wrong with CURL; Error:  %d",result);
		return false;
	}

	result = curl_easy_perform((CURL*)easyHandle);
	if (result == (CURLcode)CURLOPT_ERRORBUFFER) {
		DEBUG1("Error: server reported: %d",result);
		return false;
	}else if (result) {
		DEBUG1("Something wrong with CURL; Error: %d",result);
		return false;
	}

	result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_URL, NULL);
	if (result) {
		DEBUG1("Something wrong with CURL; Error: %d",result);
		return false;
	}
	
	if (!theData)
		return false;

	char	* newData = (char*)malloc(theLen + 1);
	memcpy(newData,theData,theLen);

	newData[theLen] = 0;
	
	data = newData;
	free(newData);

	return true;
#endif
	return false;
}

#ifdef HAVE_CURL
bool URLManager::getURL ( const std::string URL, void **data, unsigned int& size )
#else
bool URLManager::getURL (const std::string, void **, unsigned int&)
#endif // HAVE_CURL
{
	if (theData)
		free (theData);

	theData = NULL;
	theLen = 0;

#ifdef HAVE_CURL
	CURLcode result;
	if (!easyHandle) {
		return false;
	}

	result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_URL, URL.c_str());
	if (result) {
		DEBUG1("Something wrong with CURL; Error:  %d",result);
	}

	result = curl_easy_perform((CURL*)easyHandle);
	if (result == (CURLcode)CURLOPT_ERRORBUFFER) {
		DEBUG1("Error: server reported: %d",result);
		return false;
	}else if (result) {
		DEBUG1("Something wrong with CURL; Error: %d",result);
		return false;
	}

	result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_URL, NULL);
	if (result) {
		DEBUG1("Something wrong with CURL; Error: %d",result);
		return false;
	}

	if (!theData)
		return false;

	*data = malloc(theLen);
	memcpy(*data,theData,theLen);
	size = theLen;
	return true;
#endif 
	return false;
}

void URLManager::freeURLData ( void *data )
{
	free(data);
}

URLManager::URLManager()
{
	easyHandle = NULL;
	theData = NULL;
	theLen = 0;

#ifdef HAVE_CURL
	CURLcode curlResult;
	if ((curlResult = curl_global_init(CURL_GLOBAL_NOTHING)))
		DEBUG1("Unexpected error from libcurl; Error: %d",curlResult);

	easyHandle = curl_easy_init();
	if (!easyHandle) {
		DEBUG1("Something wrong with CURL");
		return;
	}

	CURLcode result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_WRITEFUNCTION, writeFunction);
	if (result)
			DEBUG1("Something wrong with CURL; Error: %d",result);

	result = curl_easy_setopt((CURL*)easyHandle, CURLOPT_WRITEDATA, this);
	if (result)
		DEBUG1("Something wrong with CURL; Error: %d",result);
#endif
}

URLManager::~URLManager()
{
	if (theData)
		free (theData);

	theData = NULL;
	theLen = 0;

#ifdef HAVE_CURL
	if (easyHandle)
		curl_easy_cleanup((CURL*)easyHandle);
	curl_global_cleanup();
#endif
}

void URLManager::collectData(char* ptr, int len)
{
	unsigned char	*newData = (unsigned char*)malloc(theLen + len);
	if (theData)
		memcpy(newData,theData,theLen);

	memcpy(&(newData[theLen]),ptr,len);
	theLen+= len;

	free(theData);
	theData = newData;
}

#ifdef HAVE_CURL
static size_t writeFunction(void *ptr, size_t size, size_t nmemb,void *stream)
{
	int len = size * nmemb;
	((URLManager *)stream)->collectData((char *)ptr, len);
	return len;
}
#endif // HAVE_CURL


