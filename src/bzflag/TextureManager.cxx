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

#ifdef _WIN32
#pragma warning( 4: 4786 )
#endif

#include "TextureManager.h"
#include "texture.h"
#include "global.h"

const int NO_VARIANT = (-1);

FileTextureInit fileLoader[] =
{
	{ TX_BOLT, RogueTeam, "ybolt", OpenGLTexture::Linear },
	{ TX_BOLT, RedTeam, "rbolt", OpenGLTexture::Linear },
	{ TX_BOLT, GreenTeam, "gbolt", OpenGLTexture::Linear },
	{ TX_BOLT, PurpleTeam, "pbolt", OpenGLTexture::Linear },
	{ TX_BOLT, BlueTeam, "bbolt", OpenGLTexture::Linear },
	{ TX_BOLT, RabbitTeam, "wbolt", OpenGLTexture::Linear },

	{ TX_TRANSBOLT, RogueTeam, "ytbolt", OpenGLTexture::Linear },
	{ TX_TRANSBOLT, RedTeam, "rtbolt", OpenGLTexture::Linear },
	{ TX_TRANSBOLT, GreenTeam, "gtbolt", OpenGLTexture::Linear },
	{ TX_TRANSBOLT, PurpleTeam, "ptbolt", OpenGLTexture::Linear },
	{ TX_TRANSBOLT, BlueTeam, "btbolt", OpenGLTexture::Linear },
	{ TX_TRANSBOLT, RabbitTeam, "wtbolt", OpenGLTexture::Linear },

	{ TX_LASER, RogueTeam, "ylaser", OpenGLTexture::Max },
	{ TX_LASER, RedTeam, "rlaser", OpenGLTexture::Max },
	{ TX_LASER, GreenTeam, "glaser", OpenGLTexture::Max },
	{ TX_LASER, PurpleTeam, "plaser", OpenGLTexture::Max },
	{ TX_LASER, BlueTeam, "blaser", OpenGLTexture::Max },
	{ TX_LASER, RabbitTeam, "wlaser", OpenGLTexture::Max },

	{ TX_THIEF, NO_VARIANT, "thief", OpenGLTexture::Max },
	{ TX_MISSILE, NO_VARIANT, "missile", OpenGLTexture::Max },

	{ TX_GROUND, NO_VARIANT, "ground", OpenGLTexture::LinearMipmapLinear },
	{ TX_CLOUDS, NO_VARIANT, "clouds", OpenGLTexture::LinearMipmapLinear },
	{ TX_MOUNTAIN, NO_VARIANT, "mountain", OpenGLTexture::LinearMipmapLinear },

	{ TX_EXPLOSION, 1, "explode1", OpenGLTexture::Linear },

	{ TX_TANK, NO_VARIANT, "flage", OpenGLTexture::LinearMipmapLinear },
	{ TX_FLAG, NO_VARIANT, "flag", OpenGLTexture::Max },
};

ProcTextureInit procLoader[] = 
{
	{ TX_NOISE, NO_VARIANT, TextureManager::noiseProc, OpenGLTexture::Nearest },
};


TextureManager::TextureManager()
{
  int i, numTextures;
  
  numTextures = sizeof( fileLoader ) / sizeof( FileTextureInit );
  for (i = 0; i < numTextures; i++)
    addTexture( fileLoader[i].type, fileLoader[i].variant, loadTexture( fileLoader[i] ));

  numTextures = sizeof( procLoader ) / sizeof( ProcTextureInit );
  for (i = 0; i < numTextures; i++)
    addTexture( procLoader[i].type, procLoader[i].variant, procLoader[i].proc( procLoader[i] ));
}

TextureManager::~TextureManager()
{
  for( std::map<int, OpenGLTexture*>::iterator it = m_Textures.begin(); it != m_Textures.end(); ++it) {
    OpenGLTexture* tex = it->second;
    if (tex != NULL) {
      delete tex;
    }
  }
  m_Textures.clear();
}

OpenGLTexture* TextureManager::getTexture( TextureType type )
{
  return getTexture(type, NO_VARIANT);
}

OpenGLTexture* TextureManager::getTexture( TextureType type, int variant )
{
  std::map<int,OpenGLTexture*>::iterator it = m_Textures.find(type<<16|(variant&65535));
  if (it != m_Textures.end())
    return it->second;
  return NULL;
}

void TextureManager::addTexture( TextureType type, OpenGLTexture *texture)
{
  addTexture(type, NO_VARIANT, texture);
}

void TextureManager::addTexture( TextureType type, int variant, OpenGLTexture *texture )
{
  if (texture == NULL)
    return;

  m_Textures.insert(std::map<int,OpenGLTexture*>::value_type(type<<16|(variant&65535), texture));
}

OpenGLTexture* TextureManager::loadTexture( FileTextureInit &init )
{
  OpenGLTexture *texture = new OpenGLTexture();
  *texture = ::getTexture( init.fileName, init.filter);
  return texture;
}

/* --- Procs --- */

OpenGLTexture *TextureManager::noiseProc(ProcTextureInit &init)
{
  const int size = 4 * 128 * 128;
  unsigned char* noise = new unsigned char[size];
  for (int i = 0; i < size; i += 4 ) {
    unsigned char n = (unsigned char)floor(256.0 * bzfrand());
    noise[i+0] = n;
    noise[i+1] = n;
    noise[i+2] = n;
    noise[i+3] = n;
  }

  OpenGLTexture *texture = new OpenGLTexture(128,128,noise,init.filter);
  delete noise;
  return texture;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
