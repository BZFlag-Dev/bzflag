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

#include "SceneManager.h"
#include "SceneNodes.h"
#include "SceneReader.h"
#include "SceneVisitorFindAll.h"
#include "StateDatabase.h"
#include "FileManager.h"
#include "ErrorHandler.h"
#include "ViewFrustum.h"
#include "OpenGLGState.h"
#include "daylight.h"
#include <math.h>
#include <stdio.h>
#include <iostream>

static const double UnixEpoch    = 2440587.5; // julian day of 12am 1/1/1970
static const double SecondsInDay = 86400.0;

static const BzfString s_timeClock("timeClock");
static const BzfString s_infoLatitude("infoLatitude");
static const BzfString s_infoLongitude("infoLongitude");
static const BzfString s_universe("universe");
static const BzfString s_world("world");
static const BzfString s_starTransform("starTransform");
static const BzfString s_time("time");
static const BzfString s_sunAzimuth("sunAzimuth");
static const BzfString s_sunAltitude("sunAltitude");
static const BzfString s_moonAzimuth("moonAzimuth");
static const BzfString s_moonAltitude("moonAltitude");
static const BzfString s_moonTwist("moonTwist");
static const BzfString s_moonPhase("moonPhase");

//
// SceneManager
//

SceneManager*			SceneManager::mgr = NULL;

SceneManager::SceneManager() : time(0.0f)
{
	// create built-in nodes
	init();

	// no fade
	setFade(0.0f, 0.0f, 0.0f, 0.0f);

	// watch for changes to the clock
	julianDay   = UnixEpoch;
	timeChanged = true;
	BZDB->addCallback(s_timeClock, onTimeOfDayCB, (void*)this);
	BZDB->addCallback(s_infoLatitude, onTimeOfDayCB, (void*)this);
	BZDB->addCallback(s_infoLongitude, onTimeOfDayCB, (void*)this);
}

SceneManager::~SceneManager()
{
	// stop watching for changes to the clock
	BZDB->removeCallback(s_timeClock, onTimeOfDayCB, this);
	BZDB->removeCallback(s_infoLatitude, onTimeOfDayCB, this);
	BZDB->removeCallback(s_infoLongitude, onTimeOfDayCB, this);

	// unref nodes
	clearModels();
	clear();
	staticScene->unref();
	dynamicScene->unref();
}

void					SceneManager::init()
{
	staticScene   = new SceneNodeGroup;			// static scene node goes here
	dynamicScene  = new SceneNodeGroup;			// dynamic nodes go here
	scene         = NULL;
	starXForm     = NULL;							// star orientation
}

void					SceneManager::clear()
{
	staticScene->clearChildren();
	clearDynamic();
}

void					SceneManager::clearModels()
{
	// unref remaining models
	for (ModelMap::iterator index = models.begin();
								index != models.end(); ++index)
		index->second->unref();
	models.clear();
	scene      = NULL;
	starXForm  = NULL;
}

void					SceneManager::clearDynamic()
{
	dynamicScene->clearChildren();
}

void					SceneManager::onTimeOfDay(double time)
{
	julianDay = time;
}

void					SceneManager::onTimeOfDayCB(
								const BzfString& name, void* _self)
{
	SceneManager* self = reinterpret_cast<SceneManager*>(_self);
	if (name == s_timeClock) {
		double t;
		if (sscanf(BZDB->get(name).c_str(), "%lf", &t) == 1)
			self->onTimeOfDay(UnixEpoch + t / SecondsInDay);
	}
	self->timeChanged = true;
}

void					SceneManager::open()
{
	// remove saved models
	clearModels();

	// remove non-built-in objects
	clear();
}

void					SceneManager::read(istream* stream,
								 const BzfString& filename)
{
	// ignore streams we can't read
	if (stream == NULL || !*stream)
		return;

	SceneVisitorFindAll finder;
	try {
		// read XML
		XMLTree xmlTree;
		xmlTree.read(*stream, XMLStreamPosition(filename));

		// parse scene
		SceneReader reader;
		SceneNode* node = reader.parse(xmlTree.begin());

		// now find all the named nodes
		if (node != NULL) {
			finder.traverse(node);
			node->unref();
		}
	}
	catch (XMLIOException& e) {
		printError("%s (%d,%d): %s",
							e.position.filename.c_str(),
							e.position.line,
							e.position.column,
							e.what());
	}

	// and transfer them to our map
	const unsigned int n = finder.size();
	for (unsigned int i = 0; i < n; ++i) {
		SceneNode* node = finder.get(i);
		node->ref();

		// replace any existing node of the same name, otherwise just insert
		ModelMap::iterator index = models.find(node->getID());
		if (index != models.end()) {
			// replace node
			index->second->unref();
			index->second = node;
		}
		else {
			// insert node
			models.insert(std::make_pair(node->getID(), node));
		}
	}
}

void					SceneManager::close()
{
	// lookup interesting nodes
	SceneNode* tmpScene = findNoRef(s_universe);
	SceneNode* tmpWorld = findNoRef(s_world);
	SceneNode* tmpStarXForm = findNoRef(s_starTransform);
	if (tmpScene != NULL) {
		scene = tmpScene;
	}
	if (tmpWorld != NULL) {
		SceneNodeGroup* world = static_cast<SceneNodeGroup*>(tmpWorld);
		world->pushChild(staticScene);
		world->pushChild(dynamicScene);
	}
	if (tmpStarXForm != NULL) {
		starXForm = static_cast<SceneNodeMatrixTransform*>(tmpStarXForm);
	}

	// force update of clock
	onTimeOfDayCB(s_timeClock, this);
}

