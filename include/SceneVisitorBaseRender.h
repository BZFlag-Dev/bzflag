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

#ifndef BZF_SCENE_VISITOR_BASE_RENDER_H
#define BZF_SCENE_VISITOR_BASE_RENDER_H

#include "SceneVisitor.h"
#include <string>

class SceneVisitorBaseRender : public SceneVisitor {
public:
	SceneVisitorBaseRender();
	virtual ~SceneVisitorBaseRender();

	// set the viewport size (in pixels)
	void				setArea(float size);

	// set the current time and frame
	void				setTime(float t);
	void				setFrame(float frameNumber);

	// instrumentation methods.  counts are reset with each call to
	// traverse().
	struct Instruments {
	public:
		float			time;
		unsigned int	nNodes;
		unsigned int	nTransforms;
		unsigned int	nPoints;
		unsigned int	nLines;
		unsigned int	nTriangles;
		unsigned int	nQuads;
	};
	const Instruments*	instrGet() const;

	// set the stipple masks to a density appropriate to alpha
	void				setStipple(float alpha);

	// SceneVisitor overrides
	virtual bool		traverse(SceneNode*);

protected:
	// returns true iff displayDebug is true in the database
	const bool			isDebugging() const;

	// get non-const instruments
	Instruments*		getInstr();

	// SceneVisitor overrides
	virtual bool		descend(SceneNodeGroup*);

private:
    Instruments			instruments;

	static const std::string	s_nameArea, s_nameTime, s_nameFrame;
	static const std::string	s_nameDebug;
	static const std::string	s_renderSmoothing;
	static const std::string	s_renderBlending;
	static const std::string	s_renderLighting;
	static const std::string	s_renderTexturing;
};

#endif
