/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface headers
#include "HUDuiServerInfo.h"

#include "playing.h"

#include "FontManager.h"
#include "FontSizer.h"
#include "LocalFontFace.h"

#include "HUDuiLabel.h"

//
// HUDuiServerInfo
//

HUDuiServerInfo::HUDuiServerInfo() : HUDuiControl(), server(NULL)
{
  readouts.push_back(new HUDuiLabel);	// 0
  readouts.push_back(new HUDuiLabel);	// 1
  readouts.push_back(new HUDuiLabel);	// 2
  readouts.push_back(new HUDuiLabel);	// 3
  readouts.push_back(new HUDuiLabel);	// 4
  readouts.push_back(new HUDuiLabel);	// 5
  readouts.push_back(new HUDuiLabel);	// 6
  readouts.push_back(new HUDuiLabel);	// 7 max shots
  readouts.push_back(new HUDuiLabel);	// 8 capture-the-flag/free-style/rabbit chase
  readouts.push_back(new HUDuiLabel);	// 9 super-flags
  readouts.push_back(new HUDuiLabel);	// 10 antidote-flag
  readouts.push_back(new HUDuiLabel);	// 11 shaking time
  readouts.push_back(new HUDuiLabel);	// 12 shaking wins
  readouts.push_back(new HUDuiLabel);	// 13 jumping
  readouts.push_back(new HUDuiLabel);	// 14 ricochet
  readouts.push_back(new HUDuiLabel);	// 15 inertia
  readouts.push_back(new HUDuiLabel);	// 16 time limit
  readouts.push_back(new HUDuiLabel);	// 17 max team score
  readouts.push_back(new HUDuiLabel);	// 18 max player score
  readouts.push_back(new HUDuiLabel);	// 19 ping time
  readouts.push_back(new HUDuiLabel);	// 20 cached status
  readouts.push_back(new HUDuiLabel);	// 21 cached age
}

HUDuiServerInfo::~HUDuiServerInfo()
{
  // Do nothing
}

void HUDuiServerInfo::setServerItem(ServerItem* item)
{
  server = item;
}

void HUDuiServerInfo::setSize(float width, float height)
{
  HUDuiControl::setSize(width, height); 

  resize();
}

void HUDuiServerInfo::setFontSize(float size)
{
  HUDuiControl::setFontSize(size); 

  resize();
}

void HUDuiServerInfo::setFontFace(const LocalFontFace* face)
{
  HUDuiControl::setFontFace(face); 

  resize();
}

void HUDuiServerInfo::setPosition(float x, float y)
{
  HUDuiControl::setPosition(x, y);

  resize();
}

void HUDuiServerInfo::resize()
{
  FontManager &fm = FontManager::instance();
  FontSizer fs = FontSizer(getWidth(), getHeight());

  // reposition server readouts
  float fontSize = fs.getFontSize(getFontFace()->getFMFace(), "alertFontSize");
  float fontHeight = fm.getStringHeight(getFontFace()->getFMFace(), fontSize);
  const float y0 = getY() + getHeight() - fontHeight/2;
  
  float y = y0;

  float x = getX() + fontHeight;
  fs.setMin(10, 10);
  for (int i=0; i<(int) readouts.size(); i++) {
    if ((i + 1) % 7 == 1) {
      x = (0.125f + 0.25f * (float)(i / 7)) * (float)getWidth();
      y = y0;
    }

    HUDuiLabel* label = readouts[i];
    label->setFontSize(fontSize);
    label->setFontFace(getFontFace());
    y -= 1.0f * fontHeight;
    label->setPosition(x, y);
  }
}

