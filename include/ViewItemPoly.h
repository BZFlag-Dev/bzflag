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

#ifndef BZF_VIEWITEMPOLY_H
#define BZF_VIEWITEMPOLY_H

#include "View.h"

class ViewItemPoly : public View {
public:
	ViewItemPoly();

	void				addVertex(const ViewSize& x,
							const ViewSize& y, const ViewSize& z,
							const ViewColor&, const float* uv);
	void				close();

protected:
	virtual ~ViewItemPoly();

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);
	virtual void		onUpdateColors();

private:
	struct Vertex {
	public:
		ViewSize		x;
		ViewSize		y;
		ViewSize		z;
		ViewColor		color;
		float			uv[2];
	};
	typedef std::vector<Vertex> Vertices;

	Vertices			vertices;
	float				alpha;
	OpenGLGState		gstate;
};

class ViewItemPolyReader : public ViewTagReader {
public:
	ViewItemPolyReader();
	virtual ~ViewItemPolyReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(const ConfigReader::Values& values);
	virtual void		close();
	virtual bool		push(const BzfString& tag,
								const ConfigReader::Values&);
	virtual void		pop(const BzfString& tag);

private:
	ViewItemPoly*		item;
	ViewColor				color;
	float				uv[2];
};

#endif
