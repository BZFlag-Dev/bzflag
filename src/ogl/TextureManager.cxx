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

TextureInit loader[] =
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
	{ TX_EXPLOSION, 2, "explode2", OpenGLTexture::Linear },
	{ TX_EXPLOSION, 3, "explode3", OpenGLTexture::Linear },
	{ TX_EXPLOSION, 4, "explode4", OpenGLTexture::Linear },
};


TextureManager *TextureManager::m_TM = NULL;

TextureManager* TextureManager::getTextureManager()
{
  if (m_TM == NULL)
    m_TM = new TextureManager();

  return m_TM;
}

void TextureManager::terminate()
{
  if (m_TM)
    delete m_TM;
  m_TM = NULL;
}
  
TextureManager::TextureManager()
{
  int numTextures = sizeof( loader ) / sizeof( TextureInit );
  for (int i = 0; i < numTextures; i++)
    addTexture( loader[i].type, loader[i].variant, loadTexture( loader[i] ));
}

TextureManager::~TextureManager()
{
  for( std::map<int, OpenGLTexture*>::iterator it = m_Textures.begin(); it != m_Textures.end(); ++it) {
    OpenGLTexture* tex = it->second;
    delete tex;
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

OpenGLTexture* TextureManager::loadTexture( TextureInit &init )
{
  OpenGLTexture *texture = new OpenGLTexture();
  *texture = ::getTexture( init.fileName, init.filter);
  return texture;
}
