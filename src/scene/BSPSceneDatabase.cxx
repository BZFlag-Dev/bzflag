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

/*
 *
 */

#include "common.h"
#include "SceneNode.h"
#include <assert.h>
#include "BSPSceneDatabase.h"
#include "ViewFrustum.h"
#include "SphereSceneNode.h"
#include "WallSceneNode.h"
#include "SceneRenderer.h"


//
// BSPSceneDatabase::Node
//

BSPSceneDatabase::Node::Node(bool _dynamic, SceneNode* _node):
				dynamic(_dynamic),
				count(0),
				node(_node),
				front(NULL),
				back(NULL)
{
  // do nothing
}


//
// BSPSceneDatabase
//

BSPSceneDatabase::BSPSceneDatabase() :
				root(NULL),
				depth(0)
{

  memset(eye, 0, sizeof(GLfloat) * 3);
}


BSPSceneDatabase::~BSPSceneDatabase()
{
  free(root);
}


bool BSPSceneDatabase::addStaticNode(SceneNode* node, bool dontFree)
{
  // make sure node has a definite plane for splitting
  assert(node->getPlane());

  // insert static node
  if (!root) {
    root = new Node(false, node);
    setDepth(1);
    return false; // node would not be freed
  } else {
    return insertStatic(1, root, node, dontFree);
  }
}


void BSPSceneDatabase::addDynamicNode(SceneNode* node)
{
  node->notifyStyleChange();
  // insert dynamic node
  if (!root) {
    root = new Node(true, node);
    setDepth(1);
  } else {
    insertDynamic(1, root, node);
  }
}


void BSPSceneDatabase::addDynamicSphere(SphereSceneNode* n)
{
  // add each part of sphere separately
  int numParts;
  SceneNode** parts = n->getParts(numParts);
  if (!parts) {
    // wouldn't split itself up -- add whole thing
    addDynamicNode(n);
  } else {
    for (int i = 0; i < numParts; i++)
      addDynamicNode(parts[i]);
  }
}


void BSPSceneDatabase::removeDynamicNodes()
{
  // scan tree removing dynamic nodes
  if (root && root->dynamic) {
    release(root);
    root = NULL;
  } else {
    removeDynamic(root);
  }
}


void BSPSceneDatabase::removeAllNodes()
{
  free(root);
  root = NULL;
  depth = 0;
}


bool BSPSceneDatabase::isOrdered()
{
  return true;
}


void BSPSceneDatabase::free(Node* node)
{
  if (!node) return;
  delete node->node;
  free(node->front);
  free(node->back);
  delete node;
}


void BSPSceneDatabase::release(Node* node)
{
  if (!node) return;
  release(node->front);
  release(node->back);
  delete node;
}


bool BSPSceneDatabase::insertStatic(int level, Node* root,
				    SceneNode* node, bool dontFree)
{
  bool wouldFree = false;

  // dynamic nodes should only be inserted after all static nodes
  assert(root->dynamic == 0);

  // split against root's plane
  SceneNode* front = NULL, *back = NULL;
  switch (node->split(root->node->getPlane(), front, back)) {
    case 0:
      // copy style to new nodes
      // FIXME -- only WallSceneNodes are static but should make type safe
      ((WallSceneNode*)front)->copyStyle((WallSceneNode*)node);
      ((WallSceneNode*)back)->copyStyle((WallSceneNode*)node);

      // done with split node so get rid of it
      if (dontFree) {
	wouldFree = true;
      } else {
	delete node;
      }
      break;
    case 1:
      // completely in front
      front = node;
      break;
    case -1:
      // completely in back
      back = node;
      break;
  }

  const bool dontFreeNext = (dontFree && !wouldFree);

  // add nodes
  if (front) {
    if (root->front) {
      wouldFree = insertStatic(level + 1, root->front, front, dontFreeNext);
    } else {
      root->front = new Node(false, front);
      setDepth(level + 1);
    }
    root->count++;
  }
  if (back) {
    if (root->back) {
      wouldFree = insertStatic(level + 1, root->back, back, dontFreeNext);
    } else {
      root->back = new Node(false, back);
      setDepth(level + 1);
    }
    root->count++;
  }

  return wouldFree;
}


