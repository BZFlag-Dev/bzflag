/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* TextSceneNode:
 *	Encapsulates information for rendering world text.
 */

#ifndef	BZF_TEXT_SCENE_NODE_H
#define	BZF_TEXT_SCENE_NODE_H


#include "common.h"

/* system headers */
#include <string>
#include <vector>

/* common headers */
#include "vectors.h"
#include "SceneNode.h"
#include "WorldText.h"
#include "MeshTransform.h"


class TextSceneNode : public SceneNode {
  public:
    TextSceneNode(const WorldText* text);
    ~TextSceneNode();

    void setRawText(const std::string& rawText);

    bool inAxisBox (const Extents&) const;

    void notifyStyleChange();

    bool cull(const ViewFrustum&) const;
    bool cullShadow(int pCount, const fvec4* planes) const;

    void addRenderNodes(SceneRenderer&);
    void addShadowNodes(SceneRenderer&);
    void renderRadar();

  protected:
    class TextRenderNode : public RenderNode {

      friend class TextSceneNode;

      public:
	TextRenderNode(TextSceneNode*, const WorldText* text);
	~TextRenderNode();

	void setText(const std::string& text);

	void render();
	void renderRadar();
	void renderShadow();

	const fvec3& getPosition() const { return sceneNode->getCenter(); }

      private:
        int  getFontID() const;

        void setRawText(const std::string& rawText);
        void countTriangles();

        bool checkDist() const;
        void singleLineXForm() const;

	void drawDebug();

        void initXFormList();
        void freeXFormList();
        static void initContext(void* data);
        static void freeContext(void* data);

        static void bzdbCallback(const std::string& name, void* userData);

      private:
	TextSceneNode* sceneNode;

        const WorldText text;

        unsigned int xformList;

        int fontID;
        float fontSize;

        std::vector<std::string> lines;
        std::vector<std::string> stripped;
        std::vector<float>       widths;

        std::vector<std::string>* linesPtr;

        float fixedWidth;

        float lineStep;

        const fvec4* colorPtr;

        bool noRadar;
        bool noShadow;

        bool usePolygonOffset;
        bool useLengthPerPixel;

        int triangles;
    };

    friend class TextRenderNode;

  protected:
    void calcPlane();
    void calcSphere(const fvec3 points[5]);
    void calcExtents(const fvec3 points[5]);
    float getMaxDist(const fvec3 points[5]) const;
    void getPoints(fvec3 points[5]) const; // corners, and the origin

  private:
    OpenGLGState   gstate;
    TextRenderNode renderNode;
};


#endif // BZF_TEXT_SCENE_NODE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
