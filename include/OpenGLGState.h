/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* OpenGLGState:
 *	Encapsulates OpenGL rendering state information.
 */

#ifndef	BZF_OPENGL_GSTATE_H
#define	BZF_OPENGL_GSTATE_H

#include "common.h"
#include "bzfgl.h"

class OpenGLMaterial;
class OpenGLGStateRep;
class OpenGLGStateState;
class RenderNode;

typedef void		(*OpenGLContextFunction)(void* userData);

class OpenGLGState {
  friend class OpenGLGStateBuilder;
  friend class ContextInitializer;
  public:
			OpenGLGState();
			OpenGLGState(const OpenGLGState&);
			OpenGLGState(const OpenGLGStateState&);
			~OpenGLGState();
    OpenGLGState&	operator=(const OpenGLGState& state);
    void		setState() const;
    bool		getNeedsSorting() const;
    bool		isBlended() const;
    bool		isTextured() const;
    bool		isTextureMatrix() const;
    bool		isSphereMap() const;
    bool		isLighted() const;
    void		addRenderNode(RenderNode* node) const;
    static void		resetState();
    static void		clearLists();
    static void		renderLists();
    static void		setStipple(GLfloat alpha);
    static void		setStippleIndex(int index);
    static int		getStippleIndex(float alpha);
    static int		getOpaqueStippleIndex();

    static void		init();
    static bool		haveGLContext();

    // these are in OpenGLGState for lack of a better place.  register...
    // is for clients to add a function to call when the OpenGL context
    // has been destroyed and must be recreated.  the function is called
    // by initContext() and initContext() will call all initializers in
    // the order they were registered, plus reset the OpenGLGState state.
    //

    //
    // The freeFunc()'s job is to make sure that all references that the
    // owning object might use are invalidated. It should also deallocate
    // any GL objects that it might have collected. If the references are
    // not invalidated, then the owner may end up deleting GL objects that
    // it does not own, because the references have been changed during the
    // context switch (ex: another routine might have received the same
    // references when making new objects.)
    //
    // The initFunc()'s job is to reallocate GL objects for its owner.
    //
    // Destroying and recreating the OpenGL context is only necessary on
    // platforms that cannot abstract the graphics system sufficiently.
    // for example, on win32, changing the display bit depth will cause
    // most OpenGL drivers to crash unless we destroy the context before
    // the switch and recreate it afterwards.
    //
    static void		registerContextInitializer(
				OpenGLContextFunction freeFunc,
				OpenGLContextFunction initFunc,
				void* userData);

    static void		unregisterContextInitializer(
				OpenGLContextFunction freeFunc,
				OpenGLContextFunction initFunc,
				void* userData);

    static void		initContext();
    static bool		getExecutingFreeFuncs();
    static bool		getExecutingInitFuncs();

  private:
    static void		initGLState();
    static void		freeStipple(void*);
    static void		initStipple(void*);

    class ContextInitializer {
      public:
	friend class OpenGLGState;
	ContextInitializer(OpenGLContextFunction freeFunc,
			   OpenGLContextFunction initFunc,
			   void* data);
	~ContextInitializer();

	static void executeFreeFuncs();
	static void executeInitFuncs();

	static ContextInitializer* find(OpenGLContextFunction freeFunc,
					OpenGLContextFunction initFunc,
					void* data);

      public:
	OpenGLContextFunction freeCallback;
	OpenGLContextFunction initCallback;
	void* userData;

	ContextInitializer* prev;
	ContextInitializer* next;
	static ContextInitializer* head;
	static ContextInitializer* tail;
    };

  private:
    OpenGLGStateRep*	rep;
    static GLuint	stipples;
  public:
    static bool executingFreeFuncs;
    static bool executingInitFuncs;
};

inline bool OpenGLGState::getExecutingFreeFuncs()
{
  return executingFreeFuncs;
}
inline bool OpenGLGState::getExecutingInitFuncs()
{
  return executingInitFuncs;
}


class OpenGLGStateBuilder {
  public:
			OpenGLGStateBuilder();
			OpenGLGStateBuilder(const OpenGLGState&);
			~OpenGLGStateBuilder();
    OpenGLGStateBuilder &operator=(const OpenGLGState&);

    void		reset();
    void		enableTexture(bool = true);
    void		enableTextureMatrix(bool = true);
    void		enableSphereMap(bool = true);
    void		enableMaterial(bool = true);
    void		resetBlending();
    void		resetSmoothing();
    void		resetAlphaFunc();
    void		setTexture(const int texture);
    void		setTextureMatrix(const GLfloat* matrix);
    void		setTextureEnvMode(GLenum mode = GL_MODULATE);
    void		setMaterial(const OpenGLMaterial& material);
    void		setBlending(GLenum sFactor = GL_SRC_ALPHA,
				    GLenum dFactor = GL_ONE_MINUS_SRC_ALPHA);
    void		setStipple(float alpha);
    void		setSmoothing(bool smooth = true);
    void		setCulling(GLenum culling);
    void		setShading(GLenum shading = GL_SMOOTH);
    void		setAlphaFunc(GLenum func = GL_GEQUAL,
				     GLclampf ref = 0.1f);
    void		setNeedsSorting(bool);
    OpenGLGState	getState() const;

  private:
    void		init(const OpenGLGState&);

  private:
    OpenGLGStateState*	state;
};


#endif // BZF_OPENGL_GSTATE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

