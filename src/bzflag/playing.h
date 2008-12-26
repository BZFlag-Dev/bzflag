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

/*
 * main game loop stuff
 */

#ifndef	BZF_PLAYING_H
#define	BZF_PLAYING_H

#include "common.h"

// system includes
#include <string>
#include <vector>

/* common headers */
#include "StartupInfo.h"
#include "CommandCompleter.h"
#include "Address.h"

/* local headers */
#include "LocalPlayer.h"
#include "MainWindow.h"
#include "ControlPanel.h"

#define MAX_DT_LIMIT 0.1f
#define MIN_DT_LIMIT 0.001f

#define MAX_MESSAGE_HISTORY (20)

typedef void		(*JoinGameCallback)(bool success, void* data);
typedef void		(*ConnectStatusCallback)(std::string& str);
typedef void		(*PlayingCallback)(void*);
struct PlayingCallbackItem {
  public:
    PlayingCallback	cb;
    void*		data;
};

class BzfDisplay;
class MainWindow;
class SceneRenderer;
class Player;
class ServerLink;
class HUDRenderer;
class WordFilter;

void			initPlaying();
BzfDisplay*		getDisplay();
MainWindow*		getMainWindow();
SceneRenderer*		getSceneRenderer();
void			setSceneDatabase();
StartupInfo*		getStartupInfo();
void			notifyBzfKeyMapChanged();
bool			setVideoFormat(int, bool test = false);
Player*			lookupPlayer(PlayerId id);
void			startPlaying(void);

bool			addExplosion(const float* pos, float size, float duration);
void			addTankExplosion(const float* pos);
void			addShotExplosion(const float* pos);
void			addShotPuff(const float* pos, float azimuth, float elevation);
void			warnAboutMainFlags();
void			warnAboutRadarFlags();
void			warnAboutRadar();
void			warnAboutConsole();
void			addPlayingCallback(PlayingCallback, void* data);
void			removePlayingCallback(PlayingCallback, void* data);

void			joinGame(JoinGameCallback, void* userData);
void			leaveGame();
std::vector<std::string>& getSilenceList();
void			updateEvents();
void			addMessage(const Player* player,
				   const std::string& msg,
				   int mode = 3,
				   bool highlight = false,
				   const char* oldColor = NULL);

int			curlProgressFunc(void* clientp,
					 double dltotal, double dlnow,
					 double ultotal, double ulnow);

void selectNextRecipient (bool forward, bool robotIn);
void handleFlagDropped(Player* tank);
void dropFlag(LocalPlayer* myTank);
void setTarget();
bool shouldGrabMouse();
void setRoamingLabel();
void drawFrame(const float dt);
void injectMessages(uint16_t code, uint16_t len, void *msg);

extern void joinGame();

extern HUDRenderer*	hud;
extern PlayerId		msgDestination;
extern ServerLink*	serverLink;
extern int		numFlags;
extern StartupInfo	startupInfo;
extern CommandCompleter	completer;
extern bool		gameOver;
extern bool		canSpawn;
extern std::string	customLimboMessage;
extern ControlPanel*	controlPanel;
extern bool		fireButton;
extern float		destructCountdown;
extern bool		pausedByUnmap;
extern int		savedVolume;
extern BzfDisplay*	display;
extern MainWindow*	mainWindow;
extern float		pauseCountdown;
extern float		clockAdjust;
extern float		roamDZoom;
extern bool		roamButton;
extern WordFilter*	wordFilter;

/* Any code surrounded by "if (!headless)" is unsafely assuming that it's
 * operating in a context where graphics and sound are available.
 */
extern bool		headless;

typedef struct _ThirdPersonVars
{
	bool b3rdPerson;
	float cameraOffsetXY;
	float cameraOffsetOffsetZ;
	float targetMultiplyer;
	float nearTargetDistance;
	float nearTargetSize;
	float farTargetDistance;
	float farTargetSize;

	void load ( void );
	void clear ( void );
} ThirdPersonVars;

extern ThirdPersonVars thirdPersonVars;

#endif // BZF_PLAYING_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
