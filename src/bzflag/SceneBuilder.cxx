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

#include "SceneBuilder.h"
#include "World.h"
#include "Matrix.h"
#include "SceneNodes.h"
#include "SceneReader.h"
#include <sstream>
typedef std::istringstream istringstream;

//
// SceneDatabaseBuilder
//

SceneDatabaseBuilder::SceneDatabaseBuilder()
{
	// do nothing
}

SceneDatabaseBuilder::~SceneDatabaseBuilder()
{
	// do nothing
}

SceneNode*				SceneDatabaseBuilder::make(const World* world)
{
	BzfString buffer(makeBuffer(world));
	istringstream stream(buffer.c_str());

	SceneReader reader;
	return reader.read(stream);
}

BzfString				SceneDatabaseBuilder::makeBuffer(const World* world)
{
	BzfString primitives;
	BzfString unlighted;

	nVertex  = 0;
	color    = "";
	normal   = "";
	texcoord = "";
	vertex   = "";

	const WallObstacles& walls       = world->getWalls();
	const BoxBuildings& boxes        = world->getBoxes();
	const Teleporters& teleporters   = world->getTeleporters();
	const PyramidBuildings& pyramids = world->getPyramids();
	const BaseBuildings& bases       = world->getBases();

	// walls
	primitives1 = "";
	primitives2 = "";
	primitives3 = "";
	primitives4 = "";
	for (WallObstacles::const_iterator wallScan = walls.begin();
								wallScan != walls.end();
								++wallScan)
		addWall(*wallScan);
	primitives +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n"
					  "<texture filename=\"wall\" />\n";
	primitives +=	  primitives1;
	primitives +=   "</gstate>\n";
	unlighted +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n";
	unlighted += 	  primitives1;
	unlighted += 	"</gstate>\n";

	// boxes
	primitives1 = "";
	primitives2 = "";
	primitives3 = "";
	primitives4 = "";
	for (BoxBuildings::const_iterator boxScan = boxes.begin();
								boxScan != boxes.end();
								++boxScan)
		addBox(*boxScan);
	primitives +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n"
					  "<texture filename=\"boxwall\" />\n";
	primitives +=	  primitives1;
	primitives +=	"</gstate>\n";
	primitives +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n"
					  "<texture filename=\"roof\" />\n";
	primitives +=	  primitives2;
	primitives +=	  primitives3;
	primitives +=	"</gstate>\n";
	unlighted +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n";
	unlighted +=	  primitives1;
	unlighted +=	  primitives2;
	unlighted +=	  primitives3;
	unlighted +=	"</gstate>\n";

	// teleporters
	primitives1 = "";
	primitives2 = "";
	primitives3 = "";
	primitives4 = "";
	for (Teleporters::const_iterator teleporterScan = teleporters.begin();
								teleporterScan != teleporters.end();
								++teleporterScan)
		addTeleporter(*teleporterScan);
	primitives +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n"
					  "<texture filename=\"caution\" />\n";
	primitives +=	  primitives1;
	primitives +=	"</gstate>\n";
	primitives +=	"<choice><mask t=\"renderBlending\">1 2</mask>\n"
					  "<gstate>\n"
					    "<shading model=\"flat\" />\n"
					    "<stipple mask=\"on\" />\n"
					    "<geometry><stipple>0.5</stipple>\n";
	primitives +=	      primitives2;
	primitives +=	    "</geometry>\n"
					  "</gstate>\n"
					  "<gstate>\n"
					    "<shading model=\"flat\" />\n"
					    "<blending src=\"sa\" dst=\"1-sa\" />\n"
					    "<depth write=\"off\" />\n";
	primitives +=	    primitives2;
	primitives +=	  "</gstate>\n"
					"</choice>\n";
	unlighted +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n";
	unlighted +=	  primitives1;
	unlighted +=	"</gstate>\n";
	unlighted +=	"<choice><mask t=\"renderBlending\">1 2</mask>\n"
					  "<gstate>\n"
					    "<shading model=\"flat\" />\n"
					    "<stipple mask=\"on\" />\n"
					    "<geometry><stipple>0.5</stipple>\n";
	unlighted +=	      primitives3;
	unlighted +=	    "</geometry>\n"
					  "</gstate>\n"
					  "<gstate>\n"
					    "<shading model=\"flat\" />\n"
					    "<blending src=\"sa\" dst=\"1-sa\" />\n"
					    "<depth write=\"off\" />\n";
	unlighted +=	    primitives3;
	unlighted +=	  "</gstate>\n"
					"</choice>\n";

	// pyramids
	primitives1 = "";
	primitives2 = "";
	primitives3 = "";
	primitives4 = "";
	for (PyramidBuildings::const_iterator pyramidScan = pyramids.begin();
								pyramidScan != pyramids.end();
								++pyramidScan)
		addPyramid(*pyramidScan);
	primitives +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n"
					  "<texture filename=\"pyrwall\" />\n";
	primitives +=	  primitives1;
	primitives +=	  primitives2;
	primitives +=   "</gstate>\n";
	unlighted +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n";
	unlighted +=	  primitives3;
	unlighted +=	"</gstate>\n";

	// bases
	primitives1 = "";
	primitives2 = "";
	primitives3 = "";
	primitives4 = "";
	for (BaseBuildings::const_iterator baseScan = bases.begin();
								baseScan != bases.end();
								++baseScan) 
		addBase(*baseScan);
	primitives +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n"
					  "<culling cull=\"off\" />\n";
	primitives +=	  primitives1;
	primitives +=	"</gstate>\n";
	unlighted +=	"<gstate>\n"
					  "<shading model=\"flat\" />\n"
					  "<culling cull=\"off\" />\n";
	unlighted +=	  primitives2;
	unlighted +=	"</gstate>\n";

	BzfString buffer;
	buffer +=	"<choice><mask t=\"renderLighting\">1 2</mask>\n"
				  "<choice><mask t=\"renderTexturing\">1 2</mask>\n"
				    "<geometry>\n"
				      "<color>\n";
	buffer +=	        color;
	buffer +=	      "</color>\n";
	buffer +=	      "<texcoord>\n";
	buffer +=	        texcoord;
	buffer +=	      "</texcoord>\n";
	buffer +=	      "<normal>\n";
	buffer +=	        normal;
	buffer +=	      "</normal>\n";
	buffer +=	      "<vertex>\n";
	buffer +=	        vertex;
	buffer +=	      "</vertex>\n";
	buffer +=	      unlighted;
	buffer +=	    "</geometry>\n";

	buffer +=	    "<geometry id=\"world_fancy\">\n"
				      "<color>"
				        "1 1 1 1"
				      "</color>\n";
	buffer +=	      "<texcoord>\n";
	buffer +=	        texcoord;
	buffer +=	      "</texcoord>\n";
	buffer +=	      "<normal>\n";
	buffer +=	        normal;
	buffer +=	      "</normal>\n";
	buffer +=	      "<vertex>\n";
	buffer +=	        vertex;
	buffer +=	      "</vertex>\n";
	buffer +=	      primitives;
	buffer +=	    "</geometry>\n";
	buffer +=	  "</choice>\n";
	buffer +=	  "<ref id=\"world_fancy\" />\n";
	buffer +=	"</choice>\n";

	return buffer;
}

