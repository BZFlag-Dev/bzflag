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

/* interface header */
#include "DisplayMenu.h"
#include "TextureManager.h"

/* system implementation headers */
#include <string>
#include <vector>
#include <math.h>

/* common implementation headers */
#include "BzfDisplay.h"
#include "SceneRenderer.h"
#include "FontManager.h"
#include "OpenGLTexture.h"
#include "StateDatabase.h"
#include "BZDBCache.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "HUDuiControl.h"
#include "HUDuiList.h"
#include "HUDuiLabel.h"
#include "MainWindow.h"

/* FIXME - from playing.h */
BzfDisplay* getDisplay();
MainWindow* getMainWindow();
SceneRenderer* getSceneRenderer();
void setSceneDatabase();

DisplayMenu::DisplayMenu() : formatMenu(NULL)
{
  // add controls
  std::vector<std::string>* options;
  std::vector<HUDuiControl*>& list  = getControls();
  HUDuiList* option;

  // cache font face id
  int fontFace = MainMenu::getFontFace();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Display Settings");
  list.push_back(label);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Dithering:");
  option->setCallback(callback, (void*)"1");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Blending:");
  option->setCallback(callback, (void*)"2");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Smoothing:");
  option->setCallback(callback, (void*)"3");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Lighting:");
  option->setCallback(callback, (void*)"4");
  options = &option->getList();
  options->push_back(std::string("None"));
  options->push_back(std::string("Fast"));
  options->push_back(std::string("Best"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Texturing:");
  option->setCallback(callback, (void*)"5");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Nearest"));
  options->push_back(std::string("Linear"));
  options->push_back(std::string("Nearest Mipmap Nearest"));
  options->push_back(std::string("Linear Mipmap Nearest"));
  options->push_back(std::string("Nearest Mipmap Linear"));
  options->push_back(std::string("Linear Mipmap Linear"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Quality:");
  option->setCallback(callback, (void*)"6");
  options = &option->getList();
  options->push_back(std::string("Low"));
  options->push_back(std::string("Medium"));
  options->push_back(std::string("High"));
  options->push_back(std::string("Experimental"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Shadows:");
  option->setCallback(callback, (void*)"7");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

#if defined(DEBUG_RENDERING)
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Hidden Line:");
  option->setCallback(callback, (void*)"a");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Wireframe:");
  option->setCallback(callback, (void*)"b");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Depth Complexity:");
  option->setCallback(callback, (void*)"c");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Culling Tree:");
  option->setCallback(callback, (void*)"d");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Collision Tree:");
  option->setCallback(callback, (void*)"e");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);
#endif

  BzfWindow* window = getMainWindow()->getWindow();
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Brightness:");
  option->setCallback(callback, (void*)"g");
  if (window->hasGammaControl()) {
    option->createSlider(15);
  } else {
    options = &option->getList();
    options->push_back(std::string("Unavailable"));
  }
  option->update();
  list.push_back(option);

  BzfDisplay* display = getDisplay();
  int numFormats = display->getNumResolutions();
  if (numFormats < 2) {
    videoFormat = NULL;
  } else {
    videoFormat = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("Change Video Format");
    list.push_back(label);
  }

  initNavigation(list, 1,list.size()-1);
}

DisplayMenu::~DisplayMenu()
{
  delete formatMenu;
}

void			DisplayMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == videoFormat) {
    if (!formatMenu)
      formatMenu = new FormatMenu;
    HUDDialogStack::get()->push(formatMenu);
  }
}

void			DisplayMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);
  int i;

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)height / 15.0f;
  const float fontSize = (float)height / 45.0f;
  FontManager &fm = FontManager::instance();
  int fontFace = MainMenu::getFontFace();

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(fontFace, titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(fontFace, titleFontSize, " ");
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width);
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(fontFace, fontSize, " ");
  const int count = list.size();
  for (i = 1; i < count; i++) {
    list[i]->setFontSize(fontSize);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  i = 1;
  // load current settings
  SceneRenderer* renderer = getSceneRenderer();
  if (renderer) {
    HUDuiList* tex;
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("dither"));
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("blend"));
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("smooth"));
    if (BZDBCache::lighting) {
      if (BZDB.isTrue("tesselation")) {
	((HUDuiList*)list[i++])->setIndex(2);
      } else {
	((HUDuiList*)list[i++])->setIndex(1);
      }
    } else {
      ((HUDuiList*)list[i++])->setIndex(0);
    }
    tex = (HUDuiList*)list[i++];
    ((HUDuiList*)list[i++])->setIndex(renderer->useQuality());
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("shadows"));
#if defined(DEBUG_RENDERING)
    ((HUDuiList*)list[i++])->setIndex(renderer->useHiddenLine() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useWireframe() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useDepthComplexity() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useCullingTree() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useCollisionTree() ? 1 : 0);
