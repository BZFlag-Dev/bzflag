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

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif

#include "TextureManager.h"
#include "texture.h"
#include "global.h"
#include "MediaFile.h"
#include "ErrorHandler.h"

/*const int NO_VARIANT = (-1); */

// initialize the singleton
template <>
TextureManager* Singleton<TextureManager>::_instance = (TextureManager*)0;

ProcTextureInit procLoader[1];

TextureManager::TextureManager()
{
	// fill out the standard proc textures
  procLoader[0].name = "noise";
  procLoader[0].filter = OpenGLTexture::Nearest;
  procLoader[0].proc = TextureManager::noiseProc;
		
  int i, numTextures;
  numTextures = countof(procLoader);
  for (i = 0; i < numTextures; i++)
    addTexture( procLoader[i].name.c_str(), procLoader[i].proc( procLoader[i] ));
}

TextureManager::~TextureManager()
{
  // we are done remove all textures
  for( TextureNameMap::iterator it = m_Textures.begin(); it != m_Textures.end(); ++it) {
    OpenGLTexture* tex = it->second;
    if (tex != NULL) {
      delete tex;
    }
  }
  m_Textures.clear();
}

OpenGLTexture* TextureManager::getTexture( const char* name, bool reportFail )
{
  if (!name)
  	return NULL;
  std::string texName = name;
  // see if we have the texture
  TextureNameMap::iterator it = m_Textures.find(texName);
  if (it != m_Textures.end())
    return it->second;
  else { // we don't have it so try and load it
	  FileTextureInit	file;
	  file.filter = OpenGLTexture::LinearMipmapLinear;
	  file.name = texName;
	  OpenGLTexture* tex = loadTexture(file,reportFail);
	  if (!tex) // well crap it failed, give them nothing
		  return NULL;
	  addTexture(name,tex);
	  return tex;
  }  
  return NULL;
}

void TextureManager::addTexture( const char* name, OpenGLTexture *texture )
{
  if (!name || !texture)
    return;

   // if the texture allready exists kill it
   // this is why IDs are way better then objects for this stuff
   TextureNameMap::iterator it = m_Textures.find(name);
   if (it != m_Textures.end())
     delete it->second;

   m_Textures[name] = texture;
}

OpenGLTexture* TextureManager::loadTexture(FileTextureInit &init, bool reportFail)
{
  int width, height;
  std::string nameToTry = "";

  if (BZDB.isSet( "altImageDir" )) {
    nameToTry = BZDB.get( "altImageDir" );
#ifdef WIN32
   nameToTry += '\\';
#else
   nameToTry += '/';
#endif
   nameToTry += init.name;
  }
  else
    nameToTry = init.name;
  unsigned char* image =NULL;
  if (nameToTry.size() && nameToTry.c_str())
    image = MediaFile::readImage( nameToTry.c_str(), &width, &height);
  if (!image)
    image = MediaFile::readImage( init.name.c_str(), &width, &height);
  if (!image) {
    if (reportFail) {
      std::vector<std::string> args;
      args.push_back(init.name);
      printError("cannot load texture: {1}", &args);
    }
    return new OpenGLTexture;
  }

  OpenGLTexture *texture = new OpenGLTexture(width, height, image, init.filter, true);
  delete[] image;

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
