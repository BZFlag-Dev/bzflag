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

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

#include "MasterBanList.h"

MasterBanList::MasterBanList()
{
#ifdef HAVE_CURL
  easyHandle = curl_easy_init();
  if (!easyHandle)
    std::cout << "Something wrong with CURL" << std::endl;
  curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION, writeFunction);
  curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, this);
#endif
}

MasterBanList::~MasterBanList()
{
#ifdef HAVE_CURL
  curl_easy_cleanup(easyHandle);
#endif
}

void MasterBanList::collectData(char* ptr, int len)
{
  std::string readData(ptr, 0, len);
  data += readData;
}

size_t MasterBanList::writeFunction(void *ptr, size_t size, size_t nmemb,
				    void *stream)
{
  int len = size * nmemb;
  ((MasterBanList *)stream)->collectData((char *)ptr, len);
  return len;
}

const std::string& MasterBanList::get ( const std::string URL )
{
  data = "";
  // get all up on the internet and go get the thing
  curl_easy_setopt(easyHandle, CURLOPT_URL, URL.c_str());
  curl_easy_perform(easyHandle);
  curl_easy_setopt(easyHandle, CURLOPT_URL, NULL);
  return data;
}