void					SceneDatabaseBuilder::prepMatrix(
								const Obstacle& o, float dz, Matrix& m)
{
	const float* pos = o.getPosition();
	m.setTranslate(pos[0], pos[1], pos[2] + dz);
	Matrix x;
	x.setRotate(0.0f, 0.0f, 1.0f, o.getRotation() * 180.0f / M_PI);
	m.mult(x);
}

void					SceneDatabaseBuilder::prepNormalMatrix(
								const Matrix& x, Matrix& n)
{
	n = x;
	n.inverse();
	n.transpose();
}

void					SceneDatabaseBuilder::addVertex(
								const Matrix& m, const float* v)
{
	float v1[3];
	m.transform3(v1, v);
	vertex += BzfString::format("%f %f %f\n", v1[0], v1[1], v1[2]);
	++nVertex;
}

void					SceneDatabaseBuilder::addVertex(
								const Matrix& m, float x, float y, float z)
{
	float v[3];
	v[0] = x;
	v[1] = y;
	v[2] = z;
	addVertex(m, v);
}

void					SceneDatabaseBuilder::addNormal(
								const Matrix& m, const float* n)
{
	float n1[3];
	m.transform3(n1, n);
	normal += BzfString::format("%f %f %f\n", n1[0], n1[1], n1[2]);
}

