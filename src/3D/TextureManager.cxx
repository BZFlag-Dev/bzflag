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
#include "global.h"
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
  tl = engine->GetTextureList();
  if (!tl) {
    csApplicationFramework::ReportError("Failed to locate Texture List!");
    return;
  }
  format = engine->GetTextureFormat();

  // fill out the standard proc textures
  procLoader[0].name = "noise";
  procLoader[0].proc = noiseProc;

  lastImageID = -1;

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
  for (TextureNameMap::iterator it = textureNames.begin();
       it != textureNames.end(); ++it) {
    ImageInfo &tex = it->second;
    tex.material = NULL;
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
  }
  // we don't have it so try and load it
  ImageInfo info;
  info.name = name;
  info.id   = ++lastImageID;
  if (!loadTexture(info))
    return -1;
  textureNames[info.name] = info;
  textureIDs[info.id]     = &textureNames[info.name];

  return info.id;
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
  info.material = NULL;

  // clear the maps
  textureIDs.erase(info.id);
  textureNames.erase(name);

  logDebugMessage(2,"TextureManager::removed: %s\n", name.c_str());

  return true;
}


bool TextureManager::reloadTextures()
{
  TextureNameMap::iterator it = textureNames.begin();
  while (it != textureNames.end()) {
    reloadTextureImage(it->first);
    it++;
  }
  return true;
}


bool TextureManager::reloadTextureImage(const std::string &name)
{
  TextureNameMap::iterator it = textureNames.find(name);
  if (it == textureNames.end()) {
    return false;
  }
  ImageInfo &info = it->second;
  loadTexture(info);
  return true;
}


bool TextureManager::bind(int)
{
  return true;
}


bool TextureManager::bind(const char*)
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


csRef<iImage> TextureManager::loadImage(std::string filename)
{
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

  // First of all, load the image file
  csRef<iDataBuffer> data = vfs->ReadFile(filename.c_str(), false);
  if (!data || !data->GetSize()) {
    csApplicationFramework::ReportInfo("Failed to load texture %s!",
				       filename.c_str());
    return NULL;
  }
  csRef<iImage> image(imageLoader->Load(data, format));
  if (!image) {
    csApplicationFramework::ReportInfo("Unknown image file format %s!",
				       filename.c_str());
    return NULL;
  }
  csRef<iDataBuffer> xname = vfs->ExpandPath(filename.c_str());
  image->SetName(**xname);

  return image;
}

bool TextureManager::loadTexture(ImageInfo &info)
{
  std::string filename = info.name;

  csRef<iImage> image(loadImage(filename));
  if (!image)
    return false;

  char textureName[20];
  if (snprintf(textureName, sizeof(textureName), "_bzflag%i", info.id) < 0)
    return false;

  // See if a texture with the same name is already registered
  csRef<iTextureWrapper> texture(engine->FindTexture(textureName));
  csRef<iTextureHandle>  textureH;
  if (texture) {
    // This texture should have been registered here. Do a check 
    if (!info.material.IsValid())
      return false;

    // So we already have a texture Wrapper and a Texture Handle
    // Update the image, reregister and get the handle
    texture->SetImageFile(image);
    texture->Register(tm);
    textureH = texture->GetTextureHandle();
    if (!textureH)
      return false;
  } else {
    // Create a texture Handle and register the image
    textureH = tm->RegisterTexture(image, CS_TEXTURE_3D);
    if (!textureH)
      return false;

    // Create a texture Wrapper
    texture = tl->NewTexture(textureH);
    if (texture == NULL)
      return false;
  }

  // Update the info values
  info.alpha = textureH->GetAlphaMap();
  textureH->GetOriginalDimensions(info.x, info.y);

  // Give the name to the Texture Wrapper
  texture->QueryObject()->SetName(textureName);

  // Don't need to recreate material if already valid I suppose nobody
  // messed up with it, so the same texture wrapper is associate
  if (!info.material.IsValid())
    // Create default material with the same name as the texture
    info.material = engine->CreateMaterial(textureName, texture);

  return true;
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

  // if the texture already exists kill it
  // this is why IDs are way better than objects for this stuff
  TextureNameMap::iterator it = textureNames.find("noise");
  if (it != textureNames.end()) {
    logDebugMessage(3,"Texture noise already exists, overwriting\n");
    textureIDs.erase(it->second.id);
    it->second.material = NULL;
  }
  ImageInfo info;
  info.name = "noise";
  info.id   = ++lastImageID;

  textureNames[info.name] = info;
  textureIDs[info.id]     = &textureNames[info.name];
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
