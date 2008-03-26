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

#ifndef __VECTOR3D_H__
#define __VECTOR3D_H__

#include "common.h"
#include <assert.h>

#define IS_ZERO(_value)  ( ((_value) > -ZERO_TOLERANCE) && ((_value) < ZERO_TOLERANCE) )

class Vector3D {
public:
  float x, y, z;

  // Constructors
  inline Vector3D() {}
  inline Vector3D( const float _x, const float _y, const float _z )
    : x( _x ), y( _y ), z( _z ) {}
  inline Vector3D( const float coords[3] ) 
    : x( coords[0] ), y( coords[1] ), z( coords[2] ) {}
  inline Vector3D( const Vector3D& v )
    : x( v.x ), y ( v.y ), z( v.z ) {}
  inline Vector3D( const float r )
    : x( r ), y ( r ), z( r ) {}

  // Assigment operators
  inline float operator[] ( const unsigned int index ) const {
    assert( ( index < 3 ) && "Bad index passed to Vector3D!");
    return *( &x + index );
  }
  inline float& operator[] ( const unsigned int index ) {
    assert( (index < 3) && "Bad index passed to Vector3D!");
    return *( &x + index );
  }
  inline Vector3D& operator= ( const Vector3D& v ) {
    x = v.x;
    y = v.y;
    z = v.z;
  }

  // Comparision operators
  inline bool operator== ( const Vector3D& v ) const {
    return ( x == v.x && y == v.y && z == v.z );
  }
  inline bool operator!= ( const Vector3D& v ) const {
    return ( x != v.x || y != v.y || z != v.z );
  }

  // Arithmetic operators
  inline Vector3D operator+ ( const Vector3D& v ) const {
    return Vector3D( x + v.x, y + v.y, z + v.z );
  }
  inline Vector3D operator- ( const Vector3D& v ) const {
    return Vector3D( x - v.x, y - v.y, z - v.z );
  }
  inline Vector3D operator* ( const Vector3D& v ) const {
    return Vector3D( x * v.x, y * v.y, z * v.z );
  }
  inline Vector3D operator* ( const float r ) const {
    return Vector3D( x * r, y * r, z * r );
  }
  inline Vector3D operator/ ( const Vector3D& v ) const {
    return Vector3D( x / v.x, y / v.y, z / v.z );
  }
  inline Vector3D operator/ ( const float r ) const {
    assert( (r != 0.0f) && "Division of Vector3D by zero would occur!");
    return Vector3D( x / r, y / r, z / r );
  }
  inline Vector3D& operator+ () {
    return *this;
  }
  inline Vector3D operator- () const {
    return Vector3D( -x, -y, -z );
  }
  
  // Vector operations
  inline float length() const {
    return sqrt( x * x + y * y + z * z );
  }
  inline float lengthSq() const {
    return x * x + y * y + z * z ;
  }
  inline float distance( const Vector3D& v ) const {
    return (*this - v).length();
  }
  inline float dot( const Vector3D& v ) const {
    return x * v.x + y * v.y + z * v.z;
  }
  inline void normalize() {
    float l = length();
    if (! NEAR_ZERO(l,ZERO_TOLERANCE) ) {
      x /= l;
      y /= l;
      z /= l;
    }
  }
  inline Vector3D norm() const {
    float l = length();
    return NEAR_ZERO(l,ZERO_TOLERANCE) ? (*this) : Vector3D((*this) / l);
  }
  inline Vector3D cross( const Vector3D& v ) const {
    return Vector3D( y * v.z - z * v.y , z * v.x - x * v.z , x * v.y - y * v.x );
  }

  inline bool equals( const Vector3D& v , const float e = ZERO_TOLERANCE ) const {
    return ( NEAR_ZERO(x-v.x,e) && NEAR_ZERO(y-v.y,e) && NEAR_ZERO(z-v.z,e) );
  }
  inline bool isZero( const float e = ZERO_TOLERANCE ) const {
    return ( NEAR_ZERO(x,e) && NEAR_ZERO(y,e) && NEAR_ZERO(z,e) );
  }

  static const Vector3D ZERO;
  static const Vector3D AXISX;
  static const Vector3D AXISY;
  static const Vector3D AXISZ;
    
  
};

#endif // __VECTOR3D_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8