void HUDuiServerInfo::fillReadouts()
{
  const ServerItem& item = *server;
  const PingPacket& ping = item.ping;

  // update server readouts
  char buf[60];
  std::vector<HUDuiLabel*>& listHUD = readouts;

  const uint8_t maxes [] = { ping.maxPlayers, ping.rogueMax, ping.redMax, ping.greenMax,
			     ping.blueMax, ping.purpleMax, ping.observerMax };

  // if this is a cached item set the player counts to "?/max count"
  if (item.cached && item.getPlayerCount() == 0) {
    //for (int i = 1; i <=7; i ++) {
    //  sprintf(buf, "?/%d", maxes[i-1]);
    //  (listHUD[i])->setLabel(buf);
    //}
  } else {  // not an old item, set players #s to info we have
    sprintf(buf, "%d/%d", ping.rogueCount + ping.redCount + ping.greenCount +
	    ping.blueCount + ping.purpleCount, ping.maxPlayers);
    (listHUD[0])->setLabel(buf);

    if (ping.rogueMax == 0)
      buf[0]=0;
    else if (ping.rogueMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.rogueCount);
    else
      sprintf(buf, "%d/%d", ping.rogueCount, ping.rogueMax);
    (listHUD[1])->setLabel(buf);

    if (ping.redMax == 0)
      buf[0]=0;
    else if (ping.redMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.redCount);
    else
      sprintf(buf, "%d/%d", ping.redCount, ping.redMax);
    (listHUD[2])->setLabel(buf);

    if (ping.greenMax == 0)
      buf[0]=0;
    else if (ping.greenMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.greenCount);
    else
      sprintf(buf, "%d/%d", ping.greenCount, ping.greenMax);
    (listHUD[3])->setLabel(buf);

    if (ping.blueMax == 0)
      buf[0]=0;
    else if (ping.blueMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.blueCount);
    else
      sprintf(buf, "%d/%d", ping.blueCount, ping.blueMax);
    (listHUD[4])->setLabel(buf);

    if (ping.purpleMax == 0)
      buf[0]=0;
    else if (ping.purpleMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.purpleCount);
    else
      sprintf(buf, "%d/%d", ping.purpleCount, ping.purpleMax);
    (listHUD[5])->setLabel(buf);

    if (ping.observerMax == 0)
      buf[0]=0;
    else if (ping.observerMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.observerCount);
    else
      sprintf(buf, "%d/%d", ping.observerCount, ping.observerMax);
    (listHUD[6])->setLabel(buf);
  }

  std::vector<std::string> args;
  sprintf(buf, "%d", ping.maxShots);
  args.push_back(buf);

  if (ping.maxShots == 1)
    (listHUD[7])->setString("{1} Shot", &args );
  else
    (listHUD[7])->setString("{1} Shots", &args );

  if (ping.gameType == ClassicCTF)
    (listHUD[8])->setString("Classic Capture-the-Flag");
  else if (ping.gameType == RabbitChase)
    (listHUD[8])->setString("Rabbit Chase");
  else if (ping.gameType == OpenFFA)
    (listHUD[8])->setString("Open (Teamless) Free-For-All");
  else
    (listHUD[8])->setString("Team Free-For-All");

  if (ping.gameOptions & SuperFlagGameStyle)
    (listHUD[9])->setString("Super Flags");
  else
    (listHUD[9])->setString("");

  if (ping.gameOptions & AntidoteGameStyle)
    (listHUD[10])->setString("Antidote Flags");
  else
    (listHUD[10])->setString("");

  if ((ping.gameOptions & ShakableGameStyle) && ping.shakeTimeout != 0) {
    std::vector<std::string> dropArgs;
    sprintf(buf, "%.1f", 0.1f * float(ping.shakeTimeout));
    dropArgs.push_back(buf);
    if (ping.shakeWins == 1)
      (listHUD[11])->setString("{1} sec To Drop Bad Flag",
			       &dropArgs);
    else
      (listHUD[11])->setString("{1} secs To Drop Bad Flag",
			       &dropArgs);
  } else {
    (listHUD[11])->setString("");
  }

  if ((ping.gameOptions & ShakableGameStyle) && ping.shakeWins != 0) {
    std::vector<std::string> dropArgs;
    sprintf(buf, "%d", ping.shakeWins);
    dropArgs.push_back(buf);
    dropArgs.push_back(ping.shakeWins == 1 ? "" : "s");
    if (ping.shakeWins == 1)
      (listHUD[12])->setString("{1} Win Drops Bad Flag",
			       &dropArgs);
    else
      (listHUD[12])->setString("{1} Wins Drops Bad Flag",
			       &dropArgs);
  } else {
    (listHUD[12])->setString("");
  }

  if (ping.gameOptions & JumpingGameStyle)
    (listHUD[13])->setString("Jumping");
  else
    (listHUD[13])->setString("");

  if (ping.gameOptions & RicochetGameStyle)
    (listHUD[14])->setString("Ricochet");
  else
    (listHUD[14])->setString("");

  if (ping.gameOptions & HandicapGameStyle)
    (listHUD[15])->setString("Handicap");
  else
    (listHUD[15])->setString("");

  if (ping.maxTime != 0) {
    std::vector<std::string> pingArgs;
    if (ping.maxTime >= 3600)
      sprintf(buf, "%d:%02d:%02d", ping.maxTime / 3600, (ping.maxTime / 60) % 60, ping.maxTime % 60);
    else if (ping.maxTime >= 60)
      sprintf(buf, "%d:%02d", ping.maxTime / 60, ping.maxTime % 60);
    else
      sprintf(buf, "0:%02d", ping.maxTime);
    pingArgs.push_back(buf);
    (listHUD[16])->setString("Time limit: {1}", &pingArgs);
  } else {
    (listHUD[16])->setString("");
  }

  if (ping.maxTeamScore != 0) {
    std::vector<std::string> scoreArgs;
    sprintf(buf, "%d", ping.maxTeamScore);
    scoreArgs.push_back(buf);
    (listHUD[17])->setString("Max team score: {1}", &scoreArgs);
  } else {
    (listHUD[17])->setString("");
  }

  if (ping.maxPlayerScore != 0) {
    std::vector<std::string> scoreArgs;
    sprintf(buf, "%d", ping.maxPlayerScore);
    scoreArgs.push_back(buf);
    (listHUD[18])->setString("Max player score: {1}", &scoreArgs);
  } else {
    (listHUD[18])->setString("");
  }

  if (ping.pingTime > 0) {
    std::vector<std::string> pingArgs;
    sprintf(buf, "%dms", ping.pingTime);  // What's the matter with a strstream. Come on!
    pingArgs.push_back(buf);              // So last decade
    ((HUDuiLabel*)listHUD[19])->setString("Ping: {1}", &pingArgs);
  } else {
    ((HUDuiLabel*)listHUD[19])->setString("");
  }

  if (item.cached) {
    (listHUD[20])->setString("Cached");
    (listHUD[21])->setString(item.getAgeString());
  } else {
    (listHUD[20])->setString("");
    (listHUD[21])->setString("");
  }
}

void HUDuiServerInfo::doRender()
{
  if (getFontFace() < 0) {
    return;
  }

  float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};

  OpenGLGState::resetState();  // fixme: shouldn't be needed
  glLineWidth(1.0f);
  glColor4fv(color);

  glBegin(GL_LINES);

  glVertex2f(getX(), getY());
  glVertex2f(getX(), getY() + getHeight());

  glVertex2f(getX(), getY());
  glVertex2f(getX() + getWidth(), getY());

  glVertex2f(getX() + getWidth(), getY());
  glVertex2f(getX() + getWidth(), getY() + getHeight());

  glVertex2f(getX() + getWidth(), getY() + getHeight());
  glVertex2f(getX(), getY() + getHeight());

  glEnd();

  if (server == NULL) {
    return;
  }

  fillReadouts();

  for (int i=0; i<(int) readouts.size(); i++)
  {
    readouts[i]->render();
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
