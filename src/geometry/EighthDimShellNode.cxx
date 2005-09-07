/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag common header
#include "common.h"

// interface header
#include "EighthDimShellNode.h"

// system headers
#include <stdlib.h>

// common headers
#include "SceneRenderer.h"
#include "OpenGLGState.h"
#include "BZDBCache.h"

EighthDimShellNode::EighthDimShellNode(SceneNode* node, bool _ownTheNode)
{
  sceneNode = node;
  ownTheNode = _ownTheNode;

  const OpenGLGState* gs = node->getGState();
  if (gs == NULL) {
    renderNodeCount = 0;
    renderNodes = NULL;
    return;
  }

  notifyStyleChange(); // setup the gstate

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
  const OpenGLGState* gs = sceneNode->getGState();
  if (gs == NULL) { // safety
    return;
  }

  OpenGLGStateBuilder gb = *gs;

  if (BZDBCache::blend && (RENDERER.useQuality() >= 3)) {
    gb.setBlending(GL_ONE, GL_ONE);
  } else {
    gb.resetBlending();
  }

  gb.setCulling(GL_FRONT); // invert the culling

  gstate = gb.getState(); // get the modified gstate

  return;
}


void EighthDimShellNode::addRenderNodes(SceneRenderer& renderer)
{
  for (int i = 0; i < renderNodeCount; i++) {
    renderer.addRenderNode(renderNodes[i], &gstate);
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
//  glLogicOp(GL_XOR);
//  glEnable(GL_COLOR_LOGIC_OP);
//  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

  if (BZDBCache::blend && RENDERER.useQuality() >= 3) {
    renderNode->render();
  }

  glPolygonMode(GL_BACK, GL_LINE);
  glLineWidth(3.0f);

  renderNode->render();

  glLineWidth(1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

//  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//  glDisable(GL_COLOR_LOGIC_OP);

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

