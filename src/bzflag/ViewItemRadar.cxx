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

#include "ViewItemRadar.h"
#include "StateDatabase.h"
#include "Team.h"
#include "Flag.h"
#include "World.h"
#include "LocalPlayer.h"
#include "RemotePlayer.h"
#include "OpenGLGState.h"
#include "OpenGLTexture.h"
#include "SceneManager.h"
#include "global.h"
#include "bzfgl.h"
#include <assert.h>
#include <stdio.h>

//
// ViewItemRadar
//

static const float		buildingColor[3]   = { 0.25f, 0.50f, 0.50f };
static const float		teleporterColor[3] = { 1.00f, 1.00f, 0.25f };
const float				ViewItemRadar::colorFactor = 20.0f;
const float				ViewItemRadar::transFactor = 20.0f;

ViewItemRadar::ViewItemRadar() : decay(0.01), noise(NULL), noiseTexture(NULL)
{
	initContext(true);
	OpenGLGState::addContextInitializer(initContextCB, (void*)this);
}

ViewItemRadar::~ViewItemRadar()
{
	OpenGLGState::removeContextInitializer(initContextCB, (void*)this);
	initContext(false);
	delete[] noise;
}

bool					ViewItemRadar::onPreRender(
								float, float, float , float)
{
	LocalPlayer* myTank = LocalPlayer::getMyTank();
	return (myTank != NULL);
}

