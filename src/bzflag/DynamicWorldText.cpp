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

#include "common.h"

/* interface header */
#include "DynamicWorldText.h"

/* system headers */
#include <string>

/* common headers */
#include "WorldText.h"
#include "TextSceneNode.h"
#include "SceneRenderer.h"
#include "CacheManager.h"

/* local headers */
#include "World.h"


DynamicWorldText DynamicWorldText::bzdbWorldText;


//============================================================================//


DynamicWorldText::DynamicWorldText()
  : needStyleChange(true) {
}


DynamicWorldText::~DynamicWorldText() {
  clear();
}


//============================================================================//

void DynamicWorldText::clear() {
  AddedMap::iterator ait;
  for (ait = addedTexts.begin(); ait != addedTexts.end(); ++ait) {
    WorldText* text = ait->second;
    delete text;
  }

  NodeMap::iterator  nit;
  for (nit = nodes.begin(); nit != nodes.end(); ++nit) {
    TextSceneNode* node = nit->second;
    delete node;
  }

  worldTexts.clear();
  addedTexts.clear();
  nodes.clear();
}


//============================================================================//

void DynamicWorldText::addRenderNodes(SceneRenderer& renderer) {
  const ViewFrustum& vf = renderer.getViewFrustum();
  NodeMap::const_iterator it;
  if (!needStyleChange) {
    for (it = nodes.begin(); it != nodes.end(); ++it) {
      TextSceneNode* node = it->second;
      if (!node->cull(vf)) {
        node->addRenderNodes(renderer);
      }
    }
  }
  else {
    for (it = nodes.begin(); it != nodes.end(); ++it) {
      TextSceneNode* node = it->second;
      node->notifyStyleChange();
      if (!node->cull(vf)) {
        node->addRenderNodes(renderer);
      }
    }
    needStyleChange = false;
  }
}


void DynamicWorldText::addShadowNodes(SceneRenderer& renderer) {
  NodeMap::const_iterator it;

  const fvec4* planes = NULL;
  const int planeCount = renderer.getShadowPlanes(&planes);

  if (planeCount <= 0) {
    for (it = nodes.begin(); it != nodes.end(); ++it) {
      TextSceneNode* node = it->second;
      node->addShadowNodes(renderer);
    }
  }
  else {
    for (it = nodes.begin(); it != nodes.end(); ++it) {
      TextSceneNode* node = it->second;
      if (!node->cullShadow(planeCount, planes)) {
        node->addShadowNodes(renderer);
      }
    }
  }
}


void DynamicWorldText::renderRadar() {
  NodeMap::const_iterator it;
  for (it = nodes.begin(); it != nodes.end(); ++it) {
    TextSceneNode* node = it->second;
    node->renderRadar();
  }
}


//============================================================================//

void DynamicWorldText::addText(const WorldText* text) {
  worldTexts.insert(text);
  nodes[text] = new TextSceneNode(text);
}


bool DynamicWorldText::insertText(WorldText* text) {
  if (World::getWorld() == NULL) {
    return false;
  }
  if (addedTexts.find(text->name) != addedTexts.end()) {
    return false;
  }
  addedTexts[text->name] = text;
  nodes[text] = new TextSceneNode(text);
  return true;
}


bool DynamicWorldText::removeText(const std::string& name) {
  AddedMap::iterator textIT = addedTexts.find(name);
  if (textIT == addedTexts.end()) {
    return false;
  }
  WorldText* text = textIT->second;
  NodeMap::iterator nodeIT = nodes.find(text);
  if (nodeIT != nodes.end()) {
    TextSceneNode* node = nodeIT->second;
    delete node;
    nodes.erase(nodeIT);
  }
  delete text;
  addedTexts.erase(textIT);
  return true;
}


void DynamicWorldText::notifyStyleChange() {
  needStyleChange = true;
}


void DynamicWorldText::getFontURLs(std::set<std::string>& fontURLs) const {
  AddedMap::const_iterator ait;
  for (ait = addedTexts.begin(); ait != addedTexts.end(); ait++) {
    const WorldText* text = ait->second;
    if (CacheManager::isCacheFileType(text->font)) {
      fontURLs.insert(text->font);
    }
  }
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
