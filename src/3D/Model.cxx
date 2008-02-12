/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "Model.h"
#include "bzfgl.h"
#include "TextUtils.h"

OBJVert::OBJVert( float _x, float _y, float _z )
{
  x = _x;
  y = _y;
  z = _z;
}

OBJVert::OBJVert ( const OBJVert& vert )
{
  x = vert.x;
  y = vert.y;
  z = vert.z;
}

OBJVert OBJVert::operator + ( const OBJVert& vert )
{
  return OBJVert(x+vert.x, y+vert.y,z+vert.z);
}

OBJVert& OBJVert::operator += ( const OBJVert& vert )
{
  x += vert.x;
  y += vert.y;
  z += vert.z;
  return *this;
}


void OBJVert::glVertex ( void ) const
{
  glVertex3f(x,y,z);
}

void OBJVert::glNormal ( void ) const
{
  glNormal3f(x,y,z);
}

void OBJVert::glTexCoord ( void ) const
{
  glTexCoord2f(x,y);
}

void OBJVert::read3 ( const char* t )
{
  sscanf(t,"%f %f %f",&x,&y,&z);
}

void OBJVert::read2 ( const char* t )
{
  sscanf(t,"%f %f",&x,&y);
}

void OBJFace::draw ( const std::vector<OBJVert> &vertList, const std::vector<OBJVert> &normList, const std::vector<OBJVert> &uvList )
{
  if (verts.size() == 3)
  {
    glBegin(GL_TRIANGLES);
  }
  else
  {
    glBegin(GL_POLYGON);
  }

  for ( size_t i = 0; i < verts.size(); i++ )
  {
    if ( verts[i] < vertList.size() )
	vertList[ verts[i ]].glVertex();
    if ( i < norms.size() && norms[i] < normList.size() )
	normList[ norms[i] ].glNormal();
    if ( i < uvs.size() &&  uvs[i] < uvList.size() )
	uvList[ uvs[i] ].glTexCoord();
  }

  glEnd();
}

int OBJModel::draw ( void )
{
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);

  for ( size_t i = 0; i < faces.size(); i++ )
    faces[i].draw(vertList,normList,uvList);

  glMatrixMode(GL_TEXTURE);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  return (int) faces.size();
}

bool OBJModel::read ( const std::string &fileName )
{
  faces.clear();
  vertList.clear();
  normList.clear();
  uvList.clear();

  std::ifstream ifs(fileName.c_str(), std::ios::in);
  std::string line;
  while (ifs.good() && !ifs.eof())
  {
    std::getline(ifs, line);
    if ( line.size() )
    {
	// parse it
	switch(line[0])
	{
	case 'v':
	  if ( line.size() > 5 ) // there have to be enough characters for a full vert
	  {
	    OBJVert v;

	    switch (line[1])
	    {
	    case 'n':
	      v.read3(line.c_str()+2);
	      normList.push_back(v);
	      break;

	    case 't':
	      v.read2(line.c_str()+2);
	      uvList.push_back(v);
	      break;

	    default:
	      v.read3(line.c_str()+1);
	      vertList.push_back(v);
	      break;
	    }
	  }
	  break;

	case 'f':
	  {
	    std::vector<std::string> verts = TextUtils::tokenize(std::string(line.c_str()+2),std::string(" "));
	    OBJFace face;
	    for ( size_t v = 0; v < verts.size(); v++ )
	    {
	      std::vector<std::string> indexes = TextUtils::tokenize(verts[v],std::string("/"));
	      if (indexes.size())
	      {
		face.verts.push_back(atoi(indexes[0].c_str())-1);

		if ( indexes.size() > 1 && indexes[1].size() )
		  face.uvs.push_back(atoi(indexes[1].c_str())-1);
		else
		  face.uvs.push_back(0);

		if ( indexes.size() > 2 && indexes[2].size() )
		  face.norms.push_back(atoi(indexes[2].c_str())-1);
		else
		  face.norms.push_back(0);
	      }
	    }
	    if (face.verts.size())
	      faces.push_back(face);
	  }
	  break;
	}
    }
  }
  ifs.close();
  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8