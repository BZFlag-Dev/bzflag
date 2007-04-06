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

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif

// BZFlag common header
#include "common.h"

// interface headers
#include "TextureManager.h"

// system headers
#include <vector>
#include <string>

// common implementation headers
#include "bzfgl.h"
#include "TextUtils.h"
#include "global.h"
#include "ErrorHandler.h"
#include "OSFile.h"
#include "CacheManager.h"


/*const int NO_VARIANT = (-1); */

// initialize the singleton
template <>
TextureManager* Singleton<TextureManager>::_instance = (TextureManager*)0;

static int noiseProc(ProcTextureInit &init);

ProcTextureInit procLoader[1];

TextureManager::TextureManager()
{
  vfs = csQueryRegistry<iVFS> (csApplicationFramework::GetObjectRegistry());
  if (!vfs) {
    csApplicationFramework::ReportError
      ("Failed to locate Virtual File System!");
    return;
  }
  imageLoader = csQueryRegistry<iImageIO>
    (csApplicationFramework::GetObjectRegistry());
  if (!imageLoader) {
    csApplicationFramework::ReportError("Failed to locate Image Loader!");
    return;
  }
  engine = csQueryRegistry<iEngine>
    (csApplicationFramework::GetObjectRegistry());
  if (!engine) {
    csApplicationFramework::ReportError("Failed to locate Engine!");
    return;
  }
  g3d = csQueryRegistry<iGraphics3D>
    (csApplicationFramework::GetObjectRegistry());
  if (!g3d) {
    csApplicationFramework::ReportError("Failed to locate 3D driver!");
    return;
  }
  tm = g3d->GetTextureManager();
  if (!tm) {
    csApplicationFramework::ReportError("Failed to locate Texture Manager!");
    return;
  }
  format = engine->GetTextureFormat();

  // fill out the standard proc textures
  procLoader[0].name = "noise";
  procLoader[0].proc = noiseProc;

  lastImageID = -1;
  lastBoundID = -1;

  int i, numTextures;
  numTextures = countof(procLoader);

  for (i = 0; i < numTextures; i++) {
    procLoader[i].manager = this;
    procLoader[i].proc(procLoader[i]);
  }
}

TextureManager::~TextureManager()
{
  // we are done remove all textures
  for (TextureNameMap::iterator it = textureNames.begin(); it != textureNames.end(); ++it) {
    ImageInfo &tex = it->second;
    if (tex.texture != NULL) {
      delete tex.texture;
    }
  }
  textureNames.clear();
  textureIDs.clear();
}

int TextureManager::getTextureID( const char* name, bool)
{
  if (!name) {
    logDebugMessage(2,"Could not get texture ID; no provided name\n");
    return -1;
  }

  // see if we have the texture
  TextureNameMap::iterator it = textureNames.find(name);
  if (it != textureNames.end()) {
    return it->second.id;
  } else { // we don't have it so try and load it

    OSFile osFilename(name); // convert to native format
    const std::string filename = osFilename.getOSName();

    FileTextureInit texInfo;
    texInfo.name = filename;

    csRef<iTextureHandle> image(loadTexture(texInfo));
    if (!image) {
      return -1;
    }
    return addTexture(name, image);
  }
  return -1;
}


bool TextureManager::isLoaded(const std::string& name)
{
  TextureNameMap::iterator it = textureNames.find(name);
  if (it == textureNames.end()) {
    return false;
  }
  return true;
}


bool TextureManager::removeTexture(const std::string& name)
{
  TextureNameMap::iterator it = textureNames.find(name);
  if (it == textureNames.end()) {
    return false;
  }

  ImageInfo& info = it->second;
  delete info.texture;
  info.texture = NULL;

  // clear the maps
  textureIDs.erase(info.id);
  textureNames.erase(name);

  logDebugMessage(2,"TextureManager::removed: %s\n", name.c_str());

  return true;
}


bool TextureManager::reloadTextures()
{
  return true;
}


bool TextureManager::reloadTextureImage(const std::string&)
{
  return true;
}