void					ViewItemRadar::onPostRender(
								float, float, float w, float h)
{
	// radar is blank if no player or player is paused
	LocalPlayer* myTank = LocalPlayer::getMyTank();
	if (myTank == NULL || myTank->isPaused())
		return;

	const bool smoothingOn   = smooth && BZDB->isTrue("renderSmoothing");
	const bool blend         = BZDB->isTrue("renderBlending");
	const bool enhancedRadar = BZDB->isTrue("displayEnhancedRadar");
	const bool coloredShots  = BZDB->isTrue("displayRadarColoredShots");

	// get range
	double range;
	if (sscanf(BZDB->get("displayRadarRange").c_str(), "%lf", &range) != 1)
		range = 0.5;
	range *= WorldSize;

	// get size of pixel in model space (assumes radar is square)
	GLfloat ps = 2.0f * range / GLfloat(w);

	// FIXME -- should be using OpenGLGState objects
	OpenGLGState::resetState();

	// if jammed then draw white noise.  occasionally draw a good frame.
	if (myTank->getFlag() == JammingFlag && bzfrand() > decay) {
		if (noiseTexture != NULL && BZDB->isTrue("renderTexturing")) {
// FIXME -- only if quality is high enough
			static const float np[][4] = {
								{ 0.00f, 0.00f, 1.00f, 1.00f },
								{ 1.00f, 1.00f, 0.00f, 0.00f },
								{ 0.50f, 0.50f, 1.50f, 1.50f },
								{ 1.50f, 1.50f, 0.50f, 0.50f },
								{ 0.25f, 0.25f, 1.25f, 1.25f },
								{ 1.25f, 1.25f, 0.25f, 0.25f },
								{ 0.00f, 0.50f, 1.00f, 1.50f },
								{ 1.00f, 1.50f, 0.00f, 0.50f },
								{ 0.50f, 0.00f, 1.50f, 1.00f },
								{ 1.50f, 1.00f, 0.50f, 0.00f },
								{ 0.75f, 0.75f, 1.75f, 1.75f },
								{ 1.75f, 1.75f, 0.75f, 0.75f }
						};
			static const int sequences = countof(np);
	  
			const float* noisePattern = np[(int)floor(sequences * bzfrand())];
			noiseTexture->execute();
			glEnable(GL_TEXTURE_2D);
			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_QUADS);
			  glTexCoord2f(noisePattern[0], noisePattern[1]);
			  glVertex2f(0.0f, 0.0f);
			  glTexCoord2f(noisePattern[2], noisePattern[1]);
			  glVertex2f(	w, 0.0f);
			  glTexCoord2f(noisePattern[2], noisePattern[3]);
			  glVertex2f(	w,	h);
			  glTexCoord2f(noisePattern[0], noisePattern[3]);
			  glVertex2f(0.0f,	h);
			glEnd();
			glDisable(GL_TEXTURE_2D);
		}
		if (decay > 0.015f)
		  decay *= 0.5f;
	}

	else {
		int i;
		World* world = World::getWorld();
		const int maxPlayers = world->getMaxPlayers();
		const int maxShots = world->getMaxShots();

		// if decay is sufficiently small then boost it so it's more
		// likely a jammed radar will get a few good frames closely
		// spaced in time.  value of 1 guarantees at least two good
		// frames in a row.
		if (decay <= 0.015f)
			decay = 1.0f;
		else
			decay *= 0.5f;

		// relative to my tank
		const float* pos = myTank->getPosition();
		float angle = myTank->getAngle();
		glPushMatrix();
		glTranslatef(0.5f * w, 0.5f * h, 0.0f);
		glScalef(0.5f * w / range, 0.5f * h / range, 0.5f / range);
		glPushMatrix();
		glRotatef(90.0f - angle * 180.0f / M_PI, 0.0f, 0.0f, 1.0f);
		glPushMatrix();
		glTranslatef(-pos[0], -pos[1], 0.0f);

		// antialiasing on for lines and points unless we're multisampling,
		// in which case it's automatic and smoothing makes them look worse.
		if (smoothingOn || (blend && enhancedRadar)) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
		}
		if (smoothingOn) {
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_POINT_SMOOTH);
		}

		// draw buildings
		drawObstacles(enhancedRadar);
		if (smoothingOn)
			drawSmoothObstacles(enhancedRadar);

		// no more blending (unless still smoothing)
		if (!smoothingOn)
			glDisable(GL_BLEND);

		// draw flags not on tanks.
		const int maxFlags = world->getMaxFlags();
		for (i = 0; i < maxFlags; i++) {
			const Flag& flag = world->getFlag(i);
			if (flag.status == FlagNoExist || flag.status == FlagOnTank)
				continue;
			glColor3fv(Flag::getColor(flag.id));
			drawFlag(flag.position[0], flag.position[1], flag.position[2],2.5*ps);
		}

		// draw antidote flag
		const float* antidotePos = myTank->getAntidoteLocation();
		if (antidotePos) {
			glColor3f(1.0f, 1.0f, 0.0f);
			drawFlag(antidotePos[0], antidotePos[1], antidotePos[2],2.5*ps);
		}

		// draw other tanks' shells
		for (i = 0; i < maxPlayers; i++) {
			RemotePlayer* player = world->getPlayer(i);
			if (!player)
				continue;
			for (int j = 0; j < maxShots; j++) {
				const ShotPath* shot = player->getShot(j);
				if (shot && shot->getFlag() != InvisibleBulletFlag) {
					if (!coloredShots)
						glColor3f(1.0f, 1.0f, 1.0f);
					else if (myTank->getFlag() == ColorblindnessFlag)
						glColor3fv(Team::getRadarColor(RogueTeam));
					else
						glColor3fv(Team::getRadarColor(player->getTeam()));
					shot->radarRender();
				}
			}
		}

		// draw my shots
		glColor3f(1.0f, 1.0f, 1.0f);
		for (i = 0; i < maxShots; i++) {
			const ShotPath* shot = myTank->getShot(i);
			if (shot)
				shot->radarRender();
		}

		// draw other tanks (and any flags on them)
		for (i = 0; i < maxPlayers; i++) {
			RemotePlayer* player = world->getPlayer(i);
			if (!player || !player->isAlive() || player->getFlag() == StealthFlag)
				continue;

			// tank is dim if not active
			float scale = 1.0f;
			if (player->isPaused() || player->isNotResponding())
				scale = 0.4f;

			// get tank color
			const float* color;
			if (myTank->getFlag() == ColorblindnessFlag)
				color = Team::getRadarColor(RogueTeam);
			else
				color = Team::getRadarColor(player->getTeam());

			// draw tank symbol
			const float* pos = player->getPosition();
			glColor3f(scale * color[0], scale * color[1], scale * color[2]);
			drawTank(pos[0], pos[1], pos[2],2.5*ps);

			// draw flag on tank
			if (player->getFlag() != NoFlag) {
				color = Flag::getColor(player->getFlag());
				glColor3f(scale * color[0], scale * color[1], scale * color[2]);
				drawFlag(pos[0], pos[1], pos[2],4*ps);
			}
		}

		// draw following markers above all others always centered
		glPopMatrix();

		// north marker
		GLfloat ns = 0.05f * range, ny = 0.9f * range;
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_STRIP);
			glVertex2f(-ns, ny - ns);
			glVertex2f(-ns, ny + ns);
			glVertex2f(ns, ny - ns);
			glVertex2f(ns, ny + ns);
		glEnd();

		// always up
		glPopMatrix();

		// my tank
		glColor3f(1.0f, 1.0f, 1.0f);
		drawTank(0.0f, 0.0f, myTank->getPosition()[2],2.5*ps);

		// my flag
		if (myTank->getFlag() != NoFlag) {
			glColor3fv(Flag::getColor(myTank->getFlag()));
			drawFlag(0.0f, 0.0f, myTank->getPosition()[2],4*ps);
		}

		// forward tick
		glBegin(GL_LINES);
			glVertex2f(0.0f, range - ps);
			glVertex2f(0.0f, range - 4.0f * ps);
		glEnd();

		// view frustum edges
		glColor3f(1.0f, 0.625f, 0.125f);
		const float fovx = SCENEMGR->getView().getFOVx();
		const float viewWidth = range * tanf(fovx * M_PI / 360.0f);
		glBegin(GL_LINE_STRIP);
			glVertex2f(-viewWidth, range);
			glVertex2f(0.0f, 0.0f);
			glVertex2f(viewWidth, range);
		glEnd();

		if (smoothingOn) {
			glDisable(GL_BLEND);
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_POINT_SMOOTH);
		}

		// restore stack
		glPopMatrix();
	}

	// FIXME
	OpenGLGState::resetState();
}

