/* bzflag
 * Copyright (c) 1993-2011 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// a base class for plugins that want to do HTTP
#ifndef _BZFS_HTTP_H_
#define _PLUGIN_HTTP_H_

#include "bzfsAPI.h"

class BZF_API bzhttp_SessionData
{
public:
public:
  bzhttp_SessionData();
  ~bzhttp_SessionData();

  unsigned int SessionID;
  
  const char* GetPrivateItem ( const char* name );
  void SetPrivateItem ( const char * name, const char* value );
  void ClearPrivateItem ( const char * name );
  void FlushPrivateItems ( void );

  const char* GetGlobalItem ( const char* name );
  void SetGlobalItem ( const char * name, const char* value );
  void ClearGlobalItem ( const char * name );

  void  *pimple;
};

typedef enum
{
  eHTTPUnknown = 0,
  eHTTPGet,
  eHTTPHead,
  eHTTPPost,
  eHTTPPut,
  eHTTPDelete,
  eHTTPTrace,
  eHTTPOptions,
  eHTTPConnect
}bzhttp_eRequestType;

class BZF_API bzhttp_Request
{
public:
  bzhttp_Request();
  ~bzhttp_Request();

  bzhttp_eRequestType RequestType;
  bz_ApiString	URL;
  bz_ApiString	Resource;

  bz_ApiString	Body;

  void AddHeader ( const char* name, const char* value);
  const char* GetHeader ( const char* name);
  const char* GetHeader ( size_t index );
  size_t GetHeaderCount ();

  void AddCookie ( const char* name, const char* value);
  const char* GetCookie ( const char* name);
  const char* GetCookie ( size_t index );
  size_t GetCookieCount ();

  void AddParamater ( const char* name, const char* value);
  const char* GetParamater ( const char* name);
  const char* GetParamater ( size_t index );
  size_t GetParamaterCount ();

  bzhttp_SessionData *Session;

protected:
  void  *pimple;
};

typedef enum {
  e200OK,
  e301Redirect,
  e302Found,
  e401Unauthorized,
  e403Forbiden,
  e404NotFound,
  e418IAmATeapot,
  e500ServerError
} bzhttp_eReturnCode;

typedef enum {
  eText,
  eOctetStream,
  eBinary,
  eHTML,
  eCSS,
  eXML,
  eJSON,
  eOther
} bzhttp_eDocumentType;

class BZF_API bzhttp_Responce
{
public:
  bzhttp_Responce();
  ~bzhttp_Responce();

  bzhttp_eReturnCode ReturnCode;
  bzhttp_eDocumentType DocumentType;

  bool ForceNoCache;

  bz_ApiString	RedirectLocation;
  bz_ApiString	MimeType;

  bz_ApiString	MD5Hash;

  void AddHeader ( const char* name, const char* value);
  void AddCookies ( const char* name, const char* value);

  void AddBodyData ( const char* value);
  void AddBodyData ( const void* value, size_t size);

  void  *pimple;
};

typedef enum
{
  eNoPage = 0,
  eWaitForIt,
  ePageDone
}bzhttp_ePageGenStatus;

class BZF_API bzhttp_VDir
{
public:
  bzhttp_VDir();
  virtual ~bzhttp_VDir();
  virtual const char* VDirName() = 0;
  virtual const char* VDirDescription(){return NULL;}

  virtual bzhttp_ePageGenStatus GeneratePage ( const bzhttp_Request& request, bzhttp_Responce &responce ) = 0;
  virtual bool SupportPut ( void ){ return false;}
  virtual bool AllowResourceDownloads ( void ){ return false; }

  bz_ApiString BaseURL;
  bz_APIStringList ResourceDirs;

  void AddMimeType(const char* extension, const char* mime );
  void AddStandardTypes ();

  void* pimple;
};

BZF_API bool bzhttp_RegisterVDir (bz_Plugin* plugin, bzhttp_VDir *vdir );
BZF_API bool bzhttp_RemoveVDir (bz_Plugin* plugin, bzhttp_VDir *vdir );
BZF_API bool bzhttp_RemoveAllVdirs (bz_Plugin* plugin );

#endif //_BZFS_HTTP_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
