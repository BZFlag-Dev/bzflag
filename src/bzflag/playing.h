/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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

#ifdef _WIN32
#pragma warning( 4 : 4786 )
#endif

#include "common.h"
#include "global.h"
#include <string>
#include <vector>

class SceneRenderer;
class BzfKeyMap;

struct StartupInfo {
  public:
			StartupInfo();

  public:
    bool		hasConfiguration;
    bool		autoConnect;
    char		serverName[80];
    int			serverPort;
    int			ttl;
    bool		useUDPconnection;
    char		multicastInterface[65];
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
class ResourceDatabase;
class PlayerId;
class Player;

BzfDisplay*		getDisplay();
MainWindow*		getMainWindow();
SceneRenderer*		getSceneRenderer();
void			setSceneDatabase();
StartupInfo*		getStartupInfo();
BzfKeyMap&			getBzfKeyMap();
void			notifyBzfKeyMapChanged();
bool			setVideoFormat(int, bool test = false);
Player*			lookupPlayer(const PlayerId& id);
void			startPlaying(BzfDisplay* display,
				SceneRenderer&,
				ResourceDatabase&,
				StartupInfo*);

bool			addExplosion(const float* pos,
				float size, float duration);
void			addTankExplosion(const float* pos);
void			addShotExplosion(const float* pos);
void			addShotPuff(const float* pos);

void			addPlayingCallback(PlayingCallback, void* data);
void			removePlayingCallback(PlayingCallback, void* data);

void			joinGame(JoinGameCallback, void* userData);

#endif // BZF_PLAYING_H
// ex: shiftwidth=2 tabstop=8