void					SceneDatabaseBuilder::addWall(const WallObstacle& o)
{
	static const float s_normal[][3] = {
								{ 1.0f, 0.0f, 0.0f },
								{ 1.0f, 0.0f, 0.0f },
								{ 1.0f, 0.0f, 0.0f },
								{ 1.0f, 0.0f, 0.0f }
						};
	static const float s_vertex[][3] = {
								{ 0.0f, -1.0f, 1.0f },
								{ 0.0f, -1.0f, 0.0f },
								{ 0.0f,  1.0f, 1.0f },
								{ 0.0f,  1.0f, 0.0f }
						};

//  const float dx = o.getWidth();
	const float dy = o.getBreadth();
	const float dz = o.getHeight();

	color    += "0.5 0.5 0.5 1  0.5 0.5 0.5 1  0.5 0.5 0.5 1  0.5 0.5 0.5 1\n";
	texcoord += BzfString::format("0 0  0 %f  %f 0  %f %f\n",
								 0.1f * dz, 0.1f * dy, 0.1f * dy, 0.1f * dz);

	Matrix m;
	prepMatrix(o, 0.0f, m);
	Matrix x;
	x.setScale(1.0f, dy, dz);
	m.mult(x);
	prepNormalMatrix(m, x);

	unsigned int i;
	unsigned int n = nVertex;
	for (i = 0; i < countof(s_vertex); ++i)
		addVertex(m, s_vertex[i]);
	for (i = 0; i < countof(s_normal); ++i)
		addNormal(x, s_normal[i]);

	primitives1 += BzfString::format("<primitive type=\"tstrip\"><index>"
								"%d %d %d %d</index></primitive>\n",
								n, n + 1, n + 2, n + 3);
}

