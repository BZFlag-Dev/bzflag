/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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

typedef enum TextureType { TX_BOLT, TX_TRANSBOLT, TX_LASER, TX_THIEF, TX_MISSILE,
                           TX_GROUND, TX_CLOUDS, TX_MOUNTAIN, TX_EXPLOSION,
			   TX_TANK, TX_FLAG, TX_WALL, TX_BOX, TX_ROOF, TX_PYRAMID,
			   TX_CAUTION, TX_TITLEFONT, TX_NOISE
			};

struct FileTextureInit
{
  TextureType		type;
  int			variant;
  char*			fileName;
  OpenGLTexture::Filter	filter;
};

struct ProcTextureInit
{
  TextureType		type;
  int			variant;
  OpenGLTexture*	(*proc)(ProcTextureInit &init);
  OpenGLTexture::Filter	filter;
};

class TextureManager : public Singleton<TextureManager>
{
public:
  OpenGLTexture* getTexture( TextureType type );
  OpenGLTexture* getTexture( TextureType type, int variant );
  void addTexture( TextureType type, OpenGLTexture *texture );
  void addTexture( TextureType type, int variant, OpenGLTexture *texture );

  static OpenGLTexture* noiseProc( ProcTextureInit &init );

protected:
  friend class Singleton<TextureManager>;
  ~TextureManager();

private:
  TextureManager();
  TextureManager(const TextureManager &tm);
  TextureManager& operator=(const TextureManager &tm);

  OpenGLTexture* loadTexture( FileTextureInit &init );
  void           loadBigTexture(TextureType type, FileTextureInit &init);


  std::map<int, OpenGLTexture*> m_Textures;
};


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
