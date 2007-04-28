/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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

// BZFlag common header
#include "common.h"

// system headers
#include <string>
#include <map>
#include <crystalspace.h>

// common implementation headers
#include "Singleton.h"


struct FileTextureInit
{
  std::string		name;
};


typedef  struct
{
  int   id;
  int   x;
  int   y;
  bool  alpha;
  csRef<iMaterialWrapper> material;
  std::string   name;
} ImageInfo;

class TextureManager;

struct ProcTextureInit
{
  std::string		name;
  TextureManager	*manager;
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
  void clearAll();

  bool bind ( int id );
  bool bind ( const char* name );

  const ImageInfo& getInfo ( int id );
  const ImageInfo& getInfo ( const char* name );

  float GetAspectRatio ( int id );

  int newTexture();
protected:
  friend class Singleton<TextureManager>;

private:
  TextureManager();
  TextureManager(const TextureManager &tm);
  TextureManager& operator=(const TextureManager &tm);
  ~TextureManager();

  bool loadTexture(ImageInfo &info);
  csRef<iImage> loadImage(std::string filename);

  typedef std::map<std::string, ImageInfo> TextureNameMap;
  typedef std::map<int, ImageInfo*> TextureIDMap;

  int	    lastImageID;
  TextureIDMap   textureIDs;
  TextureNameMap textureNames;

  iTextureManager   *tm;
  iTextureList      *tl;
  int                format;
};


#endif //_TEXTURE_MANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
