/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "SceneVisitorWrite.h"
#include "SceneNodes.h"
#include "OpenGLTexture.h"

//
// SceneVisitorWrite
//

// FIXME -- not finished yet

SceneVisitorWrite::SceneVisitorWrite(ostream* _stream) : stream(_stream)
{
	// do nothing
}

SceneVisitorWrite::~SceneVisitorWrite()
{
	// do nothing
}

void					SceneVisitorWrite::indent()
{
	indentation += "  ";
}

void					SceneVisitorWrite::exdent()
{
	indentation.truncate(indentation.size() - 2);
}

bool					SceneVisitorWrite::visit(SceneNodeAnimate* n)
{
}

bool					SceneVisitorWrite::visit(SceneNodeBillboard* n)
{
	static const char* mapType[] = { "rotate" };

	// open
	*stream << getIndent() << "<billboard type=\"" << 
								mapType[n->get()] << "\">" << endl;
	indent();

	// descend
	const bool result = n->descend(this);

	// close
	exdent();
	*stream << getIndent() << "</billboard>" << endl;

	return result;
}

bool					SceneVisitorWrite::visit(SceneNodeGState* n)
{
	static const char* mapTexEnv[] = { "modulate", "decal", "replace" };
	static const char* mapShading[] = { "flat", "smooth" };
	static const char* mapBlending[] = { "0", "1",
										"sa", "1-sa", "da", "1-da",
										"sc", "1-sc", "dc", "1-dc" };
	static const char* mapOffOn[] = { "off", "on" };
	static const char* mapFunc[] = { "0", "1", "==", "!=", "<", "<=", ">", ">=" };

	// open
	*stream << getIndent() << "<gstate>" << endl;
	indent();

	// write parameters
	OpenGLGStateBuilder g(n->get());
	// can't write texture arguments :-(
	if (g.getTexture().isValid())
		*stream << getIndent() << "#<texture filename=\"\"/>" << endl;
	*stream << getIndent() << "<texenv mode=\"" <<
								mapTexEnv[g.getTexEnv()] <<
								"\"/>" << endl;
	*stream << getIndent() << "<shading model=\"" <<
								mapShading[g.getShading()] <<
								"\"/>" << endl;
	*stream << getIndent() << "<blending src=\"" <<
								mapBlending[g.getBlendingSrc()] <<
								"\" dst=\"" <<
								mapBlending[g.getBlendingDst()] <<
								"\"/>" << endl;
	*stream << getIndent() << "<smoothing smooth=\"" <<
								mapOffOn[g.getSmoothing()] <<
								"\"/>" << endl;
	*stream << getIndent() << "<culling cull=\"" <<
								mapOffOn[g.getCulling()] <<
								"\"/>" << endl;
	*stream << getIndent() << "<alpha func=\"" <<
								mapFunc[g.getAlphaFunc()] <<
								"\" ref=\"" <<
								g.getAlphaFuncRef() <<
								"\"/>" << endl;
	*stream << getIndent() << "<depth func=\"" <<
								mapFunc[g.getDepthFunc()] <<
								"\" write=\"" <<
								mapOffOn[g.getDepthMask()] <<
								"\"/>" << endl;
	*stream << getIndent() << "<point size=\"" <<
								g.getPointSize() <<
								"\"/>" << endl;
	*stream << getIndent() << "<pass number=\"" <<
								g.getPass() <<
								"\"/>" << endl;

	// descend
	const bool result = n->descend(this);

	// close
	exdent();
	*stream << getIndent() << "</gstate>" << endl;

	return result;
}

bool					SceneVisitorWrite::visit(SceneNodeGeometry* n)
{
	// open
	*stream << getIndent() << "<geometry>" << endl;
	indent();

	// write parameters
	const SceneNodeGeometry::ColorArray&   color   = n->getColor();
	const SceneNodeGeometry::TextureArray& texture = n->getTexCoord();
	const SceneNodeGeometry::NormalArray&  normal  = n->getNormal();
	const SceneNodeGeometry::VertexArray&  vertex  = n->getVertex();
	if (color.size() > 0) {
		const unsigned int num = color.size();
		const int nc = n->getNumColorComponents();
		*stream << getIndent() << "<color components=" << nc << ">";
		indent();
		for (unsigned int i = 0; i < num; ++i) {
			if (i % nc == 0)
				*stream << endl << getIndent();
			*stream << color[i] << " ";
		}
		exdent();
		*stream << endl << getIndent() << "</color>" << endl;
	}
	if (texture.size() > 0) {
		const unsigned int num = texture.size();
		*stream << getIndent() << "<texcoord>" << endl;
		indent();
		for (unsigned int i = 0; i < num; i += 2)
			*stream << getIndent() << texture[i] << " " << texture[i + 1] << endl;
		exdent();
		*stream << getIndent() << "</texcoord>" << endl;
	}
	if (normal.size() > 0) {
		const unsigned int num = normal.size();
		*stream << getIndent() << "<normal>" << endl;
		indent();
		for (unsigned int i = 0; i < num; i += 3)
			*stream << getIndent() << normal[i] << " " << normal[i + 1] << " " << normal[i + 2] << endl;
		exdent();
		*stream << getIndent() << "</normal>" << endl;
	}
	if (vertex.size() > 0) {
		const unsigned int num = vertex.size();
		*stream << getIndent() << "<vertex>" << endl;
		indent();
		for (unsigned int i = 0; i < num; i += 3)
			*stream << getIndent() << vertex[i] << " " << vertex[i + 1] << " " << vertex[i + 2] << endl;
		exdent();
		*stream << getIndent() << "</vertex>" << endl;
	}
	const float* c = n->getSpecularColor();
	*stream << getIndent() << "<specular>" << c[0] << " " <<
								c[1] << " " << c[2] << "</specular>" << endl;
	c = n->getEmissiveColor();
	*stream << getIndent() << "<emissive>" << c[0] << " " <<
								c[1] << " " << c[2] << "</emissive>" << endl;
	*stream << getIndent() << "<shininess>" << n->getShininess() <<
								"</shininess>" << endl;

	// descend
	const bool result = n->descend(this);

	// close
	exdent();
	*stream << getIndent() << "</geometry>" << endl;

	return result;
}

