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

#include "OpenGLGState.h"
#include "OpenGLTexture.h"
#include "TimeKeeper.h"
#include "bzfgl.h"

//
// GState
//

GState::GState() :texture(),
								texEnv(kModulate),
								shadingModel(kConstant),
								blendingSrc(kOne),
								blendingDst(kZero),
								smoothing(false),
								culling(true),
								dithering(true),
								alphaFunc(kAlways),
								alphaFuncRef(0.0f),
								depthFunc(kAlways),
								depthMask(true),
								pointSize(1.0f),
								stipple(false),
								pass(0),
								forceTexture(false),
								forceBlending(false),
								forceSmoothing(false),
								forceDithering(false)
{
	// do nothing
}

//
// OpenGLGStateRep
//

class OpenGLGStateRep : public GState {
public:
	OpenGLGStateRep();
	~OpenGLGStateRep();

	void				ref();
	void				unref();
	OpenGLGStateRep*	makeUnique();

	void				freeze();
	void				setState();

	static void			initState();
	static void			resetState();
	static void			resetCurrentState();

	// client should only modify force* members
	static OpenGLGStateRep*	getDefault();

	static void			instrReset();
	static const OpenGLGState::Instruments*	instrGet();

private:
	OpenGLGStateRep(const OpenGLGStateRep&);

	void				doSetState();

	static void			init();

private:
	int					refCount;
	GLint				oglTexEnv;
	GLenum				oglShadingModel;
	GLenum				oglBlendingSrc;
	GLenum				oglBlendingDst;
	GLenum				oglAlphaFunc;
	GLenum				oglDepthFunc;

	static bool			initialized;
	static OpenGLGStateRep* defaultState;
	static OpenGLGStateRep* currentState;

	static TimeKeeper	instrTime;
	static OpenGLGState::Instruments	instruments;
};

bool					OpenGLGStateRep::initialized  = false;
OpenGLGStateRep*		OpenGLGStateRep::defaultState = NULL;
OpenGLGStateRep*		OpenGLGStateRep::currentState = NULL;
TimeKeeper				OpenGLGStateRep::instrTime;
OpenGLGState::Instruments	OpenGLGStateRep::instruments;

OpenGLGStateRep::OpenGLGStateRep() : GState(), refCount(1)
{
	init();
}

OpenGLGStateRep::OpenGLGStateRep(const OpenGLGStateRep& r) :
								GState(r),
								refCount(1)
{
	freeze();
}

OpenGLGStateRep::~OpenGLGStateRep()
{
	// do nothing
}

void					OpenGLGStateRep::init()
{
	if (!initialized) {
		initialized = true;

		// construct default state.  the force* members of the default
		// state define the globally enabled/disabled state.  if
		// forceX is false then X is enabled, if forceX is true then
		// X is disabled and can't be modified except by gstates that
		// have forceX true.
		defaultState = new OpenGLGStateRep;
		defaultState->freeze();

		// set the current state
		currentState = defaultState;
		currentState->ref();
	}
}

void					OpenGLGStateRep::ref()
{
	++refCount;
}

void					OpenGLGStateRep::unref()
{
	if (--refCount == 0)
		delete this;
}

OpenGLGStateRep*		OpenGLGStateRep::makeUnique()
{
	// if we're only ref then we're already unique
	if (refCount == 1)
		return this;

	// one less ref of this object
	--refCount;

	// return copy of this
	return new OpenGLGStateRep(*this);
}

void					OpenGLGStateRep::freeze()
{
	static const GLint mapTexEnv[] = { GL_MODULATE, GL_DECAL, GL_REPLACE };
	static const GLenum mapShadeModel[] = { GL_FLAT, GL_SMOOTH };
	static const GLenum mapBlendSrc[] = {
								GL_ZERO,      GL_ONE,
								GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
								GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
								GL_ZERO,      GL_ONE,
								GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR
						};
	static const GLenum mapBlendDst[] = {
								GL_ZERO,      GL_ONE,
								GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
								GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
								GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
								GL_ZERO,      GL_ONE
						};
	static const GLenum mapFunc[] = {
								GL_NEVER,   GL_ALWAYS,
								GL_EQUAL,   GL_NOTEQUAL,
								GL_LESS,    GL_LEQUAL,
								GL_GREATER, GL_GEQUAL
						};

	// convert to OpenGL constants
	oglTexEnv         = mapTexEnv[texEnv];
	oglShadingModel   = mapShadeModel[shadingModel];
	oglBlendingSrc    = mapBlendSrc[blendingSrc];
	oglBlendingDst    = mapBlendDst[blendingDst];
	oglAlphaFunc      = mapFunc[alphaFunc];
	oglDepthFunc      = mapFunc[depthFunc];
}

