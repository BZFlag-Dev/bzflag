/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* bzflag special common - 1st one */
#include "common.h"

/* interface header */
#include "LocalFontFace.h"

/* system interface headers */
#include <string>
#include <list>

/* common implementation headers */
#include "StateDatabase.h"
#include "World.h"
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"

std::list<LocalFontFace*> LocalFontFace::localFontFaces;

LocalFontFace* LocalFontFace::create(const std::string &genericFaceName)
{
  // find an existing match and reference it
  for (std::list<LocalFontFace*>::iterator itr = localFontFaces.begin();
       itr != localFontFaces.end(); ++itr) {
    if ((*itr)->getFaceName() == genericFaceName) {
      (*itr)->refs++;
      return (*itr);
    }
  }

  // no match? create a new one
  LocalFontFace* face = new LocalFontFace(genericFaceName);
  localFontFaces.push_back(face);
  return face;
}

void LocalFontFace::release(LocalFontFace *face)
{
  face->refs--;
  if (face->refs == 0) {
    localFontFaces.remove(face);
    delete face;
  }
}

LocalFontFace::LocalFontFace(const std::string &genericFaceName)
: faceName(genericFaceName), refs(1)
{
  // locale changed
  BZDB.addCallback("locale", bzdbCallback, this);
  // default font option changed
  BZDB.addCallback(faceName, bzdbCallback, this);

  // trigger the callback to initialize fmFace and
  // register the appropriate localized callback
  bzdbCallback("locale", this);
}

void LocalFontFace::bzdbCallback(const std::string &varName, void *data)
{
  if (!data) return;
  LocalFontFace* face = reinterpret_cast<LocalFontFace*>(data);
  face->localBZDBCallback(varName);
}

void LocalFontFace::localBZDBCallback(const std::string& varName)
{
  // locale changed, update localized callback
  if (varName == "locale") {
    if (localeSpecificBZDBVar.length() > 0)
      BZDB.removeCallback(localeSpecificBZDBVar, &bzdbCallback, this);

    localeSpecificBZDBVar = faceName + "_" + BZDB.get("locale");
    BZDB.addCallback(localeSpecificBZDBVar, &bzdbCallback, this);
  }

  // update cached font face
  FontManager& fm = FontManager::instance();
  // if we have an overridden font for this locale, use it
  if (BZDB.isSet(localeSpecificBZDBVar)) {
    fmFace = fm.getFaceID(BZDB.get(localeSpecificBZDBVar));
  } else {
    // otherwise, if we have a default font for this locale set in the .po, use it
    std::string poFaceQuery = "_" + faceName;
    std::string poFaceResponse = World::getBundleMgr()
      ->getBundle(BZDB.get("locale"), false)->getLocalString(poFaceQuery);
    if (poFaceResponse != poFaceQuery) {
      fmFace = fm.getFaceID(poFaceResponse);
    } else {
      // no locale-specific font, use the default one
      fmFace = fm.getFaceID(BZDB.get(faceName));
    }
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
