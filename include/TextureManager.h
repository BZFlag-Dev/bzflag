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


typedef enum TextureType { TX_BOLT, TX_TRANSBOLT, TX_LASER, TX_THIEF, TX_MISSILE,
                           TX_GROUND, TX_CLOUDS, TX_MOUNTAIN };

struct TextureInit
{
  TextureType		type;
  int			variant;
  char*			fileName;
  OpenGLTexture::Filter	filter;
};

class TextureManager
{
public:
	static TextureManager* getTextureManager();
	static void TextureManager::terminate();

	~TextureManager();

	OpenGLTexture* getTexture( TextureType type );
	OpenGLTexture* getTexture( TextureType type, int variant );
	void addTexture( TextureType type, OpenGLTexture *texture );
	void addTexture( TextureType type, int variant, OpenGLTexture *texture );

private:
	TextureManager();
	TextureManager(const TextureManager &tm);
	TextureManager& operator=(const TextureManager &tm);

	OpenGLTexture* loadTexture( TextureInit &init );

	static TextureManager *m_TM;
	std::map<int, OpenGLTexture*> m_Textures;
};

