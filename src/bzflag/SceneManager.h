/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_SCENE_MANAGER_H
#define BZF_SCENE_MANAGER_H

#include "BzfString.h"
#include "ViewFrustum.h"
#include "bzfio.h"
#include <map>

#define SCENEMGR (SceneManager::getInstance())

class SceneNode;
class SceneNodeGroup;
class SceneNodeMatrixTransform;
class SceneVisitorParams;

class SceneManager {
public:
	~SceneManager();

	// load models.  open() clears existing models, read() adds
	// models and may be called multiple times, and close()
	// prepares the scene.  read() overwrites previously read
	// models of the same name.  on close(), a few nodes with
	// special names are found and prepared.
	void				open();
	void				read(istream* modelStream);
	void				close();

	// set static scene node
	void				setStatic(SceneNode*);

	// change dynamic nodes
	void				openDynamic();
	void				addDynamic(SceneNode*);
	void				closeDynamic();

	// get the view frustum
	ViewFrustum&		getView();
	const ViewFrustum&	getView() const;

	// set the time.  used by setParams().
	void				setTime(float time);

	// set parameters for time, sun and moon
	void				setParams(SceneVisitorParams& paramsToModify);

	// set the color of full-scene fade.  the color includes alpha.
	void				setFade(float r, float g, float b, float a);

	// get the scene.  the node is ref()'d.
	SceneNode*			getScene() const;

	// get the static scene.  the node is ref()'d.
	SceneNode*			getStaticScene() const;

	// get state
	float				getTime() const;
	const float*		getFade() const;

	// lookup a model by name.  returns NULL if there's no model by
	// that name.  if found the SceneNode is ref()'d.
	SceneNode*			find(const BzfString& name) const;

	// just like find but the node is not ref()'d
	SceneNode*			findNoRef(const BzfString& name) const;

	// get the single instance
	static SceneManager* getInstance();

private:
	SceneManager();

	// create the built-in nodes
	void				init();

	// remove all non-built-in scene nodes
	void				clear();

	// remove all models
	void				clearModels();

	// remove all dynamic nodes
	void				clearDynamic();

	// update built-in nodes
	void				updateSkyStuff();
	void				updateStars(float latitude, float longitude);
	void				updateSun(const float* sunDir, const float* moonDir);
	void				updateMoon(const float* sunDir, const float* moonDir);

	// handle changes to time
	void				onTimeOfDay(double);
	static void			onTimeOfDayCB(const BzfString&, void*);

private:
	typedef std::map<BzfString, SceneNode*> ModelMap;

	ViewFrustum			view;

	ModelMap			models;
	SceneNode*			scene;
	SceneNodeGroup*		staticScene;
	SceneNodeGroup*		dynamicScene;
	SceneNodeMatrixTransform*	starXForm;

	bool				timeChanged;
	double				julianDay;
	float				time;

	float				fade[4];

	float				sunAzimuth;
	float				sunAltitude;
	float				moonAzimuth;
	float				moonAltitude;
	float				moonTwist;
	float				moonPhase;

	static SceneManager*	mgr;
};

#endif
