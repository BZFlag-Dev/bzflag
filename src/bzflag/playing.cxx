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

static const char copyright[] = "Copyright (c) 1993 - 2002 Tim Riker";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include "playing.h"
#include "BzfDisplay.h"
#include "BzfEvent.h"
#include "BzfWindow.h"
#include "PlatformFactory.h"
#include "PlatformMediaFactory.h"
#include "global.h"
#include "Address.h"
#include "Protocol.h"
#include "Pack.h"
#include "ServerLink.h"
#include "StateDatabase.h"
#include "SceneBuilder.h"
#include "MenuManager.h"
#include "Menu.h"
#include "World.h"
#include "Team.h"
#include "Flag.h"
#include "LocalPlayer.h"
#include "RemotePlayer.h"
#include "ShotStrategy.h"
#include "sound.h"
#include "TimeBomb.h"
#include "ErrorHandler.h"
#include "KeyManager.h"
#include "Intersect.h"
#include "Ping.h"
#include "OpenGLTexture.h"
#include "CommandManager.h"
#include "CommandsStandard.h"
#include "MessageManager.h"
#include "HUDManager.h"
#include "ViewManager.h"
#include "View.h"
#include "SceneManager.h"
#include "FileManager.h"
#include "SceneNode.h"
#include "SceneNodeBillboard.h"
#include "SceneNodeGroup.h"
#include "SceneNodeParameters.h"
#include "SceneNodeTransform.h"
#include "CommandsSearch.h"
#include "CallbackList.h"
#include "bzfgl.h"
#include <iostream>


struct ExplosionInfo {
public:
	float				duration;
	float				time;
	SceneNode*			node;
};
typedef std::vector<ExplosionInfo> ExplosionList;

const uint8_t			NoPlayer = 254;

static ServerLink*		serverLink = NULL;
static World*			world = NULL;
static LocalPlayer*		myTank = NULL;
static BzfDisplay*		display = NULL;
static BzfWindow*		mainWindow = NULL;
static Team*			teams = NULL;
static int				maxPlayers = 0;				// not including me
static RemotePlayer**	player = NULL;
static int				numFlags = 0;
static bool				serverError = false;
static bool				serverDied = false;
static bool				fireButton = false;
static bool				restartOnBase = false;
static bool				firstLife = false;
static bool				pausedByUnmap = false;
static bool				unmapped = false;
static float			pauseCountdown = 0.0f;
bool					gameOver = false;
static SceneNode*		explosion = NULL;
static ExplosionList	explosions;
static float			wallClock;
static CallbackList<PlayingCallback>	playingCallbacks;

static char				messageMessage[PlayerIdPLen + 2 + MessageLen];

static void				restartPlaying();
static void				setTarget();
static void				handleFlagDropped(Player* tank);
static void				handlePlayerMessage(uint16_t, uint16_t, void*);

static const float		warningColor[] = { 1.0f, 0.0f, 0.0f };
static const float		redColor[] = { 1.0f, 0.0f, 0.0f };
static const float		greenColor[] = { 0.0f, 1.0f, 0.0f };
static const float		yellowColor[] = { 1.0f, 1.0f, 0.0f };

enum BlowedUpReason {
						GotKilledMsg,
						GotShot,
						GotRunOver,
						GotCaptured,
						GenocideEffect
};
static const char*		blowedUpMessage[] = {
							NULL,
							"Got hit by shot",
							"Got run over by Steamroller",
							"Team flag was captured",
							"Teammate hit by Genocide"
						};
static bool				gotBlowedUp(BaseLocalPlayer* tank,
										BlowedUpReason reason,
										const PlayerId& killer,
										int shotId = -1);

//
// playing callbacks
//

void					addPlayingCallback(
								PlayingCallback callback, void* userData)
{
	playingCallbacks.add(callback, userData);
}

void					removePlayingCallback(
								PlayingCallback callback, void* userData)
{
	playingCallbacks.remove(callback, userData);
}

static bool				onPlayingCallback(
								PlayingCallback callback,
								void* userData,
								void*)
{
	callback(userData);
	return true;
}

static void				callPlayingCallbacks()
{
	playingCallbacks.iterate(onPlayingCallback, NULL);
}


//
// mouse grab stuff
//

static void				onGrabCursorChanged(const BzfString& name, void*)
{
	if (mainWindow != NULL) {
		const bool grab = BZDB->isTrue(name);
		if (grab && !unmapped && (myTank == NULL || !myTank->isPaused()))
			mainWindow->grabMouse();
		else
			mainWindow->ungrabMouse();
	}
}

static void				updateGrab()
{
	onGrabCursorChanged("displayGrabCursor", NULL);
}


//
// handle signals that should kill me quickly
//

static void				dying(Signal sig)
{
	// restore display state
	if (display != NULL)
		display->setDefaultResolution();

	// release rendering context
	OpenGLGState::freeContext();

	// install default signal handler and raise the signal.  this will
	// cause the app to die in the default way (possibly core dumping).
	PLATFORM->signalCatch(sig, kSigDFL);
	PLATFORM->signalRaise(sig);
}

//
// handle signals that should kill me nicely
//

static void				suicide(Signal)
{
	CommandsStandard::quit();
}

//
// handle signals that should disconnect me from the server
//

static void				hangup(Signal)
{
	serverDied = true;
	serverError = true;
}


//
// state database change callbacks
//

static void				onSendComposedMessage(const BzfString& message, void*)
{
	if (!message.empty()) {
		char messageBuffer[MessageLen];
		memset(messageBuffer, 0, MessageLen);
		strncpy(messageBuffer, message.c_str(), MessageLen);
		nboPackString(messageMessage + PlayerIdPLen + 2,
								      messageBuffer, MessageLen);
		serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
	}
}

static void				onConnectionMessage(const BzfString& name, void*)
{
	// report messages
	if (!BZDB->isEmpty(name))
		printError(BZDB->get(name).c_str());
}

static void				onFlagHelp(const BzfString&, void*)
{
	const FlagId id = (myTank != NULL) ? myTank->getFlag() : NoFlag;
	MSGMGR->insert("flagHelp", Flag::getHelp(id), NULL);
}


//
// user input handling
//

#if 0 // roaming, freezing, and snapping are out-of-date and so disabled
#if defined(DEBUG)
#define FREEZING
#define ROAMING
#define SNAPPING
#endif
#if defined(FREEZING)
static bool				motionFreeze = false;
#endif
#if defined(ROAMING)
static bool				roaming = false, roamTrack = false;
static float			roamPos[3] = { 0.0f, 0.0f, MuzzleHeight }, roamDPos[3];
static float			roamTheta = 0.0f, roamDTheta;
static float			roamPhi = 0.0f, roamDPhi;
static float			roamZoom = 60.0f, roamDZoom;
#endif
#endif

static void				showKeyboardStatus()
{
	if (myTank->isKeyboardMoving())
		MSGMGR->insert("messages", "Keyboard movement");
	else if (mainWindow->joystick())
		MSGMGR->insert("messages", "Joystick movement");
	else
		MSGMGR->insert("messages", "Mouse movement");
}

static bool				doKey(const BzfKeyEvent& key, bool pressed)
{
	// send to menu
	if (MENUMGR->top() != NULL) {
		if (pressed)
			MENUMGR->top()->keyPress(key);
		else
			MENUMGR->top()->keyRelease(key);
		return true;
	}

	// send to message composers
	if (pressed) {
		if (MSGMGR->keyPress(key))
			return true;
	}
	else {
		if (MSGMGR->keyRelease(key))
			return true;
	}

	// escape key is hard coded to menu
	if (key.ascii == 27) {
		if (pressed)
			CMDMGR->run("menu push main");
		return true;
	}

	// lookup and run command bound to key
	const BzfString cmd = KEYMGR->get(key, pressed);
	if (!cmd.empty()) {
		BzfString result = CMDMGR->run(cmd);
		if (!result.empty())
			printError(result.c_str());
		return true;
	}

	return false;
}

// XXX -- old keyboard functions
#if 0
#if defined(SNAPPING)
	static int snap = 0;
	if (key.button == BzfKeyEvent::F11 && pressed) {
		// snapshot
		char filename[80];
		sprintf(filename, "bzfi%04d.raw", snap++);
		FILE* f = fopen(filename, "w");
		if (f) {
			int w, h;
			mainWindow->getSize(w, h);
			unsigned char* b = (unsigned char*)malloc(w * h * 3);
			//glReadPixels(0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, b + 0 * w * h);
			//glReadPixels(0, 0, w, h, GL_GREEN, GL_UNSIGNED_BYTE, b + 1 * w * h);
			//glReadPixels(0, 0, w, h, GL_BLUE, GL_UNSIGNED_BYTE, b + 2 * w * h);
			// use something like netpbm and the following command to get usable images
			// rawtoppm -rgb 640 480 bzfi0000.raw | pnmflip -tb | pnmtojpeg --quality=100 > test.jpg
			glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, b);
			fwrite(b, 3 * w * h, 1, f);
			free(b);
			fclose(f);
			printError("%s: %dx%d", filename, w, h);
		}
	}
#endif
#if defined(FREEZING)
	if (key.ascii == '`' && pressed) {
		// toggle motion freeze
		motionFreeze = !motionFreeze;
	}
	//  else
#endif
#if defined(ROAMING)
	if (key.ascii == '~' && pressed) {
		// toggle roaming
		roaming = !roaming;
	}
	else if (roaming && pressed) {
		switch (key.button) {
			case BzfKeyEvent::Left:
				roamDTheta = 60.0f * (roamZoom / 90.0f);
				break;

			case BzfKeyEvent::Right:
				roamDTheta = -60.0f * (roamZoom / 90.0f);
				break;

			case BzfKeyEvent::Up:
				roamDPhi = 60.0f * (roamZoom / 90.0f);
				break;

			case BzfKeyEvent::Down:
				roamDPhi = -60.0f * (roamZoom / 90.0f);
				break;

			case BzfKeyEvent::End:
				roamDPos[0] = -4.0f * TankSpeed;
				break;

			case BzfKeyEvent::Home:
				roamDPos[0] =  4.0f * TankSpeed;
				break;

			case BzfKeyEvent::Delete:
				roamDPos[1] =  4.0f * TankSpeed;
				break;

			case BzfKeyEvent::PageDown:
				roamDPos[1] = -4.0f * TankSpeed;
				break;

			case BzfKeyEvent::Insert:
				roamDPos[2] = -4.0f * TankSpeed;
				break;

			case BzfKeyEvent::PageUp:
				roamDPos[2] =  4.0f * TankSpeed;
				break;

			case BzfKeyEvent::F9:
				roamDZoom =  30.0;
				break;

			case BzfKeyEvent::F10:
				roamDZoom = -30.0;
				break;

			case BzfKeyEvent::F8:
				roamTrack = !roamTrack;
				break;
		}
	}
	//  else
#endif

	else if (keymap.isMappedTo(BzfKeyMap::SlowKeyboardMotion, key)) {
		if (myTank->isKeyboardMoving())
			myTank->setSlowKeyboard(pressed);
	}
	// Might be a direction key. Save it for later.
	else if (myTank->isAlive()) {
		if (!myTank->isKeyboardMoving() && pressed)
			switch (key.button)
			{
			case BzfKeyEvent::Left:
			case BzfKeyEvent::Right:
			case BzfKeyEvent::Up:
			case BzfKeyEvent::Down:
				myTank->setKeyboardMoving(true);
				showKeyboardStatus();
				break;
			}
		if (myTank->isKeyboardMoving())
			myTank->setKey(key.button, pressed);
	}
#endif

static float getKeyValue(bool pressed)
{
	if (pressed)
		return 1;
	return 0;
}

