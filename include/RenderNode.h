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

/* RenderNode:
 *	Encapsulates information for rendering geometry with a
 *	single gstate.
 *
 * RenderNodeList:
 *	Keeps a list of RenderNode* and can render them in order.
 */

#ifndef	BZF_RENDER_NODE_H
#define	BZF_RENDER_NODE_H

#include "common.h"
#include "OpenGLGState.h"


class RenderNode {
  public:
			RenderNode() { }
    virtual		~RenderNode() { }

    virtual void	render() = 0;
    virtual void	renderShadow() { render(); }
    virtual void	renderRadar() { renderShadow(); }
    virtual const GLfloat* getPosition() const = 0;

    static int		getTriangleCount();
    static void		resetTriangleCount();

  protected:
    static void		addTriangleCount(int triCount);

  private:
    static int		triangleCount;
};


inline void RenderNode::addTriangleCount(int count)
{
  triangleCount += count;
  return;
}


class RenderNodeList {
  public:
			RenderNodeList();
			~RenderNodeList();

    void		clear();
    void		append(RenderNode*);
    void		render() const;

  private:
    // no copying (cos that'd be slow)
			RenderNodeList(const RenderNodeList&);
    RenderNodeList&	operator=(const RenderNodeList&);

    void		grow();

  private:
    int			count;
    int			size;
    RenderNode**	list;
};

inline void RenderNodeList::append(RenderNode* node)
{
  if (count == size) {
    grow();
  }
  list[count++] = node;
}


class RenderNodeGStateList {
  public:
			RenderNodeGStateList();
			~RenderNodeGStateList();

    void		clear();
    void		append(RenderNode*, const OpenGLGState*);
    void		append(RenderNode*, const OpenGLGState*, float depth);
    void		render() const;

    void		sort(const GLfloat* eye);

    // public for the qsort() comparison function
    struct Item {
      public:
	typedef const OpenGLGState* GStatePtr;
	RenderNode*	node;
	GStatePtr	gstate;
	float		depth;
    };

  private:
    // no copying (cos that'd be slow)
			RenderNodeGStateList(const RenderNodeGStateList&);
    RenderNodeGStateList& operator=(const RenderNodeGStateList&);

    void		grow();

  private:
    int			count;
    int			size;
    Item*		list;
};

inline void RenderNodeGStateList::append(RenderNode* node,
					 const OpenGLGState* gstate)
{
  if (count == size) {
    grow();
  }
  list[count].node = node;
  list[count].gstate = gstate;
  list[count].depth = 0.0f;
  count++;
}

#endif // BZF_RENDER_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
