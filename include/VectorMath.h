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

#ifndef _VECTOR_MATH_H_
#define _VECTOR_MATH_H_

#include "common.h"
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#define fastRad_con			0.01745329251995f
#define fastDeg_con			57.2957795130823f

#ifndef REALY_SMALL
#define REALY_SMALL 0.0001f
#endif

#ifndef REALY_REALY_SMALL
#define REALY_REALY_SMALL 0.000001f
#endif


// float based API
void set ( float result[3], float x = 0, float y = 0, float z = 0 );
void set2d ( float result[2], float x = 0, float y = 0);

void copy (const float rhs[3], float result[3]);
void copy2d (const float rhs[2], float result[2]);

void cross(const float lhs[3], const float rhs[3], float result[3]);

float dot(const float lhs[3], const float rhs[3]);
float dot2d(const float lhs[2], const float rhs[2]);

void add(const float lhs[3], const float rhs[3], float result[3]);
void add2d(const float lhs[2], const float rhs[2], float result[2]);

void subtract(const float lhs[3], const float rhs[3], float result[3]);
void subtract2d(const float lhs[2], const float rhs[2], float result[2]);

void multiply(const float lhs[3], const float rhs[3], float result[3]);
void multiply(const float lhs[3], const float rhs, float result[3]);
void multiply2d(const float lhs[2], const float rhs[2], float result[2]);
void multiply2d(const float lhs[2], const float rhs, float result[2]);

void divide(const float lhs[3], const float rhs[3], float result[3]);
void divide(const float lhs[3], const float rhs, float result[3]);
void divide2d(const float lhs[2], const float rhs[2], float result[2]);
void divide2d(const float lhs[2], const float rhs, float result[2]);

float normalise( float vec[3] );
float normalise2d( float vec[2] );

float magnitude( const float vec[3] );
float magnitude2d( const float vec[2] ) ;
float magnitudeSquared( const float vec[3] );
float magnitudeSquared2d( const float vec[2] ) ;

void biforcate ( const float lhs[3], const float rhs[3], float result[3] );
void biforcate2d ( const float lhs[2], const float rhs[2], float result[2] );

// class based API

class Vector2;

class Vector3
{
public:
  Vector3 ( float x = 0, float y = 0, float z = 0 );
  Vector3 ( float vec[3] );
  Vector3 ( const Vector3 &_vec );
  Vector3 ( const Vector2 &_vec );

  virtual ~Vector3();

  Vector3 operator + (const Vector3 &rhs) const;
  Vector3& operator += (const Vector3 &rhs);
  Vector3 operator - (const Vector3 &rhs) const;
  Vector3& operator -= (const Vector3 &rhs);
  Vector3 operator * (const Vector3 &rhs) const;
  Vector3& operator *= (const Vector3 &rhs);
  Vector3 operator / (const Vector3 &rhs) const;
  Vector3& operator /= (const Vector3 &rhs);

  Vector3 operator * (const float &rhs) const;
  Vector3& operator *= (const float &rhs);
  Vector3 operator / (const float &rhs) const;
  Vector3& operator /= (const float &rhs);

  Vector3& operator = (const Vector3 &rhs);

  float operator [] ( const unsigned int index ) const;

  bool operator == (const Vector3 &rhs ) const;
  bool operator != (const Vector3 &rhs ) const;

  operator const float* ( void ) const;

  float dot ( const Vector3 &rhs );
  float dot ( const float rhs[3] );
  float dot ( const float x = 0, float y = 0, float z = 1 );

  void cross ( const Vector3 &lhs, const Vector3 &rhs );
  void cross ( const float lhs[], const float rhs[] );

  float normalise ( void );
  float magnitude ( void );
  float magnitudeSquared ( void );
  float magnitude2d ( void );
  float magnitudeSquared2d ( void );

  float x ( void ) const;
  float y ( void ) const;
  float z ( void ) const;

  float x ( float _x );
  float y ( float _y );
  float z ( float _z );

  bool close ( const Vector3& v ) const;

protected:
  float vec[3];
};

class Vector2
{
public:
  Vector2 ( float x = 0, float y = 0 );
  Vector2 ( float vec[2] );
  Vector2 ( const Vector2 &_vec );
  Vector2 ( const Vector3 &_vec );

  virtual ~Vector2();