static void				doMotion()
{
#if defined(FREEZING)
	if (motionFreeze) return;
#endif

	float rotation, speed;

	if (myTank->isKeyboardMoving()) {
		rotation = myTank->getKeyboardAngVel();
		speed = myTank->getKeyboardSpeed();

		switch (myTank->getKeyButton()) {
			case BzfKeyEvent::Left:
				rotation = getKeyValue(myTank->getKeyPressed());
				break;
			case BzfKeyEvent::Right:
				rotation = - getKeyValue(myTank->getKeyPressed());
				break;
			case BzfKeyEvent::Up:
				speed = getKeyValue(myTank->getKeyPressed());
				break;
			case BzfKeyEvent::Down:
				speed = - getKeyValue(myTank->getKeyPressed()) / 2.0f;
				break;
		}

		myTank->setKeyboardAngVel(rotation);
		myTank->setKeyboardSpeed(speed);
		myTank->resetKey();

		if (myTank->getFlag() != WideAngleFlag && BZDB->isTrue("displayBinoculars"))
			rotation *= 0.2f;
		if (myTank->hasSlowKeyboard()) {
			rotation /= 2.0f;
			speed /= 2.0f;
		}
	}
	else {
		// get normalized motion coordinates
		float x, y;
		if (mainWindow->joystick()) {
			// get normalized joystick position
			float jx, jy;
			mainWindow->getJoystick(jx, jy);

			// make a dead-zone around 0,0
			static const float xDead = 0.05f;
			static const float yDead = 0.05f;
			if (jx < -xDead)
				x = (jx + xDead) / (1.0f - xDead);
			else if (jx > xDead)
				x = (jx - xDead) / (1.0f - xDead);
			else
				x = 0.0f;
			if (jy < -yDead)
				y = (jy + yDead) / (1.0f - yDead);
			else if (jy > yDead)
				y = (jy - yDead) / (1.0f - yDead);
			else
				y = 0.0f;

/* FIXME -- update joystick support;  use events to handle joystick buttons.
      static const BzfKeyEvent::Button button_map[] = { BzfKeyEvent::LeftMouse,
								BzfKeyEvent::MiddleMouse,
								BzfKeyEvent::RightMouse,
								BzfKeyEvent::F1,
								BzfKeyEvent::F2,
								BzfKeyEvent::F3,
								BzfKeyEvent::F4,
								BzfKeyEvent::F5,
								BzfKeyEvent::F6,
								BzfKeyEvent::F7,
								BzfKeyEvent::F8,
								BzfKeyEvent::F9
      };

      static unsigned long old_buttons = 0;
      unsigned long new_buttons = mainWindow->getJoyButtons();
      if (old_buttons != new_buttons)
		for (int j = 0; j<12; j++)
		  if ((old_buttons & (1<<j)) != (new_buttons & (1<<j))) {
		    BzfKeyEvent ev;
		    ev.button = button_map[j];
		    ev.ascii = 0;
		    ev.shift = 0;
		    doKeyPlaying(ev, (new_buttons&(1<<j)) != 0);
		  }
      old_buttons = new_buttons;
*/
		}
		else {
			// get mouse position
			int mx, my;
			mainWindow->getMouse(mx, my);

			// get motion box shapes
			int xCenter, yCenter;
			int wNoMotionSize, hNoMotionSize;
			int wMaxMotionSize, hMaxMotionSize;
			HUDMGR->getCenter(xCenter, yCenter);
			HUDMGR->getNoMotionSize(wNoMotionSize, hNoMotionSize);
			HUDMGR->getMaxMotionSize(wMaxMotionSize, hMaxMotionSize);

			// yCenter is flipped with respect to window coordinates
			int wWindow, hWindow;
			mainWindow->getSize(wWindow, hWindow);
			yCenter = (hWindow - 1) - yCenter;

			// transform mouse position to window relative
			mx -= xCenter;
			my -= yCenter;

			// compute normalized motion coordinates
			if (mx < -wNoMotionSize)
    			x = static_cast<float>(mx + wNoMotionSize) /
					static_cast<float>(wMaxMotionSize - wNoMotionSize);
			else if (mx > wNoMotionSize)
    			x = static_cast<float>(mx - wNoMotionSize) /
					static_cast<float>(wMaxMotionSize - wNoMotionSize);
			else
				x = 0.0f;
			if (my < -hNoMotionSize)
    			y = static_cast<float>(my + hNoMotionSize) /
					static_cast<float>(hMaxMotionSize - hNoMotionSize);
			else if (my > hNoMotionSize)
    			y = static_cast<float>(my - hNoMotionSize) /
					static_cast<float>(hMaxMotionSize - hNoMotionSize);
			else
				y = 0.0f;
		}

    	// calculate desired rotation
    	rotation = -x;
    	if (rotation > 1.0f)
    		rotation = 1.0f;
    	else if (rotation < -1.0f)
    		rotation = -1.0f;

    	// calculate desired speed
    	speed = -y;
    	if (speed > 1.0f)
    		speed = 1.0f;
    	else if (speed < -0.5f)
    		speed = -0.5f;
	}

	myTank->setDesiredAngVel(rotation);
	myTank->setDesiredSpeed(speed);
}

// ---- commands ----
static BzfString	cmdFire(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 0 && (args.size() != 1 || args[0] != "stop"))
		return "usage: fire [stop]";

	fireButton = (args.size() == 0);
	if (fireButton && myTank != NULL && myTank->isAlive())
		myTank->fireShot();

	return BzfString();
}

static BzfString	cmdDrop(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 0)
		return "usage: drop";

	if (myTank != NULL) {
		FlagId flagId = myTank->getFlag();
		if (flagId != NoFlag && !myTank->isPaused() &&
		Flag::getType(flagId) != FlagSticky &&
		!(flagId == PhantomZoneFlag && myTank->isFlagActive()) &&
		!(flagId == OscOverthrusterFlag &&
		myTank->getLocation() == LocalPlayer::InBuilding)) {
			serverLink->sendDropFlag(myTank->getPosition());
			// changed: on windows it may happen the MsgDropFlag
			// never comes back to us, so we drop it right away
			handleFlagDropped(myTank);
		}
	}

	return BzfString();
}

static BzfString	cmdIdentify(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 0)
		return "usage: identify";

	if (myTank != NULL) {
		if (!gameOver && !myTank->isAlive() && !myTank->isExploding())
			restartPlaying();
		else if (myTank->isAlive() && !myTank->isPaused())
			setTarget();
	}

	return BzfString();
}

static BzfString	cmdJump(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 0)
		return "usage: jump";

	if (myTank != NULL)
		myTank->jump();

	return BzfString();
}

static BzfString	cmdSend(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 1)
		return "usage: send {all|team|nemesis}";

	BzfString composePrompt;
	if (args[0] == "all") {
		void* buf = messageMessage;
		buf = nboPackUInt(buf, 0);
		buf = nboPackShort(buf, 0);
		buf = nboPackShort(buf, 0);
		buf = nboPackUShort(buf, uint16_t(RogueTeam));
		composePrompt = "Send to all: ";
	}
	else if (args[0] == "team") {
		void* buf = messageMessage;
		buf = nboPackUInt(buf, 0);
		buf = nboPackShort(buf, 0);
		buf = nboPackShort(buf, 0);
		buf = nboPackUShort(buf, uint16_t(myTank->getTeam()));
		composePrompt = "Send to teammates: ";
	}
	else if (args[0] == "nemesis") {
		const Player *nemesis = myTank->getNemesis();
		if (!nemesis) {
			void* buf = messageMessage;
			buf = nboPackUByte(buf, nemesis->getId());
			buf = nboPackUShort(buf, uint16_t(RogueTeam));
			composePrompt = "Send to ";
			composePrompt += nemesis->getCallSign();
			composePrompt += ": ";
		}
	}
	else {
		return "usage: send {all|team|nemesis}";
	}

	if (myTank != NULL && !composePrompt.empty()) {
		// to send to a player use:
		//   buf = myTank->getId().pack(buf);
		//   buf = nboPackUShort(buf, uint16_t(RogueTeam));
		MSGMGR->get("messages")->startComposing(composePrompt,
								&onSendComposedMessage, NULL, NULL);
	}

	return BzfString();
}

static BzfString	cmdPause(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 0)
		return "usage: pause";

	if (myTank != NULL && !pausedByUnmap && myTank->isAlive()) {
		if (myTank->isPaused()) {
			myTank->setPause(false);
			MSGMGR->insert("messages", "Resumed");

			// restore the sound
			BZDB->set("audioMute", "0");

			// grab mouse
			updateGrab();
		}
		else if (pauseCountdown > 0.0f) {
			pauseCountdown = 0.0f;
			MSGMGR->insert("alertInfo", "Pause cancelled", NULL);
		}
		else {
			pauseCountdown = 3.0f;
			MSGMGR->insert("alertInfo", BzfString::format("Pausing in %d",
								static_cast<int>(pauseCountdown + 0.99f)),
								NULL);
		}
	}

	return BzfString();
}

/* XXX -- for testing forced recreation of OpenGL context
      case 'o':
		if (pressed) {
		  // destroy context and recreate it
		  OpenGLGState::freeContext();
		  mainWindow->freeContext();
		  mainWindow->makeContext();
		  OpenGLGState::initContext();

		  // cause sun/moon to be repositioned immediately
		  BZDB->touch("timeClock");
		}
		break;
*/

struct CommandListItem {
public:
	const char*			name;
	CommandManager::CommandFunction func;
	const char*			help;
};
static const CommandListItem commandList[] = {
	{ "fire",		&cmdFire,		"fire [stop]:  start/stop firing" },
	{ "drop",		&cmdDrop,		"drop:  drop a flag" },
	{ "identify",	&cmdIdentify,	"identify:  identify/lock-on-to player in view" },
	{ "jump",		&cmdJump,		"jump:  make player jump" },
	{ "send",		&cmdSend,		"send {all|team|nemesis}:  start composing a message" },
	{ "pause",		&cmdPause,		"pause:  pause/resume" }
};
// ---- commands ----


static void				doEvent(BzfDisplay* display)
{
	BzfEvent event;
	if (!display->getEvent(event))
		return;

	switch (event.type) {
		case BzfEvent::Quit:
			CommandsStandard::quit();
			break;

		case BzfEvent::Redraw:
			// ignore -- we constantly redraw everything anyway
			break;

		case BzfEvent::Resize:
			// ignore
			break;

		case BzfEvent::Map:
			// window has been mapped.  this normally occurs when the game
			// is uniconified.  if the player was paused because of an unmap
			// then resume.
			if (pausedByUnmap) {
				pausedByUnmap = false;
				pauseCountdown = 0.0f;
				if (myTank && myTank->isAlive() && myTank->isPaused()) {
					myTank->setPause(false);
					MSGMGR->insert("messages", "Resumed");
				}
			}

			// restore the resolution we want if full screen
			BZDB->touch("windowResolution");

			// restore the sound
			BZDB->set("audioMute", "0");

			// window is now mapped
			unmapped = false;

			// grab mouse again
			updateGrab();
			break;

		case BzfEvent::Unmap:
			// begin pause countdown when unmapped if:  we're not already
			// paused because of an unmap (shouldn't happen), we're not
			// already counting down to pausing, we're alive, and we're not
			// already paused.
			if (!pausedByUnmap && pauseCountdown == 0.0f &&
		 		myTank && myTank->isAlive() && !myTank->isPaused()) {
				// get ready to pause (no cheating through instantaneous pausing)
				pauseCountdown = 3.0f;

				// set this even though we haven't really paused yet
				pausedByUnmap = true;
			}

			// reset resolution (this does nothing if resolution was unchanged)
			display->setDefaultResolution();

			// turn off the sound
			BZDB->set("audioMute", "1");

			// window is now unmapped
			unmapped = true;

			// ungrab the mouse
			updateGrab();
			break;

		case BzfEvent::KeyUp:
			doKey(event.keyUp, false);
			break;

		case BzfEvent::KeyDown:
			doKey(event.keyDown, true);
			break;

		case BzfEvent::MouseMove:
			if (myTank && myTank->isAlive() && myTank->isKeyboardMoving()) {
				myTank->setKeyboardMoving(false);
				showKeyboardStatus();
			}
			break;
	}
}

//
// misc utility routines
//

boolean					isRemotePlayer(PlayerId id)
{
	return (id != NoPlayer) && (id != myTank->getId());
}

Player*					lookupPlayer(PlayerId id)
{
	// check my tank first
	if (myTank->getId() == id)
		return myTank;
	if (id == NoPlayer)
		return NULL;
	return player[id];
}

static BaseLocalPlayer*	getLocalPlayer(const PlayerId& id)
{
	if (myTank->getId() == id) return myTank;
	return NULL;
}

static ServerLink*		lookupServer(const Player* player)
{
	const PlayerId& id = player->getId();
	if (myTank->getId() == id) return serverLink;
	return NULL;
}

static void				addMessage(const Player* player,
								const BzfString& msg,
								const GLfloat* color = NULL)
{
	BzfString fullMessage;
	if (player) {
		fullMessage += player->getCallSign();
#ifndef BWSUPPORT
    if (color == NULL) {
#endif
      fullMessage += " (";
      fullMessage += Team::getName(player->getTeam());
      fullMessage += ")";
#ifndef BWSUPPORT
    }
#endif
    fullMessage += ": ";
	}
	fullMessage += msg;
#if 0
	if (player && color == NULL)
		color = Team::getRadarColor(player->getTeam());
#endif
	MSGMGR->insert("messages", fullMessage, color);
}

static void				updateNumPlayers()
{
	int i, numPlayers[NumTeams];
	for (i = 0; i < NumTeams; i++)
		numPlayers[i] = 0;
	for (i = 0; i < maxPlayers; i++)
		if (player[i])
			numPlayers[player[i]->getTeam()]++;
	if (myTank)
		numPlayers[myTank->getTeam()]++;
// FIXME -- set BZDB?
//  controlPanel->setTeamCounts(numPlayers);
}

static void				updateHighScores()
{
	/* check scores to see if my team and/or have the high score.  change
   * `>= bestScore' to `> bestScore' if you want to share the number
   * one spot. */
	bool anyPlayers = false;
	int i;
	for (i = 0; i < maxPlayers; i++)
		if (player[i]) {
			anyPlayers = true;
			break;
		}
/* FIXME -- this turns off score blinking
	if (!anyPlayers) {
		hud->setPlayerHasHighScore(false);
		hud->setTeamHasHighScore(false);
		return;
	}
*/

	bool haveBest = true;
	int bestScore = myTank ? myTank->getScore() : 0;
	for (i = 0; i < maxPlayers; i++)
		if (player[i] && player[i]->getScore() >= bestScore) {
			haveBest = false;
			break;
		}
/* FIXME -- high score blinking
	hud->setPlayerHasHighScore(haveBest);
*/

	if (myTank && myTank->getTeam() != RogueTeam) {
		const Team& myTeam = World::getWorld()->getTeam(int(myTank->getTeam()));
		bestScore = myTeam.won - myTeam.lost;
		haveBest = true;
		for (i = 0; i < NumTeams; i++) {
			if (i == int(myTank->getTeam())) continue;
			const Team& team = World::getWorld()->getTeam(i);
			if (team.activeSize > 0 && team.won - team.lost >= bestScore) {
				haveBest = false;
				break;
			}
		}
/* FIXME -- high score blinking
    hud->setTeamHasHighScore(haveBest);
*/
	}
	else {
/* FIXME -- high score blinking
    hud->setTeamHasHighScore(false);
*/
	}
}

