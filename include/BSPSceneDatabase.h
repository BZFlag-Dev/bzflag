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

/* BSPSceneDatabase:
 *	BSP tree database of geometry to render
 */

#ifndef	BZF_BSP_SCENE_DATABASE_H
#define	BZF_BSP_SCENE_DATABASE_H

#include "bzfgl.h"
#include "common.h"
#include "SceneDatabase.h"
#include "AList.h"

class BSPSceneDatabase : public SceneDatabase {
  public:
			BSPSceneDatabase();
			~BSPSceneDatabase();

    void		addStaticNode(SceneNode*);
    void		addDynamicNode(SceneNode*);
    void		addDynamicSphere(SphereSceneNode*);
    void		removeDynamicNodes();
    void		removeAllNodes();
    boolean		isOrdered();

    SceneIterator*	getRenderIterator();

  private:
    friend class BSPSceneIterator;
    friend class BSPSceneIteratorItem;
    class Node {
      public:
			Node(boolean dynamic, SceneNode* node);
      public:
	boolean		dynamic;
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
BZF_DEFINE_ALIST(BSPSceneIteratorStack, BSPSceneIteratorItem);

class BSPSceneIterator : public SceneIterator {
  public:
			BSPSceneIterator(const BSPSceneDatabase*);
    virtual		~BSPSceneIterator();

    virtual void	resetFrustum(const ViewFrustum*);
    virtual void	reset();
    virtual SceneNode*	getNext();

  private:
    const BSPSceneDatabase*	db;
    GLfloat			eye[3];
    BSPSceneIteratorStack	stack;
};

#endif // BZF_BSP_SCENE_DATABASE_H
// ex: shiftwidth=2 tabstop=8