#endif

    if (!BZDB.isTrue("texture"))
      tex->setIndex(0);
    else
      tex->setIndex(OpenGLTexture::getFilter());
  }

  // brightness
  BzfWindow* window = getMainWindow()->getWindow();
  if (window->hasGammaControl())
    ((HUDuiList*)list[i])->setIndex(gammaToIndex(window->getGamma()));
  i++;

}

int DisplayMenu::gammaToIndex(float gamma)
{
  return (int)(0.5f + 5.0f * (1.0f + logf(gamma) / logf(2.0)));
}

float DisplayMenu::indexToGamma(int index)
{
  // map index 5 to gamma 1.0 and index 0 to gamma 0.5
  return powf(2.0f, (float)index / 5.0f - 1.0f);
}

void			DisplayMenu::callback(HUDuiControl* w, void* data) {
  HUDuiList* list = (HUDuiList*)w;
  SceneRenderer* sceneRenderer = getSceneRenderer();
  switch (((const char*)data)[0]) {
  case '1':
    BZDB.set("dither", list->getIndex() ? "1" : "0");
    sceneRenderer->notifyStyleChange();
    break;
  case '2':
    BZDB.set("blend", list->getIndex() ? "1" : "0");
    sceneRenderer->notifyStyleChange();
    break;
  case '3':
    BZDB.set("smooth", list->getIndex() ? "1" : "0");
    sceneRenderer->notifyStyleChange();
    break;
  case '4': {
    bool oldLighting = BZDBCache::lighting;
    BZDB.set("lighting", list->getIndex() == 0 ? "0" : "1");
    BZDB.set("tesselation", list->getIndex() == 2 ? "1" : "0");
    if (oldLighting != BZDBCache::lighting) {
      BZDB.set("_texturereplace", (!BZDBCache::lighting &&
				   sceneRenderer->useQuality() < 2) ? "1" : "0");
      BZDB.setPersistent("_texturereplace", false);
      sceneRenderer->notifyStyleChange();
    }
    break;
  }
  case '5':
    TextureManager::instance().setMaxFilter((eTextureFilter)list->getIndex());
    BZDB.set("texture", TextureManager::instance().getMaxFilterName());
    sceneRenderer->notifyStyleChange();
    break;
  case '6':
    sceneRenderer->setQuality(list->getIndex());
	if (list->getIndex() > 3)
	{
		BZDB.set("zbuffer","1");
		setSceneDatabase();
	}
    BZDB.set("_texturereplace", (!BZDBCache::lighting &&
				 sceneRenderer->useQuality() < 2) ? "1" : "0");
    BZDB.setPersistent("_texturereplace", false);
    sceneRenderer->notifyStyleChange();
    break;
  case '7':
    BZDB.set("shadows", list->getIndex() ? "1" : "0");
    sceneRenderer->notifyStyleChange();
    break;
#if defined(DEBUG_RENDERING)
  case 'a':
    sceneRenderer->setHiddenLine(list->getIndex() != 0);
    break;
  case 'b':
    sceneRenderer->setWireframe(list->getIndex() != 0);
    break;
  case 'c':
    sceneRenderer->setDepthComplexity(list->getIndex() != 0);
    break;
#endif
  case 'd':
    sceneRenderer->setCullingTree(list->getIndex() != 0);
    break;
  case 'e':
    sceneRenderer->setCollisionTree(list->getIndex() != 0);
    break;
  case 'g':
    BzfWindow* window = getMainWindow()->getWindow();
    if (window->hasGammaControl())
      window->setGamma(indexToGamma(list->getIndex()));
    break;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
