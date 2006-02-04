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

/* interface header */
#include "EffectsMenu.h"

/* common implementation headers */
#include "TextUtils.h"
#include "FontManager.h"
#include "StateDatabase.h"

/* local implementation headers */
#include "MainMenu.h"
#include "TrackMarks.h"
#include "HUDuiList.h"
#include "HUDuiLabel.h"
#include "effectsRenderer.h"


EffectsMenu::EffectsMenu()
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();

  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // the menu label
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Effects Settings");
  listHUD.push_back(label);

  // the menu options
  HUDuiList* option;
  std::vector<std::string>* options;

  // Rain Scale
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Rain:");
  option->setCallback(callback, (void*)"r");
  options = &option->getList();
  options->push_back(std::string("Off"));
  option->createSlider(10);
  option->update();
  listHUD.push_back(option);

  // The Mirror
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Mirror:");
  option->setCallback(callback, (void*)"m");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  // Fog Effect
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Fog:");
  option->setCallback(callback, (void*)"F");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Fast"));
  options->push_back(std::string("Nice"));
  option->update();
  listHUD.push_back(option);

  // Display Treads
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Display Treads:");
  option->setCallback(callback, (void*)"T");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  // Animated Treads
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Animated Treads:");
  option->setCallback(callback, (void*)"a");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Covered"));
  options->push_back(std::string("Exposed"));
  option->update();
  listHUD.push_back(option);

  // Track Mark Fading Scale
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Track Marks:");
  option->setCallback(callback, (void*)"t");
  options = &option->getList();
  options->push_back(std::string("Off"));
  option->createSlider(10);
  option->update();
  listHUD.push_back(option);

  // Track Mark Culling Type
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Track Mark Culling:");
  option->setCallback(callback, (void*)"c");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Fast"));
  options->push_back(std::string("Best"));
  option->update();
  listHUD.push_back(option);

  // Fancy effects from effectsRenderer
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Fancy Effects:");
  option->setCallback(callback, (void*)"f");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  std::vector<std::string> optbuf;

  // Fancy effects I: Spawn
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Spawn Effect:");
  option->setCallback(callback, (void*)"s");
  options = &option->getList();
  optbuf = EFFECTS.getSpawnEffectTypes();
  options->assign(optbuf.begin(), optbuf.end());
  option->update();
  listHUD.push_back(option);

  // Fancy effects Ia: Local spawn
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Local Spawn Effect:");
  option->setCallback(callback, (void*)"L");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Same as spawns"));
  option->update();
  listHUD.push_back(option);

  // Fancy effects II: Death
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Death Effect:");
  option->setCallback(callback, (void*)"d");
  options = &option->getList();
  optbuf = EFFECTS.getDeathEffectTypes();
  options->assign(optbuf.begin(), optbuf.end());
  option->update();
  listHUD.push_back(option);

  // Fancy effects III: Shots
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Shot Fired Effect:");
  option->setCallback(callback, (void*)"S");
  options = &option->getList();
  optbuf = EFFECTS.getShotEffectTypes();
  options->assign(optbuf.begin(), optbuf.end());
  option->update();
  listHUD.push_back(option);

  // Fancy effects IV: Local shots
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Local Shot Effect:");
  option->setCallback(callback, (void*)"l");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Same as shots"));
  option->update();
  listHUD.push_back(option);

  // Fancy effects V: Velocity for shot effects
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Use Velocity on Shot Effects:");
  option->setCallback(callback, (void*)"V");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  // Fancy effects VI: Landing effects
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Landing Effect:");
  option->setCallback(callback, (void*)"b");
  options = &option->getList();
  optbuf = EFFECTS.getLandEffectTypes();
  options->assign(optbuf.begin(), optbuf.end());
  option->update();
  listHUD.push_back(option);

  // Fancy effects VII: GM Smoke Trail effects
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("GM Smoke Effect:");
  option->setCallback(callback, (void*)"G");
  options = &option->getList();
  optbuf = EFFECTS.getGMPuffEffectTypes();
  options->assign(optbuf.begin(), optbuf.end());
  option->update();
  listHUD.push_back(option);

  // Fancy effects VIIa: GM puff timing
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("GM Effect Timing:");
  option->setCallback(callback, (void*)"g");
  option->createSlider(11);
  option->update();
  listHUD.push_back(option);

  // Fancy effects VIII: rico effects
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("  Ricochet Effect:");
  option->setCallback(callback, (void*)"R");
  options = &option->getList();
  optbuf = EFFECTS.getRicoEffectTypes();
  options->assign(optbuf.begin(), optbuf.end());
  option->update();
  listHUD.push_back(option);

  // Fancy effects IX: shot Teleporter effects
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("  Teleport Shot Effect:");
  option->setCallback(callback, (void*)"7");
  options = &option->getList();
  optbuf = EFFECTS.getShotTeleportEffectTypes();
  options->assign(optbuf.begin(), optbuf.end());
  option->update();
  listHUD.push_back(option);

  initNavigation(listHUD, 1, listHUD.size() - 1);
}