void					SceneDatabaseBuilder::addBox(const BoxBuilding& o)
{
	static const float s_normal[][3] = {
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },

								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f,  1.0f },

								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f }
						};
	static const float s_vertex[][3] = {
								{ -1.0f, -1.0f,  1.0f },
								{ -1.0f, -1.0f,  0.0f },
								{  1.0f, -1.0f,  1.0f },
								{  1.0f, -1.0f,  0.0f },
								{ -1.0f,  1.0f,  1.0f },
								{ -1.0f,  1.0f,  0.0f },
								{  1.0f,  1.0f,  1.0f },
								{  1.0f,  1.0f,  0.0f },
								{ -1.0f, -1.0f,  1.0f },
								{ -1.0f, -1.0f,  0.0f },

								{ -1.0f, -1.0f,  1.0f },
								{  1.0f, -1.0f,  1.0f },
								{ -1.0f,  1.0f,  1.0f },
								{  1.0f,  1.0f,  1.0f },

								{ -1.0f, -1.0f,  0.0f },
								{  1.0f, -1.0f,  0.0f },
								{ -1.0f,  1.0f,  0.0f },
								{  1.0f,  1.0f,  0.0f }
						};

	const float dx = o.getWidth();
	const float dy = o.getBreadth();
	const float dz = o.getHeight();
	const float tm1 = 1.0f / (0.2f * BoxHeight);

	color +=	"0.75 0.25 0.25 1\n"
				"0.75 0.25 0.25 1\n"
				"0.75 0.25 0.25 1\n"
				"0.75 0.25 0.25 1\n"
				"0.63 0.25 0.25 1\n"
				"0.63 0.25 0.25 1\n"
				"0.75 0.25 0.25 1\n"
				"0.75 0.25 0.25 1\n"
				"0.75 0.375 0.375 1\n"
				"0.75 0.375 0.375 1\n"

				"0.875 0.5 0.5 1\n"
				"0.875 0.5 0.5 1\n"
				"0.875 0.5 0.5 1\n"
				"0.875 0.5 0.5 1\n"

				"0.275 0.2 0.2 1\n"
				"0.275 0.2 0.2 1\n"
				"0.275 0.2 0.2 1\n"
				"0.275 0.2 0.2 1\n";

	texcoord += BzfString::format(
				" %f %f %f %f"
				" %f %f %f %f"
				" %f %f %f %f"
				" %f %f %f %f"
				" %f %f %f %f"
				" 0 0 %f 0 0 %f %f %f"
				" 0 0 %f 0 0 %f %f %f",
				0.0f, tm1 * dz, 0.0f, 0.0f,
				tm1 * dx, tm1 * dz, tm1 * dx, 0.0f,
				tm1 * (dx+dy), tm1 * dz, tm1 * (dx+dy), 0.0f,
				tm1 * (2*dx+dy), tm1 * dz, tm1 * (2*dx+dy), 0.0f,
				tm1 * 2*(dx+dy), tm1 * dz, tm1 * 2*(dx+dy), 0.0f,
				0.5f * dx, 0.5f * dy, 0.5f * dx, 0.5f * dy,
				0.5f * dx, 0.5f * dy, 0.5f * dx, 0.5f * dy);

	Matrix m;
	prepMatrix(o, 0.0f, m);
	Matrix x;
	x.setScale(dx, dy, dz);
	m.mult(x);
	prepNormalMatrix(m, x);

	unsigned int i;
	unsigned int n = nVertex;
	for (i = 0; i < countof(s_vertex); ++i)
		addVertex(m, s_vertex[i]);
	for (i = 0; i < countof(s_normal); ++i)
		addNormal(x, s_normal[i]);

	primitives1 += BzfString::format("<primitive type=\"tstrip\"><index>"
								"%d %d %d %d %d %d %d %d %d %d"
								"</index></primitive>\n",
								n + 0, n + 1, n + 2, n + 3, n + 6,
								n + 7, n + 4, n + 5, n + 8, n + 9);
	primitives2 += BzfString::format("<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive>\n",
								n + 10, n + 11, n + 12, n + 13);
	primitives3 += BzfString::format("<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive>\n",
								n + 15, n + 14, n + 17, n + 16);
}