bool					ViewItemRadar::makeNoise()
{
	delete[] noise;
	const int size = 4 * 128 * 128;
	noise = new unsigned char[size];
	if (noise == NULL)
		return false;
	for (int i = 0; i < size; i += 4) {
		unsigned char n = (unsigned char)floor(256.0 * bzfrand());
		noise[i+0] = n;
		noise[i+1] = n;
		noise[i+2] = n;
		noise[i+3] = n;
	}
	return true;
}

void					ViewItemRadar::makeNoiseTexture()
{
	delete noiseTexture;
	noiseTexture = new OpenGLTexture(128, 128, noise, OpenGLTexture::Nearest);
}

void					ViewItemRadar::drawTank(float x, float y, float z, float minsize)
{
	// Changes with height.
	GLfloat s = TankRadius * 3.0 + (z + BoxHeight) * 2.0f / BoxHeight;

	if (z > 0.0f) {
		glBegin(GL_LINE_LOOP);
			glVertex2f(x - s, y);
			glVertex2f(x, y - s);
			glVertex2f(x + s, y);
			glVertex2f(x, y + s);
		glEnd();
	}

	// Does not change with height.
	s = max(TankRadius, minsize);
	glRectf(x - s, y - s, x + s, y + s);
}

void					ViewItemRadar::drawFlag(float x, float y, float z, float minsize)
{
	// draw edges both ways for systems that don't filter line ends
	// correctly.  this degrades the smoothing, unfortunately.
	GLfloat s = TankRadius / 2.0 + (z + BoxHeight) * 2.0f / BoxHeight;
	if (s < minsize)
		s = minsize;

	glBegin(GL_LINES);
		glVertex2f(x - s, y);
		glVertex2f(x + s, y);
		glVertex2f(x + s, y);
		glVertex2f(x - s, y);
		glVertex2f(x, y - s);
		glVertex2f(x, y + s);
		glVertex2f(x, y + s);
		glVertex2f(x, y - s);
	glEnd();
}

void					ViewItemRadar::drawObstacles(bool enhanced)
{
	int i;
	World* world                     = World::getWorld();
	const WallObstacles& walls       = world->getWalls();
	const BoxBuildings& boxes        = world->getBoxes();
	const PyramidBuildings& pyramids = world->getPyramids();
	const BaseBuildings& bases       = world->getBases();
	const Teleporters& teleporters   = world->getTeleporters();

	// draw walls.  walls are flat so a line will do.
	int count = walls.size();
	glColor3fv(buildingColor);
	glBegin(GL_LINES);
	for (i = 0; i < count; i++) {
		const WallObstacle& wall = walls[i];
		const float w = wall.getBreadth();
		const float c = w * cosf(wall.getRotation());
		const float s = w * sinf(wall.getRotation());
		const float* pos = wall.getPosition();
		glVertex2f(pos[0] - s, pos[1] + c);
		glVertex2f(pos[0] + s, pos[1] - c);
	}
	glEnd();

	// draw box buildings.
	count = boxes.size();
	glBegin(GL_QUADS);
	for (i = 0; i < count; i++)
		drawObstacle(boxes[i], buildingColor, enhanced);
	glEnd();

	// draw pyramid buildings
	count = pyramids.size();
	glBegin(GL_QUADS);
	for (i = 0; i < count; i++)
		drawObstacle(pyramids[i], buildingColor, enhanced);
	glEnd();

	// draw team bases
	if (world->allowTeamFlags()) {
		count = bases.size();
		for (i = 0; i < count; i++) {
			glBegin(GL_LINE_LOOP);
			drawObstacle(bases[i], Team::getRadarColor((TeamColor)
										bases[i].getTeam()), enhanced);
			glEnd();
		}
	}

	// draw teleporters.  teleporters are pretty thin so use lines
	// (which, if longer than a pixel, are guaranteed to draw something;
	// not so for a polygon).
	count = teleporters.size();
	for (i = 0; i < count; i++) {
		glBegin(GL_LINE_LOOP);
		drawObstacle(teleporters[i], teleporterColor, enhanced);
		glEnd();
	}
}

