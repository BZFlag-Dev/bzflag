/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#include "DisplayMenu.h"

/* common implementation headers */
#include "FontManager.h"
#include "BZDBCache.h"
#include "TextureManager.h"
#include "OpenGLUtils.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "HUDuiList.h"
#include "playing.h"
#include "HUDui.h"

DisplayMenu::DisplayMenu() : formatMenu(NULL)
{
  // add controls
  std::vector<std::string>* options;
  std::vector<HUDuiControl*>& listHUD  = getControls();
  HUDuiList* option;

  // cache font face id
  int fontFace = MainMenu::getFontFace();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Display Settings");
  listHUD.push_back(label);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Dithering:");
  option->setCallback(callback, "1");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Blending:");
  option->setCallback(callback, "2");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Smoothing:");
  option->setCallback(callback, "3");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Lighting:");
  option->setCallback(callback, "4");
  options = &option->getList();
  options->push_back(std::string("None"));
  options->push_back(std::string("Fast"));
  options->push_back(std::string("Best"));
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Texturing:");
  option->setCallback(callback, "5");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Nearest"));
  options->push_back(std::string("Linear"));
  options->push_back(std::string("Nearest Mipmap Nearest"));
  options->push_back(std::string("Linear Mipmap Nearest"));
  options->push_back(std::string("Nearest Mipmap Linear"));
  options->push_back(std::string("Linear Mipmap Linear"));
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("AntiFlicker:");
  option->setCallback(callback, "R");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  if (OpenGLGState::hasAnisotropicFiltering) {
    static GLint maxAnisotropy = 1;
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    if (maxAnisotropy > 1) {
      option = new HUDuiList;
      option->setFontFace(fontFace);
      option->setLabel("Anisotropic:");
      option->setCallback(callback, "A");
      options = &option->getList();
      options->push_back(std::string("Off"));
      for (int i = 1; i < maxAnisotropy; i++) {
	char buffer[16];
	snprintf(buffer, 16, "%i/%i", i + 1, maxAnisotropy);
	options->push_back(std::string(buffer));
      }
      option->update();
      listHUD.push_back(option);
    }
  }

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Quality:");
  option->setCallback(callback, "6");
  options = &option->getList();
  options->push_back(std::string("Low"));
  options->push_back(std::string("Medium"));
  options->push_back(std::string("High"));
  options->push_back(std::string("Experimental"));
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Shadows:");
  option->setCallback(callback, "7");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Stipple"));
  options->push_back(std::string("Stencil"));
  option->update();
  listHUD.push_back(option);

#if defined(DEBUG_RENDERING)
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Hidden Line:");
    option->setCallback(callback, "a");
    options = &option->getList();
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
    option->update();
    listHUD.push_back(option);

    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Wireframe:");
    option->setCallback(callback, "b");
    options = &option->getList();
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
    option->update();
    listHUD.push_back(option);

    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Depth Complexity:");
    option->setCallback(callback, "c");
    options = &option->getList();
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
    option->update();
    listHUD.push_back(option);

    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Culling Tree:");
    option->setCallback(callback, "d");
    options = &option->getList();
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
    option->update();
    listHUD.push_back(option);

    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Collision Tree:");
    option->setCallback(callback, "e");
    options = &option->getList();
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
    option->update();
    listHUD.push_back(option);
#endif

  if (((BzfWindow*) getMainWindow()->getWindow())->hasGammaControl()) {
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Brightness:");
    option->setCallback(callback, "g");
    option->createSlider(15);
    option->update();
    listHUD.push_back(option);
  }

  option = new HUDuiList;
  option->setFontFace(fontFace);

#if (defined(HAVE_SDL) && !defined(HAVE_SDL2))	// only SDL 2 can make live changes
    option->setLabel("(restart required) Energy Saver:");
#else
    option->setLabel("Energy Saver:");
#endif
  option->setCallback(callback, "s");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("FPS Limit"));
  if (getMainWindow()->getWindow()->hasVerticalSync()) {
    options->push_back(std::string("Vertical Sync"));
  }
  option->update();
  listHUD.push_back(option);

  if (OpenGLGState::getMaxSamples() > 1) {
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Anti Aliasing:");
    option->setCallback(callback, "m");
    options = &option->getList();
    options->push_back(std::string("Off"));
    for (int i = 2; i <= OpenGLGState::getMaxSamples(); ++i) {
      char msaaText[11];
      snprintf(msaaText, 11, "%i", i);
      options->push_back(std::string(msaaText) + "x MSAA");
    }
    option->update();
    listHUD.push_back(option);
  }

  BzfDisplay* display = getDisplay();
  int numFormats = display->getNumResolutions();
  if (numFormats < 2) {
    videoFormat = NULL;
  } else {
    videoFormat = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("Change Video Format");
    listHUD.push_back(label);
  }

  initNavigation(listHUD, 1,listHUD.size()-1);
}

DisplayMenu::~DisplayMenu()
{
  delete formatMenu;
}

void			DisplayMenu::execute()
{
  HUDuiControl* _focus = HUDui::getFocus();
  if (_focus == videoFormat) {
    if (!formatMenu)
      formatMenu = new FormatMenu;
    HUDDialogStack::get()->push(formatMenu);
  }
}

