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

/* SceneNode:
 *	Encapsulates information for rendering an object in the scene.
 *
 * GLfloat2
 * GLfloat3
 *	Arrays of two and three GLfloat's
 *
 * GLfloat2Array
 * GLfloat3Array
 *	Arrays of GLfloat2's and GLfloat3's
 *
 * Probably shouldn't use names like this (GLfloat...).  Oh well.
 */

#ifndef	BZF_SCENE_NODE_H
#define	BZF_SCENE_NODE_H

#include "bzfgl.h"
#include "common.h"
#include "OpenGLGState.h"
#include "RenderNode.h"

#if !defined(_WIN32)
// bonehead win32 cruft.  just make it go away on other platforms.
#define	__stdcall
#endif

#define	myColor3f(r, g, b)	SceneNode::glColor3f(r, g, b)
#define	myColor4f(r, g, b, a)	SceneNode::glColor4f(r, g, b, a)
#define	myColor3fv(rgb)		SceneNode::glColor3fv(rgb)
#define	myColor4fv(rgba)	SceneNode::glColor4fv(rgba)
#define	myStipple(alpha)	SceneNode::setStipple(alpha)

class ViewFrustum;
class SceneRenderer;

class SceneNode {
  public:
			SceneNode();
    virtual		~SceneNode();

    void		getRenderNodes(SceneRenderer&);
    const GLfloat*	getSphere() const;

    virtual const GLfloat* getPlane() const;
    virtual GLfloat	getDistance(const GLfloat* eye) const;
    virtual boolean	cull(const ViewFrustum&) const;
    virtual void	addLight(SceneRenderer&);
    virtual int		split(const float* plane,
				SceneNode*& front, SceneNode*& back) const;

    static void		setColorOverride(boolean = True);
    static void		glColor3f(GLfloat r, GLfloat g, GLfloat b)
				{ (*color3f)(r, g, b); }
    static void		glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
				{ (*color4f)(r, g, b, a); }
    static void		glColor3fv(const GLfloat* rgb)
				{ (*color3fv)(rgb); }
    static void		glColor4fv(const GLfloat* rgba)
				{ (*color4fv)(rgba); }
    static void		setStipple(GLfloat alpha)
				{ (*stipple)(alpha); }

  protected:
    void		setRadius(GLfloat radiusSquared);
    void		setCenter(const GLfloat center[3]);
    void		setCenter(GLfloat x, GLfloat y, GLfloat z);
    void		setSphere(const GLfloat sphere[4]);
    void		forceNotifyStyleChange();
    virtual void	notifyStyleChange(const SceneRenderer&);
    virtual void	addRenderNodes(SceneRenderer&);
    virtual void	addShadowNodes(SceneRenderer&);

  private:
			SceneNode(const SceneNode&);
    SceneNode&		operator=(const SceneNode&);

    static void __stdcall	noColor3f(GLfloat, GLfloat, GLfloat);
    static void __stdcall	noColor4f(GLfloat, GLfloat, GLfloat, GLfloat);
    static void __stdcall	noColor3fv(const GLfloat*);
    static void __stdcall	noColor4fv(const GLfloat*);
    static void			noStipple(GLfloat);

  private:
    int			styleMailbox;
    GLfloat		sphere[4];
    static void		(__stdcall *color3f)(GLfloat, GLfloat, GLfloat);
    static void		(__stdcall *color4f)(GLfloat, GLfloat, GLfloat, GLfloat);
    static void		(__stdcall *color3fv)(const GLfloat*);
    static void		(__stdcall *color4fv)(const GLfloat*);
    static void		(*stipple)(GLfloat);
};

typedef GLfloat		GLfloat2[2];
typedef GLfloat		GLfloat3[3];

class GLfloat2Array {
  public:
			GLfloat2Array(int s) : size(s)
				{ data = new GLfloat2[size]; }
			GLfloat2Array(const GLfloat2Array&);
			~GLfloat2Array() { delete[] data; }
    GLfloat2Array&	operator=(const GLfloat2Array&);
    GLfloat*		operator[](int i) { return data[i]; }
    const GLfloat*	operator[](int i) const { return data[i]; }
    int			getSize() const { return size; }

  private:
    int			size;
    GLfloat2*		data;
};

class GLfloat3Array {
  public:
			GLfloat3Array(int s) : size(s)
				{ data = new GLfloat3[size]; }
			GLfloat3Array(const GLfloat3Array&);
			~GLfloat3Array() { delete[] data; }
    GLfloat3Array&	operator=(const GLfloat3Array&);
    GLfloat*		operator[](int i) { return data[i]; }
    const GLfloat*	operator[](int i) const { return data[i]; }
    int			getSize() const { return size; }

  private:
    int			size;
    GLfloat3*		data;
};

#endif // BZF_SCENE_NODE_H
// ex: shiftwidth=2 tabstop=8