  Vector2 operator + (const Vector2 &rhs) const;
  Vector2& operator += (const Vector2 &rhs);
  Vector2 operator - (const Vector2 &rhs) const;
  Vector2& operator -= (const Vector2 &rhs);
  Vector2 operator * (const Vector2 &rhs) const;
  Vector2& operator *= (const Vector2 &rhs);
  Vector2 operator / (const Vector2 &rhs) const;
  Vector2& operator /= (const Vector2 &rhs);

  Vector2 operator * (const float &rhs) const;
  Vector2& operator *= (const float &rhs);
  Vector2 operator / (const float &rhs) const;
  Vector2& operator /= (const float &rhs);

  Vector2& operator = (const Vector2 &rhs);

  float operator [] ( const unsigned int index ) const;

  bool operator == (const Vector2 &rhs ) const;
  bool operator != (const Vector2 &rhs ) const;

  operator const float* ( void ) const;

  float dot ( const Vector2 &rhs );
  float dot ( const float rhs[3] );
  float dot ( const float x = 0, float y = 1 );

  float normalise ( void );
  float magnitude ( void );
  float magnitudeSquared ( void );

  float x ( void ) const;
  float y ( void ) const;

  float x ( float _x );
  float y ( float _y );

protected:
  float vec[2];
};

typedef std::vector<Vector2> Vector2List;

// trig utilites
float toRad ( float angle );
float toDeg ( float angle );

Vector2 angleToVector( float angle );
float	vectorToAngle ( Vector2 &vec );

// vector utils
bool pointLeftOfVectorXY( const Vector3 &sp, const Vector3 &ep, const Vector3 &point );
Vector3 getVectorNormalXY( const Vector3 &vec );
bool pointInSphere ( const Vector3 &point, const Vector3 &cp, const float rad );
bool sphereInSphere ( const Vector3 &point, const float r1, const Vector3 &cp, const float r2 );
bool pointInCircle ( const Vector2 &point, const Vector2 &cp, const float rad );
bool pointInPolygon ( const Vector2 &point, const Vector2List verts, bool ccw = true );

// matrix math

class Matrix34
{
public:
  typedef enum
  {
    XY, 
    XZ, 
    YZ,
    XR,
    XD,
    NonPrimary
  }Plane;

  Matrix34 ( void );
  Matrix34 ( const float v[12] );
  Matrix34 ( const Vector3 &trans );
  Matrix34 (const Vector3 &d, const Vector3 &h = Vector3(1,0,0), const Vector3 &pos = Vector3(0,0,0) );

  Matrix34& operator = ( const Matrix34 &rhs );
  bool operator == ( const Matrix34 &rhs );
  bool operator != ( const Matrix34 &rhs );

  Matrix34& operator += ( const Matrix34 &rhs );
  Matrix34& operator -= ( const Matrix34 &rhs );

  Matrix34& operator *= ( const Matrix34 &rhs );

  float operator [] ( const unsigned int index ) const;
  operator const float* ( void ) const;

  void classify ( void );
  void normalise ( void );
  void identity ( void );

  bool scaleNonUniform( void );
  bool scaleNonUniform2d( void );

  Vector3 matrixScale( void ) const;

  void translate( const Vector3 &vec );
  void translation( const Vector3 &vec );
  void scale( const Vector3 &scale );
  void scaling( const Vector3 &scale );
  void rotation( const Vector3 &axis , float sin, float cosin );
  void rotation( const Vector3 &axis , float angle );
  void rotation( float sin, float cosin );
  void rotation( float angle );
  void rotate2d( float angle, const Vector3 &origin );
  void mirror( const Vector3 &axisPolarity, const Vector3 &mirrorPoint );
  void forceDepth( float depth );
  void invert( void );
  void transpose ( void );

  Vector3 &transformPos( Vector3 &point ) const;
  Vector3 &transformPos2d( Vector3 &point ) const;
  Vector3 &transformNorm( Vector3 &point ) const;
  Vector3 &transformNorm2d( Vector3 &point ) const;

  bool samePlane ( const Matrix34& matrix, bool *reversed = 0 ) const;

  float determinant( void ) const;

protected:
  void invert3x3 ( void );

  typedef enum
  {
    transFlag = 1,
    scaleFlag = 2,
    rotateFlag = 4
  }Flags;

  int		flags;
  float mat[3][4];
};

#endif //_VECTOR_MATH_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8