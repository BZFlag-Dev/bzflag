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

/* BSPSceneDatabase:
 *	BSP tree database of geometry to render
 */

#ifndef	BZF_BSP_SCENE_DATABASE_H
#define	BZF_BSP_SCENE_DATABASE_H

// common goes first
#include "common.h"

// system headers
#include <vector>

// common implementation headers
#include "bzfgl.h"
#include "SceneDatabase.h"

class BSPSceneDatabase : public SceneDatabase {
  public:
			BSPSceneDatabase();
			~BSPSceneDatabase();

    void		addStaticNode(SceneNode*);
    void		addDynamicNode(SceneNode*);
    void		addDynamicSphere(SphereSceneNode*);
    void		addShadowNodes(SceneRenderer &renderer);
    void		removeDynamicNodes();
    void		removeAllNodes();
    bool		isOrdered();

    SceneIterator*	getRenderIterator();

  private:
    friend class BSPSceneIterator;
    friend class BSPSceneIteratorItem;
    class Node {
      public:
			Node(bool dynamic, SceneNode* node);
			void addShadowNodes(SceneRenderer& renderer);
      public:
	bool		dynamic;
	int		count;
	SceneNode*	node;
	Node*		front;
	Node*		back;
    };

    void		insertStatic(int, Node*, SceneNode*);
    void		insertDynamic(int, Node*, SceneNode*);
    void		removeDynamic(Node*);
    void		free(Node*);
    void		release(Node*);
    void		setDepth(int newDepth);

  private:
    Node*		root;
    int			depth;
    GLfloat		eye[3];
};

class BSPSceneIteratorItem {
  public:
    enum Side { None = 0, Back = 1, Front = 2, Center = 4 };
			BSPSceneIteratorItem(BSPSceneDatabase::Node* _node) :
				node(_node), side(None) { }
  public:
    BSPSceneDatabase::Node*	node;
    int				side;
};

typedef std::vector<BSPSceneIteratorItem> BSPSceneIteratorStack;

class BSPSceneIterator : public SceneIterator {
  public:
			BSPSceneIterator(const BSPSceneDatabase*);
    virtual		~BSPSceneIterator();

    void	        resetFrustum(const ViewFrustum*);
    void	        reset();
    SceneNode*	        getNext();
    void		drawCuller();

  private:
    const BSPSceneDatabase*	db;
    GLfloat			eye[3];
    BSPSceneIteratorStack	stack;
};

#endif // BZF_BSP_SCENE_DATABASE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

