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

#ifndef BZF_VIEW_H
#define BZF_VIEW_H

#include "common.h"
#include <string>
#include "XMLTree.h"
#include "OpenGLTexture.h"
#include "OpenGLTexFont.h"
#include <vector>

class ViewSize {
public:
	ViewSize();
	ViewSize(float pixel, float fraction);

	float				get(float fullSize) const;

public:
	float				pixel;
	float				fraction;
};

class ViewColor {
public:
	ViewColor();

	void				update();
	void				set() const;

	enum Source { Black, White, Red, Green, Blue, Purple, Rogue, MyTeam };
	static void			setColor(Source, const float*);
	static void			setMyTeam(Source);

public:
	// final color
	float				color[4];

	// source arguments
	Source				source;
	float				scale[3];
	float				shift[3];

private:
	static float		colors[8][3];
	static Source		myTeam;
};

class ViewState {
public:
	ViewState();

public:
	enum Align { Left, Center, Right, Bottom = Left, Top = Right };

	ViewColor			color;
	Align				hAlign;
	Align				vAlign;
	ViewSize			fontSize;
	std::string			fontName;
	OpenGLTexFont		font;
	OpenGLTexture		texture;
};

class View {
public:
	View();

	int					ref();
	int					unref();

	void				pushChild(View*);
	void				setState(const ViewState&);

	void				render(float x, float y, float w, float h);

	static void			setColorsDirty();

protected:
	virtual ~View();

	const ViewState&	getState() const { return state; }

	// prepare drawing transformations, do pre-child rendering and
	// return true or don't modify rendering state and return false.
	// if false is returned then child views and postRender() are
	// not called.  x,y is the lower-left corner position of the
	// region to draw to in window coordinates (with 0,0 in the
	// lower-left).  w,h is the size in pixels.
	//
	// default sets up a 1-to-1 pixel orthographic projection, pushes
	// an identity modelview transform, and returns true.
	virtual bool		onPreRender(float x, float y, float w, float h);

	// do post-child rendering and restore rendering state.  default
	// pops the modelview transform.  x,y,w,h are as in onPreRender().
	virtual void		onPostRender(float x, float y, float w, float h);

	// render child views.  x,y,w,h are as in onPreRender().
	void				renderChildren(float x, float y, float w, float h);

	// called before onRender() when colors may be dirty
	virtual void		onUpdateColors() { }

private:
	typedef std::vector<View*> Views;

	int					refCount;
	ViewState			state;
	Views				views;
	int					style;
	int					colorMark;
	static int			colorMailbox;
};

class ViewTagReader {
public:
	ViewTagReader() { }
	virtual ~ViewTagReader() { }

	// create a copy of this tag reader
	virtual ViewTagReader* clone() const = 0;

	// called to initialize the view item.  return NULL on failure, the
	// ref()'d view on success.  readers are expected to keep a view
	// around for parse().  clone() should create a reader with a new
	// view.
	virtual View*		open(XMLTree::iterator) = 0;

	// called to parse a child XMLNode.  return true if the node was
	// recognized and handled, false otherwise.
	virtual bool		parse(XMLTree::iterator) { return false; }

	// called to do any clean up after parsing
	virtual void		close() { }
};

#endif
