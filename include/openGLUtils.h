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
#ifndef _OPENGL_H_
#define _OPENGL_H_

#ifdef _WIN32 // this file only has windows stuff
	#include <windows.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#else
	#ifdef __APPLE__
		#include <Carbon/Carbon.h>
		#include <AGL/agl.h>
		#include <AGL/gl.h>
		#include <AGL/glu.h>
	#else	// linux
		#include <GL/gl.h>
		#include <GL/glu.h>
	#endif
#endif // _WIN32

#include <map>
#include <vector>

#define GL_INVALID_ID 0xffffffff

float getFloatColor ( int val );
void setColor ( float c[3], int r, int g, int b );
void glSetColor ( float c[3], float alpha = 1.0f );
void glTranslatefv ( float v[3] );

typedef enum
{
	eCenter,
	eLowerLeft,
	eLowerRight,
	eUpperLeft,
	eUpperRight,
	eCenterLeft,
	eCenterRight,
	eCenterTop,
	eCenterBottom
}eAlignment;

void glQuad ( float x, float y, eAlignment align, float scale = 1.0f );
void glLineRing ( float radius, float width = 1 );

typedef unsigned int GLDisplayList;

class GLDisplayListCreator
{
public:
	virtual ~GLDisplayListCreator(){};

	virtual void buildGeometry ( GLDisplayList displayList ) = 0;
};

class DisplayListSystem
{
public:
	static DisplayListSystem& Instance()
	{
		static DisplayListSystem dls;
		return dls;
	}

	~DisplayListSystem();

	GLDisplayList newList (GLDisplayListCreator *creator);
	void freeList (GLDisplayList displayList);

	void flushLists ( void );

	void callList (GLDisplayList displayList);
	void callListsV (std::vector<GLDisplayList> &displayLists);

protected:
	DisplayListSystem();

	typedef struct 
	{
		GLDisplayListCreator	*creator;
		GLuint					glList;
	}DisplayList;

	std::map<GLDisplayList,DisplayList>	lists;
	GLDisplayList						lastList;
};


#endif //_OPENGL_H_