void					SceneManager::setStatic(SceneNode* node)
{
	staticScene->clearChildren();
	staticScene->pushChild(node);
}

void					SceneManager::openDynamic()
{
	clearDynamic();
}

void					SceneManager::addDynamic(SceneNode* node)
{
	dynamicScene->pushChild(node);
}

void					SceneManager::closeDynamic()
{
	// do nothing
}

ViewFrustum&			SceneManager::getView()
{
	return view;
}

const ViewFrustum&		SceneManager::getView() const
{
	return view;
}

void					SceneManager::setTime(float _time)
{
	time = _time;
}

void					SceneManager::initParams(
								SceneVisitorParams& params)
{
	params.pushFloat(s_time, 0.0f);
	params.pushFloat(s_sunAzimuth, 0.0f);
	params.pushFloat(s_sunAltitude, 0.0f);
	params.pushFloat(s_moonAzimuth, 0.0f);
	params.pushFloat(s_moonAltitude, 0.0f);
	params.pushFloat(s_moonTwist, 0.0f);
	params.pushFloat(s_moonPhase, 0.0f);
	setParams(params);
}

void					SceneManager::setParams(
								SceneVisitorParams& params)
{
	updateSkyStuff();
	params.setFloat(s_time, time);
	params.setFloat(s_sunAzimuth, fmodf(360.0f + sunAzimuth, 360.0f) / 360.0f);
	params.setFloat(s_sunAltitude, (sunAltitude + 90.0f) / 180.0f);
	params.setFloat(s_moonAzimuth, fmodf(360.0f + moonAzimuth, 360.0f) / 360.0f);
	params.setFloat(s_moonAltitude, (moonAltitude + 90.0f) / 180.0f);
	params.setFloat(s_moonTwist, fmodf(360.0f + moonTwist, 360.0f) / 360.0f);
	params.setFloat(s_moonPhase, (1.0f - moonPhase) / 2.0f);
}

void					SceneManager::setFade(
								float r, float g, float b, float a)
{
	fade[0] = r;
	fade[1] = g;
	fade[2] = b;
	fade[3] = a;
}

SceneNode*				SceneManager::getScene() const
{
	if (scene != NULL)
		scene->ref();
	return scene;
}

SceneNode*				SceneManager::getStaticScene() const
{
	SceneNode* node;
	if (staticScene->size() == 0) {
		node = NULL;
	}
	else {
		node = staticScene->getChild(0);
		if (node != NULL)
			node->ref();
	}
	return node;
}

float					SceneManager::getTime() const
{
	return time;
}

const float*				SceneManager::getFade() const
{
	return fade;
}

SceneNode*				SceneManager::find(const BzfString& name) const
{
	SceneNode* node = findNoRef(name);
	if (node != NULL)
		node->ref();
	return node;
}

SceneNode*				SceneManager::findNoRef(const BzfString& name) const
{
	ModelMap::const_iterator index = models.find(name);
	if (index == models.end())
		return NULL;
	return index->second;
}

SceneManager*			SceneManager::getInstance()
{
	if (mgr == NULL)
		mgr = new SceneManager;
	return mgr;
}

void					SceneManager::updateSkyStuff()
{
	if (!timeChanged)
		return;
	timeChanged = false;

	// get the latitude and longitude
	float latitude = (float)atof(BZDB->get(s_infoLatitude).c_str());
	if (latitude < -90.0f)
		latitude = -90.0f;
	else if (latitude > 90.0f)
		latitude = 90.0f;
	float longitude = fmodf((float)atof(BZDB->get(s_infoLongitude).c_str()), 360.0f);

	// get position of sun and moon at 0,0 lat/long
	float sunDir[3], moonDir[3];
	getSunPosition(julianDay, latitude, longitude, sunDir);
	getMoonPosition(julianDay, latitude, longitude, moonDir);

	// update sky stuff
	updateStars(latitude, longitude);
	updateSun(sunDir, moonDir);
	updateMoon(sunDir, moonDir);
}

void					SceneManager::updateStars(
								float latitude, float longitude)
{
	if (starXForm != NULL) {
		float matrix[16];
		getCelestialTransform(julianDay, latitude, longitude,
								reinterpret_cast<float(*)[4]>(matrix));
		starXForm->matrix.set(matrix, 16);
	}
}

void					SceneManager::updateSun(
								const float* sunDir, const float* /*moonDir*/)
{
	// compute rotation angles
	sunAzimuth  = atan2f(sunDir[1], sunDir[0]) * 180.0f / M_PI;
	sunAltitude = asinf(sunDir[2]) * 180.0f / M_PI;
}

void					SceneManager::updateMoon(
								const float* sunDir, const float* moonDir)
{
	// compute rotation angles
	float sun2[3];
	moonAzimuth  = atan2f(moonDir[1], moonDir[0]);
	moonAltitude = asinf(moonDir[2]);
	sun2[0] = sunDir[0] * cosf(moonAzimuth)  + sunDir[1] * sinf(moonAzimuth);
	sun2[1] = sunDir[1] * cosf(moonAzimuth)  - sunDir[0] * sinf(moonAzimuth);
	sun2[2] = sunDir[2] * cosf(moonAltitude) - sun2[0]   * sinf(moonAltitude);
	moonTwist     = atan2f(sun2[2], sun2[1]) * 180.0f / M_PI;
	moonAzimuth  *= 180.0f / M_PI;
	moonAltitude *= 180.0f / M_PI;

	// compute display list for moon
	moonPhase =   moonDir[0] * sunDir[0] +
				moonDir[1] * sunDir[1] +
				moonDir[2] * sunDir[2];
}