bool					SceneVisitorWrite::visit(SceneNodeGroup* n)
{
	// open
	*stream << getIndent() << "<group>" << endl;
	indent();

	// descend
	const bool result = n->descend(this);

	// close
	exdent();
	*stream << getIndent() << "</group>" << endl;

	return result;
}

bool					SceneVisitorWrite::visit(SceneNodeLight* n)
{
	// open
	*stream << getIndent() << "<light>" << endl;
	indent();

	// write parameters
	float x;
	const float* v;
	v = n->getAmbientColor();
	*stream << getIndent() << "<ambient>" <<
								v[0] << " " <<
								v[1] << " " <<
								v[2] << "</ambient>" << endl;
	v = n->getDiffuseColor();
	*stream << getIndent() << "<diffuse>" <<
								v[0] << " " <<
								v[1] << " " <<
								v[2] << "</diffuse>" << endl;
	v = n->getSpecularColor();
	*stream << getIndent() << "<specular>" <<
								v[0] << " " <<
								v[1] << " " <<
								v[2] << "</specular>" << endl;
	v = n->getPosition();
	*stream << getIndent() << "<position>" <<
								v[0] << " " <<
								v[1] << " " <<
								v[2] << " " <<
								v[3] << "</position>" << endl;
	v = n->getSpotDirection();
	*stream << getIndent() << "<direction>" <<
								v[0] << " " <<
								v[1] << " " <<
								v[2] << "</direction>" << endl;
	x = n->getSpotExponent();
	*stream << getIndent() << "<exponent>" << x << "</exponent>" << endl;
	x = n->getSpotCutoff();
	*stream << getIndent() << "<cutoff>" << x << "</cutoff>" << endl;
	v = n->getAttenuation();
	*stream << getIndent() << "<attenuation>" <<
								v[0] << " " <<
								v[1] << " " <<
								v[2] << "</attenuation>" << endl;

	// descend
	const bool result = n->descend(this);

	// close
	exdent();
	*stream << getIndent() << "</light>" << endl;

	return result;
}

bool					SceneVisitorWrite::visit(SceneNodeMetadata* n)
{
	// open
	*stream << getIndent() << "<metadata id=\"" <<
								ConfigReader::escape(n->getID()).c_str() <<
								"\">" << endl;
	indent();

	// write parameters
	*stream << ConfigReader::escape(n->getData()).c_str() << endl;

	// descend
	const bool result = n->descend(this);

	// close
	exdent();
	*stream << getIndent() << "</metadata>" << endl;

	return result;
}

bool					SceneVisitorWrite::visit(SceneNodePrimitive* n)
{
	// open
	*stream << getIndent() << "<" << n->getType() << ">" << endl;
	indent();

	// write parameters
	*stream << getIndent() << "<index>" << endl;
	indent();
	*stream << getIndent();
	const SceneNodePrimitive::IndexArray& index = n->getIndex();
	const unsigned int num = index.size();
	for (unsigned int i = 0; i < num; ++i) {
		*stream << index[i] << " ";
	}
	*stream << endl;
	exdent();
	*stream << getIndent() << "</index>" << endl;

	// close
	exdent();
	*stream << getIndent() << "</" << n->getType() << ">" << endl;

	return true;
}

bool					SceneVisitorWrite::visit(SceneNodeXForm* n)
{
	static const char* mapType[] = { "model", "projection", "texture" };

	// open
	*stream << getIndent() << "<transform type=\"" <<
								mapType[n->getType()] << "\">" << endl;
	indent();

	// write parameters
	const float* m = n->get();
	*stream << getIndent() << "<matrix>" << endl;
	indent();
	*stream << getIndent() << m[0] << " " << m[4] << " " << m[8]  << " " << m[12] << endl;
	*stream << getIndent() << m[1] << " " << m[5] << " " << m[9]  << " " << m[13] << endl;
	*stream << getIndent() << m[2] << " " << m[6] << " " << m[10] << " " << m[14] << endl;
	*stream << getIndent() << m[3] << " " << m[7] << " " << m[11] << " " << m[15] << endl;
	exdent();
	*stream << getIndent() << "</matrix>" << endl;

	// descend
	const bool result = n->descend(this);

	// close
	exdent();
	*stream << getIndent() << "</transform>" << endl;

	return result;
}
