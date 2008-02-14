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

#ifndef _MODEL_H_
#define _MODEL_H_

#include "common.h"

#include <vector>
#include <string>
#include <fstream>

class OBJVert
{
public:
  float x,y,z;

  OBJVert( float _x = 0, float _y = 0, float _z = 0 );

  OBJVert ( const OBJVert& vert );

  OBJVert operator + ( const OBJVert& vert );
  OBJVert& operator += ( const OBJVert& vert );

  void glVertex ( void ) const;
  void glNormal ( void ) const;
  void glTexCoord ( void ) const;

  void read3 ( const char* t );
  void read2 ( const char* t );
};

class OBJFace
{
public:
  std::vector<size_t> verts;
  std::vector<size_t> norms;
  std::vector<size_t> uvs;

  void draw ( const std::vector<OBJVert> &vertList, const std::vector<OBJVert> &normList, const std::vector<OBJVert> &uvList );
};

class OBJModel
{
public:
  std::vector<OBJFace> faces;
  std::vector<OBJVert>	vertList,normList,uvList;

  int draw ( void );
  bool read ( const std::string &fileName );
  bool read ( const  char* fileName ) {return read(std::string(fileName));}

};

#endif // _MODEL_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
