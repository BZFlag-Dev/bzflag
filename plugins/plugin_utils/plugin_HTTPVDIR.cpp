/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "plugin_HTTPVDIR.h"
#include "plugin_utils.h"
BZFSHTTPVDir::BZFSHTTPVDir()
{
  bz_loadPlugin("HTTPServer",NULL);
}

void BZFSHTTPVDir::registerVDir ( void )
{
  bz_callCallback("RegisterHTTPDVDir",this);
}

BZFSHTTPVDir::~BZFSHTTPVDir()
{
  bz_callCallback("RemoveHTTPDVDir",this);
}
std::string BZFSHTTPVDir::getBaseURL ( void )
{
  std::string URL = "http://";
  std::string host = "localhost";
  if (bz_getPublicAddr().size())
    host = bz_getPublicAddr().c_str();

  // make sure it has the port
  if ( strrchr(host.c_str(),':') == NULL )
    host += format(":%d",bz_getPublicPort());

  URL += host +"/";
  URL += getVDir();
  URL += "/";

  return URL;
}

bool HTTPRequest::getParam ( const char* param, std::string &val ) const
{
  val = "";
  if (!param)
    return false;

  std::map<std::string, std::string>::const_iterator itr = paramaters.find(tolower(std::string(param)));
  if ( itr != paramaters.end() )
  {
    val = itr->second;
    return true;
  } 
  return false;
}

bool HTTPRequest::getParam ( const std::string &param, std::string &val ) const
{
  val = "";

  std::map<std::string, std::string>::const_iterator itr = paramaters.find(tolower(param));
  if ( itr != paramaters.end() )
  {
    val = itr->second;
    return true;
  } 
  return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
