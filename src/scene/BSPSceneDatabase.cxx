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
#include "BSPSceneDatabase.h"
#include "ViewFrustum.h"
#include "SphereSceneNode.h"
#include "WallSceneNode.h"

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

void			BSPSceneDatabase::addStaticNode(SceneNode* node)
{
  // make sure node has a definite plane for splitting
  assert(node->getPlane());

  // insert static node
  if (!root) {
    root = new Node(false, node);
    setDepth(1);
  } else {
    insertStatic(1, root, node);
  }
}

void			BSPSceneDatabase::addDynamicNode(SceneNode* node)
{
  // insert dynamic node
  if (!root) {
    root = new Node(true, node);
    setDepth(1);
  } else {
    insertDynamic(1, root, node);
  }
}

void			BSPSceneDatabase::addDynamicSphere(SphereSceneNode* n)
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

void			BSPSceneDatabase::removeDynamicNodes()
{
  // scan tree removing dynamic nodes
  if (root && root->dynamic) {
    release(root);
    root = NULL;
  } else {
    removeDynamic(root);
  }
}

void			BSPSceneDatabase::removeAllNodes()
{
  free(root);
  root = NULL;
  depth = 0;
}

bool			BSPSceneDatabase::isOrdered()
{
  return true;
}

void			BSPSceneDatabase::free(Node* node)
{
  if (!node) return;
  delete node->node;
  free(node->front);
  free(node->back);
  delete node;
}

void			BSPSceneDatabase::release(Node* node)
{
  if (!node) return;
  release(node->front);
  release(node->back);
  delete node;
}

void			BSPSceneDatabase::insertStatic(int level,
						Node* root, SceneNode* node)
{
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
      delete node;
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

  // add nodes
  if (front) {
    if (root->front) {
      insertStatic(level + 1, root->front, front);
    } else {
      root->front = new Node(false, front);
      setDepth(level + 1);
    }
    root->count++;
  }
  if (back) {
    if (root->back) {
      insertStatic(level + 1, root->back, back);
    } else {
      root->back = new Node(false, back);
      setDepth(level + 1);
    }
    root->count++;
  }
}

void			BSPSceneDatabase::insertDynamic(int level, Node* root,
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

void			BSPSceneDatabase::removeDynamic(Node* node)
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

void			BSPSceneDatabase::setDepth(int newDepth)
{
  if (newDepth <= depth) return;
  depth = newDepth;
}

SceneIterator*		BSPSceneDatabase::getRenderIterator()
{
  return new BSPSceneIterator(this);
}


//
// BSPSceneIterator
//

BSPSceneIterator::BSPSceneIterator(const BSPSceneDatabase* _db) :
				SceneIterator(),
				db(_db)
{
  eye[0] = 0.0f;
  eye[1] = 0.0f;
  eye[2] = 0.0f;
}

BSPSceneIterator::~BSPSceneIterator()
{
  // do nothing
}

void			BSPSceneIterator::resetFrustum(
				const ViewFrustum* frustum)
{
  const GLfloat* _eye = frustum->getEye();
  eye[0] = _eye[0];
  eye[1] = _eye[1];
  eye[2] = _eye[2];
}

void			BSPSceneIterator::reset()
{
  stack.clear();
  if (db->root != NULL) {
    stack.push_back(BSPSceneIteratorItem(db->root));
  }
}

SceneNode*		BSPSceneIterator::getNext()
{
restart:
  if (stack.size() == 0) return NULL;

  BSPSceneIteratorItem& item = stack[stack.size() - 1];
  switch (item.side) {
    case BSPSceneIteratorItem::None: {
      // pick first part
      const GLfloat* plane = item.node->node->getPlane();
      if (plane) {
	// has a split plane -- see which side eye is on
	if (plane[0] * eye[0] + plane[1] * eye[1] +
	    plane[2] * eye[2] + plane[3] >= 0.0f) {
	  // eye is in front so render:  back, node, front
	  item.side = BSPSceneIteratorItem::Back;
	  if (item.node->back)
	    stack.push_back(BSPSceneIteratorItem(item.node->back));
	} else {
	  // eye is in back so render:  front, node, back
	  item.side = BSPSceneIteratorItem::Front;
	  if (item.node->front)
	    stack.push_back(BSPSceneIteratorItem(item.node->front));
	}
      } else {
	// nodes without split planes should be rendered back, node, front
	item.side = BSPSceneIteratorItem::Back;
	if (item.node->back)
	  stack.push_back(BSPSceneIteratorItem(item.node->back));
      }
      goto restart;
    }

    case BSPSceneIteratorItem::Back:
      // did back side;  now do node
      item.side += BSPSceneIteratorItem::Center;
      return item.node->node;

    case BSPSceneIteratorItem::Front:
      // did front side;  now do node
      item.side += BSPSceneIteratorItem::Center;
      return item.node->node;

    case BSPSceneIteratorItem::Back + BSPSceneIteratorItem::Center: {
      // did back and center;  now do front
      BSPSceneDatabase::Node* front = item.node->front;
      stack.pop_back();
      if (front)
	stack.push_back(BSPSceneIteratorItem(front));
      goto restart;
    }

    case BSPSceneIteratorItem::Front + BSPSceneIteratorItem::Center: {
      // did front and center;  now do back
      BSPSceneDatabase::Node* back = item.node->back;
      stack.pop_back();
      if (back)
	stack.push_back(BSPSceneIteratorItem(back));
      goto restart;
    }
  }

  assert(0);
  return NULL;
}

void        		BSPSceneIterator::drawCuller()
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