static void				updateFlag()
{
	const FlagId id = (myTank != NULL) ? myTank->getFlag() : NoFlag;
	if (id == NoFlag) {
		static const float hudColor[] = { 1.0f, 0.625f, 0.125f };
		HUDMGR->setColor(hudColor);
		MSGMGR->get("alertFlag")->clear();
		BZDB->unset("outputFlag");
	}
	else {
		HUDMGR->setColor(Flag::getColor(id));
		MSGMGR->insert("alertFlag", Flag::getName(id),
								Flag::getType(id) == FlagSticky ?
								warningColor : NULL);
		BZDB->set("outputFlag", Flag::getName(id));
	}

	// update flag help
	MSGMGR->insert("flagHelp", Flag::getHelp(id), NULL);
}

void					notifyBzfKeyMapChanged()
{
/* FIXME -- this sets the name of the key used to start playing
	hud->setRestartKeyLabel(BzfKeyMap::getKeyEventString(
										keymap.get(BzfKeyMap::Identify)));
*/
}

//
// server message handling
//

static Player*			addPlayer(PlayerId id, void* msg, int showMessage)
{
	uint16_t team, type, wins, losses;
	char callsign[CallSignLen];
	char email[EmailLen];
	msg = nboUnpackUShort(msg, type);
	msg = nboUnpackUShort(msg, team);
	msg = nboUnpackUShort(msg, wins);
	msg = nboUnpackUShort(msg, losses);
	msg = nboUnpackString(msg, callsign, CallSignLen);
	msg = nboUnpackString(msg, email, EmailLen);

	if (player[id] != NULL) {
		printError("Server error when adding player");
		serverError = true;
		return NULL;
	}


	// add player
	if (PlayerType(type) == TankPlayer || PlayerType(type) == ComputerPlayer) {
		player[id] = new RemotePlayer(id, TeamColor(team), callsign, email);
		player[id]->changeScore(short(wins), short(losses));
	}

	if (showMessage) {
		BzfString message("joining as a");
		switch (PlayerType(type)) {
			case TankPlayer:
				message += " tank";
				break;
			case JAFOPlayer:
				message += "n observer";
				break;
			case ComputerPlayer:
				message += " robot tank";
				break;
			default:
				message += "n unknown type";
				break;
		}
		if (!player[id]) {
			BzfString name(callsign);
			name += ": ";
			name += message;
			message = name;
		}
		addMessage(player[id], message);
	}

	// restore player's local score if player had been playing earlier
	world->reviveDeadPlayer(player[id]);

	return player[id];
}

static void				handleServerMessage(bool human, uint16_t code,
												uint16_t, void* msg)
{
	bool checkScores = false;
	switch (code) {

		case MsgUDPLinkRequest:
			uint16_t portNo;
			msg = nboUnpackUShort(msg, portNo);
			printError("Server sent downlink endpoint information, port %d",portNo);
     		serverLink->setUDPRemotePort(portNo);
			break;

		case MsgSuperKill:
			printError("Server forced a disconnect");
			serverError = true;
			break;

		case MsgTimeUpdate: {
			uint16_t timeLeft;
			msg = nboUnpackUShort(msg, timeLeft);
/* FIXME -- set BZDB
      hud->setTimeLeft(timeLeft);
*/
			if (timeLeft == 0) {
				gameOver = true;
				myTank->explodeTank();
				MSGMGR->insert("alertGameOver", "Time Expired", warningColor);
			}
			break;
    	}

		case MsgScoreOver: {
			// unpack packet
			PlayerId id;
			uint16_t team;
			msg = nboUnpackUByte(msg, id);
			msg = nboUnpackUShort(msg, team);
			Player* player = lookupPlayer(id);

			// make a message
			BzfString msg;
			if (team == (uint16_t)NoTeam) {
				// a player won
				if (player) {
					msg = player->getCallSign();
					msg += " (";
					msg += Team::getName(player->getTeam());
					msg += ")";
				}
				else {
					msg = "[unknown player]";
				}
			}
			else {
				// a team won
				msg = Team::getName(TeamColor(team));
			}
			msg += " won the game";

			gameOver = true;
/* FIXME -- unset BZDB
			hud->setTimeLeft(-1);
*/
			myTank->explodeTank();
			MSGMGR->insert("alertGameOver", msg, warningColor);
			break;
		}

		case MsgAddPlayer: {
			PlayerId id;
			msg = nboUnpackUByte(msg, id);
			if (id == myTank->getId())
				break;			// that's odd -- it's me!
			addPlayer(id, msg, true);
			updateNumPlayers();
			checkScores = true;
			break;
		}

		case MsgRemovePlayer: {
			PlayerId id;
			msg = nboUnpackUByte(msg, id);
			if (isRemotePlayer(id)) {
				addMessage(player[id], "signing off");
				world->addDeadPlayer(player[id]);
				delete player[id];
				player[id] = NULL;
				updateNumPlayers();
				checkScores = true;
			}
			break;
		}

		case MsgFlagUpdate: {
			uint16_t flagIndex;
			msg = nboUnpackUShort(msg, flagIndex);
			msg = world->getFlag(int(flagIndex)).unpack(msg);
			world->initFlag(int(flagIndex));
			break;
		}

		case MsgTeamUpdate: {
			uint16_t team;
			msg = nboUnpackUShort(msg, team);
			msg = teams[int(team)].unpack(msg);
			updateNumPlayers();
			checkScores = true;
			break;
		}

		case MsgAlive: {
			PlayerId id;
			float pos[3], forward[3];
			msg = nboUnpackUByte(msg, id);
			msg = nboUnpackVector(msg, pos);
			msg = nboUnpackVector(msg, forward);
			if (isRemotePlayer(id)) {
				static const float zero[3] = { 0.0f, 0.0f, 0.0f };
				Player* tank = lookupPlayer(id);
				tank->setStatus(Player::Alive);
				tank->move(pos, atan2f(forward[1], forward[0]));
				tank->setVelocity(zero);
				tank->setAngularVelocity(0.0f);
				tank->setDeadReckoning();
				playWorldSound(SFX_POP, pos[0], pos[1], pos[2], true);
			}
			break;
		}

		case MsgKilled: {
			PlayerId victim, killer;
			int16_t shotId;
			msg = nboUnpackUByte(msg, victim);
			msg = nboUnpackUByte(msg, killer);
			msg = nboUnpackShort(msg, shotId);
			BaseLocalPlayer* victimLocal = getLocalPlayer(victim);
			BaseLocalPlayer* killerLocal = getLocalPlayer(killer);
			Player* victimPlayer = lookupPlayer(victim);
			Player* killerPlayer = lookupPlayer(killer);
			if (victimLocal) {
				// uh oh, local player is dead
				if (victimLocal->isAlive()) {
					gotBlowedUp(victimLocal, GotKilledMsg, killer);
				}
			}
			else if (victimPlayer) {
				victimPlayer->setExplode(TimeKeeper::getTick());
				const float* pos = victimPlayer->getPosition();
				playWorldSound(SFX_EXPLOSION, pos[0], pos[1], pos[2],
													  killerLocal == myTank);
				float explodePos[3];
				explodePos[0] = pos[0];
				explodePos[1] = pos[1];
				explodePos[2] = pos[2] + MuzzleHeight;
				addTankExplosion(explodePos);
			}
			if (killerLocal) {
				// local player did it
				if (shotId >= 0) {
					// terminate the shot
					killerLocal->endShot(shotId, true);
				}
				if (killerLocal != victimPlayer) {
					if (victimPlayer->getTeam() == killerLocal->getTeam() &&
						killerLocal->getTeam() != RogueTeam) {
						if (killerLocal == myTank) {
							 MSGMGR->insert("alertInfo",
										"Don't shoot teammates!!!", warningColor);
						 	playLocalSound( SFX_KILL_TEAM );
						}
						// teammate
						killerLocal->changeScore(0, 1);
					}
					else {
						// enemy
						killerLocal->changeScore(1, 0);
					}
				}
			}
			// handle my personal score against other players
			if ((killerPlayer == myTank || victimPlayer == myTank) &&
				!(killerPlayer == myTank && victimPlayer == myTank)) {
				if (killerLocal == myTank) {
					if (victimPlayer)
						victimPlayer->changeLocalScore(1, 0);
					myTank->setNemesis(victimPlayer);
				}
				else {
					if (killerPlayer)
						killerPlayer->changeLocalScore(0, 1);
					myTank->setNemesis(killerPlayer);
				}
			}

			// add message
			if (human && victimPlayer) {
				if (killerPlayer == victimPlayer)
					addMessage(victimPlayer, "blew myself up");
				else if (!killerPlayer)
					addMessage(victimPlayer, "destroyed by <unknown>");
				else if (killerPlayer->getTeam() == victimPlayer->getTeam() &&
						killerPlayer->getTeam() != RogueTeam) {
					BzfString message("destroyed by teammate ");
					message += killerPlayer->getCallSign();
					addMessage(victimPlayer, message);
				}
				else {
					BzfString message("destroyed by ");
					message += killerPlayer->getCallSign();
					addMessage(victimPlayer, message);
				}
			}

			// blow up if killer has genocide flag and i'm on same team as victim
			// (and we're not rogues)
			if (human && killerPlayer && victimPlayer &&
					victimPlayer != myTank &&
					victimPlayer->getTeam() == myTank->getTeam() &&
					myTank->getTeam() != RogueTeam && shotId >= 0) {
				// now see if shot was fired with a GenocideFlag
				const ShotPath* shot = killerPlayer->getShot(int(shotId));
				if (shot && shot->getFlag() == GenocideFlag) {
					gotBlowedUp(myTank, GenocideEffect, killerPlayer->getId());
				}
			}

			checkScores = true;
			break;
		}

		case MsgGrabFlag: {
			PlayerId id;
			uint16_t flagIndex;
			msg = nboUnpackUByte(msg, id);
			msg = nboUnpackUShort(msg, flagIndex);
			msg = world->getFlag(int(flagIndex)).unpack(msg);
			Player* tank = lookupPlayer(id);
			if (!tank)
				break;

			// player now has flag
			tank->setFlag(world->getFlag(flagIndex).id);
			if (tank == myTank) {
				// not allowed to grab it if not on the ground
				if (myTank->getLocation() != LocalPlayer::OnGround &&
					myTank->getLocation() != LocalPlayer::OnBuilding) {
					serverLink->sendDropFlag(myTank->getPosition());
				}
				else {
					// grabbed flag
					playLocalSound(Flag::getType(myTank->getFlag()) != FlagSticky ?
											SFX_GRAB_FLAG : SFX_GRAB_BAD);
					updateFlag();
				}
			}
			else if (tank && tank->getTeam() != myTank->getTeam() &&
				int(world->getFlag(flagIndex).id) == int(myTank->getTeam())) {
				MSGMGR->insert("alertInfo", "Flag Alert!!!", warningColor);
				playLocalSound(SFX_ALERT);
			}
			if (tank) {
				BzfString message("grabbed ");
				message += Flag::getName(tank->getFlag());
				message += " flag";
				addMessage(tank, message);
			}
			break;
		}

		case MsgDropFlag: {
			PlayerId id;
			uint16_t flagIndex;
			msg = nboUnpackUByte(msg, id);
			msg = nboUnpackUShort(msg, flagIndex);
			msg = world->getFlag(int(flagIndex)).unpack(msg);
			Player* tank = lookupPlayer(id);
			if (!tank)
				break;
			handleFlagDropped(tank);
			break;
		}

		case MsgCaptureFlag: {
			PlayerId id;
			uint16_t flagIndex, team;
			msg = nboUnpackUByte(msg, id);
			msg = nboUnpackUShort(msg, flagIndex);
			msg = nboUnpackUShort(msg, team);
			Player* capturer = lookupPlayer(id);
			int capturedTeam = int(world->getFlag(int(flagIndex)).id);

			// player no longer has flag
			if (capturer) {
				capturer->setFlag(NoFlag);
				if (capturer == myTank) {
					updateFlag();
				}

				// add message
				if (int(capturer->getTeam()) == capturedTeam) {
					BzfString message("took my flag into ");
					message += Team::getName(TeamColor(team));
					message += " territory";
					addMessage(capturer, message);
				}
				else {
					BzfString message("captured ");
					message += Team::getName(TeamColor(capturedTeam));
					message += "'s flag";
					addMessage(capturer, message);
				}
			}

			// play sound -- if my team is same as captured flag then my team lost,
			// but if I'm on the same team as the capturer then my team won.
			if (capturedTeam == int(myTank->getTeam()))
				playLocalSound(SFX_LOSE);
			else if (capturer->getTeam() == myTank->getTeam())
				playLocalSound(SFX_CAPTURE);

			// blow up if my team flag captured
			if (capturedTeam == int(myTank->getTeam())) {
				gotBlowedUp(myTank, GotCaptured, id);
				restartOnBase = true;
			}

			// everybody who's alive on capture team will be blowing up
			// but we're not going to get an individual notification for
			// each of them, so add an explosion for each now.  don't
			// include me, though;  I already blew myself up.
			for (int i = 0; i < maxPlayers; i++) {
				if (player[i] &&
					player[i]->isAlive() &&
					player[i]->getTeam() == capturedTeam) {
					const float* pos = player[i]->getPosition();
					playWorldSound(SFX_EXPLOSION, pos[0], pos[1], pos[2], false);
					float explodePos[3];
					explodePos[0] = pos[0];
					explodePos[1] = pos[1];
					explodePos[2] = pos[2] + MuzzleHeight;
					addTankExplosion(explodePos);
				}
			}

			checkScores = true;
			break;
		}

		case MsgShotBegin: {
			FiringInfo firingInfo;
			msg = firingInfo.unpack(msg);
			for (int i = 0; i < maxPlayers; i++)
				if (player[i] && player[i]->getId() == firingInfo.shot.player) {
					const float* pos = firingInfo.shot.pos;
					player[i]->addShot(firingInfo);
					if (human) {
						if (firingInfo.flag == ShockWaveFlag)
							playWorldSound(SFX_SHOCK, pos[0], pos[1], pos[2]);
						else if (firingInfo.flag == LaserFlag)
							playWorldSound(SFX_LASER, pos[0], pos[1], pos[2]);
						else if (firingInfo.flag == GuidedMissileFlag)
							playWorldSound(SFX_MISSILE, pos[0], pos[1], pos[2]);
						else
							playWorldSound(SFX_FIRE, pos[0], pos[1], pos[2]);
					}
					break;
				}
			break;
		}

		case MsgShotEnd: {
			PlayerId id;
			int16_t shotId;
			uint16_t reason;
			msg = nboUnpackUByte(msg, id);
			msg = nboUnpackShort(msg, shotId);
			msg = nboUnpackUShort(msg, reason);
			BaseLocalPlayer* localPlayer = getLocalPlayer(id);

			if (localPlayer)
				localPlayer->endShot(int(shotId), false, reason == 0);
			else for (int i = 0; i < maxPlayers; i++)
				if (player[i] && player[i]->getId() == id) {
					player[i]->endShot(int(shotId), false, reason == 0);
					break;
				}
			break;
		}

		case MsgScore: {
			PlayerId id;
			uint16_t wins, losses;
			msg = nboUnpackUByte(msg, id);
			msg = nboUnpackUShort(msg, wins);
			msg = nboUnpackUShort(msg, losses);
			// only update score of remote players (local score is already known)
			for (int i = 0; i < maxPlayers; i++)
				if (player[i] && player[i]->getId() == id) {
					player[i]->changeScore(wins - player[i]->getWins(),
										losses - player[i]->getLosses());
					break;
				}
			break;
		}

		case MsgTeleport: {
			PlayerId id;
			uint16_t from, to;
			msg = nboUnpackUByte(msg, id);
			msg = nboUnpackUShort(msg, from);
			msg = nboUnpackUShort(msg, to);
			Player* tank = lookupPlayer(id);
			if (tank && tank != myTank) {
				int face;
				const Teleporter* teleporter = world->getTeleporter(int(to), face);
				const float* pos = teleporter->getPosition();
				tank->setTeleport(TimeKeeper::getTick(), short(from), short(to));
				playWorldSound(SFX_TELEPORT, pos[0], pos[1], pos[2]);
			}
			break;
		}

		case MsgMessage: {
			PlayerId src;
			PlayerId dst;
			uint16_t team;
			msg = nboUnpackUByte(msg, src);
			msg = nboUnpackUByte(msg, dst);
			msg = nboUnpackUShort(msg, team);
			Player* srcPlayer = lookupPlayer(src);
			Player* dstPlayer = lookupPlayer(dst);
			if (srcPlayer == myTank || dstPlayer == myTank || (!dstPlayer &&
				(int(team) == int(RogueTeam) ||
				int(team) == int(myTank->getTeam())))) {
				// message is for me
				BzfString fullMsg;
				if (int(team) != int(RogueTeam)) {
#ifdef BWSUPPORT
					fullMsg = "[to ";
					fullMsg += Team::getName(TeamColor(team));
					fullMsg += "] ";
#else
					fullMsg = "[Team] ";
#endif
				}
				if (dstPlayer) {
					if (dstPlayer==myTank && srcPlayer==myTank) {
						fullMsg=(const char*)msg;
					} else {
						fullMsg="[";
						if (srcPlayer == myTank) {
							fullMsg += "->";
							fullMsg += dstPlayer ? dstPlayer->getCallSign() : "(UNKNOWN)";
						} else {
							fullMsg += srcPlayer ? srcPlayer->getCallSign() : "(UNKNOWN)";
							fullMsg += "->";
							if (srcPlayer)
								myTank->setNemesis(srcPlayer);
						}
						fullMsg += "] ";
						fullMsg += (const char*)msg;
					}
					addMessage(NULL, fullMsg, Team::getRadarColor(RogueTeam));
					break;
				}
				if (!srcPlayer) {
					/* may unkown not harm us */
					fullMsg = "(UNKNOWN) ";
					fullMsg += (const char*)msg;

					addMessage(NULL, fullMsg, Team::getRadarColor(RogueTeam));
					break;
				}
				if (!strncmp((char *)msg,"CLIENTQUERY",strlen("CLIENTQUERY"))) {
					char messageBuffer[MessageLen];
					memset(messageBuffer, 0, MessageLen);
					sprintf(messageBuffer,"Version %d.%d%c%d+UDP",
						(VERSION / 10000000) % 100, (VERSION / 100000) % 100,
						(char)('a' - 1 + (VERSION / 1000) % 100), VERSION % 1000);

					nboPackString(messageMessage + PlayerIdPLen + 2, messageBuffer, MessageLen);
					serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
					const GLfloat* msgColor;
					if (int(team) == int(RogueTeam) || srcPlayer->getTeam() == NoTeam)
						msgColor = Team::getRadarColor(RogueTeam);
					else
						msgColor = Team::getRadarColor(srcPlayer->getTeam());

					addMessage(srcPlayer,"[Sent versioninfo per request]", msgColor);
					break;
				}
				fullMsg += (const char*)msg;
				const GLfloat* msgColor;
				if (srcPlayer->getTeam() == NoTeam)
					msgColor = Team::getRadarColor(RogueTeam);
				else
					msgColor = Team::getRadarColor(srcPlayer->getTeam());
				addMessage(srcPlayer, fullMsg, msgColor);

				// HUD one line display
				fullMsg = srcPlayer->getCallSign();
#ifdef BWSUPPORT
				fullMsg += " (";
				fullMsg += Team::getName(srcPlayer->getTeam());
				fullMsg += ")";
#endif
				fullMsg += ": ";
				fullMsg += (const char*)msg;
				MSGMGR->insert("alertInfo", fullMsg, NULL);
			}
			break;
		}

		case MsgAcquireRadio: {
			PlayerId id;
			uint16_t types;
			msg = nboUnpackUByte(msg, id);
			msg = nboUnpackUShort(msg, types);
			Player* tank = lookupPlayer(id);
			if (tank == myTank) {
				// FIXME -- i now have radio
			}
			else if (tank) {
				// FIXME -- start receiving from player
			}
			break;
		}

		case MsgReleaseRadio: {
			PlayerId id;
			msg = nboUnpackUByte(msg, id);
			Player* tank = lookupPlayer(id);
			if (tank == myTank) {
				// FIXME -- i lost the radio, disable transmission
			}
			else if (tank) {
				// FIXME -- stop receiving from player
			}
			break;
		}

		// inter-player relayed message
		case MsgPlayerUpdate:
		case MsgGMUpdate:
		case MsgAudio:
		case MsgVideo:
			handlePlayerMessage(code, 0, msg);
			break;
	}

	if (checkScores)
		updateHighScores();
}

