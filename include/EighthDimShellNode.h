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

/* EighthDimShellNode:
 *  Wraps a normal SceneNode within an inverted-view environment.
 */

#ifndef	BZF_EIGHTH_DIM_SHELL_NODE_H
#define	BZF_EIGHTH_DIM_SHELL_NODE_H

#include "common.h"
#include "SceneNode.h"
#include "OpenGLGState.h"

class EighthDimShellNode : public SceneNode {
  public:
    EighthDimShellNode(SceneNode *sceneNode, bool ownTheNode);
    ~EighthDimShellNode();

    bool cull(const ViewFrustum&) const;
    void addRenderNodes(SceneRenderer&);
    void notifyStyleChange();

  protected:
    class ShellRenderNode : public RenderNode {
      public:
	ShellRenderNode(RenderNode *renderNode,
			const OpenGLGState* gstate);
	~ShellRenderNode();
	void render();
	void renderShadow() { return; }
	const GLfloat* getPosition() const { return renderNode->getPosition(); }
      public:
	const OpenGLGState* getGState() const;
      private:
	OpenGLGState gstate;
	RenderNode* renderNode;
    };

  private:
    void makeNodes();
    void killNodes();

  private:
    bool ownTheNode;
    SceneNode* sceneNode;

    int shellNodeCount;
    ShellRenderNode** shellNodes;
};

#endif // BZF_EIGHTH_DIM_SHELL_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