void					SceneDatabaseBuilder::addPyramid(const PyramidBuilding& o)
{
	static const float s_normal[][3] = {
						        {  0.0f,  0.0f,  1.0f },
						        {  0.0f, -1.0f,  1.0f },
								{  0.0f, -1.0f,  1.0f },
						        {  1.0f,  0.0f,  1.0f },
								{  1.0f,  0.0f,  1.0f },
						        {  0.0f,  1.0f,  1.0f },
								{  0.0f,  1.0f,  1.0f },
						        { -1.0f,  0.0f,  1.0f },
								{ -1.0f,  0.0f,  1.0f },

								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f }
						};
	static const float s_vertex[][3] = {
								{  0.0f,  0.0f,  1.0f },
								{ -1.0f, -1.0f,  0.0f },
								{ -1.0f, -1.0f,  0.0f },
								{  1.0f, -1.0f,  0.0f },
								{  1.0f, -1.0f,  0.0f },
								{ -1.0f,  1.0f,  0.0f },
								{ -1.0f,  1.0f,  0.0f },
								{  1.0f,  1.0f,  0.0f },
								{  1.0f,  1.0f,  0.0f },

								{ -1.0f, -1.0f,  0.0f },
								{  1.0f, -1.0f,  0.0f },
								{ -1.0f,  1.0f,  0.0f },
								{  1.0f,  1.0f,  0.0f }
						};

	const float dx = o.getWidth();
	const float dy = o.getBreadth();
	const float dz = o.getHeight();

	Matrix m;
	prepMatrix(o, 0.0f, m);
	Matrix x;
	x.setScale(dx, dy, dz);
	m.mult(x);
	prepNormalMatrix(m, x);

	color +=	"0.25 0.25 0.63 1\n"
				"0.25 0.25 0.63 1\n"
				"0.25 0.25 0.63 1\n"

				"0.13 0.13 0.51 1\n"
				"0.13 0.13 0.51 1\n"

				"0.375 0.375 0.75 1\n"
				"0.375 0.375 0.75 1\n"

				"0.25 0.25 0.63 1\n"
				"0.25 0.25 0.63 1\n"

				"0.25 0.25 0.63 1\n"
				"0.25 0.25 0.63 1\n"
				"0.25 0.25 0.63 1\n"
				"0.25 0.25 0.63 1\n";
	texcoord += BzfString::format(
				" 0 %f"
				" %f 0 %f 0"
				" %f 0 %f 0"
				" %f 0 %f 0"
				" %f 0 %f 0"
				"  0 0 %f 0 0 %f %f %f",
				0.25f * dz,
				-0.25f * dx, 0.25f * dx, -0.25f * dx, 0.25f * dx,
				-0.25f * dx, 0.25f * dx, -0.25f * dx, 0.25f * dx,
				0.25f * dx, 0.25f * dz, 0.25f * dx, 0.25f * dz);

	unsigned int i;
	unsigned int n = nVertex;
	for (i = 0; i < countof(s_vertex); ++i)
		addVertex(m, s_vertex[i]);
	for (i = 0; i < countof(s_normal); ++i)
		addNormal(x, s_normal[i]);

	primitives1 += BzfString::format("<geometry><color>0.25 0.25 0.63 1</color>"
								"<primitive type=\"triangles\"><index>"
								"%d %d %d %d %d %d %d %d %d %d %d %d"
								"</index></primitive></geometry>\n",
								n + 0, n + 1, n + 4, n + 0, n + 3, n + 8,
								n + 0, n + 7, n + 6, n + 0, n + 5, n + 2);
	primitives2 += BzfString::format("<geometry><color>0.25 0.25 0.63 1</color>"
								"<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive></geometry>\n",
								n + 9, n + 10, n + 12, n + 11);

	primitives3 += BzfString::format("<primitive type=\"triangles\"><index>"
								"%d %d %d %d %d %d %d %d %d %d %d %d"
								"</index></primitive>"
								"<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive>\n",
								n + 0, n + 1, n + 4, n + 0, n + 3, n + 8,
								n + 0, n + 7, n + 6, n + 0, n + 5, n + 2,
								n + 9, n + 10, n + 12, n + 11);
}

