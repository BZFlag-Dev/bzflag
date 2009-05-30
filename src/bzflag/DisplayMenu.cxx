/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "BZDBCache.h"
#include "BzfDisplay.h"
#include "FontManager.h"
#include "SceneRenderer.h"
#include "TextureManager.h"
#include "VerticalSync.h"
#include "bzfgl.h"

/* local implementation headers */
#include "FontSizer.h"
#include "HUDDialogStack.h"
#include "HUDuiList.h"
#include "LocalFontFace.h"
#include "MainMenu.h"
#include "Roaming.h"
#include "playing.h"
#include "guiplaying.h"


DisplayMenu::DisplayMenu()
: formatMenu(NULL)
, gridOptions(debugLevel > 0)
{
  // add controls
  std::vector<std::string>* options;
  HUDuiList* option;

  // cache font face id
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Display Settings");
  addControl(label, false);

  // setup a darkened string for unavailable entries
  const std::string unavailable = ANSI_STR_FG_BLACK "Unavailable";

  // Quality
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Quality:");
  option->setCallback(callback, (void*)"Q");
  options = &option->getList();
  options->push_back(std::string("Low"));
  options->push_back(std::string("Medium"));
  options->push_back(std::string("High"));
  options->push_back(std::string("Experimental"));
  option->update();
  addControl(option);

  // Texturing
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Texturing:");
  option->setCallback(callback, (void*)"T");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Nearest"));
  options->push_back(std::string("Linear"));
  options->push_back(std::string("Nearest Mipmap Nearest"));
  options->push_back(std::string("Linear Mipmap Nearest"));
  options->push_back(std::string("Nearest Mipmap Linear"));
  options->push_back(std::string("Linear Mipmap Linear"));
  option->update();
  addControl(option);

  // Texture Remapping
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("AntiFlicker:");
  option->setCallback(callback, (void*)"R");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  // Texture Anisotropy
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Anisotropic:");
  option->setCallback(callback, (void*)"A");
  options = &option->getList();
  if (GLEW_EXT_texture_filter_anisotropic) {
    static GLint maxAnisotropy = 1;
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    if (maxAnisotropy > 1) {
      options->push_back(std::string("Off"));
      for (int i = 1; i < maxAnisotropy; i++) {
	char buffer[16];
	snprintf(buffer, 16, "%i/%i", i + 1, maxAnisotropy);
	options->push_back(std::string(buffer));
      }
    } else {
      options->push_back(unavailable);
    }
  } else {
    options->push_back(unavailable);
  }
  option->update();
  addControl(option);

  // Lighting
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Lighting:");
  option->setCallback(callback, (void*)"L");
  options = &option->getList();
  options->push_back(std::string("None"));
  options->push_back(std::string("Fast"));
  options->push_back(std::string("Best"));
  option->update();
  addControl(option);

  // Shadows
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Shadows:");
  option->setCallback(callback, (void*)"S");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Stipple"));
  if (RENDERER.useStencil()) {
    options->push_back(std::string("Stencil"));
  }
  option->update();
  addControl(option);

  // Blending
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Blending:");
  option->setCallback(callback, (void*)"B");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  // Smoothing
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Smoothing:");
  option->setCallback(callback, (void*)"s");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  // Dithering
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Dithering:");
  option->setCallback(callback, (void*)"D");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  // Brightness
  BzfWindow* window = getMainWindow()->getWindow();
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Brightness:");
  option->setCallback(callback, (void*)"b");
  if (window->hasGammaControl()) {
    option->createSlider(15);
  } else {
    options = &option->getList();
    options->push_back(unavailable);
  }
  option->update();
  addControl(option);

  // Vertical Sync
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Vertical Sync:");
  option->setCallback(callback, (void*)"V");
  options = &option->getList();
  if (!verticalSyncAvailable()) {
    options->push_back(unavailable);
  } else {
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
  }
  option->update();
  addControl(option);

  // Energy Saver
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Energy Saver:");
  option->setCallback(callback, (void*)"E");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  // Special Mode, only available to Observers
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Special Mode:");
  option->setCallback(callback, (void*)"M");
  options = &option->getList();
  options->push_back(std::string("None"));
  options->push_back(std::string("WireFrame"));
  options->push_back(std::string("HiddenLine"));
  options->push_back(std::string("DepthComplexity"));
  option->update();
  addControl(option);

  if (gridOptions) {
    // Culling Grid
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Culling Tree:");
    option->setCallback(callback, (void*)"G");
    options = &option->getList();
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
    option->update();
    addControl(option);

    // Collision Grid
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Collision Tree:");
    option->setCallback(callback, (void*)"g");
    options = &option->getList();
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
    option->update();
    addControl(option);
  }

  // Video Format
  int numFormats = display->getNumResolutions();
  if (numFormats < 2) {
    videoFormat = NULL;
  } else {
    videoFormat = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("Change Video Format");
    addControl(label);
  }

  initNavigation();
}


