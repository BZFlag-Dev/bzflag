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
#ifndef _TEXTURE_MANAGER_H
#define _TEXTURE_MANAGER_H

#include <string>
#include <map>

#include "OpenGLTexture.h"
#include "Singleton.h"

typedef enum {
	Off,
	Nearest,
	Linear,
	NearestMipmapNearest,
	LinearMipmapNearest,
	NearestMipmapLinear,
	LinearMipmapLinear,
	Max = LinearMipmapLinear
} eTextureFilter;

struct FileTextureInit
{
  std::string		name;
  eTextureFilter	filter;
};


typedef  struct
{
  int   id;
  int   x;
  int   y;
  bool  alpha;
  OpenGLTexture *texture;
  std::string   name;
} ImageInfo;

class TextureManager;

struct ProcTextureInit
{
  std::string		name;
  TextureManager	*manager;
  eTextureFilter	filter;
  int			(*proc)(ProcTextureInit &init);
};

typedef std::map<std::string, ImageInfo> TextureNameMap;
typedef std::map<int, ImageInfo*> TextureIDMap;

class TextureManager : public Singleton<TextureManager>
{
public:
  int getTextureID( const char* name, bool reportFail = true );
  int addTexture( const char*, OpenGLTexture *texture  );

  bool bind ( int id );
  bool bind ( const char* name );

  const ImageInfo& getInfo ( int id );
  const ImageInfo& getInfo ( const char* name );

  eTextureFilter getMaxFilter ( void ) { return currentMaxFilter;}
  std::string getMaxFilterName ( void );
  void setMaxFilter ( eTextureFilter filter );
  void setMaxFilter ( std::string filter );

  float GetAspectRatio ( int id );

  int newTexture ( const char* name, int x, int y, unsigned char* data, eTextureFilter filter, bool repeat = true, int format = 0 );
protected:
  friend class Singleton<TextureManager>;

private:
  TextureManager();
  TextureManager(const TextureManager &tm);
  TextureManager& operator=(const TextureManager &tm);
  ~TextureManager();

  OpenGLTexture* loadTexture( FileTextureInit &init, bool reportFail = true  );

  int            lastImageID;
  int            lastBoundID;
  TextureIDMap   textureIDs;
  TextureNameMap textureNames;

  eTextureFilter currentMaxFilter;
  std::string	 configFilterValues[Max + 1];
};

#endif //_TEXTURE_MANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
