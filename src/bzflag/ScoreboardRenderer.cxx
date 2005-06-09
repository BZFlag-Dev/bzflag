/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Id$
 */


/* interface header */
#include "ScoreboardRenderer.h"

/* system implementation headers */
#include <stdlib.h>

/* common implementation headers */
#include "StateDatabase.h"
#include "BundleMgr.h"
#include "Bundle.h"
#include "Team.h"
#include "FontManager.h"
#include "BZDBCache.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "World.h"
#include "RemotePlayer.h"


//
// ScoreboardRenderer
//

std::string		ScoreboardRenderer::scoreSpacingLabel("88% 888 (888-888)[88]");
std::string		ScoreboardRenderer::scoreLabel("Score");
std::string		ScoreboardRenderer::killSpacingLabel("888/888 Hunt->");
std::string		ScoreboardRenderer::killLabel("Kills");
std::string		ScoreboardRenderer::teamScoreSpacingLabel("888 (888-888) 88");
std::string		ScoreboardRenderer::teamCountSpacingLabel("88");
std::string		ScoreboardRenderer::playerLabel("Player");




ScoreboardRenderer::ScoreboardRenderer() :
        winWidth (0.0),
				dim(false),
				huntIndicator(false),
				hunting(false),
				huntPosition(0),
				huntSelection(false),
				showHunt(false)
{

  // initialize message color (white)
  messageColor[0] = 1.0f;
  messageColor[1] = 1.0f;
  messageColor[2] = 1.0f;
}


/*  Set window size and location to be used to render the scoreboard.
 *  Values are relative to .... when render() is invoked.
*/
void  ScoreboardRenderer::setWindowSize (float x, float y, float width, float height)
{
  winX = x;
  winY = y;
  winWidth = width;
  winHeight = height;
  setMinorFontSize (winHeight);
  setLabelsFontSize (winHeight);
}



ScoreboardRenderer::~ScoreboardRenderer()
{
  // no destruction needed
}


void			ScoreboardRenderer::setMinorFontSize(float height)
{
  FontManager &fm = FontManager::instance();
  minorFontFace = fm.getFaceID(BZDB.get("consoleFont"));

  switch (static_cast<int>(BZDB.eval("scorefontsize"))) {
  case 0: { // auto
    const float s = height / 72.0f;
    minorFontSize = floorf(s);
    break;
  }
  case 1: // tiny
    minorFontSize = 6;
    break;
  case 2: // small
    minorFontSize = 8;
    break;
  case 3: // medium
    minorFontSize = 12;
    break;
  case 4: // big
    minorFontSize = 16;
    break;
  }

  huntArrowWidth = fm.getStrLength(minorFontFace, minorFontSize, "-> ");
  huntedArrowWidth = fm.getStrLength(minorFontFace, minorFontSize, "Hunt->");
  scoreLabelWidth = fm.getStrLength(minorFontFace, minorFontSize, scoreSpacingLabel);
  killsLabelWidth = fm.getStrLength(minorFontFace, minorFontSize, killSpacingLabel);
  teamScoreLabelWidth = fm.getStrLength(minorFontFace, minorFontSize, teamScoreSpacingLabel);
  teamCountLabelWidth = fm.getStrLength(minorFontFace, minorFontSize, teamCountSpacingLabel);
  const float spacing = fm.getStrLength(minorFontFace, minorFontSize, " ");
  scoreLabelWidth += spacing;
  killsLabelWidth += spacing;
}


void			ScoreboardRenderer::setLabelsFontSize(float height)
{
  const float s = height / 96.0f;
  FontManager &fm = FontManager::instance();
  labelsFontFace = fm.getFaceID(BZDB.get("consoleFont"));
  labelsFontSize = floorf(s);
}


void			ScoreboardRenderer::setDim(bool _dim)
{
  dim = _dim;
}


static const float dimFactor = 0.2f;

void			ScoreboardRenderer::hudColor3fv(const GLfloat* c)
{
  if (dim)
    glColor3f(dimFactor * c[0], dimFactor * c[1], dimFactor * c[2]);
  else
    glColor3fv(c);
}


void			ScoreboardRenderer::render()
{
  if (winWidth == 0.0f)
    return    // can't do anything if window size not set
  OpenGLGState::resetState();
  renderScoreboard();
}



int ScoreboardRenderer::tankScoreCompare(const void* _a, const void* _b)
{
  RemotePlayer* a = World::getWorld()->getPlayer(*(int*)_a);
  RemotePlayer* b = World::getWorld()->getPlayer(*(int*)_b);
  if (World::getWorld()->allowRabbit())
    return b->getRabbitScore() - a->getRabbitScore();
  else
    return b->getScore() - a->getScore();
}

