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

/* OpenGLGState:
 *	Encapsulates OpenGL rendering state information.
 */

#ifndef	BZF_OPENGL_GSTATE_H
#define	BZF_OPENGL_GSTATE_H

#include "bzfgl.h"
#include "common.h"
#if defined(_MACOSX_)
#include <OpenGL/gl.h>
#endif

class OpenGLTexture;
class OpenGLMaterial;
class OpenGLGStateRep;
class OpenGLGStateState;
class RenderNode;

typedef void		(*OpenGLContextInitializer)(void* userData);

class OpenGLGState {
  friend class OpenGLGStateBuilder;
  public:
			OpenGLGState();
			OpenGLGState(const OpenGLGState&);
			OpenGLGState(const OpenGLGStateState&);
			~OpenGLGState();
    OpenGLGState&	operator=(const OpenGLGState& state);
    void		setState() const;
    boolean		isBlended() const;
    boolean		isTextured() const;
    boolean		isTextureReplace() const;
    boolean		isLighted() const;
    void		addRenderNode(RenderNode* node) const;
    static void		resetState();
    static void		clearLists();
    static void		renderLists();
    static void		setStipple(GLfloat alpha);
    static void		setStippleIndex(int index);
    static int		getStippleIndex(float alpha);
    static int		getOpaqueStippleIndex();

    static void		init();

    // these are in OpenGLGState for lack of a better place.  register...
    // is for clients to add a function to call when the OpenGL context
    // has been destroyed and must be recreated.  the function is called
    // by initContext() and initContext() will call all initializers in
    // the order they were registered, plus reset the OpenGLGState state.
    //
    // destroying and recreating the OpenGL context is only necessary on
    // platforms that cannot abstract the graphics system sufficiently.
    // for example, on win32, changing the display bit depth will cause
    // most OpenGL drivers to crash unless we destroy the context before
    // the switch and recreate it afterwards.
    static void		registerContextInitializer(
				OpenGLContextInitializer,
				void* userData = NULL);
    static void		unregisterContextInitializer(
				OpenGLContextInitializer,
				void* userData = NULL);
    static void		initContext();

  private:
    static void		initGLState();
    static void		initStipple(void* = NULL);

    struct ContextInitializer {
      public:
	ContextInitializer(OpenGLContextInitializer, void*);
	~ContextInitializer();
	static void		   execute();
	static ContextInitializer* find(OpenGLContextInitializer, void*);

      public:
	OpenGLContextInitializer callback;
	void*			userData;
	ContextInitializer*	prev;
	ContextInitializer*	next;
	static ContextInitializer* head;
	static ContextInitializer* tail;
    };

  private:
    OpenGLGStateRep*	rep;
    static GLuint	stipples;
};

class OpenGLGStateBuilder {
  public:
			OpenGLGStateBuilder();
			OpenGLGStateBuilder(const OpenGLGState&);
			~OpenGLGStateBuilder();
    OpenGLGStateBuilder &operator=(const OpenGLGState&);

    void		reset();
    void		enableTexture(boolean = True);
    void		enableTextureReplace(boolean = True);
    void		enableMaterial(boolean = True);
    void		resetBlending();
    void		resetSmoothing();
    void		resetAlphaFunc();
    void		setTexture(const OpenGLTexture& texture);
    void		setMaterial(const OpenGLMaterial& material);
    void		setBlending(GLenum sFactor = GL_SRC_ALPHA,
				    GLenum dFactor = GL_ONE_MINUS_SRC_ALPHA);
    void		setStipple(float alpha);
    void		setSmoothing(boolean smooth = True);
    void		setCulling(GLenum culling);
    void		setShading(GLenum shading = GL_SMOOTH);
    void		setAlphaFunc(GLenum func = GL_GEQUAL,
				     GLclampf ref = 0.1f);
    OpenGLGState	getState() const;

  private:
    void		init(const OpenGLGState&);

  private:
    OpenGLGStateState*	state;
};

#endif // BZF_OPENGL_GSTATE_H
// ex: shiftwidth=2 tabstop=8
