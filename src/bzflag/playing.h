/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * main game loop stuff
 */

#ifndef	BZF_PLAYING_H
#define	BZF_PLAYING_H

#if defined(_WIN32)
#pragma warning(disable: 4786)
#pragma warning(disable: 4100)
#pragma warning(disable: 4511)
#endif

// system includes
#include <string>
#include <vector>

#include "BzfDisplay.h"
#include "MainWindow.h"
#include "SceneRenderer.h"
#include "Player.h"

#define MAX_MESSAGE_HISTORY (20)


class SceneRenderer;

struct StartupInfo {
  public:
			StartupInfo();

  public:
    bool		hasConfiguration;
    bool		autoConnect;
    char		serverName[80];
    int			serverPort;
    bool		useUDPconnection;
    TeamColor		team;
    char		callsign[CallSignLen];
    char		email[EmailLen];
    std::string		listServerURL;
    int			listServerPort;
    bool		joystick;
    std::string		joystickName;
};

typedef void		(*JoinGameCallback)(bool success, void* data);
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

BzfDisplay*		getDisplay();
MainWindow*		getMainWindow();
SceneRenderer*		getSceneRenderer();
void			setSceneDatabase();
StartupInfo*		getStartupInfo();
void			notifyBzfKeyMapChanged();
bool			setVideoFormat(int, bool test = false);
Player*			lookupPlayer(PlayerId id);
void			startPlaying(BzfDisplay* display,
				SceneRenderer&,
				StartupInfo*);

bool			addExplosion(const float* pos,
				float size, float duration);
void			addTankExplosion(const float* pos);
void			addShotExplosion(const float* pos);
void			addShotPuff(const float* pos);

void			addPlayingCallback(PlayingCallback, void* data);
void			removePlayingCallback(PlayingCallback, void* data);

void			joinGame(JoinGameCallback, void* userData);
std::string		getCacheDirectoryName();
std::vector<std::string>& getSilenceList();
void			updateEvents();

#endif // BZF_PLAYING_H

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

