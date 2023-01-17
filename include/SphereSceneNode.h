/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SphereSceneNode:
 *  Encapsulates information for rendering a sphere.
 */

#ifndef BZF_SPHERE_SCENE_NODE_H
#define BZF_SPHERE_SCENE_NODE_H

// Inherits from
#include "SceneNode.h"


/******************************************************************************/

class SphereSceneNode : public SceneNode
{
public:
    SphereSceneNode(const GLfloat pos[3], GLfloat radius);

    void setShockWave(bool value);

    void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f);
    void setColor(const GLfloat* rgba);
    void move(const GLfloat pos[3], GLfloat radius);
    void notifyStyleChange();

    void addRenderNodes(SceneRenderer&);
    void addShadowNodes(SceneRenderer&);

    static void init();
    static void kill();
    static void initContext(void*);
    static void freeContext(void*);

protected:
    class SphereLodRenderNode : public RenderNode
    {
        friend class SphereLodSceneNode;
    public:
        SphereLodRenderNode(const SphereSceneNode*);
        ~SphereLodRenderNode();
        void setLod(int lod);
        void render();
        const glm::vec3 getPosition() const override;

    private:
        const SphereSceneNode* sceneNode;
        int lod;
    };

    GLfloat     radius;
    glm::vec4   color;
    bool        transparent;
    OpenGLGState    gstate;
    SphereLodRenderNode renderNode;
    bool shockWave;
    bool inside;

    static bool initialized;
    static const int sphereLods = 5;
    static GLuint lodLists[sphereLods];
    static float lodPixelsSqr[sphereLods];
    static int listTriangleCount[sphereLods];

    friend class SphereSceneNode::SphereLodRenderNode;
};


#endif // BZF_SPHERE_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