void					SceneDatabaseBuilder::addBase(const BaseBuilding& o)
{
	static const float s_normal[][3] = {
								{ 0.0f, 0.0f, 1.0f },
								{ 0.0f, 0.0f, 1.0f },
								{ 0.0f, 0.0f, 1.0f },
								{ 0.0f, 0.0f, 1.0f }
						};
	static const float s_vertex[][3] = {
								{ -1.0f, -1.0f,  0.0f },
								{  1.0f, -1.0f,  0.0f },
								{ -1.0f,  1.0f,  0.0f },
								{  1.0f,  1.0f,  0.0f }
						};

	const float dx = o.getWidth();
	const float dy = o.getBreadth();
	const float dz = o.getHeight();
	const float* c = Team::getTankColor(static_cast<TeamColor>(o.getTeam()));

	// lift bases on the ground a little so they don't flimmer
	Matrix m;
	prepMatrix(o, dz == 0.0f ? 0.04f : 0.0f, m);
	Matrix x;
	x.setScale(dx, dy, dz);
	m.mult(x);
	prepNormalMatrix(m, x);

	color += BzfString::format("%f %f %f 1 %f %f %f 1 %f %f %f 1 %f %f %f 1\n",
								c[0], c[1], c[2],
								c[0], c[1], c[2],
								c[0], c[1], c[2],
								c[0], c[1], c[2]);
	texcoord += " 0 0 0 0 0 0 0 0\n";

	unsigned int i;
	unsigned int n = nVertex;
	for (i = 0; i < countof(s_vertex); ++i)
		addVertex(m, s_vertex[i]);
	for (i = 0; i < countof(s_normal); ++i)
		addNormal(x, s_normal[i]);

	primitives1 += BzfString::format("<geometry><color>%f %f %f 1</color>"
								"<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive></geometry>\n",
								c[0], c[1], c[2],
								n + 1, n + 0, n + 3, n + 2);
	primitives2 += BzfString::format("<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive>\n",
								n + 1, n + 0, n + 3, n + 2);
}

