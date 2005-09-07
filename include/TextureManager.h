/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef _TEXTURE_MANAGER_H
#define _TEXTURE_MANAGER_H

#include <string>
#include <map>

#include "OpenGLTexture.h"
#include "Singleton.h"


struct FileTextureInit
{
  std::string		name;
  OpenGLTexture::Filter	filter;
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
  OpenGLTexture::Filter	filter;
  int			(*proc)(ProcTextureInit &init);
};


class TextureManager : public Singleton<TextureManager>
{
public:
  int getTextureID( const char* name, bool reportFail = true );

  bool isLoaded(const std::string& name);
  bool removeTexture(const std::string& name);
  bool reloadTextures();
  bool reloadTextureImage(const std::string& name);

  void updateTextureFilters();
  void setTextureFilter(int texId, OpenGLTexture::Filter filter);
  OpenGLTexture::Filter getTextureFilter(int texId);

  bool bind ( int id );
  bool bind ( const char* name );

  const ImageInfo& getInfo ( int id );
  const ImageInfo& getInfo ( const char* name );

  OpenGLTexture::Filter getMaxFilter ( void );
  std::string getMaxFilterName ( void );
  void setMaxFilter ( OpenGLTexture::Filter filter );
  void setMaxFilter ( std::string filter );

  float GetAspectRatio ( int id );

  int newTexture (const char* name, int x, int y, unsigned char* data,
		  OpenGLTexture::Filter filter, bool repeat = true, int format = 0);
protected:
  friend class Singleton<TextureManager>;

private:
  TextureManager();
  TextureManager(const TextureManager &tm);
  TextureManager& operator=(const TextureManager &tm);
  ~TextureManager();

  int addTexture( const char*, OpenGLTexture *texture  );
  OpenGLTexture* loadTexture( FileTextureInit &init, bool reportFail = true  );

  typedef std::map<std::string, ImageInfo> TextureNameMap;
  typedef std::map<int, ImageInfo*> TextureIDMap;

  int	    lastImageID;
  int	    lastBoundID;
  TextureIDMap   textureIDs;
  TextureNameMap textureNames;
};

#endif //_TEXTURE_MANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
