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

#ifndef BZF_SCENE_VISITOR_RENDER_H
#define BZF_SCENE_VISITOR_RENDER_H

#include "SceneVisitorBaseRender.h"
#include "SceneNodeParticleSystem.h"
#include "OpenGLGState.h"
#include "BoundingBox.h"
#include "math3D.h"
#include <string>
#include <vector>

class SceneVisitorRender : public SceneVisitorBaseRender {
public:
	SceneVisitorRender();
	virtual ~SceneVisitorRender();

	// SceneVisitor overrides
	virtual bool		traverse(SceneNode*);
	virtual bool		visit(SceneNodeAnimate*);
	virtual bool		visit(SceneNodeBaseTransform*);
	virtual bool		visit(SceneNodeGState*);
	virtual bool		visit(SceneNodeGeometry*);
	virtual bool		visit(SceneNodeLight*);
	virtual bool		visit(SceneNodeParameters*);
	virtual bool		visit(Particle*);
	virtual bool		visit(SceneNodeParticleSystem*);
	virtual bool		visit(SceneNodePrimitive*);
	virtual bool		visit(SceneNodeSelector*);

private:
	void				sort();
	void				draw();

	void				computeFrustum();
	bool				isCulled(const Real aabb[2][3]) const;
	void				setPlane(unsigned int index, const Vec3& p,
								const Vec3& v1, const Vec3& v2);

	struct Job {
	public:
		Job(const OpenGLGState&);

	public:
		SceneNodePrimitive*			primitive;
		Particle*					particle;
		float						depth;
		OpenGLGState				gstate;
		const GState*				compare;
		const SceneNodeVFFloat*		stipple;
		const SceneNodeVFFloat*		color;
		const SceneNodeVFFloat*		texcoord;
		const SceneNodeVFFloat*		normal;
		const SceneNodeVFFloat*		vertex;
		unsigned int				xformView;
		unsigned int				xformProjection;
		unsigned int				xformTexture;
		unsigned int				lightSet;
	};
	static bool			jobLess(const Job&, const Job&);

private:
	typedef std::vector<OpenGLGState>	GStateStack;
	typedef std::vector<Matrix>		XFormStack;
	typedef std::vector<const SceneNodeVFFloat*> GeometryStack;
	typedef std::vector<float>		ShininessStack;
	typedef std::vector<SceneNodeGeometry*> GeometryNodeStack;

	struct LightInfo {
	public:
		float			ambient[4];
		float			diffuse[4];
		float			specular[4];
		float			position[4];
		float			spotDirection[3];
		float			spotExponent;
		float			spotCutoff;
		float			attenuation[3];
	};
	typedef std::vector<unsigned int>	IndexStack;
	typedef std::vector<LightInfo>		LightStack;
	typedef std::vector<LightInfo>		LightList;
	struct LightSet {
	public:
		unsigned int	size;
		unsigned int	index[8];
	};
	typedef std::vector<LightSet>	LightSetList;
	typedef std::vector<Matrix>		MatrixList;
	typedef std::vector<Job>		JobList;
	enum CullingState { kCullOld, kCullDirty, kCullNo, kCullYes };

	std::string			nameMask, nameLighting;

	SceneNodeVFFloat	dummyParams;
	GStateStack			gstateStack;
	XFormStack			modelXFormStack;
	XFormStack			projectionXFormStack;
	XFormStack			textureXFormStack;
	IndexStack			modelXFormIndexStack;
	IndexStack			projectionXFormIndexStack;
	IndexStack			textureXFormIndexStack;
	GeometryStack		stippleStack;
	GeometryStack		colorStack;
	GeometryStack		texcoordStack;
	GeometryStack		normalStack;
	GeometryStack		vertexStack;
	GeometryNodeStack	geometryStack;
	LightStack			lightStack;
	IndexStack			lightIndexStack;
	IndexStack			lightSetIndexStack;
	unsigned int		maxLights;

	MatrixList			matrixList;
	LightList			lightList;
	LightSetList		lightSetList;

	BoundingBox			boundingBox;
	Real				aaBoundingBox[2][3];
	CullingState		boundingBoxCull;
	bool				frustumDirty;
	Plane				frustum[6];
	unsigned int		frustumDir[6][3];

	SceneVisitor*		resetter;
	JobList				jobs;
};

#endif
// ex: shiftwidth=4 tabstop=4