int ScoreboardRenderer::teamScoreCompare(const void* _c, const void* _d)
{
  Team* c = World::getWorld()->getTeams()+*(int*)_c;
  Team* d = World::getWorld()->getTeams()+*(int*)_d;
  return (d->won-d->lost) - (c->won-c->lost);
}

void		ScoreboardRenderer::setHuntPosition(int _huntPosition)
{
  huntPosition = _huntPosition;
}

int			ScoreboardRenderer::getHuntPosition() const
{
  return huntPosition;
}

bool			ScoreboardRenderer::getHuntSelection() const
{
  return huntSelection;
}

void			ScoreboardRenderer::setHuntSelection(bool _huntSelection)
{
  huntSelection = _huntSelection;
}

void		ScoreboardRenderer::setHuntIndicator(bool _huntIndicator)
{
  huntIndicator = _huntIndicator;
}

bool			ScoreboardRenderer::getHuntIndicator() const
{
  return huntIndicator;
}

void		ScoreboardRenderer::setHunting(bool _hunting)
{
  hunting = _hunting;
}

bool			ScoreboardRenderer::getHunting() const
{
  return hunting;
}

bool			ScoreboardRenderer::getHunt() const
{
  return showHunt;
}

void			ScoreboardRenderer::setHunt(bool _showHunt)
{
  showHunt = _showHunt;
}


void			ScoreboardRenderer::renderTeamScores (float x, float y, float dy){
  // print teams sorted by score
  int teams[NumTeams];
  int teamCount = 0;
  int i;
  float xn, xl;
  std::string label;

  if (World::getWorld()->allowRabbit())
    return;

  Bundle *bdl = BundleMgr::getCurrentBundle();
  FontManager &fm = FontManager::instance();
  hudColor3fv(messageColor);

  label = bdl->getLocalString("Team Score");
  xl = xn = x - teamScoreLabelWidth;
  fm.drawString(xl, y, 0, minorFontFace, minorFontSize, label);

  for (i = RedTeam; i < NumTeams; i++) {
    if (!Team::isColorTeam(TeamColor(i))) continue;
    const Team* team = World::getWorld()->getTeams() + i;
    if (team->size == 0) continue;
    teams[teamCount++] = i;
  }
  qsort(teams, teamCount, sizeof(int), teamScoreCompare);
  y -= dy;

  char score[44];
  for (i = 0 ; i < teamCount; i++){
    Team& team = World::getWorld()->getTeam(teams[i]);
    sprintf(score, "%d (%d-%d) %d", team.won - team.lost, team.won, team.lost, team.size);
    hudColor3fv(Team::getRadarColor((TeamColor)teams[i]));
    fm.drawString(xn, y, 0, minorFontFace, minorFontSize, score);
    y -= dy;
  }
}


// not used yet - new feature coming
void			ScoreboardRenderer::renderCtfFlags (){
  int i;
  RemotePlayer* player;
  const int curMaxPlayers = World::getWorld()->getCurMaxPlayers();
  char flagColor[200];

  FontManager &fm = FontManager::instance();
  const float x = winX;
  const float y = winY;
  const float dy = fm.getStrHeight(minorFontFace, minorFontSize, " ");
  float y0 = y - dy;

  hudColor3fv(messageColor);
  fm.drawString(x, y, 0, minorFontFace, minorFontSize, "Team Flags");
    
  for (i=0; i < curMaxPlayers; i++) {
    if ( (player = World::getWorld()->getPlayer(i)) ) {
      FlagType* flagd = player->getFlag();
      TeamColor teamIndex = player->getTeam();
      if (flagd!=Flags::Null && flagd->flagTeam != NoTeam) {   // if player has team flag ...
        std::string playerInfo = dim ? ColorStrings[DimColor] : "";
        playerInfo += ColorStrings[flagd->flagTeam];
        sprintf (flagColor, "%-12s", flagd->flagName);
        playerInfo += flagColor;
        playerInfo += ColorStrings[teamIndex];
        playerInfo += player->getCallSign();

        fm.setDimFactor(dimFactor);
        fm.drawString(x, y0, 0, minorFontFace, minorFontSize, playerInfo);
        y0 -= dy;
      }
    }
  }
  renderTeamScores (winWidth, y, dy);
}



