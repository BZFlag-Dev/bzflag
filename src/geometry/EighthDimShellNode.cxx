/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag common header
#include "common.h"

// interface header
#include "EighthDimShellNode.h"

// system headers
#include <stdlib.h>

// common headers
#include "SceneRenderer.h"


EighthDimShellNode::EighthDimShellNode(SceneNode* node, bool _ownTheNode)
{
  sceneNode = node;
  ownTheNode = _ownTheNode;

  renderNodeCount = sceneNode->getRenderNodeCount();
  if (renderNodeCount > 0) {
    renderNodes = new ShellRenderNode*[renderNodeCount];
    int count = 0;
    for (int i = 0; i < renderNodeCount; i++) {
      RenderNode* rnode = node->getRenderNode(i);
      if (rnode != NULL) {
	renderNodes[count] = new ShellRenderNode(rnode);
	count++;
      }
    }
    renderNodeCount = count;
  } else {
    renderNodes = NULL;
  }

  return;
}

EighthDimShellNode::~EighthDimShellNode()
{
  if (ownTheNode) {
    delete sceneNode;
  }
  for (int i = 0; i < renderNodeCount; i++) {
    delete renderNodes[i];
  }
  delete[] renderNodes;
  return;
}


bool EighthDimShellNode::cull(const ViewFrustum&) const
{
  // no culling
  return false;
}


void EighthDimShellNode::notifyStyleChange()
{
  return;
}


void EighthDimShellNode::addRenderNodes(SceneRenderer& renderer)
{
  for (int i = 0; i < renderNodeCount; i++) {

    OpenGLGState* gstate = sceneNode->getGState(i);
    if ((renderNodes[i] != NULL) && (gstate != NULL)) {
      renderer.addRenderNode(renderNodes[i], gstate);
    }
  }
  return;
}


//
// EighthDimShellNode::ShellRenderNode
//

EighthDimShellNode::ShellRenderNode::ShellRenderNode(RenderNode *node)
{
  renderNode = node;
  return;
}


EighthDimShellNode::ShellRenderNode::~ShellRenderNode()
{
  return;
}


void EighthDimShellNode::ShellRenderNode::render()
{
  SceneNode::enableInvertView();

  renderNode->render();

  SceneNode::disableInvertView();

  return;
}


void EighthDimShellNode::ShellRenderNode::renderShadow()
{
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