bool TextureManager::bind ( int id )
{
  return true;
}


bool TextureManager::bind ( const char* name )
{
  return true;
}


float TextureManager::GetAspectRatio ( int id )
{
  TextureIDMap::iterator it = textureIDs.find(id);
  if (it == textureIDs.end())
    return 0.0;

  return (float)it->second->y/(float)it->second->x;
}

const ImageInfo& TextureManager::getInfo ( int id )
{
 static ImageInfo   crapInfo;
  crapInfo.id = -1;
  TextureIDMap::iterator it = textureIDs.find(id);
  if (it == textureIDs.end())
    return crapInfo;

  return *(it->second);
}
const ImageInfo& TextureManager::getInfo ( const char* name )
{
  static ImageInfo crapInfo;
  crapInfo.id = -1;
  std::string nameStr = name;

  TextureNameMap::iterator it = textureNames.find(nameStr);
  if (it == textureNames.end())
    return crapInfo;

  return it->second;
}


int TextureManager::addTexture(const char *name, csRef<iTextureHandle> texture)
{
  if (!name || !texture)
    return -1;

  // if the texture already exists kill it
  // this is why IDs are way better than objects for this stuff
  TextureNameMap::iterator it = textureNames.find(name);
  if (it != textureNames.end()) {
   logDebugMessage(3,"Texture %s already exists, overwriting\n", name);
   textureIDs.erase(it->second.id);
   delete it->second.texture;
  }
  ImageInfo info;
  info.name = name;
  info.texture = texture;
  info.id = ++lastImageID;
  info.alpha = texture->GetAlphaMap();
  texture->GetOriginalDimensions(info.x, info.y);

  textureNames[name] = info;
  textureIDs[info.id] = &textureNames[name];

  logDebugMessage(4,"Added texture %s: id %d\n", name, info.id);

  return info.id;
}

csRef<iTextureHandle> TextureManager::loadTexture(FileTextureInit &init)
{
  std::string filename = init.name;

  // get the absolute filename for cache textures
  if (CACHEMGR.isCacheFileType(filename)) {
    filename = CACHEMGR.getLocalName(filename);
  }

  if (vfs->Exists(filename.c_str())) {
    ;
  } else if (vfs->Exists((filename + ".png").c_str())) {
    filename += ".png";
  } else if (vfs->Exists((filename + ".rgb").c_str())) {
    filename += ".rgb";
  }
  if (!vfs->Exists(filename.c_str())) {
    csApplicationFramework::ReportInfo("Failed to locate file %s!",
				       filename.c_str());
    return NULL;
  }
  csRef<iDataBuffer> buf = vfs->ReadFile(filename.c_str(), false);
  if (!buf.IsValid()) {
    csApplicationFramework::ReportInfo("Failed to load texture %s!",
				       filename.c_str());
    return NULL;
  }

  csRef<iImage> image(imageLoader->Load(buf, format));
  if (!image) {
    csApplicationFramework::ReportInfo("Failed to load texture %s!",
				       filename.c_str());
    return NULL;
  }
  csRef<iDataBuffer> xname = vfs->ExpandPath(filename.c_str());
  image->SetName(**xname);

  csRef<iTextureHandle> texture(tm->RegisterTexture(image, CS_TEXTURE_3D));

  return texture;
}


int TextureManager::newTexture()
{
  int noizeSize = 128;
  const int size = 4 * noizeSize * noizeSize;
  unsigned char* noise = new unsigned char[size];
  for (int i = 0; i < size; i += 4 ) {
    unsigned char n = (unsigned char)floor(256.0 * bzfrand());
    noise[i+0] = n;
    noise[i+1] = n;
    noise[i+2] = n;
    noise[i+3] = n;
  }
//   csRef<iTextureHandle> texture(tm->CreateTexture(noizeSize, noizeSize, csimg2D,
// 						  NULL, CS_TEXTURE_3D));
  delete[] noise;
//   int result = addTexture("noise", texture);
  return -1;
}


/* --- Procs --- */

int noiseProc(ProcTextureInit &init)
{
  return init.manager->newTexture();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