//
// player message handling
//

static void				handlePlayerMessage(uint16_t code, uint16_t,
																void* msg)
{
	switch (code) {
		case MsgPlayerUpdate: {
			PlayerId id;
			msg = nboUnpackUByte(msg, id);
			Player* tank = lookupPlayer(id);
			if (!tank || tank == myTank) break;
			short oldStatus = tank->getStatus();
			tank->unpack(msg);
			short newStatus = tank->getStatus();
			if ((oldStatus & short(Player::Paused)) !=
								(newStatus & short(Player::Paused)))
				addMessage(tank, (tank->getStatus() & Player::Paused) ?
												"Paused" : "Resumed");
			if ((oldStatus & short(Player::Exploding)) == 0 &&
				(newStatus & short(Player::Exploding)) != 0) {
				// player has started exploding and we haven't gotten killed
				// message yet -- set explosion now, play sound later (when we
				// get killed message).  status is already !Alive so make player
				// alive again, then call setExplode to kill him.
				tank->setStatus(newStatus | short(Player::Alive));
				tank->setExplode(TimeKeeper::getTick());
			}
			break;
		}

		case MsgGMUpdate: {
			ShotUpdate shot;
			msg = shot.unpack(msg);
			Player* tank = lookupPlayer(shot.player);
			if (!tank || tank == myTank) break;
			RemotePlayer* remoteTank = (RemotePlayer*)tank;
			RemoteShotPath* shotPath =
				(RemoteShotPath*)remoteTank->getShot(shot.id);
			if (shotPath) shotPath->update(shot, code, msg);
			PlayerId targetId;
			msg = nboUnpackUByte(msg, targetId);
			Player* targetTank = lookupPlayer(targetId);
			if (targetTank && (targetTank == myTank)) {
				static TimeKeeper lastLockMsg;
				if (TimeKeeper::getTick() - lastLockMsg > 0.75) {
					playWorldSound(SFX_LOCK, shot.pos[0], shot.pos[1], shot.pos[2]);
					lastLockMsg=TimeKeeper::getTick();
					addMessage(tank, "locked on me");
				}
			}
			break;
		}
	}
}

//
// message handling
//

static void				doMessages()
{
	char msg[MaxPacketLen];
	uint16_t code, len;
	int e = 0;

	// handle server messages
	if (serverLink) {
		while (!serverError && (e = serverLink->read(code, len, msg, 0)) == 1)
			handleServerMessage(true, code, len, msg);
		if (e == -2) {
			printError("Server communication error");
			serverError = true;
			return;
		}
	}
}

//
// local update utility functions
//

static float				minSafeRange(float angleCosOffBoresight,
								     double fractionOfTries)
{
	// anything farther than this much from dead-center is okay to
	// place at MinRange
	static const float		SafeAngle = 0.5f;				// cos(angle)

	// don't ever place within this range
	static const float		MinRange = 2.5f * ShotSpeed;	// meters

	// anything beyond this range is okay at any angle
	static const float		MaxRange = 5.0f * ShotSpeed;	// meters

	// if more than SafeAngle off boresight then MinRange is okay
	if (angleCosOffBoresight < SafeAngle)
		return MinRange;

	// ramp up to MaxRange as target comes to dead center
	const float f = (angleCosOffBoresight - SafeAngle) / (1.0f - SafeAngle);
	return (float)((MinRange + f * (MaxRange - MinRange)) * (1.0f - fractionOfTries * 0.6));
}

