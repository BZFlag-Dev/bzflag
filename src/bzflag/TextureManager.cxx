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
#include "MediaFile.h"
#include "ErrorHandler.h"

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

	{ TX_EXPLOSION, 1, "explode1", OpenGLTexture::Linear },

	{ TX_TANK, NO_VARIANT, "flage", OpenGLTexture::LinearMipmapLinear },
	{ TX_FLAG, NO_VARIANT, "flag", OpenGLTexture::Max },

	{ TX_WALL, NO_VARIANT, "wall", OpenGLTexture::LinearMipmapLinear },
	{ TX_BOX, NO_VARIANT, "boxwall", OpenGLTexture::LinearMipmapLinear },
	{ TX_ROOF, NO_VARIANT, "roof", OpenGLTexture::LinearMipmapLinear },
	{ TX_PYRAMID, NO_VARIANT, "pyrwall", OpenGLTexture::LinearMipmapLinear },
	{ TX_CAUTION, NO_VARIANT, "caution", OpenGLTexture::LinearMipmapLinear },

	{ TX_TITLEFONT, NO_VARIANT, "title", OpenGLTexture::Linear },
};

FileTextureInit fileBigLoader[] =
{
	{ TX_MOUNTAIN, NO_VARIANT, "mountain", OpenGLTexture::LinearMipmapLinear },
};

ProcTextureInit procLoader[] =
{
	{ TX_NOISE, NO_VARIANT, TextureManager::noiseProc, OpenGLTexture::Nearest },
};


TextureManager::TextureManager()
{
  int i, numTextures;

  numTextures = countof(fileLoader);
  for (i = 0; i < numTextures; i++)
    addTexture( fileLoader[i].type, fileLoader[i].variant, loadTexture( fileLoader[i] ));

  numTextures = countof(fileBigLoader);
  for (i = 0; i < numTextures; i++)
    loadBigTexture(fileBigLoader[i].type, fileBigLoader[i]);

  numTextures = countof(procLoader);
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
  int width, height;
  unsigned char* image =  MediaFile::readImage( init.fileName, &width, &height);
  if (!image) {
    std::vector<std::string> args;
    args.push_back(init.fileName);
    printError("cannot load texture: {1}", &args);
    return new OpenGLTexture();
  }

  OpenGLTexture *texture = new OpenGLTexture(width, height, image, init.filter, true);
  delete[] image;

  return texture;
}

void TextureManager::loadBigTexture(TextureType type, FileTextureInit &init)
{
  // get mountain texture then break up into textures no larger than
  // 256 pixels wide and no higher than wide, whichever is less (go
  // by height rounded up to a power of 2).  also, will be using
  // border pixels from adjacent textures, so must add 2 pixels to
  // width of each texture (if the texture gets split up).
  int width, height;
  unsigned char* image =  MediaFile::readImage( init.fileName, &width, &height);
  if (!image) {
    std::vector<std::string> args;
    args.push_back(init.fileName);
    printError("cannot load texture: {1}", &args);
    return;
  }

  // find power of two at least as large as height
  int scaledHeight = 1;
  while (scaledHeight < height)
    scaledHeight <<= 1;

  // choose minimum width
  int minWidth = scaledHeight;
  if (minWidth > 256)
    minWidth = 256;

  int numTextures;
  // prepare each texture
  if (width <= minWidth) {
    OpenGLTexture *texture = new OpenGLTexture(width, height,
					       image,
					       OpenGLTexture::Linear,
					       true);
    m_Textures.insert(std::map<int,OpenGLTexture*>::value_type(type << 16, texture));
  } else {
    numTextures = (width + minWidth - 3) / (minWidth - 2);
    unsigned char* subimage = new unsigned char[4 * minWidth * height];
    const int subwidth = width / numTextures;
    for (int n = 0; n < numTextures; n++) {
      // pick size of subtexture
      int dx = subwidth;
      if (n == numTextures - 1)
	dx += width % numTextures;
    
      // copy subimage
      const unsigned char* src = image + 4 * n * subwidth;
      unsigned char* dst = subimage + 4;
      int i, j;
      for (j = 0; j < height; j++) {
	for (i = 0; i < subwidth; i++) {
	  dst[4 * i + 0] = src[4 * i + 0];
	  dst[4 * i + 1] = src[4 * i + 1];
	  dst[4 * i + 2] = src[4 * i + 2];
	  dst[4 * i + 3] = src[4 * i + 3];
	}
	src += 4 * width;
	dst += 4 * minWidth;
      }
    
      // copy left border
      if (n == 0)
	src = image + 4 * (width - 1);
      else
	src = image + 4 * n * subwidth - 4;
      dst = subimage;
      for (j = 0; j < height; j++) {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
	src += 4 * width;
	dst += 4 * minWidth;
      }

      // copy right border
      if (n == numTextures - 1)
	src = image;
      else
	src = image + 4 * (n + 1) * subwidth - 4;
      dst = subimage + 4 * (minWidth - 1);
      for (j = 0; j < height; j++) {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
	src += 4 * width;
	dst += 4 * minWidth;
      }

      // make texture and set gstate
      OpenGLTexture *texture = new OpenGLTexture(dx + 2, height,
						 subimage,
						 OpenGLTexture::Linear,
						 false);
      m_Textures.insert(std::map<int,OpenGLTexture*>::value_type(type << 16 | n, texture));
    }
    delete[] subimage;
  }

  delete[] image;
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