DisplayMenu::~DisplayMenu()
{
  delete formatMenu;
}


void DisplayMenu::execute()
{
  HUDuiControl* _focus = getNav().get();
  if (_focus == videoFormat) {
    if (!formatMenu)
      formatMenu = new FormatMenu;
    HUDDialogStack::get()->push(formatMenu);
  }
}


void DisplayMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);
  int i;

  FontManager &fm = FontManager::instance();
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace->getFMFace(), "headerFontSize");

  fs.setMin(0, 20);
  const float fontSize = fs.getFontSize(fontFace->getFMFace(), "menuFontSize");

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(fontFace->getFMFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStringHeight(fontFace->getFMFace(), titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width);
  y -= 1.0f * titleHeight;
  const float h = fm.getStringHeight(fontFace->getFMFace(), fontSize);
  const int count = (const int)listHUD.size();
  for (i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  TextureManager& tm = TextureManager::instance();

  i = 1;
  // load current settings

  // Quality
  ((HUDuiList*)listHUD[i++])->setIndex(RENDERER.useQuality());

  // Texturing
  ((HUDuiList*)listHUD[i++])->setIndex(tm.getMaxFilter());

  // TexRemap
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("remapTexCoords") ? 1 : 0);

  // Anisotropy
  int aniso = BZDB.evalInt("aniso");
  aniso = (aniso < 1) ? 1 : aniso;
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("aniso") - 1);

  // Lighting
  int lighting = 0;
  if (BZDBCache::lighting) {
    lighting = BZDBCache::tesselation ? 2 : 1;
  }
  ((HUDuiList*)listHUD[i++])->setIndex(lighting);

  // Shadows
  int shadowVal = 0;
  if (BZDBCache::shadows) {
    shadowVal = 1;
    if (RENDERER.useStencil() && BZDBCache::stencilShadows) {
      shadowVal = 2;
    }
  }
  ((HUDuiList*)listHUD[i++])->setIndex(shadowVal);

  // Blending
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("blend"));

  // Smoothing
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("smooth"));

  // Dithering
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("dither"));

  // Brightness
  BzfWindow* window = getMainWindow()->getWindow();
  if (window->hasGammaControl()) {
    ((HUDuiList*)listHUD[i])->setIndex(gammaToIndex(window->getGamma()));
  }
  i++;

  // Vertical Sync
  ((HUDuiList*)listHUD[i++])->setIndex((BZDBCache::vsync > 0) ? 1 : 0);

  // Energy Saver
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("saveEnergy"));

  // Special Mode
  const int specialModeIndex = i++;
  {
    HUDuiList* hudUI = (HUDuiList*)listHUD[specialModeIndex];
    std::vector<std::string>& hudList = hudUI->getList();
    hudList.clear();
    if (ROAM.isRoaming()) {
      hudList.push_back(std::string("None"));
      hudList.push_back(std::string("WireFrame"));
      hudList.push_back(std::string("HiddenLine"));
      hudList.push_back(std::string("DepthComplexity"));
      hudUI->setIndex(RENDERER.getSpecialMode());
      hudUI->update();
    }
    else {
      hudList.push_back(std::string(ANSI_STR_FG_BLACK "Only for Observers"));
      hudUI->setIndex(0);
      hudUI->update();
    }
  }

  // Culling Grid and Collision Grid
  if (gridOptions) {
    const int cullingGridIndex = i++;
    {
      HUDuiList* hudUI = (HUDuiList*)listHUD[cullingGridIndex];
      std::vector<std::string>& hudList = hudUI->getList();
      hudList.clear();
      if (ROAM.isRoaming()) {
        hudList.push_back(std::string("Off"));
        hudList.push_back(std::string("On"));
        hudUI->setIndex(BZDBCache::showCullingGrid ? 1 : 0);
        hudUI->update();
      }
      else {
        hudList.push_back(std::string(ANSI_STR_FG_BLACK "Only for Observers"));
        hudUI->setIndex(0);
        hudUI->update();
      }
    }

    const int collisionGridIndex = i++;
    {
      HUDuiList* hudUI = (HUDuiList*)listHUD[collisionGridIndex];
      std::vector<std::string>& hudList = hudUI->getList();
      hudList.clear();
      if (ROAM.isRoaming()) {
        hudList.push_back(std::string("Off"));
        hudList.push_back(std::string("On"));
        hudUI->setIndex(BZDBCache::showCollisionGrid ? 1 : 0);
        hudUI->update();
      }
      else {
        hudList.push_back(std::string(ANSI_STR_FG_BLACK "Only for Observers"));
        hudUI->setIndex(0);
        hudUI->update();
      }
    }
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


void DisplayMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;
  switch (((const char*)data)[0]) {
    case 'Q': {
      RENDERER.setQuality(list->getIndex());
      if (list->getIndex() > 3) {
        BZDB.set("zbuffer","1");
        setSceneDatabase();
      }
      BZDB.set("texturereplace", (!BZDBCache::lighting &&
                                   RENDERER.useQuality() < _MEDIUM_QUALITY) ? "1" : "0");
      BZDB.setPersistent("texturereplace", false);
      RENDERER.notifyStyleChange();
      break;
    }
    case 'T': {
      TextureManager& tm = TextureManager::instance();
      tm.setMaxFilter((OpenGLTexture::Filter)list->getIndex());
      BZDB.set("texture", tm.getMaxFilterName());
      RENDERER.notifyStyleChange();
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
      RENDERER.notifyStyleChange();
      break;
    }
    case 'L': {
      bool oldLighting = BZDBCache::lighting;
      BZDB.set("lighting", list->getIndex() == 0 ? "0" : "1");
      BZDB.set("tesselation", list->getIndex() == 2 ? "1" : "0");
      if (oldLighting != BZDBCache::lighting) {
        BZDB.set("texturereplace", (!BZDBCache::lighting &&
                                     RENDERER.useQuality() < _MEDIUM_QUALITY) ? "1" : "0");
        BZDB.setPersistent("texturereplace", false);
        RENDERER.notifyStyleChange();
      }
      break;
    }
    case 'S': {
      const int shadowVal = list->getIndex();
      BZDB.set("shadows", shadowVal > 0 ? "1" : "0");
      BZDB.set("stencilShadows", shadowVal > 1 ? "1" : "0");
      RENDERER.notifyStyleChange();
      break;
    }
    case 'B': {
      BZDB.set("blend", list->getIndex() ? "1" : "0");
      RENDERER.notifyStyleChange();
      break;
    }
    case 's': {
      BZDB.set("smooth", list->getIndex() ? "1" : "0");
      RENDERER.notifyStyleChange();
      break;
    }
    case 'D': {
      BZDB.set("dither", list->getIndex() ? "1" : "0");
      RENDERER.notifyStyleChange();
      break;
    }
    case 'b': {
      BzfWindow* window = getMainWindow()->getWindow();
      if (window->hasGammaControl()) {
        window->setGamma(indexToGamma(list->getIndex()));
      }
      break;
    }
    case 'V': {
      BZDB.setInt("vsync", (list->getIndex() == 0) ? -1 : +1);
      break;
    }
    case 'E': {
      BZDB.setBool("saveEnergy", list->getIndex() != 0);
      break;
    }
    case 'M': {
      RENDERER.setSpecialMode((SceneRenderer::SpecialMode)list->getIndex());
      break;
    }
    case 'G': {
      BZDB.setBool("showCullingGrid", list->getIndex() != 0);
      break;
    }
    case 'g': {
      BZDB.setBool("showCollisionGrid", list->getIndex() != 0);
      break;
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