void BSPSceneDatabase::insertDynamic(int level, Node* root,
								SceneNode* node)
{
  GLfloat d;
  if (!root->dynamic && root->node->getPlane()) {
    const GLfloat* plane = root->node->getPlane();
    const GLfloat* pos = node->getSphere();
    d = pos[0] * plane[0] + pos[1] * plane[1] + pos[2] * plane[2] + plane[3];
  } else {
    d = root->node->getDistance(eye) - node->getDistance(eye);
  }

  if (d >= 0.0) {
    if (root->front) {
      insertDynamic(level + 1, root->front, node);
    } else {
      root->front = new Node(true, node);
      setDepth(level + 1);
    }
  } else {
    if (root->back) {
      insertDynamic(level + 1, root->back, node);
    } else {
      root->back = new Node(true, node);
      setDepth(level + 1);
    }
  }
}


void BSPSceneDatabase::removeDynamic(Node* node)
{
  if (!node) return;
  if (node->front && node->front->dynamic) {
    release(node->front);
    node->front = NULL;
  } else {
    removeDynamic(node->front);
  }
  if (node->back && node->back->dynamic) {
    release(node->back);
    node->back = NULL;
  } else {
    removeDynamic(node->back);
  }
}


void BSPSceneDatabase::setDepth(int newDepth)
{
  if (newDepth <= depth) {
    return;
  }
  depth = newDepth;
}


void BSPSceneDatabase::updateNodeStyles()
{
  if (root) {
    setNodeStyle(root);
  }
  return;
}


void BSPSceneDatabase::addLights(SceneRenderer& _renderer)
{
  if (root) {
    renderer = &_renderer;
    nodeAddLights(root);
  }
  return;
}


void BSPSceneDatabase::addShadowNodes(SceneRenderer& _renderer)
{
  if (root) {
    renderer = &_renderer;
    nodeAddShadowNodes(root);
  }
  return;
}


void BSPSceneDatabase::addRenderNodes(SceneRenderer& _renderer)
{
  if (root) {
    renderer = &_renderer;
    frustum = &renderer->getViewFrustum();
    const GLfloat* _eye = frustum->getEye();
    memcpy (eye, _eye, sizeof(GLfloat[3]));
    nodeAddRenderNodes(root);
  }
  return;
}


void BSPSceneDatabase::setNodeStyle(Node *node)
{
  Node* back = node->back;
  Node* front = node->front;
  // dive into the child BSP nodes
  if (front) {
    setNodeStyle(node->front);
  }
  if (back) {
    setNodeStyle(node->back);
  }
  // add this node's style
  node->node->notifyStyleChange();

  return;
}


void BSPSceneDatabase::nodeAddLights(Node* node)
{
  Node* back = node->back;
  Node* front = node->front;
  // dive into the child BSP nodes
  if (front) {
    nodeAddLights(node->front);
  }
  if (back) {
    nodeAddLights(node->back);
  }
  // add this node's light, if it's dynamic
  if (node->dynamic) {
    node->node->addLight(*renderer);
  }
  return;
}


void BSPSceneDatabase::nodeAddShadowNodes(Node* node)
{
  Node* back = node->back;
  Node* front = node->front;
  // dive into the child BSP nodes
  if (front) {
    nodeAddShadowNodes(node->front);
  }
  if (back) {
    nodeAddShadowNodes(node->back);
  }
  // add this node's shadows
  node->node->addShadowNodes(*renderer);
  return;
}


void BSPSceneDatabase::nodeAddRenderNodes(Node* node)
{
  Node* back = node->back;
  Node* front = node->front;
  SceneNode* snode = node->node;

  const GLfloat* plane = snode->getPlane();
  if (plane) {
    if (((plane[0] * eye[0]) + (plane[1] * eye[1]) +
	 (plane[2] * eye[2]) + plane[3]) >= 0.0f) {
      // eye is in front so render:  back, node, front
      if (back) {
	nodeAddRenderNodes(back);
      }
      if (!snode->cull(*frustum)) {
        snode->addRenderNodes(*renderer);
      }
      if (front) {
	nodeAddRenderNodes(front);
      }
    }
    else {
      // eye is in back so render:  front, node, back
      if (front) {
	nodeAddRenderNodes(front);
      }
      if (!snode->cull(*frustum)) {
        snode->addRenderNodes(*renderer);
      }
      if (back) {
	nodeAddRenderNodes(back);
      }
    }
  }
  else {
    // nodes without split planes should be rendered back, node, front
    if (back) {
      nodeAddRenderNodes(back);
    }
    if (!snode->cull(*frustum)) {
      snode->addRenderNodes(*renderer);
    }
    if (front) {
      nodeAddRenderNodes(front);
    }
  }

  return;
}


void BSPSceneDatabase::drawCuller()
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

