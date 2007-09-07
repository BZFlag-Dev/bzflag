/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
* Writen By Jeffrey Myers
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "OpenGLUtils.h"

#include <math.h>
#define GL_INVALID_ID 0xffffffff

float getFloatColor ( int val )
{
	return val/255.0f;
}

void setColor ( float c[3], int r, int g, int b )
{
	c[0] = getFloatColor(r);
	c[1] = getFloatColor(g);
	c[2] = getFloatColor(b);
}

void glSetColor ( float c[3], float alpha )
{
	glColor4f(c[0],c[1],c[2],alpha);
}

void glTranslatefv ( float v[3] )
{
	glTranslatef(v[0],v[1],v[2]);
}

void glQuad ( float x, float y, eAlignment align, float scale )
{
	glPushMatrix();

	x *= scale;
	y *= scale;

	switch ( align )
	{
	case eCenter:
		glTranslatef(-x*0.5f,-y*0.5f,0);
		break;
	case eLowerLeft:
		break;
	case eLowerRight:
		glTranslatef(-x,0,0);
		break;
	case eUpperLeft:
		glTranslatef(0,-y,0);
		break;
	case eUpperRight:
		glTranslatef(-x,-y,0);
		break;
	case eCenterLeft:
		glTranslatef(0,-y*0.5f,0);
		break;
	case eCenterRight:
		glTranslatef(-x,-y*0.5f,0);
		break;
	case eCenterTop:
		glTranslatef(-x*0.5f,-y,0);
		break;
	case eCenterBottom:
		glTranslatef(-x*0.5f,0,0);
		break;
	}

	glBegin(GL_QUADS);

	glNormal3f(0,0,1);
	glTexCoord2f(0,1);
	glVertex3f(0,0,0);

	glTexCoord2f(1,1);
	glVertex3f(x,0,0);

	glTexCoord2f(1,0);
	glVertex3f(x,y,0);

	glTexCoord2f(0,0);
	glVertex3f(0,y,0);

	glEnd();

	glPopMatrix();
}

#define GL_RAD_CON 0.017453292519943295769236907684886f

void glLineRing ( float radius, float width )
{
	int segments = 180;

	glDisable(GL_TEXTURE_2D);
	glLineWidth(width);
	glBegin(GL_LINE_LOOP);
	for (float a = 0; a < 360.0f; a += segments/360.0f)
		glVertex3f(cosf(a*GL_RAD_CON)*radius,sinf(a*GL_RAD_CON)*radius,0);
	glEnd();
	glLineWidth(1);
	glEnable(GL_TEXTURE_2D);
}

// DisplayListSystem

DisplayListSystem::~DisplayListSystem()
{
	flushLists();
}

void DisplayListSystem::flushLists ( void )
{
	std::map<GLDisplayList,DisplayList>::iterator itr = lists.begin();

	while (itr != lists.end())
	{
		if (itr->second.glList != GL_INVALID_ID)
			glDeleteLists(itr->second.glList,1);

		itr->second.glList = GL_INVALID_ID;
		itr++;
	}
}

GLDisplayList DisplayListSystem::newList (GLDisplayListCreator *creator)
{
	if (!creator)
		return 0;

	DisplayList displayList;
	displayList.creator = creator;
	displayList.glList = GL_INVALID_ID;

	lastList++;
	lists[lastList] = displayList;
	return lastList;
}

void DisplayListSystem::freeList (GLDisplayList displayList)
{
	std::map<GLDisplayList,DisplayList>::iterator itr = lists.find(displayList);
	if (itr == lists.end())
		return;

	if (itr->second.glList != GL_INVALID_ID)
		glDeleteLists(itr->second.glList,1);

	lists.erase(itr);
}

void DisplayListSystem::callList (GLDisplayList displayList)
{
	std::map<GLDisplayList,DisplayList>::iterator itr = lists.find(displayList);
	if (itr == lists.end())
		return;

	if (itr->second.glList == GL_INVALID_ID)
	{
		itr->second.glList = glGenLists(1);
		glNewList(itr->second.glList,GL_COMPILE);
		itr->second.creator->buildGeometry(displayList);
		glEndList();
	}
	
	glCallList(itr->second.glList);
}

void DisplayListSystem::callListsV (std::vector<GLDisplayList> &displayLists)
{
	for (unsigned int i = 0; i < displayLists.size(); i++)
		callList(displayLists[i]);
}

DisplayListSystem::DisplayListSystem()
{
	lastList = 0;
}