static void				restartPlaying()
{
	// maximum tries to find a safe place
	static const int	MaxTries = 1000;

	// minimum time before an existing shot can hit us
	static const float	MinShotImpact = 2.0f;				// seconds

	// restart my tank
	float startPoint[3];
	float startAzimuth;
	bool located = false;
	int locateCount = 0;

	// check for valid starting (no unfair advantage to player or enemies)
	// should find a good location in a few tries... locateCount is a safety
	// check that will probably be invoked when restarting on the team base
	// if the enemy is loitering around waiting for players to reappear.
	// also have to make sure new position isn't in a building;  that must
	// be enforced no matter how many times we need to try new locations.
	startPoint[2] = 0.0f;
	do {
		do {
			if (restartOnBase) {
				const float* base = world->getBase(int(myTank->getTeam()));
				const float x = (base[4] - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
				const float y = (base[5] - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
				startPoint[0] = base[0] + x * cosf(base[3]) - y * sinf(base[3]);
				startPoint[1] = base[1] + x * sinf(base[3]) + y * cosf(base[3]);
				if(base[2] != 0) {
					startPoint[2] = base[2] + 1;
				} else {
					startPoint[2] = base[2];
				}
			}
			else {
				if (world->allowTeamFlags()) {
					startPoint[0] = 0.4f * WorldSize * ((float)bzfrand() - 0.5f);
					startPoint[1] = 0.4f * WorldSize * ((float)bzfrand() - 0.5f);
				}
				else {
					startPoint[0] = (WorldSize - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
					startPoint[1] = (WorldSize - 2.0f * TankRadius) * ((float)bzfrand() - 0.5f);
				}
			}
			startAzimuth = 2.0f * M_PI * (float)bzfrand();
		} while (world->inBuilding(startPoint, 2.0f * TankRadius));

		// get info on my tank
		const TeamColor myColor = myTank->getTeam();
		const float myCos = cosf(-startAzimuth);
		const float mySin = sinf(-startAzimuth);

		// check each enemy tank
		located = true;
		for (int i = 0; i < maxPlayers; i++) {
			// ignore missing player
			if (!player[i]) continue;

			// test against all existing shots of all players except mine
			// (mine don't count because I can't come alive before all my
			// shots have expired anyway)
			const int maxShots = World::getWorld()->getMaxShots();
			for (int j = 0; j < maxShots; j++) {
				// get shot and ignore non-existent ones
				ShotPath* shot = player[i]->getShot(j);
				if (!shot) continue;

				// get shot's current position and velocity and see if it'll
				// hit my tank earlier than MinShotImpact.  use something
				// larger than the actual tank size to give some leeway.
				const Ray ray(shot->getPosition(), shot->getVelocity());
				const float t = Intersect::timeRayHitsBlock(
								ray, startPoint, startAzimuth,
								4.0f * TankLength, 4.0f * TankWidth,
								2.0f * TankHeight);
				if (t >= 0.0f && t < MinShotImpact) {
					located = false;
					break;
				}
			}
			if (!located) break;

			// test against living enemy tanks
			if (!player[i]->isAlive() ||
		  (myColor != RogueTeam  && player[i]->getTeam() == myColor)) continue;

			// compute enemy position in my local coordinate system
			const float* enemyPos = player[i]->getPosition();
			const float enemyX = myCos * (enemyPos[0] - startPoint[0]) -
						   mySin * (enemyPos[1] - startPoint[1]);
			const float enemyY = mySin * (enemyPos[0] - startPoint[0]) +
						   myCos * (enemyPos[1] - startPoint[1]);

			// get distance and angle of enemy from boresight
			const float enemyDist = hypotf(enemyX, enemyY);
			const float enemyCos = enemyX / enemyDist;

			// don't allow tank placement if enemy tank is +/- 30 degrees of
			// my boresight and in firing range (our unfair advantage)
			if (enemyDist < minSafeRange(enemyCos,double(locateCount)/MaxTries)) {
				located = false;
				break;
			}

			// compute my position in enemy coordinate system
			// cos = enemyUnitVect[0], sin = enemyUnitVect[1]
			const float* enemyUnitVect = player[i]->getForward();
			const float myX = enemyUnitVect[0] * (startPoint[0] - enemyPos[0]) -
						enemyUnitVect[1] * (startPoint[1] - enemyPos[1]);
			const float myY = enemyUnitVect[1] * (startPoint[0] - enemyPos[0]) +
						enemyUnitVect[0] * (startPoint[1] - enemyPos[1]);

			// get distance and angle of enemy from boresight
			const float myDist = hypotf(myX, myY);
			const float myCos = myX / myDist;

			// don't allow tank placement if my tank is +/- 30 degrees of
			// the enemy's boresight and in firing range (enemy's unfair advantage)
			if (myDist < minSafeRange(myCos,double(locateCount)/MaxTries)) {
				located = false;
				break;
			}
		}
	} while (!located && ++locateCount <= MaxTries);

	// restart the tank
	myTank->restart(startPoint, startAzimuth);
	serverLink->sendAlive(myTank->getPosition(), myTank->getForward());
	restartOnBase = false;
	firstLife = false;
	playLocalSound(SFX_POP);

	// warp mouse to center of HUD so player feels under control.
	// HUD center is flipped wrt to window coordinates.
	int x, y, w, h;
	HUDMGR->getCenter(x, y);
	mainWindow->getSize(w, h);
	mainWindow->warpMouse(x, h - 1 - y);
}

static void				updateFlags(float dt)
{
	for (int i = 0; i < numFlags; i++) {
		Flag& flag = world->getFlag(i);
		if (flag.status == FlagOnTank) {
			// position flag on top of tank
			Player* tank = lookupPlayer(flag.owner);
			if (tank) {
				const float* pos = tank->getPosition();
				flag.position[0] = pos[0];
				flag.position[1] = pos[1];
				flag.position[2] = pos[2] + TankHeight;
			}
		}
		world->updateFlag(i, dt);
	}
}

bool					addExplosion(const float* _pos,
								float size, float duration)
{
	// ignore if no explosion available
	if (explosion == NULL)
		return false;

	// make a parameter node for the explosion
	SceneNodeParameters* params = new SceneNodeParameters;
	params->src.resize(1);    params->src.set(0, "time");
	params->dst.resize(1);    params->dst.set(0, "time");
	params->scale.resize(1);  params->scale.set(0, 1.0f / duration);
	params->bias.resize(1);   params->bias.set(0, -wallClock);

	// choose number of explosion billboards
	int boom = 1;
	if (size >= 3.0f * TankLength)
		boom += static_cast<int>(bzfrand() * 4.0) + 3;

	// make the explosions
	for (; boom > 0; --boom) {
		// transform the explosion
		SceneNodeTransform* xform = new SceneNodeTransform;
		if (boom == 1)
			xform->translate.push(_pos[0], _pos[1], _pos[2]);
		else
			xform->translate.push(
						_pos[0] + static_cast<float>(bzfrand() * 12.0 - 6.0),
						_pos[1] + static_cast<float>(bzfrand() * 12.0 - 6.0),
						_pos[2] + static_cast<float>(bzfrand() * 10.0));
		xform->rotate.push(0.0f, 0.0f, 1.0f,
								static_cast<float>(2.0 * M_PI * bzfrand()));
		xform->scale.push(0.5f * size, 0.5f * size, 0.5f * size);

		// add explosion to group
		params->pushChild(xform);
		xform->pushChild(explosion);
		xform->unref();
	}

	// save explosion group
	// FIXME -- should save animation node for later updating
	ExplosionInfo info;
	info.duration = duration;
	info.time     = 0.0f;
	info.node     = params;
	explosions.push_back(info);

	return true;
}

void					addTankExplosion(const float* pos)
{
	addExplosion(pos, 3.5f * TankLength, 1.2f);
}

void					addShotExplosion(const float* pos)
{
	// only play explosion sound if you see an explosion
	if (addExplosion(pos, 1.2f * TankLength, 0.8f))
		playWorldSound(SFX_SHOT_BOOM, pos[0], pos[1], pos[2]);
}

void					addShotPuff(const float* pos)
{
	addExplosion(pos, 0.3f * TankLength, 0.8f);
}

static void				updateExplosions(float dt)
{
	// update time of all explosions
	unsigned int i;
	const unsigned int count = explosions.size();
	for (i = 0; i < count; i++) {
		explosions[i].time += dt;
		// FIXME -- update animation parameter
	}

	// reap expired explosions
	for (i = count; i > 0; ) {
		--i;
		if (explosions[i].time >= explosions[i].duration) {
			explosions[i].node->unref();
			explosions.erase(&explosions[i]);
		}
	}
}

static void				handleFlagDropped(Player* tank)
{
	// skip it if player doesn't actually have a flag
	if (tank->getFlag() == NoFlag)
		return;

	// add message
	BzfString message("dropped ");
	message += Flag::getName(tank->getFlag());
	message += " flag";
	addMessage(tank, message);

	// player no longer has flag
	tank->setFlag(NoFlag);

	// update display and play sound effects
	if (tank == myTank) {
		playLocalSound(SFX_DROP_FLAG);
		updateFlag();
	}
}

static bool				gotBlowedUp(BaseLocalPlayer* tank,
										BlowedUpReason reason,
										const PlayerId& killer,
										int shotId)
{
	if (!tank->isAlive())
		return false;

	// you can't take it with you
	const FlagId flag = tank->getFlag();
	if (flag != NoFlag) {
		// tell other players I've dropped my flag
		lookupServer(tank)->sendDropFlag(tank->getPosition());

		// drop it
		handleFlagDropped(tank);
	}

	// take care of explosion business -- don't want to wait for
	// round trip of killed message.  waiting would simplify things,
	// but the delay (2-3 frames usually) can really fool and irritate
	// the player.  we have to be careful to ignore our own Killed
	// message when it gets back to us -- do this by ignoring killed
	// message if we're already dead.
	// don't die if we had the shield flag and we've been shot.
	if (reason != GotShot || flag != ShieldFlag) {
		// blow me up
		tank->explodeTank();
		if (tank == myTank) {
			playLocalSound(SFX_DIE);
		}
		else {
			const float* pos = tank->getPosition();
			playWorldSound(SFX_EXPLOSION, pos[0], pos[1], pos[2],
								getLocalPlayer(killer) == myTank);

			float explodePos[3];
			explodePos[0] = pos[0];
			explodePos[1] = pos[1];
			explodePos[2] = pos[2] + MuzzleHeight;
			addTankExplosion(explodePos);
		}

		// i lose a point
		if (reason != GotCaptured)
			tank->changeScore(0, 1);

		// tell server I'm dead if it won't already know
		if (reason == GotShot || reason == GotRunOver || reason == GenocideEffect)
			lookupServer(tank)->sendKilled(killer, shotId);
	}

	// print reason if it's my tank
	if (tank == myTank && blowedUpMessage[reason]) {
		MSGMGR->insert("alertInfo", blowedUpMessage[reason], warningColor);
	}

	// make sure shot is terminated locally (if not globally) so it can't
	// hit me again if I had the shield flag.  this is important for the
	// shots that aren't stopped by a hit and so may stick around to hit
	// me on the next update, making the shield useless.
	return (reason == GotShot && flag == ShieldFlag && shotId != -1);
}

static void				checkEnvironment()
{
	if (!myTank) return;

	// skip this if i'm dead or paused
	if (!myTank->isAlive() || myTank->isPaused()) return;

	FlagId flagId = myTank->getFlag();
	if (flagId != NoFlag && int(flagId) >= int(FirstTeamFlag) &&
      int(flagId) <= int(LastTeamFlag)) {
		// have I captured a flag?
		TeamColor base = world->whoseBase(myTank->getPosition());
		TeamColor team = myTank->getTeam();
		if ((base != NoTeam) &&
		((int(flagId) == int(team) && base != team) ||
		(int(flagId) != int(team) && base == team)))
			serverLink->sendCaptureFlag(base);
	}
	else if (flagId == NoFlag && (myTank->getLocation() == LocalPlayer::OnGround ||
      myTank->getLocation() == LocalPlayer::OnBuilding)) {
		// Don't grab too fast
		static TimeKeeper lastGrabSent;
		if (TimeKeeper::getTick()-lastGrabSent > 0.2) {
			// grab any and all flags i'm driving over
			const float* tpos = myTank->getPosition();
			const float radius = myTank->getRadius();
			const float radius2 = (radius + FlagRadius) * (radius + FlagRadius);
			for (int i = 0; i < numFlags; i++) {
				if (world->getFlag(i).id == NoFlag || world->getFlag(i).status != FlagOnGround)
					continue;
				const float* fpos = world->getFlag(i).position;
				if ((fabs(tpos[2] - fpos[2]) < 0.1f) && ((tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
		    (tpos[1] - fpos[1]) * (tpos[1] - fpos[1]) < radius2)) {
					serverLink->sendGrabFlag(i);
					lastGrabSent=TimeKeeper::getTick();
				}
			}
		}
	}
	else if (flagId == IdentifyFlag) {
		// identify closest flag
		const float* tpos = myTank->getPosition();
		BzfString message("Closest Flag: ");
		float minDist = IdentityRange * IdentityRange;
		int closestFlag = -1;
		for (int i = 0; i < numFlags; i++) {
			if (world->getFlag(i).id == NoFlag ||
		  world->getFlag(i).status != FlagOnGround) continue;
			const float* fpos = world->getFlag(i).position;
			const float dist = (tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
						 (tpos[1] - fpos[1]) * (tpos[1] - fpos[1]) +
						 (tpos[2] - fpos[2]) * (tpos[2] - fpos[2]);
			if (dist < minDist) {
				minDist = dist;
				closestFlag = i;
			}
		}
		if (closestFlag != -1) {
			// Set HUD alert about what the flag is
			message += Flag::getName(world->getFlag(closestFlag).id);
			MSGMGR->insert("alertFlag", message,
				Flag::getType(world->getFlag(closestFlag).id) == FlagSticky ?
				warningColor : NULL);
		}
	}

	// see if i've been shot
	const ShotPath* hit = NULL;
	float minTime = Infinity;
	myTank->checkHit(myTank, hit, minTime);
	int i;
	for (i = 0; i < maxPlayers; i++)
		if (player[i])
			myTank->checkHit(player[i], hit, minTime);
	if (hit) {
		// i got shot!  terminate the shot that hit me and blow up.
		// force shot to terminate locally immediately (no server round trip);
		// this is to ensure that we don't get shot again by the same shot
		// after dropping our shield flag.
		if (hit->isStoppedByHit())
			serverLink->sendEndShot(hit->getPlayer(), hit->getShotId(), 1);
		const bool stopShot =
		gotBlowedUp(myTank, GotShot, hit->getPlayer(), hit->getShotId());
		if (stopShot || hit->isStoppedByHit()) {
			Player* hitter = lookupPlayer(hit->getPlayer());
			if (hitter) hitter->endShot(hit->getShotId());
		}
	}

	// if not dead yet, see if i got run over by the steamroller
	else {
		const float* myPos = myTank->getPosition();
		const float myRadius = myTank->getRadius();
		for (i = 0; i < maxPlayers; i++)
			if (player[i] &&
		  player[i]->getFlag() == SteamrollerFlag &&
		  !player[i]->isPaused()) {
				const float* pos = player[i]->getPosition();
				if (!(flagId == PhantomZoneFlag && myTank->isFlagActive())) {
					const float radius = myRadius + SRRadiusMult * player[i]->getRadius();
					if (hypot(hypot(myPos[0] - pos[0], myPos[1] - pos[1]), myPos[2] - pos[2]) < radius)
						gotBlowedUp(myTank, GotRunOver, player[i]->getId());
				}
			}
	}
}

static void				setTarget()
{
	// get info about my tank
	const float c = cosf(-myTank->getAngle());
	const float s = sinf(-myTank->getAngle());
	const float x0 = myTank->getPosition()[0];
	const float y0 = myTank->getPosition()[1];

	// initialize best target
	Player* bestTarget = NULL;
	float bestDistance = Infinity;
	bool lockedOn = false;

	// figure out which tank is centered in my sights
	for (int i = 0; i < maxPlayers; i++) {
		if (!player[i] || !player[i]->isAlive()) continue;

		// compute position in my local coordinate system
		const float* pos = player[i]->getPosition();
		const float x = c * (pos[0] - x0) - s * (pos[1] - y0);
		const float y = s * (pos[0] - x0) + c * (pos[1] - y0);

		// ignore things behind me
		if (x < 0.0f) continue;

		// get distance and sin(angle) from directly forward
		const float d = hypotf(x, y);
		const float a = fabsf(y / d);

		// see if it's inside lock-on angle (if we're trying to lock-on)
		if (a < 0.15f &&							// about 8.5 degrees
			myTank->getFlag() == GuidedMissileFlag &&	// am i locking on?
			player[i]->getFlag() != StealthFlag &&	// can't lock on stealth
			!player[i]->isPaused() &&				// can't lock on paused
			!player[i]->isNotResponding() &&		// can't lock on not responding
			d < bestDistance) {						// is it better?
			bestTarget = player[i];
			bestDistance = d;
			lockedOn = true;
		}
		else if (a < 0.3f &&						// about 17 degrees
			player[i]->getFlag() != StealthFlag &&	// can't "see" stealth
			d < bestDistance && !lockedOn) {		// is it better?
			bestTarget = player[i];
			bestDistance = d;
		}
	}
	if (!lockedOn)
		myTank->setTarget(NULL);
	if (!bestTarget)
		return;

	if (lockedOn) {
		myTank->setTarget(bestTarget);
		myTank->setNemesis(bestTarget);

		BzfString msg("Locked on ");
		msg += bestTarget->getCallSign();
		msg += " (";
		msg += Team::getName(bestTarget->getTeam());
		if (bestTarget->getFlag() != NoFlag) {
			msg += ") with ";
			msg += Flag::getName(bestTarget->getFlag());
		}
		else {
			msg += ")";
		}
		addMessage(NULL, msg);
		MSGMGR->insert("alertInfo", msg, warningColor);
	}
	else if (myTank->getFlag() == ColorblindnessFlag) {
		addMessage(NULL, "Looking at a tank");
		MSGMGR->insert("alertInfo", "Looking at a tank", NULL);
	}
	else {
		BzfString msg("Looking at ");
		msg += bestTarget->getCallSign();
		msg += " (";
		msg += Team::getName(bestTarget->getTeam());
		if (bestTarget->getFlag() != NoFlag) {
			msg += ") with ";
			msg += Flag::getName(bestTarget->getFlag());
		}
		else {
			msg += ")";
		}
		addMessage(NULL, msg);
		MSGMGR->insert("alertInfo", msg, NULL);
		myTank->setNemesis(bestTarget);
	}
}

//
// join/leave a game
//

static World*			makeWorld(ServerLink* serverLink)
{
	uint16_t code, len, size;
	char msg[MaxPacketLen];

	// ask for world and wait for it (ignoring all other messages)
	nboPackUShort(msg, 0);
	serverLink->send(MsgGetWorld, 2, msg);
	if (serverLink->read(code, len, msg, 5000) <= 0) return NULL;
	if (code == MsgNull || code == MsgSuperKill) return NULL;
	if (code != MsgGetWorld) return NULL;

	// get size of entire world database and make space
	nboUnpackUShort(msg, size);
	char* worldDatabase = new char[size];

	// get world database
	uint16_t ptr = 0, bytesLeft = size;
	while (bytesLeft != 0) {
		// ask and wait for next chunk
		nboPackUShort(msg, ptr);
		serverLink->send(MsgGetWorld, 2, msg);
		if (serverLink->read(code, len, msg, 5000) < 0 ||
		code == MsgNull || code == MsgSuperKill) {
			delete[] worldDatabase;
			return NULL;
		}

		// get bytes left
		void* buf = msg;
		buf = nboUnpackUShort(buf, bytesLeft);

		// add chunk to database so far
		::memcpy(worldDatabase + int(ptr), buf, len - 2);

		// increment pointer
		ptr += len - 2;
	}

	// make world
	WorldBuilder worldBuilder;
	worldBuilder.unpack(worldDatabase);
	delete[] worldDatabase;

	// return world
	return worldBuilder.getWorld();
}

static bool				enterServer(ServerLink* serverLink, World* world,
												LocalPlayer* myTank)
{

	time_t timeout=time(0) + 10;  // give us 10 sec

	// tell server we want to join
	serverLink->sendEnter(TankPlayer, myTank->getTeam(),
				myTank->getCallSign(), myTank->getEmailAddress());

	// wait for response
	uint16_t code, len;
	char msg[MaxPacketLen];
	if (serverLink->read(code, len, msg, -1) < 0) {
		BZDB->set("connectError",  "No response from server.");
		return false;
	}
	if (code == MsgSuperKill) {
		BZDB->set("connectError",  "Server forced disconnection.");
		return false;
	}
	if (code != MsgAccept && code != MsgReject) {
		BZDB->set("connectError",  "Protocol error with server.");
		printError("Wrong code %04x", code);
		return false;
	}
	if (code == MsgReject) {
		uint16_t rejectCode;
		nboUnpackUShort(msg, rejectCode);
		switch (rejectCode) {
			default:
			case RejectBadRequest:
			case RejectBadTeam:
			case RejectBadType:
				BZDB->set("connectError",  "Rejected by server.");
				printError("Rejection code %04x", rejectCode);
				break;

			case RejectNoRogues:
				BZDB->set("connectError",  "Rogues not allowed.  Try another team.");
				break;

			case RejectTeamFull:
				BZDB->set("connectError",  "The team is full.  Try another team.");
				break;

			case RejectServerFull:
				BZDB->set("connectError",  "The game is full.  Try again later.");
				break;
		}
		return false;
	}
	else if (code == MsgAccept) {
		uint8_t id;
		nboUnpackUByte(msg, id);
		myTank->setId(id);
	}

	// get updates
	if (serverLink->read(code, len, msg, -1) < 0) {
		goto failed;
	}
	while (code == MsgAddPlayer || code == MsgTeamUpdate ||
		 code == MsgFlagUpdate || code == MsgUDPLinkRequest) {
		void* buf = msg;
		switch (code) {
			case MsgAddPlayer: {
				PlayerId id;
				buf = nboUnpackUByte(buf, id);
				if (id == myTank->getId()) {
					// scan through flags and, for flags on
					// tanks, tell the tank about its flag.
					const int maxFlags = world->getMaxFlags();
					for (int i = 0; i < maxFlags; i++) {
						const Flag& flag = world->getFlag(i);
						if (flag.status == FlagOnTank)
							for (int j = 0; j < maxPlayers; j++)
								if (player[j] && player[j]->getId() == flag.owner) {
									player[j]->setFlag(flag.id);
									break;
								}
					}
					return true;
				}
				addPlayer(id, buf, false);
				break;
			}
			case MsgTeamUpdate: {
				uint16_t team;
				buf = nboUnpackUShort(buf, team);
				buf = teams[int(team)].unpack(buf);
				break;
			}
			case MsgFlagUpdate: {
				uint16_t flag;
				buf = nboUnpackUShort(buf, flag);
				buf = world->getFlag(int(flag)).unpack(buf);
				world->initFlag(int(flag));
				break;
			}
			case MsgUDPLinkRequest:
				printError("*** Received UDP Link Granted");
				// internally
				break;
		}

		if (time(0)>timeout) goto failed;

		if (serverLink->read(code, len, msg, -1) < 0) goto failed;
	}

failed:
	BZDB->set("connectError",  "Communication error joining game.");
	return false;
}

static void				leaveGame()
{
	// delete scene database
	SCENEMGR->setStatic(NULL);

	// use rogue color as team color when there's no team
	ViewColor::setMyTeam(ViewColor::Rogue);

	// my tank goes away
	const bool sayGoodbye = (myTank != NULL);
	LocalPlayer::setMyTank(NULL);
	delete myTank;
	myTank = NULL;

	// time goes back to current time if previously constrained by server
	if (world && !world->allowTimeOfDayAdjust()) {
		// use can change clock again
		BZDB->setPermission("timeClock", StateDatabase::ReadWrite);

		// reset clock
		BZDB->set("timeClock", BzfString::format("%f", (double)time(NULL)));
	}

	// delete world
	World::setWorld(NULL);
	delete world;
	world = NULL;
	teams = NULL;
	maxPlayers = 0;
	numFlags = 0;
	player = NULL;
	wallClock = 0.0f;

	// clean up explosions
	{
		if (explosion != NULL) {
			explosion->unref();
			explosion = NULL;
		}
		const int count = explosions.size();
		for (int i = 0; i < count; i++)
			explosions[i].node->unref();
		explosions.clear();
	}

	// update UI
// FIXME -- unset BZDB?
//  controlPanel->resetTeamCounts();
/* FIXME -- removed hud stuff
	hud->setPlaying(false);
	hud->setCracks(false);
	hud->setPlayerHasHighScore(false);
	hud->setTeamHasHighScore(false);
*/
	HUDMGR->removeHeadingMarker("team");
	HUDMGR->removeHeadingMarker("antidote");
	updateFlag();
	MSGMGR->get("alertGameOver")->clear();
	MSGMGR->get("alertInfo")->clear();
	MSGMGR->get("alertFlag")->clear();
	MSGMGR->get("alertStatus")->clear();

	// shut down server connection
	if (sayGoodbye) serverLink->send(MsgExit, 0, NULL);
	ServerLink::setServer(NULL);
	delete serverLink;
	serverLink = NULL;

	// reset viewpoint
	int wWindow, hWindow;
	mainWindow->getSize(wWindow, hWindow);
	float eyePoint[3], targetPoint[3];
	eyePoint[0] = 0.0f;
	eyePoint[1] = 0.0f;
	eyePoint[2] = 0.0f + MuzzleHeight;
	targetPoint[0] = eyePoint[0] - 1.0f;
	targetPoint[1] = eyePoint[1] + 0.0f;
	targetPoint[2] = eyePoint[2] + 0.0f;
	SCENEMGR->getView().setProjection(60.0f,
								static_cast<float>(wWindow) /
									static_cast<float>(hWindow),
								1.1f, 2.0f * WorldSize);
	SCENEMGR->getView().setView(eyePoint, targetPoint);

	// reset some flags
	gameOver = false;
	serverError = false;
	serverDied = false;
}

/*
// XXX -- make into a command?
#include "SceneVisitorWrite.h"
#include <fstream>
static void dumpScene()
{
	SceneNode* fullScene = SCENEMGR->getScene();
	if (fullScene) {
		ofstream stream("dump.bzg");
		SceneVisitorWrite write(&stream);
		write.traverse(fullScene);
		fullScene->unref();
	}
}
*/

static bool				joinGame(ServerLink* _serverLink)
{
	// this stuff should be ready
	assert(!BZDB->isEmpty("infoTeam"));
	assert(Team::getEnum(BZDB->get("infoTeam")) != NoTeam);
	assert(!BZDB->isEmpty("infoPort"));
	assert(atoi(BZDB->get("infoPort").c_str()) != 0);

	// assume everything's okay for now
	serverDied = false;
	serverError = false;

	serverLink = _serverLink;

	if (!serverLink) {
		BZDB->set("connectError",  "Out of memory error.");
		leaveGame();
		return false;
	}

	// check server
	if (serverLink->getState() != ServerLink::Okay) {
		switch (serverLink->getState()) {
			case ServerLink::BadVersion: {
				static char versionError[] = "Incompatible server version XXXXXXXX.";
				strncpy(versionError + strlen(versionError) - 9,
										serverLink->getVersion(), 8);
				BZDB->set("connectError",  versionError);
				break;
			}

			case ServerLink::Rejected:
				// the server is probably full or the game is over.  if not then
				// the server is having network problems.
				BZDB->set("connectError",  "Game is full or over.  Try again later.");
				break;

			case ServerLink::SocketError:
				BZDB->set("connectError",  "Error connecting to server.");
				break;

			case ServerLink::CrippledVersion:
				// can't connect to (otherwise compatible) non-crippled server
				BZDB->set("connectError",  "Cannot connect to full version server.");
				break;

			default:
				BZDB->set("connectError",  "Internal error connecting to server.");
				break;
		}

		leaveGame();
		return false;
	}

	// prepare to get models
	SCENEMGR->open();

	// load default models
	{
		istream* stream = FILEMGR->createDataInStream("models.bzg");
		if (stream != NULL) {
			SCENEMGR->read(stream);
			delete stream;
		}
	}

	// create world
	// FIXME -- should download models from server here
	world = makeWorld(serverLink);

	// done loading models
	SCENEMGR->close();

	// check for failure
	if (!world) {
		leaveGame();
		BZDB->set("connectError",  "Error downloading world database");
		return false;
	}

	// make static scene database
	// FIXME -- this will become part of the world download
	{
		SceneDatabaseBuilder sceneBuilder;
		SceneNode* scene = sceneBuilder.make(world);
		if (scene != NULL) {
			SCENEMGR->setStatic(scene);
			scene->unref();
		}
	}

	// get the explosion scene node
	if (explosion != NULL)
		explosion->unref();
	explosion = SCENEMGR->find("explosion");

	ServerLink::setServer(serverLink);
	World::setWorld(world);

	// prep teams
	teams = world->getTeams();

	// prep players
	maxPlayers = world->getMaxPlayers();
	player = world->getPlayers();

	// prep flags
	numFlags = world->getMaxFlags();

	// make local player
	TeamColor myTeam = Team::getEnum(BZDB->get("infoTeam"));
	myTank = new LocalPlayer(serverLink->getId(),
								BZDB->get("infoCallsign").c_str(),
								BZDB->get("infoEmail").c_str());
	myTank->setTeam(myTeam);
	LocalPlayer::setMyTank(myTank);

	// update team color in view
	switch (myTeam) {
		case NoTeam:
		case RogueTeam:
			ViewColor::setMyTeam(ViewColor::Rogue);
			break;

		case RedTeam:
			ViewColor::setMyTeam(ViewColor::Red);
			break;

		case GreenTeam:
			ViewColor::setMyTeam(ViewColor::Green);
			break;

		case BlueTeam:
			ViewColor::setMyTeam(ViewColor::Blue);
			break;

		case PurpleTeam:
			ViewColor::setMyTeam(ViewColor::Purple);
			break;
	}

	// hack:  force color updates in view.  this will cause the team
	// color to be updated to our new team.  we know that the view
	// manager updates all team colors when any changes so we only
	// touch one.
	BZDB->touch("colorRadarRogue");

	// enter server
	if (!enterServer(serverLink, world, myTank)) {
		delete myTank;
		myTank = NULL;
		leaveGame();
		return false;
	}

	// use parallel UDP
	serverLink->enableUDPCon();

	// decide how start for first time
	restartOnBase = world->allowTeamFlags() && myTank->getTeam() != RogueTeam;

	// if server constrains time then lock it and adjust it
	if (!world->allowTimeOfDayAdjust()) {
		BZDB->setPermission("timeClock", StateDatabase::Locked);
		BZDB->set("timeClock", BzfString::format("%f",
								(double)world->getEpochOffset()),
								StateDatabase::Server);
	}

	// initialize some other stuff
	wallClock = 0.0f;
	updateNumPlayers();
	updateFlag();
	updateHighScores();
	BZDB->set("displayRadarRange", "0.50");
/* FIXME -- unset BZDB
	hud->setTimeLeft(-1);
*/
	fireButton = false;
	firstLife = true;

	return true;
}

static bool				joinInternetGame()
{
	// this stuff should be ready
	assert(!BZDB->isEmpty("infoTeam"));
	assert(Team::getEnum(BZDB->get("infoTeam")) != NoTeam);
	assert(!BZDB->isEmpty("infoServer"));
	assert(!BZDB->isEmpty("infoPort"));
	assert(atoi(BZDB->get("infoPort").c_str()) != 0);

	// open server
	Address serverAddress(BZDB->get("infoServer"));
	if (serverAddress.isAny())
		return false;
	ServerLink* serverLink = new ServerLink(serverAddress,
								atoi(BZDB->get("infoPort").c_str()));

	return joinGame(serverLink);
}

static void				tryConnecting()
{
	// check to make sure we've got everything
	if (BZDB->isEmpty("infoCallsign")) {
		BZDB->set("connectStatus", "Cannot connect");
		BZDB->set("connectError",  "No callsign specified");
		return;
	}
	if (BZDB->isEmpty("infoTeam")) {
		BZDB->set("connectStatus", "Cannot connect");
		BZDB->set("connectError",  "No team specified");
		return;
	}
	if (Team::getEnum(BZDB->get("infoTeam")) == NoTeam) {
		BZDB->set("connectStatus", "Invalid information");
		BZDB->set("connectError",  "Unknown team specified");
		return;
	}
	if (BZDB->isEmpty("infoServer")) {
		BZDB->set("connectStatus", "Cannot connect");
		BZDB->set("connectError",  "No server specified");
		return;
	}
	if (BZDB->isEmpty("infoPort")) {
		BZDB->set("connectStatus", "Cannot connect");
		BZDB->set("connectError",  "No port specified");
		return;
	}
	int port = atoi(BZDB->get("infoPort").c_str());
	if (port <= 0 || port > 65535) {
		BZDB->set("connectStatus", "Invalid information");
		BZDB->set("connectError",  "Port must be between 1 and 65535");
		return;
	}

	// try to connect
	BZDB->set("connectStatus", "Trying...");
	BZDB->unset("connectError");

	// if already connected to a game then first sign off
	if (myTank)
		leaveGame();

	// now connect
	bool joined = joinInternetGame();

	// handle success
	if (joined) {
		// remove status message
		BZDB->unset("connectStatus");
		BZDB->unset("connectError");

		// pop menus, if any are displayed
		while (MENUMGR->top() != NULL)
			MENUMGR->pop();
	}

	// handle failure
	else {
		BZDB->set("connectStatus", "Connection failed");
	}
}


//
// main playing loop
//

static void				playingLoop()
{
	int i;

	// start timing
	int frameCount = 0;
	float cumTime = 0.0f;
	double clockAdjust = 0.0f;
	TimeKeeper::setTick();

	// main loop
	while (!CommandsStandard::isQuit()) {
		// get delta time
		TimeKeeper prevTime = TimeKeeper::getTick();
		TimeKeeper::setTick();
		const float dt = TimeKeeper::getTick() - prevTime;
		wallClock += dt;

		// handle incoming packets
		doMessages();

		// do dead reckoning on remote players
		for (i = 0; i < maxPlayers; i++)
			if (player[i]) {
				const bool wasNotResponding = player[i]->isNotResponding();
				player[i]->doDeadReckoning();
				const bool isNotResponding = player[i]->isNotResponding();
				if (!wasNotResponding && isNotResponding) {
					addMessage(player[i], BzfString("not responding"));
				}
				else if (wasNotResponding && !isNotResponding) {
					addMessage(player[i], BzfString("okay"));
				}
			}

		// try to join a game if requested.  do this *before* handling
		// events so we do a redraw after the request is posted and
		// before we actually try to join.
		if (BZDB->isTrue("connect")) {
			BZDB->unset("connect");
			tryConnecting();
		}

		// handle events
#if defined(ROAMING)
		roamDPos[0] = roamDPos[1] = roamDPos[2] = 0.0f;
		roamDTheta = roamDPhi = roamDZoom = 0.0f;
#endif
#ifndef macintosh
		while (!CommandsStandard::isQuit() && display->isEventPending())
#endif
			doEvent(display);

		// invoke callbacks
		callPlayingCallbacks();

		// quick out
		if (CommandsStandard::isQuit())
			break;

		// get window size
		int wWindow, hWindow;
		mainWindow->getSize(wWindow, hWindow);

		// if server died then leave the game (note that this may cause
		// further server errors but that's okay).
		if (serverError ||
			(serverLink && serverLink->getState() == ServerLink::Hungup)) {
			// if we haven't reported the death yet then do so now
			if (serverDied ||
				(serverLink && serverLink->getState() == ServerLink::Hungup))
				printError("Server has unexpectedly disconnected");
			leaveGame();
		}

		// update time of day -- update sun and sky every few seconds
		clockAdjust += (double)dt;
		if (fabs(clockAdjust) >= 4.0) {
			double t;
			if (sscanf(BZDB->get("timeClock").c_str(), "%lf", &t) == 1) {
			  BZDB->set("timeClock", BzfString::format("%f", t + clockAdjust),
									StateDatabase::Server);
				clockAdjust = 0.0;
			}
		}

#if defined(ROAMING)
		{
			// move roaming camera
			float c, s;
			c = cosf(roamTheta * M_PI / 180.0f);
			s = sinf(roamTheta * M_PI / 180.0f);
			roamPos[0] += dt * (c * roamDPos[0] - s * roamDPos[1]);
			roamPos[1] += dt * (c * roamDPos[1] + s * roamDPos[0]);
			roamPos[2] += dt * roamDPos[2];
			roamTheta  += dt * roamDTheta;
			roamPhi    += dt * roamDPhi;
			roamZoom   += dt * roamDZoom;
			if (roamZoom < 1.0f) roamZoom = 1.0f;
			else if (roamZoom > 179.0f) roamZoom = 179.0f;
		}
#endif

		// update pause countdown
		if (!myTank) pauseCountdown = 0.0f;
		if (pauseCountdown > 0.0f && !myTank->isAlive()) {
			pauseCountdown = 0.0f;
			MSGMGR->get("alertInfo")->clear();
		}
//		if (maxPauseCountdown > 0.0f) {
//			const int oldMaxPauseCountdown = (int)(maxPauseCountdown + 0.99f);
//			maxPauseCountdown -= dt;
//			if (maxPauseCountdown <= 0.0f) {
//				maxPauseCountdown=0.0;
//			}
//			if ((int)(maxPauseCountdown + 0.99f) != oldMaxPauseCountdown) {
//				char msgBuf[40];
//				sprintf(msgBuf, "Pause Countdown %d", (int)(maxPauseCountdown + 0.99f));
//				MSGMGR->insert("alertInfo", msgBuf, NULL);
//			}
//		}
		if (pauseCountdown > 0.0f) {
			const int oldPauseCountdown = (int)(pauseCountdown + 0.99f);
			pauseCountdown -= dt;
			if (pauseCountdown <= 0.0f) {
				// okay, now we pause.  first drop any team flag we may have.
				const FlagId flagId = myTank->getFlag();
				if (flagId >= FirstTeamFlag && flagId <= LastTeamFlag)
					serverLink->sendDropFlag(myTank->getPosition());

				// now actually pause
				myTank->setPause(true);
				MSGMGR->get("alertInfo")->clear();
				MSGMGR->insert("messages", "Paused");

				// turn off the sound
				BZDB->set("audioMute", "1");

				// ungrab mouse
				updateGrab();
			}
			else if ((int)(pauseCountdown + 0.99f) != oldPauseCountdown &&
															  !pausedByUnmap) {
				// update countdown alert
				char msgBuf[40];
				sprintf(msgBuf, "Pausing in %d", (int)(pauseCountdown + 0.99f));
				MSGMGR->insert("alertInfo", msgBuf, NULL);
			}
		}

		// reposition flags
		updateFlags(dt);

		// update explosion animations
		updateExplosions(dt);

		// update other tank's shots
		for (i = 0; i < maxPlayers; i++)
			if (player[i])
				player[i]->updateShots(dt);

		// stuff to draw a frame
		if (!unmapped) {
			// compute fps
			frameCount++;
			cumTime += float(dt);
			if (cumTime >= 2.0) {
				BZDB->set("timeFPS", BzfString::format("% 3d",
									static_cast<int>(frameCount / cumTime)));
				cumTime = 0.0;
				frameCount = 0;
			}

			// get tank camera info
/* FIXME -- must move this to continue supporting roaming
#if defined(ROAMING)
			if (roaming) {
#ifdef FOLLOWTANK
				eyePoint[0] = myTankPos[0] - myTankDir[0] * 20;
				eyePoint[1] = myTankPos[1] - myTankDir[1] * 20;
				eyePoint[2] = myTankPos[2] + MuzzleHeight * 3;
				targetPoint[0] = eyePoint[0] + myTankDir[0];
				targetPoint[1] = eyePoint[1] + myTankDir[1];
				targetPoint[2] = eyePoint[2] + myTankDir[2];
#else
				float dir[3];
				dir[0] = cosf(roamPhi * M_PI / 180.0f) * cosf(roamTheta * M_PI / 180.0f);
				dir[1] = cosf(roamPhi * M_PI / 180.0f) * sinf(roamTheta * M_PI / 180.0f);
				dir[2] = sinf(roamPhi * M_PI / 180.0f);
				eyePoint[0] = roamPos[0];
				eyePoint[1] = roamPos[1];
				eyePoint[2] = roamPos[2];
				if (!roamTrack) {
					targetPoint[0] = eyePoint[0] + dir[0];
					targetPoint[1] = eyePoint[1] + dir[1];
					targetPoint[2] = eyePoint[2] + dir[2];
				}
#endif
				fov = roamZoom * M_PI / 180.0f;
			}
#endif
*/

			// if we're paused or blind then swap out the static scene and
			// don't collect dynamic nodes
			SceneNode* staticScene = NULL;
			if (myTank != NULL && (myTank->isPaused() ||
				myTank->getFlag() == BlindnessFlag)) {
				staticScene = SCENEMGR->getStaticScene();
				SCENEMGR->setStatic(NULL);
				SCENEMGR->openDynamic();
				SCENEMGR->closeDynamic();
			}
			else {
				// add dynamic nodes
				SceneNodeGroup* dynamicGroup = new SceneNodeGroup;
				if (myTank != NULL) {
					// add my shadow
					if (myTank->getFlag() != CloakingFlag) {
						// FIXME -- not sure how we're doing shadows yet
					}

					// add my shots
					myTank->addShotsSceneNodes(dynamicGroup, false);

					// add my antidote flag
					myTank->addAntidoteSceneNode(dynamicGroup);

					// add flags
					world->addFlagsSceneNodes(dynamicGroup);

					// add other tanks and shells
					const bool colorblind = (myTank->getFlag() == ColorblindnessFlag);
					for (i = 0; i < maxPlayers; i++) {
						Player* p = player[i];
						if (p != NULL) {
							p->updateSparks(dt);
							p->addShotsSceneNodes(dynamicGroup, colorblind);
							if (p->getFlag() != CloakingFlag)
								p->addPlayerSceneNode(dynamicGroup, colorblind);
						}
					}

					// add explosions
					if (atoi(BZDB->get("renderQuality").c_str()) >= 2) {
						const unsigned int count = explosions.size();
						for (unsigned int i = 0; i < count; i++)
							dynamicGroup->pushChild(explosions[i].node);
					}

					// if i'm inside a building then add eighth dimension scene node.
/* FIXME
					if (myTank->getContainingBuilding()) {
						SceneNode* node = world->getInsideSceneNode(myTank->
												getContainingBuilding());
						if (node) scene->addDynamicNode(node);
					}
*/
				}
				SCENEMGR->openDynamic();
				SCENEMGR->addDynamic(dynamicGroup);
				SCENEMGR->closeDynamic();
				dynamicGroup->unref();
			}

			// set hud state
/* FIXME -- no longer supported;  move into BZDB and views
			hud->setPlaying(myTank && (myTank->isAlive() && !myTank->isPaused()));
			hud->setCracks(myTank && !firstLife && !myTank->isAlive());
*/

			// set the view parameters
			{
				// choose field-of-view
				float fov = 60.0f;
				if (myTank != NULL) {
					if (myTank->getFlag() == WideAngleFlag)
						fov *= 2.0f;
					else if (BZDB->isTrue("displayBinoculars"))
						fov *= 0.25f;
				}
				// compute eye and target points (with rotated view direction)
				const float* pos;
				const float* dir;
				if (myTank == NULL) {
					static const float defaultPos[3] = { 0.0f, 0.0f, 0.0f };
					static const float defaultDir[3] = { 1.0f, 0.0f, 0.0f };
					pos = defaultPos;
					dir = defaultDir;
				}
				else {
					pos = myTank->getPosition();
					dir = myTank->getForward();
				}
				float eye[3], focus[3];
				eye[0]   = pos[0];
				eye[1]   = pos[1];
				eye[2]   = pos[2] + MuzzleHeight;
				focus[0] = eye[0] + dir[0];
				focus[1] = eye[1] + dir[1];
				focus[2] = eye[2] + dir[2];

				// set the view parameters
				SCENEMGR->getView().setView(eye, focus);
				SCENEMGR->getView().setProjection(fov,
									static_cast<float>(wWindow) /
										static_cast<float>(hWindow),
									1.1f, 2.0f * WorldSize);
				SCENEMGR->getView().setOffset(0.0f, 0.0f);
			}

			// set scene fade
			SCENEMGR->setFade(0.0f, 0.0f, 0.0f, 0.0f);
			if (myTank != NULL) {
				if (!myTank->isAlive() && !myTank->isExploding()) {
					SCENEMGR->setFade(0.0f, 0.0f, 0.0f, 0.5f);
				}
				else {
					float proximity = myTank->getTeleporterProximity();
					if (proximity > 0.0f) {
						const float alpha = (proximity > 0.75f) ? 1.0f : proximity / 0.75f;
						SCENEMGR->setFade(1.0f, 1.0f, 0.0f, alpha);
					}
				}
			}

			// set the time on the scene
			SCENEMGR->setTime(wallClock);

			// get frame start time
			if (BZDB->isTrue("displayDebug"))
				glFinish();
			double stopwatch = PLATFORM->getClock();

			// normal rendering
			BzfString viewName = BZDB->get("displayView");
			View* view = VIEWMGR->get(viewName);
			if (view == NULL && viewName != "normal")
				view = VIEWMGR->get("normal");
			OpenGLGState::resetState();
			if (SCENEMGR->getScene() == NULL) {
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
			if (view != NULL) {
				view->render(0.0f, 0.0f, wWindow, hWindow);
			}

			// get frame end time
			if (BZDB->isTrue("displayDebug"))
				glFinish();
			stopwatch = PLATFORM->getClock() - stopwatch;
			BZDB->set("timeFrame", BzfString::format("% 8d",
								static_cast<int>(1000000.0 * stopwatch)));

			// restore the static scene
			if (staticScene != NULL) {
				SCENEMGR->setStatic(staticScene);
				staticScene->unref();
			}

			// draw a fake cursor if requested.  this is mostly intended for
			// pass through 3D cards that don't have cursor support.
			if (BZDB->isTrue("displayCrosshair")) {
				int mx, my;
				mainWindow->getMouse(mx, my);

				// screen coordinates are flipped wrt drawing coordinates
				my = (hWindow - 1) - my;

				glScissor(0, 0, wWindow, hWindow);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(0.0, wWindow, 0.0, hWindow, -1.0, 1.0);
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();

				OpenGLGState::init();
				glColor3f(0.0f, 0.0f, 0.0f);
				glRecti(mx - 8, my - 2, mx - 2, my + 2);
				glRecti(mx + 2, my - 2, mx + 8, my + 2);
				glRecti(mx - 2, my - 8, mx + 2, my - 2);
				glRecti(mx - 2, my + 2, mx + 2, my + 8);

				glColor3f(1.0f, 1.0f, 1.0f);
				glRecti(mx - 7, my - 1, mx - 3, my + 1);
				glRecti(mx + 3, my - 1, mx + 7, my + 1);
				glRecti(mx - 1, my - 7, mx + 1, my - 3);
				glRecti(mx - 1, my + 3, mx + 1, my + 7);

				glPopMatrix();
			}

			mainWindow->swapBuffers();
		}
		else {
			// wait around a little to avoid spinning the CPU when iconified
			PLATFORM->sleep(0.05f);
		}

		updateSound();

		// do motion
		if (myTank) {
			if (myTank->isAlive() && !myTank->isPaused()) {
				doMotion();
				if (fireButton && myTank->getFlag() == MachineGunFlag)
					myTank->fireShot();
			}
			myTank->update();
		}

		// prep the HUD
		bool markedTeam = false, markedAntidote = false;
		if (myTank) {
			const float* myPos = myTank->getPosition();
			if (world->allowTeamFlags() &&
				myTank->getTeam() != RogueTeam &&
				int(myTank->getFlag()) != int(myTank->getTeam())) {
				// marker for my team flag
				for (i = 0; i < numFlags; i++) {
				  Flag& flag = world->getFlag(i);
				  if (int(flag.id) == int(myTank->getTeam())) {
					const float* flagPos = flag.position;
					HUDMGR->addHeadingMarker("team",
										atan2f(flagPos[1] - myPos[1],
												flagPos[0] - myPos[0]),
										Team::getTankColor(myTank->getTeam()));
					markedTeam = true;
					break;
				  }
				}
			}
			if (myTank->getAntidoteLocation()) {
				// marker for my antidote flag
				static const float antidoteColor[] = { 1.0f, 1.0f, 0.0f };
				const GLfloat* antidotePos = myTank->getAntidoteLocation();
				HUDMGR->addHeadingMarker("antidote",
										atan2f(antidotePos[1] - myPos[1],
												antidotePos[0] - myPos[0]),
										antidoteColor);
				markedAntidote = true;
			}
		}
		if (!markedTeam)
			HUDMGR->removeHeadingMarker("team");
		if (!markedAntidote)
			HUDMGR->removeHeadingMarker("antidote");

		// check for flags and hits
		checkEnvironment();

		// update transient data
		if (myTank) {
			BZDB->set("outputScore", BzfString::format("%d", myTank->getScore()));

			switch (myTank->getFiringStatus()) {
				case LocalPlayer::Deceased:
					MSGMGR->insert("alertStatus", "Dead", redColor);
					break;

				case LocalPlayer::Ready:
					if (myTank->getFlag() != NoFlag &&
						Flag::getType(myTank->getFlag()) == FlagSticky &&
						world->allowShakeTimeout()) {
					  /* have a bad flag -- show time left 'til we shake it */
					  MSGMGR->insert("alertStatus", BzfString::format("%.1f",
									myTank->getFlagShakingTime()), yellowColor);
					}
					else {
					  MSGMGR->insert("alertStatus", "Ready", greenColor);
					}
					break;

				case LocalPlayer::Loading:
					MSGMGR->insert("alertStatus",
									BzfString::format("Reloaded in %.1f",
									myTank->getReloadTime()), redColor);
					break;

				case LocalPlayer::Sealed:
					MSGMGR->insert("alertStatus", "Sealed", redColor);
					break;

				case LocalPlayer::Zoned:
					MSGMGR->insert("alertStatus", "Zoned", redColor);
					break;
			}
		}
		else {
			BZDB->unset("outputScore");
			MSGMGR->get("alertStatus")->clear();
		}

		// send my data
		if (serverLink && myTank->isDeadReckoningWrong()) {
			serverLink->sendPlayerUpdate(myTank);
		}
	}
}

//
// game initialization
//

static void				defaultErrorCallback(const char* msg)
{
	MSGMGR->insert("console", msg);
}

static void				startupErrorCallback(const char* msg)
{
	MSGMGR->insert("console", msg);
/* FIXME -- force redraw of message view?  or just force general redraw.
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	controlPanel->render();
	mainWindow->swapBuffers();
*/
}

void					startPlaying(BzfDisplay* _display,
								BzfWindow* _mainWindow)
{
	unsigned int i;

	// register some commands
	for (i = 0; i < countof(commandList); ++i)
		CMDMGR->add(commandList[i].name, commandList[i].func, commandList[i].help);

	// initalization
	display = _display;
	mainWindow = _mainWindow;

	// initialize graphics state
	mainWindow->makeCurrent();
	OpenGLGState::init();

	// show window and clear it immediately
	mainWindow->showWindow(true);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	mainWindow->swapBuffers();
	glClear(GL_COLOR_BUFFER_BIT);
	mainWindow->swapBuffers();

	// if we're running on 3Dfx fullscreen add a fake cursor.
	// let the defaults file override this, though.
	if (!BZDB->isSet("displayCrosshair")) {
		// check that the renderer is Mesa Glide
		const char* renderer = (const char*)glGetString(GL_RENDERER);
		if (strncmp(renderer, "Mesa Glide", 10) == 0 ||
			strncmp(renderer, "3Dfx", 4) == 0)
			BZDB->set("displayCrosshair", "1");
	}

	// startup error callback adds message to control panel and
	// forces an immediate redraw.
	setErrorCallback(startupErrorCallback);

	// catch signals before changing video mode so we can
	// put it back even if we die.  ignore a few signals.
	if (PLATFORM->signalCatch(kSigINT,  kSigIGN) != kSigIGN)
		PLATFORM->signalCatch(kSigINT,  suicide);
	if (PLATFORM->signalCatch(kSigQUIT, kSigIGN) != kSigIGN)
		PLATFORM->signalCatch(kSigQUIT, dying);
	PLATFORM->signalCatch(kSigILL,  dying);
	PLATFORM->signalCatch(kSigABRT, dying);
	PLATFORM->signalCatch(kSigSEGV, dying);
	PLATFORM->signalCatch(kSigTERM, suicide);
	PLATFORM->signalCatch(kSigPIPE, hangup);
	PLATFORM->signalCatch(kSigHUP,  kSigIGN);
	PLATFORM->signalCatch(kSigBUS,  dying);
	PLATFORM->signalCatch(kSigUSR1, kSigIGN);
	PLATFORM->signalCatch(kSigUSR2, kSigIGN);

	// force resolution to change (if so configured)
	BZDB->touch("windowResolution");

	// hack to work around resolution changing bug on my system,
	// which doesn't sync on the first change of resolution.
#if defined(__linux__)
	display->setDefaultResolution();
	BZDB->touch("windowResolution");
#endif

	// clear again
	glClear(GL_COLOR_BUFFER_BIT);
	mainWindow->swapBuffers();

	// initialize BZDB output values
	updateNumPlayers();
	updateFlag();
	updateHighScores();
	notifyBzfKeyMapChanged();

	// normal error callback (doesn't force a redraw)
	setErrorCallback(defaultErrorCallback);

	// print version
	MSGMGR->insert("messages", " ");
	MSGMGR->insert("messages", BzfString::format(
				"BZFlag version %d.%d%c%d",
				(VERSION / 10000000) % 100, (VERSION / 100000) % 100,
				(char)('a' - 1 + (VERSION / 1000) % 100), VERSION % 1000));

	// print expiration
	if (timeBombString())
		MSGMGR->insert("messages", BzfString::format(
				"This release will expire on %s", timeBombString()));

	// print copyright
	MSGMGR->insert("messages", copyright);
	MSGMGR->insert("messages", "Author: Chris Schoeneman <crs23@bigfoot.com>");
	MSGMGR->insert("messages", "Maintainer: Tim Riker <Tim@Rikers.org>");

	// print OpenGL renderer
	MSGMGR->insert("messages", BzfString::format("Renderer: %s",
								(const char*)glGetString(GL_RENDERER)));

	// install database callbacks
	BZDB->addCallback("connectStatus", onConnectionMessage, NULL);
	BZDB->addCallback("connectError",  onConnectionMessage, NULL);
	BZDB->addCallback("displayFlagHelp",  onFlagHelp, NULL);
	BZDB->addCallback("displayGrabCursor", onGrabCursorChanged, NULL);

	// add server finding commands
	CommandsSearch::add();

	// grab mouse if we should
	updateGrab();

	// pop up main menu if we're not auto-connecting at startup
	if (!BZDB->isTrue("connect"))
		MENUMGR->push("main");

	// start game loop
	playingLoop();

	// clean up
	CommandsSearch::remove();
	BZDB->removeCallback("displayGrabCursor", onGrabCursorChanged, NULL);
	BZDB->removeCallback("displayFlagHelp",  onFlagHelp, NULL);
	BZDB->removeCallback("connectStatus", onConnectionMessage, NULL);
	BZDB->removeCallback("connectError",  onConnectionMessage, NULL);
	leaveGame();
	setErrorCallback(NULL);
	while (MENUMGR->top() != NULL)
		MENUMGR->pop();

	// restore resolution
	display->setDefaultResolution();
	OpenGLGState::freeContext();

	// ungrab mouse and hide window
	unmapped = true;
	updateGrab();
	mainWindow->showWindow(false);

	// don't use these anymore
	mainWindow = NULL;
	display = NULL;
}
