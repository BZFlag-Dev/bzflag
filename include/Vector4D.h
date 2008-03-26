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

#ifndef __VECTOR4D_H__
#define __VECTOR4D_H__

#include <assert.h>
#include "common.h"
#include "Vector3D.h"

class Vector4D {
public:
  float x, y, z, w;

  // Constructors
  inline Vector4D() {}
  inline Vector4D( const float _x, const float _y, const float _z , const float _w )
    : x( _x ), y( _y ), z( _z ), w( _w ) {}
  inline Vector4D( const float coords[4] ) 
    : x( coords[0] ), y( coords[1] ), z( coords[2] ) , w( coords[3] ) {}
  inline Vector4D( const Vector4D& v )
    : x( v.x ), y ( v.y ), z( v.z ), w( v.w ) {}
  inline Vector4D( const Vector3D& v )
    : x( v.x ), y ( v.y ), z( v.z ), w( 1.0f ) {}
  inline Vector4D( const float r )
    : x( r ), y ( r ), z( r ) , w( r ) {}

  // Assigment operators
  inline float operator[] ( const unsigned int index ) const {
    assert( ( index < 4 ) && "Bad index passed to Vector4D!");
    return *(&x+index);
  }
  inline float& operator[] ( const unsigned int index ) {
    assert( ( index < 4 ) && "Bad index passed to Vector4D!");
    return *(&x+index);
  }
  inline Vector4D& operator= ( const Vector4D& v ) {
    x = v.x;
    y = v.y;
    z = v.z;
    w = v.w;
  }
  inline Vector4D& operator= ( const Vector3D& v ) {
    x = v.x;
    y = v.y;
    z = v.z;
    w = 1.0f;
  }

  // Comparision operators
  inline bool operator== ( const Vector4D& v ) const {
    return ( x == v.x && y == v.y && z == v.z && w == v.w );
  }
  inline bool operator!= ( const Vector4D& v ) const {
    return ( x != v.x || y != v.y || z != v.z || w != v.w );
  }

  // Arithmetic operators
  inline Vector4D operator+ ( const Vector4D& v ) const {
    return Vector4D( x + v.x, y + v.y, z + v.z, w + v.w );
  }
  inline Vector4D operator- ( const Vector4D& v ) const {
    return Vector4D( x - v.x, y - v.y, z - v.z, w - v.w );
  }
  inline Vector4D operator* ( const Vector4D& v ) const {
    return Vector4D( x * v.x, y * v.y, z * v.z, w * v.w );
  }
  inline Vector4D operator* ( const float r ) const {
    return Vector4D( x * r, y * r, z * r, w * r );
  }
  inline Vector4D operator/ ( const Vector4D& v ) const {
    return Vector4D( x / v.x, y / v.y, z / v.z, w / v.w );
  }
  inline Vector4D operator/ ( const float r ) const {
    assert( ( r != 0.0f ) && "Division of Vector4D by zero would occur!");
    return Vector4D( x / r, y / r, z / r, w / r );
  }
  inline Vector4D& operator+ () {
    return *this;
  }
  inline Vector4D operator- () const {
    return Vector4D( -x, -y, -z, -w );
  }
  
  // Vector operations
  inline float dot( const Vector4D& v ) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
  }

  static const Vector4D ZERO;
  
};

#endif // __VECTOR4D_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

