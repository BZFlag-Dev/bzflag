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

#ifndef __VECTOR2D_H__
#define __VECTOR2D_H__

#include <assert.h>
#include "common.h"

class Vector2D {
public:
  float x, y;

  // Constructors
  inline Vector2D() {}
  inline Vector2D( const float _x, const float _y )
    : x( _x ), y( _y ) {}
  inline Vector2D( const float coords[2] ) 
    : x( coords[0] ), y( coords[1] ) {}
  inline Vector2D( const Vector2D& v )
    : x( v.x ), y ( v.y ) {}
  inline Vector2D( const float r )
    : x( r ), y ( r ) {}

  // Assigment operators
  inline float operator[] ( const unsigned int index ) const {
    assert( ( index < 2 ) && "Bad index passed to Vector2D!");
    return *( &x+index );
  }
  inline float& operator[] ( const unsigned int index ) {
    assert( ( index < 2 ) && "Bad index passed to Vector2D!");
    return *( &x+index );
  }
  inline Vector2D& operator= ( const Vector2D& v ) {
    x = v.x;
    y = v.y;
  }

  // Comparision operators
  inline bool operator== ( const Vector2D& v ) const {
    return ( x == v.x && y == v.y );
  }
  inline bool operator!= ( const Vector2D& v ) const {
    return ( x != v.x || y != v.y );
  }

  // Arithmetic operators
  inline Vector2D operator+ ( const Vector2D& v ) const {
    return Vector2D( x + v.x, y + v.y );
  }
  inline Vector2D operator- ( const Vector2D& v ) const {
    return Vector2D( x - v.x, y - v.y );
  }
  inline Vector2D operator* ( const Vector2D& v ) const {
    return Vector2D( x * v.x, y * v.y );
  }
  inline Vector2D operator* ( const float r ) const {
    return Vector2D( x * r, y * r );
  }
  inline Vector2D operator/ ( const Vector2D& v ) const {
    return Vector2D( x / v.x, y / v.y );
  }
  inline Vector2D operator/ ( const float r ) const {
    assert( ( r != 0.0f ) && "Division of Vector2D by zero would occur!");
    return Vector2D( x / r, y / r );
  }
  inline Vector2D& operator+ () {
    return *this;
  }
  inline Vector2D operator- () const {
    return Vector2D(-x,-y);
  }

  // Vector operations
  inline float length() const {
    return sqrt( x * x + y * y );
  }
  inline float lengthSq() const {
    return x * x + y * y;
  }
  inline float distance( const Vector2D& v ) const {
    return (*this - v).length();
  }
  inline float dot( const Vector2D& v ) const {
    return x * v.x + y * v.y;
  }
  inline void normalize() {
    float l = length();
    if ( NEAR_ZERO(l,ZERO_TOLERANCE) ) {
      x /= l;
      y /= l;
    }
  }
  inline Vector2D norm() const {
    float l = length();
    return NEAR_ZERO(l,ZERO_TOLERANCE) ? (*this) : Vector2D((*this) / l);
  }

  inline bool equals( const Vector2D& v , const float e = ZERO_TOLERANCE ) const {
    return ( NEAR_ZERO(x-v.x,e) && NEAR_ZERO(y-v.y,e) );
  }
  inline bool isZero( const float e = ZERO_TOLERANCE ) const {
    return ( NEAR_ZERO(x,e) && NEAR_ZERO(y,e) );
  }

  static const Vector2D ZERO;
  static const Vector2D AXISX;
  static const Vector2D AXISY;

};

#endif // __VECTOR2D_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
