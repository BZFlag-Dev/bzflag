/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#define _BZFS_HTTP_H_

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
} bzhttp_eRequestType;

class BZF_API bzhttp_Request
{
public:
  bzhttp_Request();
  virtual ~bzhttp_Request();

  bzhttp_eRequestType RequestType;
  bz_ApiString	URL;
  bz_ApiString	Resource;

  bz_ApiString	RequesterIP;
  bz_ApiString	RequesterHost;

  bz_ApiString	BZID;
  bz_ApiString	BZIDCallsign;
  bz_APIStringList BZIDGroups;

  virtual bool UserHasPerm ( const char* perm ) const;

  bz_ApiString	Body;

  virtual void AddHeader ( const char* name, const char* value);
  virtual const char* GetHeader ( const char* name) const;
  virtual const char* GetHeader ( size_t index ) const;
  virtual size_t GetHeaderCount ()const;

  virtual void AddCookie ( const char* name, const char* value);
  virtual const char* GetCookie ( const char* name) const;
  virtual const char* GetCookie ( size_t index ) const;
  virtual size_t GetCookieCount () const;

  virtual void AddParamater ( const char* name, const char* value);
  virtual const char* GetParamater ( const char* name) const;
  virtual const char* GetParamater ( size_t index ) const;
  virtual size_t GetParamaterCount () const;

  virtual bool InBZIDGroup (const char *) const { return false; }

  bzhttp_SessionData *Session;

  int Version;
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

class BZF_API bzhttp_Response
{
public:
  bzhttp_Response();
  virtual ~bzhttp_Response();

  bzhttp_eReturnCode ReturnCode;
  bzhttp_eDocumentType DocumentType;

  bool ForceNoCache;

  bz_ApiString	RedirectLocation;
  bz_ApiString	MimeType;

  bz_ApiString	MD5Hash;

  bz_ApiString CookieDomain;
  bz_ApiString CookiePath;

  virtual void AddHeader ( const char* name, const char* value);
  virtual void AddCookies ( const char* name, const char* value);

  virtual void AddBodyData ( const char* value);
  virtual void AddBodyData ( const void* value, size_t size);

  int Version;

  void  *pimple;
};

typedef enum
{
  eNoPage = 0,
  eWaitForIt,
  ePageDone
} bzhttp_ePageGenStatus;

typedef enum
{
  eNoAuth = 0,
  eHTTPBasic,
  eHTTPOther,
  eBZID
} bzhttp_eAuthenticationMethod;

typedef enum
{
  eAuthFail = 0,
  eAuthOK,
  eNotAuthedYet
} bzhttp_eAuthenticationStatus;

class BZF_API bzhttp_VDir
{
public:
  bzhttp_VDir();
  virtual ~bzhttp_VDir();
  virtual const char* VDirName() = 0;
  virtual const char* VDirDescription() { return NULL; }

  virtual bzhttp_ePageGenStatus GeneratePage ( const bzhttp_Request& request, bzhttp_Response &response ) = 0;
  virtual bool SupportPut ( void ) { return false; }
  virtual bool AllowResourceDownloads ( void ) { return false; }

  bz_ApiString BaseURL;
  bz_APIStringList ResourceDirs;

  bzhttp_eAuthenticationMethod RequiredAuthentiction;
  bz_ApiString OtherAuthenicationMethod;
  bz_ApiString HTTPAuthenicationRelalm;

  // server groups are automatically included
  bz_APIStringList BZIDAuthenicationGroups;
  bool CacheAuthentication;

  virtual bzhttp_eAuthenticationStatus AuthenticateHTTPUser ( const char* /*ipAddress*/, const char* /*user*/, const char* /*password*/, const bzhttp_Request& /*request*/ ) { return eAuthFail; }
  virtual bool GenerateNoAuthPage ( const bzhttp_Request& /*request*/, bzhttp_Response &/*response*/ ) { return false; }

  // data sizes
  int MaxRequestSize;
  int MaxRequestBody;

  void AddMimeType(const char* extension, const char* mime );
  void AddStandardTypes ();

  void* pimple;
};

BZF_API bool bzhttp_RegisterVDir (bz_Plugin* plugin, bzhttp_VDir *vdir );
BZF_API bool bzhttp_RemoveVDir (bz_Plugin* plugin, bzhttp_VDir *vdir );
BZF_API bool bzhttp_RemoveAllVdirs (bz_Plugin* plugin );

// templates
class BZF_API bzhttp_TemplateMetaData
{
public:
  bzhttp_TemplateMetaData();
  bzhttp_TemplateMetaData( const bzhttp_TemplateMetaData& /*r*/);
  virtual ~bzhttp_TemplateMetaData();

  const char * Get ( const char* key, unsigned int index = 0 );
  unsigned int Count ( const char* key );

  void Add ( const char* key, const char* val );

protected:
  void *pimple;
};

class BZF_API bzhttp_TemplateCallback
{
public:
  virtual ~bzhttp_TemplateCallback() {};
  virtual bz_ApiString GetTemplateKey(const char* /* key */) { return bz_ApiString(""); }
  virtual bool GetTemplateLoop(const char* /* key */, const char* /*param*/) { return false; }
  virtual bool GetTemplateIF(const char* /* key */, const char* /*param*/) { return false; }

  bzhttp_TemplateMetaData* MetaData;
};

BZF_API bz_ApiString bzhttp_RenderTemplate ( const char* file, bzhttp_TemplateCallback* callback, const char* pathSet = NULL );
BZF_API bz_ApiString bzhttp_RenderTemplateFromText ( const char* text, bzhttp_TemplateCallback* callback, const char* pathSet = NULL );

BZF_API bzhttp_TemplateMetaData bzhttp_GetTemplateMetaData( const char* file );

// path utilities
BZF_API bool bzhttp_AddSearchPath ( const char* pathSet, const char* path );
BZF_API const char* bzhttp_FindFile ( const char* pathSet, const char* filename );


#endif //_BZFS_HTTP_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