void					OpenGLGStateRep::initState()
{
	// initialize GL state to what the default state is
	init();
	currentState->unref();
	currentState = defaultState;
	currentState->ref();

	// misc
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glClearStencil(0);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glFrontFace(GL_CCW);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_SCISSOR_TEST);
	glDisable(GL_LINE_STIPPLE);
	glDisable(GL_POLYGON_STIPPLE);

	// texturing
	if (currentState->texture.isValid() && !currentState->forceTexture) {
		currentState->texture.execute();
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, currentState->oglTexEnv);
		glEnable(GL_TEXTURE_2D);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	// shading
	glShadeModel(currentState->oglShadingModel);

	// blending
	if (currentState->forceBlending ||
		(currentState->oglBlendingSrc == GL_ONE &&
		currentState->oglBlendingDst == GL_ZERO)) {
		glDisable(GL_BLEND);
	}
	else {
		glBlendFunc(currentState->oglBlendingSrc, currentState->oglBlendingDst);
		glEnable(GL_BLEND);
	}

	// smoothing
	if (!currentState->forceSmoothing && currentState->smoothing) {
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
	}
	else {
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}

	// culling
	if (currentState->culling) {
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	// dithering
	if (currentState->dithering) {
		glEnable(GL_DITHER);
	}
	else {
		glDisable(GL_DITHER);
	}

	// alpha func
	if (currentState->oglAlphaFunc == GL_ALWAYS) {
		glDisable(GL_ALPHA_TEST);
	}
	else {
		glAlphaFunc(currentState->oglAlphaFunc, currentState->alphaFuncRef);
		glEnable(GL_ALPHA_TEST);
	}

	// depth func
	if (currentState->oglDepthFunc == GL_ALWAYS) {
		glDisable(GL_DEPTH_TEST);
	}
	else {
		glDepthFunc(currentState->oglDepthFunc);
		glEnable(GL_DEPTH_TEST);
	}

	// depth mask
	glDepthMask(currentState->depthMask ? GL_TRUE : GL_FALSE);

	// point size
	glPointSize(currentState->pointSize);

	// stipple
	if (currentState->stipple) {
		glEnable(GL_LINE_STIPPLE);
		glEnable(GL_POLYGON_STIPPLE);
	}
	else {
		glDisable(GL_LINE_STIPPLE);
		glDisable(GL_POLYGON_STIPPLE);
	}
}

void					OpenGLGStateRep::resetState()
{
	defaultState->setState();
}

void					OpenGLGStateRep::resetCurrentState()
{
	currentState->doSetState();
}

void					OpenGLGStateRep::setState()
{
	++instruments.nState;

	// ignore if setting to the same state
	if (this == currentState)
		return;

	// do it
	doSetState();
}