void					SceneDatabaseBuilder::addTeleporter(const Teleporter& o)
{
	static const float s_normal[][3] = {
								{  0.0f, -1.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  1.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },

								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },

								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f }
						};

	const float dx = o.getWidth();
	const float dy = o.getBreadth();
	const float dz = o.getHeight();
	const float db = o.getBorder();

	const float yo = dy + db;
	const float yi = dy;

	Matrix m;
	prepMatrix(o, 0.0f, m);
	Matrix x;
	prepNormalMatrix(m, x);

	color +=	"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"
				"1 0.875 0 1\n"

				"0 0 0 0.5\n"
				"0 0 0 0.5\n"
				"0 0 0 0.5\n"
				"0 0 0 0.5\n"

				"0 0 0 0.5\n"
				"0 0 0 0.5\n"
				"0 0 0 0.5\n"
				"0 0 0 0.5\n";
	texcoord += "0.0  0.0   0.0  9.5\n"
		      "0.5  0.0   0.5  9.0\n"
		      "1.0  0.0   1.0  9.0\n"
		      "1.5  0.0   1.5  9.5\n"
		      "2.0  0.0   2.0  9.5\n"
		      "0.0  9.5   0.0  4.5\n"
		      "0.5  9.0   0.5  5.0\n"
		      "1.0  9.0   1.0  13.0\n"
		      "1.5  9.5   1.5  14.5\n"
		      "1.5  9.5   1.5  12.5\n"
		      "2.0  9.5   2.0  12.5\n"
		      "0.5  9.0   0.5  13.0\n"
		      "1.0  9.0   1.0  13.0\n"
		      "0.5  0.0   0.5  9.5\n"
		      "1.0  0.0   1.0  9.0\n"
		      "1.5  0.0   1.5  9.0\n"
		      "2.0  0.0   2.0  9.5\n"
		      "2.5  0.0   2.5  9.5\n"

		      "0 0 0 0 0 0 0 0\n"
		      "0 0 0 0 0 0 0 0\n";

	unsigned int i;
	unsigned int n = nVertex;

	addVertex(m, -dx, -yo, 0.0f);
	addVertex(m, -dx, -yo, dz + db);
	addVertex(m, -dx, -yi, 0.0f);
	addVertex(m, -dx, -yi, dz);
	addVertex(m,  dx, -yi, 0.0f);
	addVertex(m,  dx, -yi, dz);
	addVertex(m,  dx, -yo, 0.0f);
	addVertex(m,  dx, -yo, dz + db);
	addVertex(m, -dx, -yo, 0.0f);
	addVertex(m, -dx, -yo, dz + db);

	addVertex(m, -dx, -yo, dz + db);
	addVertex(m, -dx,  yo, dz + db);
	addVertex(m, -dx, -yi, dz);
	addVertex(m, -dx,  yi, dz);
	addVertex(m,  dx, -yi, dz);
	addVertex(m,  dx,  yi, dz);
	addVertex(m,  dx, -yo, dz + db);
	addVertex(m,  dx,  yo, dz + db);
	addVertex(m,  dx, -yo, dz + db);
	addVertex(m,  dx,  yo, dz + db);
	addVertex(m, -dx, -yo, dz + db);
	addVertex(m, -dx,  yo, dz + db);
	addVertex(m, -dx, -yi, dz);
	addVertex(m, -dx,  yi, dz);
	addVertex(m,  dx, -yi, dz);    
	addVertex(m,  dx,  yi, dz);

	addVertex(m,  dx,  yo, 0.0f);
	addVertex(m,  dx,  yo, dz + db);
	addVertex(m,  dx,  yi, 0.0f);
	addVertex(m,  dx,  yi, dz);
	addVertex(m, -dx,  yi, 0.0f);
	addVertex(m, -dx,  yi, dz);
	addVertex(m, -dx,  yo, 0.0f);
	addVertex(m, -dx,  yo, dz + db);
	addVertex(m,  dx,  yo, 0.0f);
	addVertex(m,  dx,  yo, dz + db);

	addVertex(m,  dx, -yi, 0.0f);
	addVertex(m,  dx,  yi, 0.0f);
	addVertex(m,  dx, -yi, dz);
	addVertex(m,  dx,  yi, dz);
	addVertex(m, -dx, -yi, 0.0f);
	addVertex(m, -dx,  yi, 0.0f);
	addVertex(m, -dx, -yi, dz);
	addVertex(m, -dx,  yi, dz);

	for (i = 0; i < countof(s_normal); ++i)
		addNormal(x, s_normal[i]);

	primitives1 += BzfString::format("<primitive type=\"tstrip\"><index>"
								"%d %d %d %d %d %d %d %d %d %d"
								"</index></primitive>\n",
								n + 0, n + 1, n + 2, n + 3, n + 4,
								n + 5, n + 6, n + 7, n + 8, n + 9);
	primitives1 += BzfString::format("<primitive type=\"tstrip\"><index>"
								"%d %d %d %d %d %d %d %d %d %d"
								"</index></primitive>\n",
								n + 26, n + 27, n + 28, n + 29, n + 30,
								n + 31, n + 32, n + 33, n + 34, n + 35);
	primitives1 += BzfString::format("<primitive type=\"triangles\"><index>"
								"%d %d %d %d %d %d "
								"%d %d %d %d %d %d "
								"%d %d %d %d %d %d "
								"%d %d %d %d %d %d"
								"</index></primitive>\n",
								n + 10, n + 11, n + 12, n + 13, n + 12, n + 11,
								n + 14, n + 15, n + 16, n + 17, n + 16, n + 15,
								n + 18, n + 19, n + 20, n + 21, n + 20, n + 19,
								n + 22, n + 23, n + 24, n + 25, n + 24, n + 23);
	primitives2 += BzfString::format("<geometry><color>0 0 0 0.5</color>"
								"<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive>"
								"<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive>"
								"</geometry>\n",
								n + 36, n + 37, n + 38, n + 39,
								n + 41, n + 40, n + 43, n + 42);
	primitives3 += BzfString::format("<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive>"
								"<primitive type=\"tstrip\"><index>"
								"%d %d %d %d"
								"</index></primitive>\n",
								n + 36, n + 37, n + 38, n + 39,
								n + 41, n + 40, n + 43, n + 42);
}