void			DisplayMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  int i;

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  const float fontSize = (float)_height / 45.0f;
  FontManager &fm = FontManager::instance();
  int fontFace = MainMenu::getFontFace();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(fontFace, titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(fontFace, titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width);
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(fontFace, fontSize, " ");
  const int count = listHUD.size();
  for (i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  i = 1;
  // load current settings
  SceneRenderer* renderer = getSceneRenderer();
  if (renderer) {
    TextureManager& tm = TextureManager::instance();
    ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("dither"));
    ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("blend"));
    ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("smooth"));
    if (BZDBCache::lighting) {
      if (BZDB.isTrue("tesselation")) {
	((HUDuiList*)listHUD[i++])->setIndex(2);
      } else {
	((HUDuiList*)listHUD[i++])->setIndex(1);
      }
    } else {
      ((HUDuiList*)listHUD[i++])->setIndex(0);
    }
    ((HUDuiList*)listHUD[i++])->setIndex(tm.getMaxFilter());
    ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("remapTexCoords") ? 1 : 0);
    if (OpenGLGState::hasAnisotropicFiltering) {
      static GLint maxAnisotropy = 1;
      glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
      if (maxAnisotropy > 1) {
	int aniso = BZDB.evalInt("aniso");
	aniso = (aniso < 1) ? 1 : aniso;
	((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("aniso") - 1);
      }
    }
    ((HUDuiList*)listHUD[i++])->setIndex(renderer->useQuality());
    int shadowVal = 0;
    if (BZDBCache::shadows) {
      shadowVal++;
      if (BZDBCache::stencilShadows) {
	shadowVal++;
      }
    }
    ((HUDuiList*)listHUD[i++])->setIndex(shadowVal);
#if defined(DEBUG_RENDERING)
      ((HUDuiList*)listHUD[i++])->setIndex(renderer->useHiddenLine() ? 1 : 0);
      ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("wireframe") ? 1 : 0);
      ((HUDuiList*)listHUD[i++])->setIndex(renderer->useDepthComplexity() ? 1
					   : 0);
      ((HUDuiList*)listHUD[i++])->setIndex(BZDBCache::showCullingGrid ? 1 : 0);
      ((HUDuiList*)listHUD[i++])->setIndex(BZDBCache::showCollisionGrid ? 1
					   : 0);
#endif
  }

  // brightness
  BzfWindow* window = getMainWindow()->getWindow();
  if (window->hasGammaControl())
    ((HUDuiList*)listHUD[i++])->setIndex(gammaToIndex(window->getGamma()));

  // energy saver
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("saveEnergy"));

  if (OpenGLGState::getMaxSamples() > 1) {
    // multisampling
    ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("multisample") - 1);
  }
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

void			DisplayMenu::callback(HUDuiControl* w, const void* data) {
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
      BZDB.set("texturereplace", (!BZDBCache::lighting &&
				   sceneRenderer->useQuality() < 2) ? "1" : "0");
      BZDB.setPersistent("texturereplace", false);
      sceneRenderer->notifyStyleChange();
    }
    break;
  }
  case '5': {
    TextureManager& tm = TextureManager::instance();
    tm.setMaxFilter((OpenGLTexture::Filter)list->getIndex());
    BZDB.set("texture", tm.getMaxFilterName());
    sceneRenderer->notifyStyleChange();
    break;
  }
  case 'R': {
    BZDB.setBool("remapTexCoords", list->getIndex() == 1);
    setSceneDatabase();
    break;
  }
  case 'A': {
    int aniso = list->getIndex() + 1;
    BZDB.setInt("aniso", aniso);
    TextureManager& tm = TextureManager::instance();
    tm.setMaxFilter(tm.getMaxFilter());
    sceneRenderer->notifyStyleChange();
    break;
  }
  case '6':
    sceneRenderer->setQuality(list->getIndex());
    if (list->getIndex() > 3) {
      BZDB.set("zbuffer","1");
      setSceneDatabase();
    }
    BZDB.set("texturereplace", (!BZDBCache::lighting &&
				 sceneRenderer->useQuality() < 2) ? "1" : "0");
    BZDB.setPersistent("texturereplace", false);
    sceneRenderer->notifyStyleChange();
    break;
  case '7': {
    const int shadowVal = list->getIndex();
    BZDB.set("shadows", shadowVal > 0 ? "1" : "0");
    BZDB.set("stencilShadows", shadowVal > 1 ? "1" : "0");
    sceneRenderer->notifyStyleChange();
    break;
  }
  case 'a':
    sceneRenderer->setHiddenLine(list->getIndex() != 0);
    break;
  case 'b':
    BZDB.setBool("wireframe", (list->getIndex() != 0));
    break;
  case 'c':
    sceneRenderer->setDepthComplexity(list->getIndex() != 0);
    break;
  case 'd':
    BZDB.setBool("showCullingGrid", list->getIndex() != 0);
    break;
  case 'e':
    BZDB.setBool("showCollisionGrid", list->getIndex() != 0);
    break;
  case 's':
    BZDB.setInt("saveEnergy", list->getIndex());
    getMainWindow()->getWindow()->setVerticalSync(list->getIndex() == 2);
    break;
  case 'm':
    BZDB.setInt("multisample", list->getIndex() + 1);
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
