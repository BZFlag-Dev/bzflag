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

#include <string>
#include <map>

#include "OpenGLTexture.h"
#include "Singleton.h"

struct FileTextureInit
{
  std::string		name;
  OpenGLTexture::Filter	filter;
};

class TextureManager;

struct ProcTextureInit
{
  std::string		        name;
  int	(*proc)(ProcTextureInit &init);
  TextureManager                *manager;
  OpenGLTexture::Filter	        filter;
};


typedef std::map<std::string, OpenGLTexture*> TextureNameMap;

class TextureManager : public Singleton<TextureManager>
{
public:
  OpenGLTexture* getTexture( const char* name, bool reportFail = true );
  int addTexture( const char*, OpenGLTexture *texture );

  int newTexture ( const char* name, int x, int y, unsigned char* data, OpenGLTexture::Filter filter );
protected:
  friend class Singleton<TextureManager>;

private:
  TextureManager();
  TextureManager(const TextureManager &tm);
  TextureManager& operator=(const TextureManager &tm);
  ~TextureManager();

  OpenGLTexture* loadTexture( FileTextureInit &init, bool reportFail = true  );


  TextureNameMap m_Textures;
};


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