EffectsMenu::~EffectsMenu()
{
}


void EffectsMenu::execute()
{
}


void EffectsMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  const float fontSize = (float)_height / 65.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth =
    fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight =
    fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * (float)_width;
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  const int count = listHUD.size();
  int i;
  for (i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    if ((i == 3) || (i == 5) || (i == 7)) {
      y -= 1.75f * h;
    } else {
      y -= 1.0f * h;
    }
  }

  // load current settings
  i = 1;
  ((HUDuiList*)listHUD[i++])->setIndex(int((BZDB.eval("userRainScale")
					    * 10.0f) + 0.5f));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("userMirror") ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("fogEffect"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("showTreads") ? 1 : 0);
  int treadIndex = 0;
  if (BZDB.isTrue("animatedTreads")) {
    treadIndex++;
    if (BZDB.isTrue("treadStyle")) {
      treadIndex++;
    }
  }
  ((HUDuiList*)listHUD[i++])->setIndex(treadIndex);
  ((HUDuiList*)listHUD[i++])->setIndex(int((TrackMarks::getUserFade() * 10.0f)
					   + 0.5f));
  TrackMarks::AirCullStyle style = TrackMarks::getAirCulling();
  if (style == TrackMarks::NoAirCull) {
    ((HUDuiList*)listHUD[i++])->setIndex(0);
  } else if (style != TrackMarks::FullAirCull) {
    ((HUDuiList*)listHUD[i++])->setIndex(1);
  } else {
    ((HUDuiList*)listHUD[i++])->setIndex(2);
  }
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("useFancyEffects") ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("spawnEffect"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("enableLocalSpawnEffect") ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("deathEffect"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("shotEffect"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("enableLocalShotEffect") ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("useVelOnShotEffects") ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("landEffect"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("gmPuffEffect"));
  ((HUDuiList*)listHUD[i++])->setIndex(static_cast<int>(1 / BZDB.eval("gmPuffTime") - 3));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("ricoEffect"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("tpEffect"));
}


void EffectsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  switch (((const char*)data)[0]) {
    case 'r': {
      int scale = list->getIndex();
      BZDB.setFloat("userRainScale", float(scale) / 10.0f);
      break;
    }
    case 'm': {
      BZDB.set("userMirror", list->getIndex() ? "1" : "0");
      break;
    }
    case 'F': {
      BZDB.setInt("fogEffect", list->getIndex());
      break;
    }
    case 'T': {
      BZDB.set("showTreads", list->getIndex() ? "1" : "0");
      break;
    }
    case 'a': {
      switch (list->getIndex()) {
	case 1:
	  BZDB.set("animatedTreads", "1");
	  BZDB.set("treadStyle", "0");
	  break;
	case 2:
	  BZDB.set("animatedTreads", "1");
	  BZDB.set("treadStyle", "1");
	  break;
	default:
	  BZDB.set("animatedTreads", "0");
	  BZDB.set("treadStyle", "0");
	  break;
      }
      RENDERER.setRebuildTanks();
      break;
    }
    case 't': {
      int fade = list->getIndex();
      TrackMarks::setUserFade(float(fade) / 10.0f);
      break;
    }
    case 'c': {
      int culling = list->getIndex();
      if (culling <= 0) {
	TrackMarks::setAirCulling(TrackMarks::NoAirCull);
      } else if (culling == 1) {
	TrackMarks::setAirCulling(TrackMarks::InitAirCull);
      } else {
	TrackMarks::setAirCulling(TrackMarks::FullAirCull);
      }
      break;
    }
    case 'f': {
      BZDB.set("useFancyEffects", list->getIndex() ? "1" : "0");
      break;
    }
    case 's': {
      BZDB.set("spawnEffect", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case 'L': {
      BZDB.set("enableLocalSpawnEffect", list->getIndex() ? "1" : "0");
      break;
    }
    case 'd': {
      BZDB.set("deathEffect", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case 'S': {
      BZDB.set("shotEffect", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case 'l': {
      BZDB.set("enableLocalShotEffect", list->getIndex() ? "1" : "0");
      break;
    }
    case 'V': {
      BZDB.set("useVelOnShotEffects", list->getIndex() ? "1" : "0");
      break;
    }
    case 'b': {
      BZDB.set("landEffect", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case 'G': {
      BZDB.set("gmPuffEffect", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case 'g': {
      BZDB.set("gmPuffTime", TextUtils::format("1/(%d+3)", list->getIndex()));
      break;
    }
    case 'R': {
      BZDB.set("ricoEffect", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case '7': {
      BZDB.set("tpEffect", TextUtils::format("%d", list->getIndex()));
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
