/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* FlagSceneNode:
 *  Encapsulates information for rendering a flag.
 */

#pragma once

// Inherits from
#include "SceneNode.h"

class FlagSceneNode : public SceneNode
{
public:
    FlagSceneNode(const GLfloat pos[3]);
    ~FlagSceneNode();

    static void     waveFlag(float dt);
    static void     freeFlag();

    void        move(const GLfloat pos[3]);
    void        setAngle(GLfloat angle);
    void        setWind(const GLfloat wind[3], float dt);
    void        setBillboard(bool billboard);

    const GLfloat*  getColor() const
    {
        return color;
    }
    void        setColor(GLfloat r, GLfloat g,
                         GLfloat b, GLfloat a = 1.0f);
    void        setColor(const GLfloat* rgba);
    void        setTexture(const int);

    void        notifyStyleChange() override;
    void        addRenderNodes(SceneRenderer&) override;
    void        addShadowNodes(SceneRenderer&) override;

    bool        cullShadow(int planeCount,
                           const float (*planes)[4]) const override;
protected:
    class FlagRenderNode : public RenderNode
    {
    public:
        FlagRenderNode(const FlagSceneNode*);
        ~FlagRenderNode();
        void        render() override;
        const GLfloat*  getPosition() const override;
    private:
        const FlagSceneNode* sceneNode;
        int      waveReference;
    };
    friend class FlagRenderNode;

private:
    bool        billboard;
    GLfloat     angle;
    GLfloat     tilt;
    GLfloat     hscl;
    GLfloat     color[4];
    bool        transparent;
    bool        texturing;
    OpenGLGState    gstate;
    FlagRenderNode  renderNode;
};


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