void					OpenGLGStateRep::doSetState()
{
	// change state that's not the same

	// texture
	if ((!defaultState->forceTexture &&
			texture != currentState->texture) ||
		(defaultState->forceTexture &&
			forceTexture != currentState->forceTexture)) {
		++instruments.nTexture;
		if (texture.isValid() &&
			(forceTexture || !defaultState->forceTexture)) {
			texture.execute();
			if (!currentState->texture.isValid())
				glEnable(GL_TEXTURE_2D);
		}
		else {
			glDisable(GL_TEXTURE_2D);
		}
	}

	// tex env
	if (oglTexEnv != currentState->oglTexEnv) {
		++instruments.nTexEnv;
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, oglTexEnv);
	}

	// shading model
	if (oglShadingModel != currentState->oglShadingModel) {
		++instruments.nShading;
		glShadeModel(oglShadingModel);
	}

	// blending func
	if ((!defaultState->forceBlending &&
			(oglBlendingSrc != currentState->oglBlendingSrc ||
			 oglBlendingDst != currentState->oglBlendingDst)) ||
		(defaultState->forceBlending &&
			forceBlending != currentState->forceBlending)) {
		++instruments.nBlending;
		if ((oglBlendingSrc != GL_ONE || oglBlendingDst != GL_ZERO) &&
			(forceBlending || !defaultState->forceBlending)) {
			glBlendFunc(oglBlendingSrc, oglBlendingDst);
			glEnable(GL_BLEND);
		}
		else {
			glDisable(GL_BLEND);
		}
	}


	// smoothing
	if ((!defaultState->forceSmoothing &&
			smoothing != currentState->smoothing) ||
		(defaultState->forceSmoothing &&
			forceSmoothing != currentState->forceSmoothing)) {
		++instruments.nSmoothing;
		if (smoothing && (forceSmoothing || !defaultState->forceSmoothing)) {
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_POINT_SMOOTH);
		}
		else {
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_POINT_SMOOTH);
		}
	}

	// culling
	if (culling != currentState->culling) {
		++instruments.nCulling;
		if (culling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

	// dithering
	if ((!defaultState->forceDithering &&
			dithering != currentState->dithering) ||
		(defaultState->forceDithering &&
			forceDithering != currentState->forceDithering)) {
		if (dithering && (forceDithering || !defaultState->forceDithering)) {
			glEnable(GL_DITHER);
		}
		else {
			glDisable(GL_DITHER);
		}
	}

	// alpha func
	if (oglAlphaFunc != currentState->oglAlphaFunc ||
		alphaFuncRef != currentState->alphaFuncRef) {
		++instruments.nAlphaFunc;
		if (oglAlphaFunc == GL_ALWAYS) {
			glDisable(GL_ALPHA_TEST);
		}
		else {
			glAlphaFunc(oglAlphaFunc, alphaFuncRef);
			glEnable(GL_ALPHA_TEST);
		}
	}

	// depth func
	if (oglDepthFunc != currentState->oglDepthFunc) {
		++instruments.nDepthFunc;
		if (oglDepthFunc == GL_ALWAYS) {
			glDisable(GL_DEPTH_TEST);
		}
		else {
			glDepthFunc(oglDepthFunc);
			glEnable(GL_DEPTH_TEST);
		}
	}

	// depth mask
	if (depthMask != currentState->depthMask) {
		++instruments.nDepthMask;
		glDepthMask(depthMask ? GL_TRUE : GL_FALSE);
	}

	// point size
	if (pointSize != currentState->pointSize) {
		++instruments.nPointSize;
		glPointSize(pointSize);
	}

	// stipple
	if (stipple != currentState->stipple) {
		++instruments.nStipple;
		if (stipple) {
			glEnable(GL_LINE_STIPPLE);
			glEnable(GL_POLYGON_STIPPLE);
		}
		else {
			glDisable(GL_LINE_STIPPLE);
			glDisable(GL_POLYGON_STIPPLE);
		}
	}

	// record most recent rep
	ref();
	currentState->unref();
	currentState = this;
}

OpenGLGStateRep*		OpenGLGStateRep::getDefault()
{
	return defaultState;
}

void					OpenGLGStateRep::instrReset()
{
	instrTime  = TimeKeeper::getCurrent();
	instruments.time       = 0.0f;
	instruments.nState     = 0;
	instruments.nTexture   = 0;
	instruments.nTexEnv    = 0;
	instruments.nShading   = 0;
	instruments.nBlending  = 0;
	instruments.nSmoothing = 0;
	instruments.nCulling   = 0;
	instruments.nAlphaFunc = 0;
	instruments.nDepthFunc = 0;
	instruments.nDepthMask = 0;
	instruments.nPointSize = 0;
	instruments.nStipple   = 0;
}

const OpenGLGState::Instruments*	OpenGLGStateRep::instrGet()
{
	instruments.time = TimeKeeper::getCurrent() - instrTime;
	return &instruments;
}


//
// OpenGLGState
//

CallbackList<GraphicsContextInitializer> OpenGLGState::initList;

OpenGLGState::OpenGLGState()
{
	rep = new OpenGLGStateRep();
	rep->freeze();
}

OpenGLGState::OpenGLGState(const OpenGLGState& state)
{
	rep = state.rep;
	rep->ref();
}

OpenGLGState::OpenGLGState(OpenGLGStateRep* _rep)
{
	rep = _rep;
}

OpenGLGState::~OpenGLGState()
{
	rep->unref();
}

OpenGLGState&			OpenGLGState::operator=(const OpenGLGState& state)
{
	state.rep->ref();
	rep->unref();
	rep = state.rep;
	return *this;
}


void					OpenGLGState::setState() const
{
	rep->setState();
}

const GState*			OpenGLGState::getState() const
{
	return rep;
}

void					OpenGLGState::resetState()
{
	OpenGLGStateRep::resetState();
}

void					OpenGLGState::enableTexture(bool on)
{
	OpenGLGStateRep* rep = OpenGLGStateRep::getDefault();
	if (rep->forceTexture == on) {
		rep->forceTexture = !on;
		OpenGLGStateRep::resetCurrentState();
	}
}

void					OpenGLGState::enableBlending(bool on)
{
	OpenGLGStateRep* rep = OpenGLGStateRep::getDefault();
	if (rep->forceBlending == on) {
		rep->forceBlending = !on;
		OpenGLGStateRep::resetCurrentState();
	}
}

void					OpenGLGState::enableSmoothing(bool on)
{
	OpenGLGStateRep* rep = OpenGLGStateRep::getDefault();
	if (rep->forceSmoothing == on) {
		rep->forceSmoothing = !on;
		OpenGLGStateRep::resetCurrentState();
	}
}

void					OpenGLGState::enableDithering(bool on)
{
	OpenGLGStateRep* rep = OpenGLGStateRep::getDefault();
	if (rep->forceDithering == on) {
		rep->forceDithering = !on;
		OpenGLGStateRep::resetCurrentState();
	}
}

void					OpenGLGState::init()
{
	// initialize GL state to what we expect
	OpenGLGStateRep::initState();
}

void					OpenGLGState::addContextInitializer(
								GraphicsContextInitializer callback,
								void* userData)
{
	initList.add(callback, userData);
}

void					OpenGLGState::removeContextInitializer(
								GraphicsContextInitializer callback,
								void* userData)
{
	initList.remove(callback, userData);
}

static bool				onInitContext(
								GraphicsContextInitializer func,
								void* userData, void* flag)
{
	func(*reinterpret_cast<bool*>(flag), userData);
	return true;
}

void					OpenGLGState::freeContext()
{
	// call each initializer in order with destroy == true
	bool flag = true;
	initList.iterate(&onInitContext, &flag);
}

void					OpenGLGState::initContext()
{
	// initialize GL state and reset shadow state
	OpenGLGStateRep::initState();

	// call each initializer in order with destroy == false
	bool flag = false;
	initList.iterate(&onInitContext, &flag);

	// initialize the GL state again in case one of the initializers
	// messed it up.
	OpenGLGStateRep::initState();
}

void					OpenGLGState::instrReset()
{
	OpenGLGStateRep::instrReset();
}

const OpenGLGState::Instruments*	OpenGLGState::instrGet()
{
	return OpenGLGStateRep::instrGet();
}


//
// OpenGLGStateBuilder
//

OpenGLGStateBuilder::OpenGLGStateBuilder()
{
	rep  = new OpenGLGStateRep;
	data = rep;
}

OpenGLGStateBuilder::OpenGLGStateBuilder(const OpenGLGState& gstate)
{
	gstate.rep->ref();
	rep  = gstate.rep->makeUnique();
	data = rep;
}

OpenGLGStateBuilder::~OpenGLGStateBuilder()
{
	rep->unref();
}

OpenGLGStateBuilder&	OpenGLGStateBuilder::operator=(
								const OpenGLGState& gstate)
{
	gstate.rep->ref();
	rep->unref();
	rep  = gstate.rep->makeUnique();
	data = rep;
	return *this;
}

void					OpenGLGStateBuilder::reset()
{
	rep->unref();
	rep  = new OpenGLGStateRep;
	data = rep;
}

OpenGLGState			OpenGLGStateBuilder::getState() const
{
	OpenGLGStateRep* unique = rep;
	unique->ref();
	unique = unique->makeUnique();
	unique->freeze();
	return OpenGLGState(unique);
}