void					ViewItemRadar::drawSmoothObstacles(bool enhanced)
{
	int i;
	World* world                     = World::getWorld();
	const BoxBuildings& boxes        = world->getBoxes();
	const PyramidBuildings& pyramids = world->getPyramids();

	int count = boxes.size();
	for (i = 0; i < count; i++) {
		glBegin(GL_LINE_LOOP);
			drawObstacle(boxes[i], buildingColor, enhanced);
		glEnd();
	}

	count = pyramids.size();
	for (i = 0; i < count; i++) {
		glBegin(GL_LINE_LOOP);
			drawObstacle(pyramids[i], buildingColor, enhanced);
		glEnd();
	}
}

void					ViewItemRadar::drawObstacle(
								const Obstacle& object,
								const float* scale,
								bool enhancedRadar)
{
	const float cs   = colorScale(object, enhancedRadar);
	const float c    = cosf(object.getRotation());
	const float s    = sinf(object.getRotation());
	const float wx   =  c * object.getWidth(),   wy = s * object.getWidth();
	const float hx   = -s * object.getBreadth(), hy = c * object.getBreadth();
	const float* pos = object.getPosition();
	glColor4f(scale[0] * cs, scale[1] * cs, scale[2] * cs, transScale(object));
	glVertex2f(pos[0] - wx - hx, pos[1] - wy - hy);
	glVertex2f(pos[0] + wx - hx, pos[1] + wy - hy);
	glVertex2f(pos[0] + wx + hx, pos[1] + wy + hy);
	glVertex2f(pos[0] - wx + hx, pos[1] - wy + hy);
}

float					ViewItemRadar::colorScale(const Obstacle& o, bool enhancedRadar)
{
	float scaleColor;
	if (enhancedRadar == true) {
		const LocalPlayer* myTank = LocalPlayer::getMyTank();

		// Scale color so that objects that are close to tank's level are opaque
		const float zTank = myTank->getPosition()[2];
		const float zObstacle = o.getPosition()[2];
		const float hObstacle = o.getHeight();
		if (zTank >= (zObstacle + hObstacle))
			scaleColor = 1.0f - (zTank - (zObstacle + hObstacle)) / colorFactor;
		else if (zTank <= zObstacle)
			scaleColor = 1.0f - (zObstacle - zTank) / colorFactor;
		else
			scaleColor = 1.0f;

		// Can't get blacker than black!
		if (scaleColor < 0.5f)
			scaleColor = 0.5f;
	}
	else {
		scaleColor = 1.0f;
	}

	return scaleColor;
}

float					ViewItemRadar::transScale(const Obstacle& o)
{
	float scaleColor;
	const LocalPlayer* myTank = LocalPlayer::getMyTank();

	// Scale color so that objects that are close to tank's level are opaque
	const float zTank = myTank->getPosition()[2];
	const float zObstacle = o.getPosition()[2];
	const float hObstacle = o.getHeight();
	if (zTank >= (zObstacle + hObstacle))
		scaleColor = 1.0f - (zTank - (zObstacle + hObstacle)) / colorFactor;
	else if (zTank <= zObstacle)
		scaleColor = 1.0f - (zObstacle - zTank) / colorFactor;
	else
		scaleColor = 1.0f;

	if (scaleColor < 0.5f)
		scaleColor = 0.5f;

	return scaleColor;
}

void					ViewItemRadar::initContext(bool destroy)
{
	if (destroy) {
		smooth = true;
#if defined(GLX_SAMPLES_SGIS) && defined(GLX_SGIS_multisample)
		GLint bits;
		glGetIntergerv(GL_SAMPLES_SGIS, &bits);
		if (bits > 0)
			smooth = false;
#endif

		// destroy jamming noise
		delete noiseTexture;
		noiseTexture = NULL;
	}
	else {
		// recreate jamming noise
		if (makeNoise())
			makeNoiseTexture();
	}
}

void					ViewItemRadar::initContextCB(
								bool destroy, void* self)
{
	reinterpret_cast<ViewItemRadar*>(self)->initContext(destroy);
}


//
// ViewItemRadarReader
//

ViewItemRadarReader::ViewItemRadarReader() : item(NULL)
{
	// do nothing
}

ViewItemRadarReader::~ViewItemRadarReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* 			ViewItemRadarReader::clone() const
{
	return new ViewItemRadarReader;
}

View*					ViewItemRadarReader::open(XMLTree::iterator)
{
	assert(item == NULL);
	item = new ViewItemRadar;
	return item;
}