void			ScoreboardRenderer::renderScoreboard(void)
{
  int i, j;
  bool huntPlayerAlive = false;

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!myTank || !World::getWorld()) return;

  Bundle *bdl = BundleMgr::getCurrentBundle();
  FontManager &fm = FontManager::instance();

  const float x1 = winX;
  const float x2 = x1 + scoreLabelWidth;
  const float x3 = x2 + killsLabelWidth;
  const float y0 = winY;
  hudColor3fv(messageColor);
  fm.drawString(x1, y0, 0, minorFontFace, minorFontSize, bdl->getLocalString(scoreLabel));
  fm.drawString(x2, y0, 0, minorFontFace, minorFontSize, bdl->getLocalString(killLabel));
  fm.drawString(x3, y0, 0, minorFontFace, minorFontSize, bdl->getLocalString(playerLabel));
  const float dy = fm.getStrHeight(minorFontFace, minorFontSize, " ");
  int y = (int)(y0 - dy);

  // make room for the status marker
  const float xs = x3 - fm.getStrLength(minorFontFace, minorFontSize, "+|");

  // grab the tk warning ratio
  tkWarnRatio = BZDB.eval("tkwarnratio");

  // print non-observing players sorted by score, print observers last
  int plrCount = 0;
  int obsCount = 0;
  const int curMaxPlayers = World::getWorld()->getCurMaxPlayers();
  int* players = new int[curMaxPlayers];
  RemotePlayer* rp;

  for (j = 0; j < curMaxPlayers; j++) {
    if ((rp = World::getWorld()->getPlayer(j))) {
      if (rp->getTeam() != ObserverTeam)
      	players[plrCount++] = j;
      else
      	players[curMaxPlayers - (++obsCount)] = j;
    }
  }

  qsort(players, plrCount, sizeof(int), tankScoreCompare);

  // list player scores
  bool drewMyScore = false;
  for (i = 0; i < plrCount; i++) {
    RemotePlayer* player = World::getWorld()->getPlayer(players[i]);
    if(getHunt()) {
      // Make the selection marker wrap.
      if(getHuntPosition() >= plrCount) setHuntPosition(0);
      if(getHuntPosition() < 0) setHuntPosition(plrCount-1);

      // toggle the hunt indicator if this is the current player pointed to
      if(getHuntPosition() == i) {
      	setHuntIndicator(true);
	      // If hunt is selected set this player to be hunted
	      if(getHuntSelection()) {
	        player->setHunted(true);
	        setHunting(true);
	        setHuntSelection(false);
	        setHunt(false);
	        huntPlayerAlive = true; // hunted player is alive since you selected him
	      }
      } else {
      	setHuntIndicator(false);
      }
    } else {
      setHuntIndicator(false);
      if (!getHunting()) player->setHunted(false); // if not hunting make sure player isn't hunted
      else if (player->isHunted()) huntPlayerAlive = true; // confirm hunted player is alive
    }
    bool myTurn = false;
    if (!drewMyScore && myTank->getTeam() != ObserverTeam)
      if (World::getWorld()->allowRabbit()) {
      	myTurn = myTank->getRabbitScore() > player->getRabbitScore();
      } else {
      	myTurn = myTank->getScore() > player->getScore();
      }
    if (myTurn) {
      setHuntIndicator(false); // don't hunt myself
      // if i have greater score than remote player draw my name here
      drawPlayerScore(myTank, x1, x2, x3, xs, (float)y);
      drewMyScore = true;
      y -= (int)dy;
    }
    if(getHunt() && getHuntPosition() == i) setHuntIndicator(true);// set hunt indicator back to normal
    drawPlayerScore(player, x1, x2, x3, xs, (float)y);//then draw the remote player
    y -= (int)dy;
  }
  if (!huntPlayerAlive && getHunting()) setHunting(false); //stop hunting if hunted player is dead
  if (!drewMyScore && (myTank->getTeam() != ObserverTeam)) {
    setHuntIndicator(false); // don't hunt myself
    // if my score is smaller or equal to last remote player draw my score here
    drawPlayerScore(myTank, x1, x2, x3, xs, (float)y);
    y -= (int)dy;
    drewMyScore = true;
  }

  // list observers
  y -= (int)dy;
  for (i = curMaxPlayers - 1; i >= curMaxPlayers - obsCount; --i) {
    setHuntIndicator(false); // don't hunt observer
    RemotePlayer* player = World::getWorld()->getPlayer(players[i]);
    drawPlayerScore(player, x1, x2, x3, xs, (float)y);
    y -= (int)dy;
  }
  if (!drewMyScore) {
    // if I am an observer, list my name
    drawPlayerScore(myTank, x1, x2, x3, xs, (float)y);
  }

  delete[] players;
  renderTeamScores (winWidth, y0, dy);
}




void			ScoreboardRenderer::drawPlayerScore(const Player* player,
			    float x1, float x2, float x3, float xs, float y)
{
  // dim the font if we're dim
  const std::string dimString = dim ? ColorStrings[DimColor] : "";

  // score
  char score[40], kills[40];

  bool highlightTKratio = false;
  if (tkWarnRatio > 0.0) {
    if (((player->getWins() > 0) && (player->getTKRatio() > tkWarnRatio)) ||
        ((player->getWins() == 0) && (player->getTeamKills() >= 3))) {
      highlightTKratio = true;
    }
  }

  if (World::getWorld()->allowRabbit())
    sprintf(score, "%s%2d%% %d(%d-%d)%s[%d]", dimString.c_str(),
	    player->getRabbitScore(),
	    player->getScore(), player->getWins(), player->getLosses(),
	    highlightTKratio ? ColorStrings[CyanColor].c_str() : "",
	    player->getTeamKills());
  else
    sprintf(score, "%s%d (%d-%d)%s[%d]", dimString.c_str(),
	    player->getScore(), player->getWins(), player->getLosses(),
	    highlightTKratio ? ColorStrings[CyanColor].c_str() : "",
	    player->getTeamKills());
  if (LocalPlayer::getMyTank() != player)
    sprintf(kills, "%d/%d", player->getLocalWins(), player->getLocalLosses());
  else
    strcpy(kills, "");


  // team color
  TeamColor teamIndex = player->getTeam();
  if (teamIndex < RogueTeam) {
    teamIndex = RogueTeam;
  }

  // authentication status
  std::string statusInfo = dimString;
  if (BZDBCache::colorful) {
    statusInfo += ColorStrings[CyanColor];
  } else {
    statusInfo += ColorStrings[teamIndex];;
  }
  if (player->isAdmin()) {
    statusInfo += '@';
  } else if (player->isVerified()) {
    statusInfo += '+';
  } else if (player->isRegistered()) {
    statusInfo += '-';
  } else {
    statusInfo = ""; // don't print
  }

  std::string playerInfo = dimString;
  // team color
  playerInfo += ColorStrings[teamIndex];
  //Slot number only for admins
  LocalPlayer* localPlayer = LocalPlayer::getMyTank();
  if (localPlayer->isAdmin()){
    char slot[10];
    sprintf(slot, "%3d",player->getId());
    playerInfo += slot;
    playerInfo += " - ";
  }
  // callsign
  playerInfo += player->getCallSign();
  // email in parenthesis
  if (player->getEmailAddress()[0] != '\0' && !BZDB.isTrue("hideEmails")) {
    playerInfo += " (";
    playerInfo += player->getEmailAddress();
    playerInfo += ")";
  }
  // carried flag
  bool coloredFlag = false;
  FlagType* flagd = player->getFlag();
  if (flagd != Flags::Null) {
    // color special flags
    if (BZDBCache::colorful) {
      if ((flagd == Flags::ShockWave)
      ||  (flagd == Flags::Genocide)
      ||  (flagd == Flags::Laser)
      ||  (flagd == Flags::GuidedMissile)) {
	      playerInfo += ColorStrings[WhiteColor];
      } else if (flagd->flagTeam != NoTeam) {
      	// use team color for team flags
      	playerInfo += ColorStrings[flagd->flagTeam];
      }
      coloredFlag = true;
    }
    playerInfo += "/";
    playerInfo += (flagd->endurance == FlagNormal ? flagd->flagName : flagd->flagAbbv);
    // back to original color
    if (coloredFlag) {
      playerInfo += ColorStrings[teamIndex];
    }
  }
  // status
  if (player->isPaused())
    playerInfo += "[p]";
  else if (player->isNotResponding())
    playerInfo += "[nr]";
  else if (player->isAutoPilot())
    playerInfo += "[auto]";

  FontManager &fm = FontManager::instance();
  fm.setDimFactor(dimFactor);

  // draw
  if (player->getTeam() != ObserverTeam) {
    hudColor3fv(Team::getRadarColor(teamIndex));
    fm.drawString(x1, y, 0, minorFontFace, minorFontSize, score);
    hudColor3fv(Team::getRadarColor(teamIndex));
    fm.drawString(x2, y, 0, minorFontFace, minorFontSize, kills);
  }
  fm.drawString(x3, y, 0, minorFontFace, minorFontSize, playerInfo);
  if (statusInfo.size() > 0) {
    fm.drawString(xs, y, 0, minorFontFace, minorFontSize, statusInfo);
  }
  if (BZDB.isTrue("debugHud")) {
    printf ("playerInfo: %s\n", playerInfo.c_str()); //FIXME
  }

  // draw hunting status
  const float x4 = xs - huntArrowWidth;
  const float x5 = xs - huntedArrowWidth;
  if (player->isHunted()) {
    fm.drawString(x5, y, 0, minorFontFace, minorFontSize, "Hunt->");
  } else if (getHuntIndicator()) {
    fm.drawString(x4, y, 0, minorFontFace, minorFontSize, "->");
  }
}


  
// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
