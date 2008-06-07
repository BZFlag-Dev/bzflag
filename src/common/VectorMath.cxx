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

#include "vectorMath.h"
#include <memory.h>

#define LIN_TOL				0.0000005f		

#define EQUAL_X(v0, v1, z)	(fabs((v0) - (v1)) < (z))
#define EQUAL_X2(v0, v1, z)	(EQUAL_X((v0).x, (v1).x, z) && EQUAL_X((v0).y, (v1).y, z))
#define EQUAL_X3(v0, v1, z)	(EQUAL_X2(v0, v1, z) && EQUAL_X((v0).z, (v1).z, z))

void set ( float result[3], float x , float y, float z )
{	
  result[0] = x;
  result[1] = y;
  result[2] = z;
}

void set2d ( float result[2], float x, float y )
{	
  result[0] = x;
  result[1] = y;
}

void copy (const float rhs[3], float result[3])
{
  for(int i =0; i < 3; i++)
    result[i] = rhs[i];
}

void copy2d (const float rhs[2], float result[2])
{
  for(int i =0; i < 2; i++)
    result[i] = rhs[i];
}

void cross(const float lhs[3], const float rhs[3], float result[3])
{
  float temp_x, temp_y;		// this allows result to be same as lhs or rhs

  temp_x = lhs[2] * rhs[1] - lhs[1] * rhs[2];
  temp_y = lhs[2] * rhs[0] - lhs[0] * rhs[2];
  result[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
  result[0] = temp_x;
  result[1] = temp_y;
}

float dot(const float lhs[3], const float rhs[3])
{
  return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}

float dot2d(const float lhs[2], const float rhs[2])
{
  return lhs[0] * rhs[0] + lhs[1] * rhs[1];
}

void add(const float lhs[3], const float rhs[3], float result[3])
{
  for(int i =0; i < 3; i++)
    result[i] = lhs[i] + rhs[i];
}

void add2d(const float lhs[2], const float rhs[2], float result[2])
{
  for(int i =0; i < 2; i++)
    result[i] = lhs[i] + rhs[i];
}

void subtract(const float lhs[3], const float rhs[3], float result[3])
{
  for(int i =0; i < 3; i++)
    result[i] = lhs[i] - rhs[i];
}

void subtract2d(const float lhs[2], const float rhs[2], float result[2])
{
  for(int i =0; i < 2; i++)
    result[i] = lhs[i] - rhs[i];
}

void multiply(const float lhs[3], const float rhs[3], float result[3])
{
  for(int i =0; i < 3; i++)
    result[i] = lhs[i] * rhs[i];
}

void multiply(const float lhs[3], const float rhs, float result[3])
{
  for(int i =0; i < 3; i++)
    result[i] = lhs[i] * rhs;
}

void multiply2d(const float lhs[2], const float rhs[2], float result[2])
{
  for(int i =0; i < 2; i++)
    result[i] = lhs[i] * rhs[i];
}

void multiply2d(const float lhs[2], const float rhs, float result[2])
{
  for(int i =0; i < 2; i++)
    result[i] = lhs[i] * rhs;
}

void divide(const float lhs[3], const float rhs[3], float result[3])
{
  for(int i =0; i < 3; i++)
    result[i] = lhs[i] / rhs[i];
}

void divide(const float lhs[3], const float rhs, float result[3])
{
  for(int i =0; i < 3; i++)
    result[i] = lhs[i] / rhs;
}

void divide2d(const float lhs[2], const float rhs[2], float result[2])
{
  for(int i =0; i < 2; i++)
    result[i] = lhs[i] / rhs[i];
}

void divide2d(const float lhs[2], const float rhs, float result[2])
{
  for(int i =0; i < 2; i++)
    result[i] = lhs[i] / rhs;
}

float normalise( float vec[3] )
{
  float dist = sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
  vec[0] /= dist;
  vec[1] /= dist;
  vec[2] /= dist;

  return dist;
}

float normalise2d( float vec[2] )
{
  float dist = sqrt(vec[0]*vec[0]+vec[1]*vec[1]);
  vec[0] /= dist;
  vec[1] /= dist;

  return dist;
}

float magnitude( const float vec[3] )
{
  return sqrt(magnitudeSquared(vec));
}

float magnitude2d( const float vec[2] )
{
  return sqrt(magnitudeSquared2d(vec));
}

float magnitudeSquared( const float vec[3] )
{
  return vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2];
}

float magnitudeSquared2d( const float vec[2] )
{
  return vec[0]*vec[0]+vec[1]*vec[1];
}

void biforcate ( const float lhs[3], const float rhs[3], float result[3] )
{
  float avMag = (magnitude(lhs) + magnitude(rhs)) *0.5f;
  add(lhs,rhs,result);
  normalise(result);
  multiply(result,avMag,result);
}

void biforcate2d ( const float lhs[2], const float rhs[2], float result[2] )
{
  float avMag = (magnitude2d(lhs) + magnitude2d(rhs)) *0.5f;
  add2d(lhs,rhs,result);
  normalise2d(result);
  multiply2d(result,avMag,result);
}

// Vector3--------------------------------------------------------------
Vector3::Vector3 ( float _x, float _y, float _z )
{
  vec[0] = _x;
  vec[1] = _y;
  vec[2] = _z;
}

Vector3::Vector3 ( float _vec[3] )
{
  memcpy(vec,_vec,sizeof(float)*3);
}

Vector3::Vector3 ( const Vector3 &_vec )
{
  memcpy(vec,_vec.vec,sizeof(float)*3);
}

Vector3::Vector3 ( const Vector2 &_vec )
{
  vec[0] = _vec.x();
  vec[1] = _vec.y();
  vec[2] = 0;
}

Vector3::~Vector3()
{
}

Vector3 Vector3::operator + (const Vector3 &rhs) const
{
  Vector3 temp;
  add(vec,rhs.vec,temp.vec);
  return temp;
}

Vector3& Vector3::operator += (const Vector3 &rhs)
{
  add(vec,rhs.vec,vec);
  return *this;
}

Vector3 Vector3::operator - (const Vector3 &rhs) const
{
  Vector3 temp;
  subtract(vec,rhs.vec,temp.vec);
  return temp;
}

Vector3& Vector3::operator -= (const Vector3 &rhs)
{
  subtract(vec,rhs.vec,vec);
  return *this;
}

Vector3 Vector3::operator * (const Vector3 &rhs) const
{
  Vector3 temp;
  multiply(vec,rhs.vec,temp.vec);
  return temp;
}

Vector3& Vector3::operator *= (const Vector3 &rhs)
{
  multiply(vec,rhs.vec,vec);
  return *this;
}

Vector3 Vector3::operator / (const Vector3 &rhs) const
{
  Vector3 temp;
  divide(vec,rhs.vec,temp.vec);
  return temp;
}

Vector3& Vector3::operator /= (const Vector3 &rhs)
{
  divide(vec,rhs.vec,vec);
  return *this;
}

Vector3 Vector3::operator * (const float &rhs) const
{
  Vector3 temp;
  multiply(vec,rhs,temp.vec);
  return temp;
}

Vector3& Vector3::operator *= (const float &rhs)
{
  multiply(vec,rhs,vec);
  return *this;
}

Vector3 Vector3::operator / ( const float &rhs) const
{
  Vector3 temp;
  divide(vec,rhs,temp.vec);
  return temp;
}

Vector3& Vector3::operator /= (const float &rhs) 
{
  divide(vec,rhs,vec);
  return *this;
}

Vector3& Vector3::operator = (const Vector3 &rhs)
{
  memcpy(vec,rhs.vec,sizeof(float)*3);
  return *this;
}

float Vector3::operator [] ( const unsigned int index ) const
{
  if (index > 2)
    return 0;

  return vec[index];
}

bool Vector3::operator == (const Vector3 &rhs ) const
{
  for ( int i = 0; i < 3; i++ )
  {
    if ( vec[i] != rhs.vec[i] )
      return false;
  }
  return true;
}

bool Vector3::operator != (const Vector3 &rhs ) const
{
  for ( int i = 0; i < 3; i++ )
  {
    if ( vec[i] == rhs.vec[i] )
      return false;
  }
  return true;
}

Vector3::operator const float* ( void ) const
{
  return vec;
}

float Vector3::dot ( const Vector3 &rhs )
{
  return ::dot(vec,rhs.vec);
}

float Vector3::dot ( const float rhs[3] )
{
  return ::dot(vec,rhs);
}

float Vector3::dot ( const float x, float y, float z)
{
  float t[3];
  t[0] = x; t[1] = y; t[2] = z;
  return ::dot(vec,t);
}

void Vector3::cross ( const Vector3 &lhs, const Vector3 &rhs )
{
  ::cross(lhs.vec,rhs.vec,vec);
}

void Vector3::cross ( const float lhs[], const float rhs[] )
{
  ::cross(lhs,rhs,vec);
}

float Vector3::normalise ( void )
{
  return ::normalise(vec);
}

float Vector3::magnitude ( void )
{
  return ::magnitude(vec);
}

float Vector3::magnitude2d ( void )
{
  return ::magnitude2d(vec);
}

float Vector3::magnitudeSquared ( void )
{
  return ::magnitudeSquared (vec);
}

float Vector3::magnitudeSquared2d( void )
{
  return ::magnitudeSquared2d (vec);
}

float Vector3::x ( void ) const
{
  return vec[0];
}

float Vector3::y ( void ) const
{
  return vec[1];
}

float Vector3::z ( void ) const
{
  return vec[2];
}

float Vector3::x ( float _x )
{
  vec[0] = _x;
  return vec[0];
}

float Vector3::y ( float _y ){
  vec[1] = _y;
  return vec[1];
}

float Vector3::z ( float _z ){
  vec[2] = _z;
  return vec[2];
}

bool Vector3::close( const Vector3 &v ) const
{
  for (int i = 0; i < 3; i++ )
  {
    if ( fabs(vec[i] - v.vec[i]) > REALY_SMALL )
      return false;
  }

  return true;
}

// Vector2 -----------------------------------------

Vector2::Vector2 ( float _x, float _y)
{
  vec[0] = _x;
  vec[1] = _y;
}

Vector2::Vector2 ( float _vec[2] )
{
  memcpy(vec,_vec,sizeof(float)*2);
}

Vector2::Vector2 ( const Vector2 &_vec )
{
  memcpy(vec,_vec.vec,sizeof(float)*2);
}

Vector2::Vector2 ( const Vector3 &_vec )
{
  vec[0] = _vec.x();
  vec[1] = _vec.y();
}

Vector2::~Vector2()
{
}

Vector2 Vector2::operator + (const Vector2 &rhs) const
{
  Vector2 temp;
  add2d(vec,rhs.vec,temp.vec);
  return temp;
}

Vector2& Vector2::operator += (const Vector2 &rhs)
{
  add2d(vec,rhs.vec,vec);
  return *this;
}

Vector2 Vector2::operator - (const Vector2 &rhs) const
{
  Vector2 temp;
  subtract2d(vec,rhs.vec,temp.vec);
  return temp;
}

Vector2& Vector2::operator -= (const Vector2 &rhs)
{
  subtract2d(vec,rhs.vec,vec);
  return *this;
}

Vector2 Vector2::operator * (const Vector2 &rhs) const
{
  Vector2 temp;
  multiply2d(vec,rhs.vec,temp.vec);
  return temp;
}

Vector2& Vector2::operator *= (const Vector2 &rhs)
{
  multiply2d(vec,rhs.vec,vec);
  return *this;
}

Vector2 Vector2::operator / (const Vector2 &rhs) const
{
  Vector2 temp;
  divide2d(vec,rhs.vec,temp.vec);
  return temp;
}

Vector2& Vector2::operator /= (const Vector2 &rhs)
{
  divide2d(vec,rhs.vec,vec);
  return *this;
}

Vector2 Vector2::operator * (const float &rhs) const
{
  Vector2 temp;
  multiply2d(vec,rhs,temp.vec);
  return temp;
}

Vector2& Vector2::operator *= (const float &rhs)
{
  multiply2d(vec,rhs,vec);
  return *this;
}

Vector2 Vector2::operator / ( const float &rhs) const
{
  Vector2 temp;
  divide2d(vec,rhs,temp.vec);
  return temp;
}

Vector2& Vector2::operator /= (const float &rhs) 
{
  divide2d(vec,rhs,vec);
  return *this;
}

Vector2& Vector2::operator = (const Vector2 &rhs)
{
  memcpy(vec,rhs.vec,sizeof(float)*2);
  return *this;
}

float Vector2::operator [] ( const unsigned int index ) const
{
  if (index > 2)
    return 0;

  return vec[index];
}

bool Vector2::operator == (const Vector2 &rhs ) const
{
  for ( int i = 0; i < 2; i++ )
  {
    if ( vec[i] != rhs.vec[i] )
      return false;
  }
  return true;
}

bool Vector2::operator != (const Vector2 &rhs ) const
{
  for ( int i = 0; i < 2; i++ )
  {
    if ( vec[i] == rhs.vec[i] )
      return false;
  }
  return true;
}

Vector2::operator const float* ( void ) const
{
  return vec;
}

float Vector2::dot ( const Vector2 &rhs )
{
  return ::dot2d(vec,rhs.vec);
}

float Vector2::dot ( const float rhs[2] )
{
  return ::dot2d(vec,rhs);
}

float Vector2::dot ( const float x, float y )
{
  float t[2];
  t[0] = x; t[1] = y;
  return ::dot2d(vec,t);
}

float Vector2::normalise ( void )
{
  return ::normalise2d(vec);
}

float Vector2::magnitude ( void )
{
  return ::magnitude2d(vec);
}

float Vector2::magnitudeSquared ( void )
{
  return ::magnitudeSquared2d(vec);
}


float Vector2::x ( void ) const
{
  return vec[0];
}

float Vector2::y ( void ) const
{
  return vec[1];
}

float Vector2::x ( float _x )
{
  vec[0] = _x;
  return vec[0];
}

float Vector2::y ( float _y ){
  vec[1] = _y;
  return vec[1];
}

// angle utils
float	deg2Rad = (float)M_PI/180.0f;
float	rad2Deg = 180.0f/(float)M_PI;

float toRad ( float angle )
{
  return angle * deg2Rad;
}

float toDeg ( float angle )
{
  return angle * rad2Deg;
}

Vector2 angleToVector( float angle )
{
  return Vector2(sinf(toRad(angle)),cosf(toRad(angle)));
}

float	vectorToAngle ( Vector2 &vec )
{
  return toDeg(atan2(vec.y(),vec.x()));
}


// vector math
bool pointLeftOfVectorXY( const Vector3 &sp, const Vector3 &ep, const Vector3 &point )
{
  Vector3 v1 = ep-sp;
  v1.normalise();

  Vector3 v2 = point-sp;
  v2.normalise();
  return getVectorNormalXY(v1).dot(v2) > 0;
}

Vector3 getVectorNormalXY( const Vector3 &vec )
{
  if (0)
  {
    Vector3 planeNormal(0,0,1);
    Vector3 norm;
    norm.cross(planeNormal,vec);
    norm.normalise();
    return norm;
  }
  else if (1)
    return Vector3(vec.y()*-1.0f,vec.x(),vec.z()*-1.0f);
  else if (0)
    return Vector3(angleToVector(vectorToAngle(Vector2(vec))+90));
}


bool pointInSphere ( const Vector3 &point, const Vector3 &cp, const float rad )
{
  Vector3 vec = point - cp;
  if ( vec.magnitudeSquared() > rad*rad)
    return false;
  return true;
}

bool sphereInSphere ( const Vector3 &point, const float r1, const Vector3 &cp, const float r2 )
{
  Vector3 vec = point - cp;
  if ( vec.magnitudeSquared() > r1*r1+r2*r2)
    return false;
  return true;
}

bool pointInCircle ( const Vector2 &point, const Vector2 &cp, const float rad )
{
  Vector2 vec = point - cp;
  if ( vec.magnitudeSquared() > rad*rad)
    return false;
  return true;
}

bool pointInPolygon ( const Vector2 &point, const Vector2List verts, bool ccw )
{
  if (!verts.size())
    return false;

  for ( int i = 1; i < (int)verts.size(); i++)
  {
    bool left = pointLeftOfVectorXY(verts[i-1],verts[i],point);
    if (!ccw)
      left = !left;
    if (!left)
      return false;
  }

  // check the last line that goes from the last point back to the start
  bool left = pointLeftOfVectorXY(verts[verts.size()-1],verts[0],point);
  if (!ccw)
    left = !left;
  if (!left)
    return false;

  return true;
}


// matrix stuff

Matrix34::Matrix34 ( void )
{
  flags = 0;
  identity();
  classify();
}

Matrix34::Matrix34 ( const float v[12] )
{
  flags = 0;
  identity();
  memcpy(mat,v,sizeof(float)*12);
  classify();
}

Matrix34::Matrix34 ( const Vector3 &trans )
{
  flags = 0;
  identity();
  classify();
  translate(trans);
}

Matrix34::Matrix34 (const Vector3 &d, const Vector3 &h, const Vector3 &pos)
{
  flags = 0;
  identity();

  float mag;
  Vector3 n, n2;
  Matrix34 rMatrix;

  n.x(-d.y());
  n.y(d.x());
  n.z(0.0f);

  mag = n.magnitude2d();
  if (mag < 0.000000001f)
  {
    // use an arbitrary vector normal to z...
    n.x(1.0f);
    n.y(0.0f);
  }
  else
  {
    // normalize vector...
    n.x(n.x()/ mag);
    n.y(n.y()/ mag);
  }
  rotation( n, mag, d.z());									// rotate @ n till z == d

  rMatrix.rotation( n.y(), n.x());							// rotate @ z till x == n
  *this *= rMatrix;
  rMatrix = *this;

  n2.cross(n,h);
  rotation( d, n2.dot(d), n.dot(h));							// rotate @ d till n == h
  *this *= rMatrix;

  // move matrix into position...	
  translate(pos);

  classify();
}

Matrix34& Matrix34::operator = ( const Matrix34 &rhs )
{
  flags = rhs.flags;
  memcpy(mat,rhs.mat,sizeof(float)*12);
  return *this;
}

bool Matrix34::operator == ( const Matrix34 &rhs )
{
  if (flags != rhs.flags)
    return false;

  for ( int i = 0; i < 12; i++ )
  {
    if ( mat[i] != rhs.mat[i] )
      return false;
  }
  return true;
}

bool Matrix34::operator != ( const Matrix34 &rhs )
{
  if (flags == rhs.flags)
    return false;

  for ( int i = 0; i < 12; i++ )
  {
    if ( mat[i] == rhs.mat[i] )
      return false;
  }
  return true;
}

Matrix34& Matrix34::operator *= ( const Matrix34 &rhs )
{
  Matrix34 rOutMatrix;

  /* output goes to prIn2Matrix... */

  if (rhs.flags == 0)
    return *this;

  if (flags == 0)
  {
    *this = rhs;
    return *this;
  }

  rOutMatrix.flags = rhs.flags | flags;

  if (flags & rotateFlag)
  {
    if (rhs.flags & rotateFlag)
    {
      /* both rotated... */
      rOutMatrix.mat[0][0] = mat[0][0] * rhs.mat[0][0] + mat[0][1] * rhs.mat[1][0] + mat[0][2] * rhs.mat[2][0];
      rOutMatrix.mat[1][0] = mat[1][0] * rhs.mat[0][0] + mat[1][1] * rhs.mat[1][0] + mat[1][2] * rhs.mat[2][0];
      rOutMatrix.mat[2][0] = mat[2][0] * rhs.mat[0][0] + mat[2][1] * rhs.mat[1][0] + mat[2][2] * rhs.mat[2][0];
      rOutMatrix.mat[0][1] = mat[0][0] * rhs.mat[0][1] + mat[0][1] * rhs.mat[1][1] + mat[0][2] * rhs.mat[2][1];
      rOutMatrix.mat[1][1] = mat[1][0] * rhs.mat[0][1] + mat[1][1] * rhs.mat[1][1] + mat[1][2] * rhs.mat[2][1];
      rOutMatrix.mat[2][1] = mat[2][0] * rhs.mat[0][1] + mat[2][1] * rhs.mat[1][1] + mat[2][2] * rhs.mat[2][1];
      rOutMatrix.mat[0][2] = mat[0][0] * rhs.mat[0][2] + mat[0][1] * rhs.mat[1][2] + mat[0][2] * rhs.mat[2][2];
      rOutMatrix.mat[1][2] = mat[1][0] * rhs.mat[0][2] + mat[1][1] * rhs.mat[1][2] + mat[1][2] * rhs.mat[2][2];
      rOutMatrix.mat[2][2] = mat[2][0] * rhs.mat[0][2] + mat[2][1] * rhs.mat[1][2] + mat[2][2] * rhs.mat[2][2];
    }
    else
    {
      if (rhs.flags & scaleFlag)
      {
	/* second rotated, first scaled only... */
	rOutMatrix.mat[0][0] = rhs.mat[0][0] * rhs.mat[0][0];
	rOutMatrix.mat[1][0] = rhs.mat[1][0] * rhs.mat[0][0];
	rOutMatrix.mat[2][0] = rhs.mat[2][0] * rhs.mat[0][0];
	rOutMatrix.mat[0][1] = rhs.mat[0][1] * rhs.mat[1][1];
	rOutMatrix.mat[1][1] = rhs.mat[1][1] * rhs.mat[1][1];
	rOutMatrix.mat[2][1] = rhs.mat[2][1] * rhs.mat[1][1];
	rOutMatrix.mat[0][2] = rhs.mat[0][2] * rhs.mat[2][2];
	rOutMatrix.mat[1][2] = rhs.mat[1][2] * rhs.mat[2][2];
	rOutMatrix.mat[2][2] = rhs.mat[2][2] * rhs.mat[2][2];
      }
      else
      {
	/* second rotated, first not scaled or rotated... */
	rOutMatrix.mat[0][0] = rhs.mat[0][0];
	rOutMatrix.mat[1][0] = rhs.mat[1][0];
	rOutMatrix.mat[2][0] = rhs.mat[2][0];
	rOutMatrix.mat[0][1] = rhs.mat[0][1];
	rOutMatrix.mat[1][1] = rhs.mat[1][1];
	rOutMatrix.mat[2][1] = rhs.mat[2][1];
	rOutMatrix.mat[0][2] = rhs.mat[0][2];
	rOutMatrix.mat[1][2] = rhs.mat[1][2];
	rOutMatrix.mat[2][2] = rhs.mat[2][2];
      }
    }
  }
  else
  {
    if (flags & scaleFlag)
    {
      if (rhs.flags & rotateFlag)
      {
	/* second scaled only, first rotated... */
	rOutMatrix.mat[0][0] = rhs.mat[0][0] * rhs.mat[0][0];
	rOutMatrix.mat[1][0] = rhs.mat[1][1] * rhs.mat[1][0];
	rOutMatrix.mat[2][0] = rhs.mat[2][2] * rhs.mat[2][0];
	rOutMatrix.mat[0][1] = rhs.mat[0][0] * rhs.mat[0][1];
	rOutMatrix.mat[1][1] = rhs.mat[1][1] * rhs.mat[1][1];
	rOutMatrix.mat[2][1] = rhs.mat[2][2] * rhs.mat[2][1];
	rOutMatrix.mat[0][2] = rhs.mat[0][0] * rhs.mat[0][2];
	rOutMatrix.mat[1][2] = rhs.mat[1][1] * rhs.mat[1][2];
	rOutMatrix.mat[2][2] = rhs.mat[2][2] * rhs.mat[2][2];
      }
      else
      {
	if (rhs.flags & scaleFlag)
	{
	  /* second scaled only, first scaled only... */
	  rOutMatrix.mat[0][0] = rhs.mat[0][0] * rhs.mat[0][0];
	  rOutMatrix.mat[1][1] = rhs.mat[1][1] * rhs.mat[1][1];
	  rOutMatrix.mat[2][2] = rhs.mat[2][2] * rhs.mat[2][2];
	  rOutMatrix.mat[1][0] = 0.0f;
	  rOutMatrix.mat[2][0] = 0.0f;
	  rOutMatrix.mat[0][1] = 0.0f;
	  rOutMatrix.mat[2][1] = 0.0f;
	  rOutMatrix.mat[0][2] = 0.0f;
	  rOutMatrix.mat[1][2] = 0.0f;
	}
	else
	{
	  /* second scaled only, first not scaled or rotated... */
	  rOutMatrix.mat[0][0] = rhs.mat[0][0];
	  rOutMatrix.mat[1][1] = rhs.mat[1][1];
	  rOutMatrix.mat[2][2] = rhs.mat[2][2];
	  rOutMatrix.mat[1][0] = 0.0f;
	  rOutMatrix.mat[2][0] = 0.0f;
	  rOutMatrix.mat[0][1] = 0.0f;
	  rOutMatrix.mat[2][1] = 0.0f;
	  rOutMatrix.mat[0][2] = 0.0f;
	  rOutMatrix.mat[1][2] = 0.0f;
	}
      }
    }
    else
    {
      /* second not rotated or scaled... */
      rOutMatrix.mat[0][0] = rhs.mat[0][0];
      rOutMatrix.mat[1][0] = rhs.mat[1][0];
      rOutMatrix.mat[2][0] = rhs.mat[2][0];
      rOutMatrix.mat[0][1] = rhs.mat[0][1];
      rOutMatrix.mat[1][1] = rhs.mat[1][1];
      rOutMatrix.mat[2][1] = rhs.mat[2][1];
      rOutMatrix.mat[0][2] = rhs.mat[0][2];
      rOutMatrix.mat[1][2] = rhs.mat[1][2];
      rOutMatrix.mat[2][2] = rhs.mat[2][2];
    }
  }

  if (rhs.flags & transFlag)
  {
    if (flags & rotateFlag)
    {
      rOutMatrix.mat[0][3] = rhs.mat[0][0] * rhs.mat[0][3] + rhs.mat[0][1] * rhs.mat[1][3] + rhs.mat[0][2] * rhs.mat[2][3] + rhs.mat[0][3];
      rOutMatrix.mat[1][3] = rhs.mat[1][0] * rhs.mat[0][3] + rhs.mat[1][1] * rhs.mat[1][3] + rhs.mat[1][2] * rhs.mat[2][3] + rhs.mat[1][3];
      rOutMatrix.mat[2][3] = rhs.mat[2][0] * rhs.mat[0][3] + rhs.mat[2][1] * rhs.mat[1][3] + rhs.mat[2][2] * rhs.mat[2][3] + rhs.mat[2][3];
    }
    else
    {
      if (flags & scaleFlag)
      {
	rOutMatrix.mat[0][3] = rhs.mat[0][0] * rhs.mat[0][3] + rhs.mat[0][3];
	rOutMatrix.mat[1][3] = rhs.mat[1][1] * rhs.mat[1][3] + rhs.mat[1][3];
	rOutMatrix.mat[2][3] = rhs.mat[2][2] * rhs.mat[2][3] + rhs.mat[2][3];
      }
      else
      {
	rOutMatrix.mat[0][3] = rhs.mat[0][3] + rhs.mat[0][3];
	rOutMatrix.mat[1][3] = rhs.mat[1][3] + rhs.mat[1][3];
	rOutMatrix.mat[2][3] = rhs.mat[2][3] + rhs.mat[2][3];
      }
    }
  }
  else
  {
    rOutMatrix.mat[0][3] = rhs.mat[0][3];
    rOutMatrix.mat[1][3] = rhs.mat[1][3];
    rOutMatrix.mat[2][3] = rhs.mat[2][3];
  }

  flags = rOutMatrix.flags;
  memcpy(mat,rOutMatrix.mat,sizeof(float)*12);

  return *this;
}

Matrix34& Matrix34::operator += ( const Matrix34 &rhs )
{
  short i, ii;

  for (i=0; i<3; i++)
    for (ii=0; ii<3; ii++)
      mat[i][ii] += rhs.mat[i][ii];
  return *this;
}

Matrix34& Matrix34::operator -= ( const Matrix34 &rhs )
{
  short i, ii;

  for (i=0; i<3; i++)
    for (ii=0; ii<3; ii++)
      mat[i][ii] -= rhs.mat[i][ii];
  return *this;
}


float Matrix34::operator [] ( const unsigned int index ) const
{
  if (index >= 12)
    return 0;
  return mat[index][0];
}

Matrix34::operator const float* ( void ) const
{
  return (const float*)mat;
}

float Matrix34::determinant( void ) const
{
  float xTemp;
  xTemp =  mat[0][0] * (mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2]);
  xTemp -= mat[0][1] * (mat[1][0] * mat[2][2] - mat[2][0] * mat[1][2]);
  xTemp += mat[0][2] * (mat[1][0] * mat[2][1] - mat[2][0] * mat[1][1]);
  return xTemp;
}

void Matrix34::classify ( void )
{
  float prec = LIN_TOL / 2.0f;
  float det;

  flags = 0;

  if ((fabs(mat[0][3]) >= prec) || (fabs(mat[1][3]) >= prec) || (fabs(mat[2][3]) >= prec))
    flags |= transFlag;

  // 1.0f +- 3 * accy ~= (1.0f +- accy) ** 3, so use 3 * accy...
  prec = 3.0f * LIN_TOL;
  det = determinant();
  if (!EQUAL_X(det, 1.0f, prec))
    flags |= scaleFlag;

  prec = LIN_TOL;

  if ((mat[0][0] < 1.0f - prec) || (fabs(mat[0][1]) >= prec) || (fabs(mat[0][2]) >= prec))
    flags |= rotateFlag;
  else if ((mat[1][1] < 1.0f - prec) || (fabs(mat[1][0]) >= prec) || (fabs(mat[1][2]) >= prec))
    flags |= rotateFlag;
  else if ((mat[2][2] < 1.0f - prec) || (fabs(mat[2][0]) >= prec) || (fabs(mat[2][1]) >= prec))
    flags |= rotateFlag;
}

void Matrix34::normalise ( void )
{
  const short iTimesLimit = 1;
  const short iPowerLimit = 3;
  static float coef[10] = {1.0f, -1/2., 3/8., -5/16., 35/128., -63/256., 231/1024., -429/2048., 6435/32768., -12155/65536.};
  short iTime, iPower;
  Vector3 rScale;
  Matrix34 rMatrix, rTempMatrix;
  Matrix34 X, X_power, rSumMatrix;

  /* don't pass scaling matrices in here! */
  /* for algorithm and original code, see graphics gems I, pp 464 & 765 */

  rMatrix = *this;
  rMatrix.flags = rotateFlag;			// do math with 3 x 3, no shortcuts
  rMatrix.mat[0][3] = 0.0f;
  rMatrix.mat[1][3] = 0.0f;
  rMatrix.mat[2][3] = 0.0f;

  for (iTime=0; iTime<iTimesLimit; iTime++)
  {
    rTempMatrix = rMatrix;
    rTempMatrix.transpose();
    rTempMatrix *= rMatrix;
    X = rTempMatrix;
    rTempMatrix.identity();
    X -= rTempMatrix;
    X_power.identity();
    X_power.flags = rotateFlag;
    rSumMatrix.identity();
    rSumMatrix.flags = rotateFlag;

    for (iPower=1; iPower<=iPowerLimit; iPower++)
    {
      X_power *= X;
      rTempMatrix = X_power;
      rScale.x(coef[iPower]);
      rScale.y(coef[iPower]);
      rScale.z(coef[iPower]);
      rTempMatrix.scale(rScale);
      rSumMatrix += rTempMatrix;
    }

    rMatrix *= rSumMatrix;
  }

  rMatrix.mat[0][3] = mat[0][3];
  rMatrix.mat[1][3] = mat[1][3];
  rMatrix.mat[2][3] = mat[2][3];

  flags = rMatrix.flags;
  memcpy(mat,rMatrix.mat,sizeof(float)*3);
  classify();
}

void Matrix34::identity ( void )
{
  flags = 0;
  mat[0][0] = 1.0f;
  mat[1][0] = 0.0f;
  mat[2][0] = 0.0f;
  mat[0][1] = 0.0f;
  mat[1][1] = 1.0f;
  mat[2][1] = 0.0f;
  mat[0][2] = 0.0f;
  mat[1][2] = 0.0f;
  mat[2][2] = 1.0f;
  mat[0][3] = 0.0f;
  mat[1][3] = 0.0f;
  mat[2][3] = 0.0f;
}

bool Matrix34::scaleNonUniform( void )
{
  float xScale, xTemp;

  if (~flags & scaleFlag)
    return false;

  xScale = mat[0][0] * mat[0][0] + mat[0][1] * mat[0][1] + mat[0][2] * mat[0][2];
  xTemp = mat[1][0] * mat[1][0] + mat[1][1] * mat[1][1] + mat[1][2] * mat[1][2];
  if (!EQUAL_X(xTemp, xScale, LIN_TOL))
    return true;
  xTemp = mat[2][0] * mat[2][0] + mat[2][1] * mat[2][1] + mat[2][2] * mat[2][2];
  return !EQUAL_X(xTemp, xScale, LIN_TOL);
}

bool Matrix34::scaleNonUniform2d( void )
{
  float xScale, xTemp;

  if (~flags & scaleFlag)
    return false;

  xScale = mat[0][0] * mat[0][0] + mat[0][1] * mat[0][1];
  xTemp = mat[1][0] * mat[1][0] + mat[1][1] * mat[1][1];
  return !EQUAL_X(xTemp, xScale, LIN_TOL);
}

Vector3 Matrix34::matrixScale( void ) const
{
  float xTemp;
  Vector3 temp;

  xTemp = mat[0][0] * mat[0][0] + mat[1][0] * mat[1][0] + mat[2][0] * mat[2][0];
  temp.x((float)sqrt(xTemp));
  xTemp = mat[0][1] * mat[0][1] + mat[1][1] * mat[1][1] + mat[2][1] * mat[2][1];
  temp.y ((float)sqrt(xTemp));
  xTemp = mat[0][2] * mat[0][2] + mat[1][2] * mat[1][2] + mat[2][2] * mat[2][2];
  temp.z((float)sqrt(xTemp));

  // if determinant is negative, negate all three scale values...
  if (determinant() < 0.0f)
    temp *= - 1.0f;

  return temp;
}

void Matrix34::translate( const Vector3 &vec )
{
  if ((fabs(vec.x()) > 0.00000001f) || (fabs(vec.y()) > 0.00000001f) || (fabs(vec.z()) > 0.00000001f))
  {
    if (flags & transFlag)
    {
      mat[0][3] += vec.x();		
      mat[1][3] += vec.y();		
      mat[2][3] += vec.z();	
    }
    else
    {
      mat[0][3] = vec.x();		
      mat[1][3] = vec.y();		
      mat[2][3] = vec.z();	
    }
    flags |= transFlag;
  }
}

void Matrix34::translation( const Vector3 &vec )
{
  identity();

  if ((fabs(vec.x()) > 0.00000001f) || (fabs(vec.y()) > 0.00000001f) || (fabs(vec.z()) > 0.00000001f))
  {
    flags = transFlag;
    mat[0][3] = vec.x();		
    mat[1][3] = vec.y();		
    mat[2][3] = vec.z();	
  }
}

void Matrix34::scale( const Vector3 &scale )
{
  if (flags & rotateFlag)
  {
    mat[0][0] *= scale.x();
    mat[0][1] *= scale.x();
    mat[0][2] *= scale.x();
    mat[1][0] *= scale.y();
    mat[1][1] *= scale.y();
    mat[1][2] *= scale.y();
    mat[2][0] *= scale.z();
    mat[2][1] *= scale.z();
    mat[2][2] *= scale.z();
  }
  else
  {
    if (flags & scaleFlag)
    {
      mat[0][0] *= scale.x();
      mat[1][1] *= scale.y();
      mat[2][2] *= scale.z();
    }
    else
    {
      mat[0][0] = scale.x();
      mat[1][1] = scale.y();
      mat[2][2] = scale.z();
    }
  }
  if (flags & transFlag)
  {
    mat[0][3] *= scale.x();
    mat[1][3] *= scale.y();
    mat[2][3] *= scale.z();
  }
  flags |= scaleFlag;
}

void Matrix34::scaling( const Vector3 &scale )
{
  identity();
  flags = scaleFlag;
  mat[0][0] = scale.x();
  mat[1][1] = scale.y();
  mat[2][2] = scale.z();
}

void Matrix34::rotation( const Vector3 &axis , float sin, float cosin )
{
  float xTemp, t;

  /* *axis is a unit vector, s and c are sine and cosine of angle, respectively... */

  identity();

  if ((fabs(sin) > 0.0000000001f) || (cosin < 0.0f))
  {
    t = 1.0f - cosin;
    flags = rotateFlag;
    mat[0][0] = t * axis.x() * axis.x() + cosin;
    mat[1][1] = t * axis.y() * axis.y() + cosin;
    mat[2][2] = t * axis.z() * axis.z() + cosin;
    mat[0][1] = mat[1][0] = t * axis.x() * axis.y();
    xTemp = sin * axis.z();
    mat[0][1] -= xTemp;
    mat[1][0] += xTemp;
    mat[0][2] = mat[2][0] = t * axis.x() * axis.z();
    xTemp = sin * axis.y();
    mat[0][2] += xTemp;
    mat[2][0] -= xTemp;
    mat[1][2] = mat[2][1] = t * axis.y() * axis.z();
    xTemp = sin * axis.x();
    mat[1][2] -= xTemp;
    mat[2][1] += xTemp;
  }
}
void Matrix34::rotation( const Vector3 &axis , float angle )
{
  rotation(axis,sinf(toRad(angle)),cosf(toRad(angle)));
}

void Matrix34::rotation( float sin, float cosin )
{
  identity();

  if ((fabs(sin) > 0.000000001f) || (cosin < 0.0f))
  {
    flags = rotateFlag;
    mat[0][0] = cosin;
    mat[1][1] = cosin;
    mat[0][1] = -sin;
    mat[1][0] = sin;
  }
}

void Matrix34::rotation( float angle )
{
  rotation(sinf(toRad(angle)),cosf(toRad(angle)));
}

void Matrix34::rotate2d( float angle, const Vector3 &origin )
{
  Vector3 rOrigin;
  Matrix34	rTransMatrix;

  angle = toRad(angle);

  rOrigin = origin * -1.0f;
  rOrigin.z(0);

  rTransMatrix.translation(rOrigin);
  rotation(sinf(angle),cosf(angle));

  *this *= rTransMatrix;

  rOrigin *= -1.0f;
  translate(rOrigin);
}

void Matrix34::mirror( const Vector3 &axisPolarity, const Vector3 &mirrorPoint )
{
  identity();

  flags = rotateFlag | transFlag;
  mat[0][0] = axisPolarity.x();
  mat[1][1] = axisPolarity.y();
  mat[2][2] = axisPolarity.z();
  mat[0][3] = (1.0f - axisPolarity.x()) * mirrorPoint.x();
  mat[1][3] = (1.0f - axisPolarity.y()) * mirrorPoint.y();
  mat[2][3] = (1.0f - axisPolarity.z()) * mirrorPoint.z();
}

void Matrix34::forceDepth( float depth )
{
  identity();
  flags = rotateFlag | transFlag;
  mat[2][2] = 0.0f;
  mat[2][3] = depth;
}

void Matrix34::invert( void )
{
  if (flags == 0)
    return;

  if (flags & rotateFlag)
  {
    if (flags & scaleFlag)
      invert3x3();	/* use cramer's rule... */
    else
      transpose();

    if (flags & transFlag)
    {
      float xtrans = mat[0][3];
      float ytrans = mat[1][3];
      mat[0][3] = - mat[0][0] * xtrans - mat[0][1] * ytrans - mat[0][2] * mat[2][3];
      mat[1][3] = - mat[1][0] * xtrans - mat[1][1] * ytrans - mat[1][2] * mat[2][3];
      mat[2][3] = - mat[2][0] * xtrans - mat[2][1] * ytrans - mat[2][2] * mat[2][3];
    }
  }
  else
  {
    if (flags & scaleFlag)
    {
      /* invert the scale... */
      mat[0][0] = 1.0f / mat[0][0];
      mat[1][1] = 1.0f / mat[1][1];
      mat[2][2] = 1.0f / mat[2][2];

      if (flags & transFlag)
      {
	mat[0][3] = - mat[0][0] * mat[0][3];
	mat[1][3] = - mat[1][1] * mat[1][3];
	mat[2][3] = - mat[2][2] * mat[2][3];
      }
    }
    else
    {
      if (flags & transFlag)
      {
	mat[0][3] = - mat[0][3];
	mat[1][3] = - mat[1][3];
	mat[2][3] = - mat[2][3];
      }
    }
  }
}

void Matrix34::invert3x3( void )
{
  float one_over_det;
  Matrix34 rTempMatrix;

  /* use cramer's rule to invert a 3 by 3 matrix... */

  rTempMatrix.mat[0][0] = mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2];
  rTempMatrix.mat[1][0] = mat[0][1] * mat[2][2] - mat[2][1] * mat[0][2];
  rTempMatrix.mat[2][0] = mat[0][1] * mat[1][2] - mat[1][1] * mat[0][2];
  rTempMatrix.mat[0][1] = mat[1][0] * mat[2][2] - mat[2][0] * mat[1][2];
  rTempMatrix.mat[1][1] = mat[0][0] * mat[2][2] - mat[2][0] * mat[0][2];
  rTempMatrix.mat[2][1] = mat[0][0] * mat[1][2] - mat[1][0] * mat[0][2];
  rTempMatrix.mat[0][2] = mat[1][0] * mat[2][1] - mat[2][0] * mat[1][1];
  rTempMatrix.mat[1][2] = mat[0][0] * mat[2][1] - mat[2][0] * mat[0][1];
  rTempMatrix.mat[2][2] = mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1];

  one_over_det = mat[0][0] * rTempMatrix.mat[0][0] - mat[0][1] * rTempMatrix.mat[0][1] + mat[0][2] * rTempMatrix.mat[0][2];
  one_over_det = 1.0f / one_over_det;

  mat[0][0] = rTempMatrix.mat[0][0] * one_over_det;
  mat[1][0] = - rTempMatrix.mat[0][1] * one_over_det;
  mat[2][0] = rTempMatrix.mat[0][2] * one_over_det;
  mat[0][1] = - rTempMatrix.mat[1][0] * one_over_det;
  mat[1][1] = rTempMatrix.mat[1][1] * one_over_det;
  mat[2][1] = - rTempMatrix.mat[1][2] * one_over_det;
  mat[0][2] = rTempMatrix.mat[2][0] * one_over_det;
  mat[1][2] = - rTempMatrix.mat[2][1] * one_over_det;
  mat[2][2] = rTempMatrix.mat[2][2] * one_over_det;
}

/*--------------------------------------- TransposeMatrix ------------------------------------------*/

void Matrix34::transpose( void )
{
  float xTemp;

  /* does 3 x 3 only */

  xTemp = mat[0][1];
  mat[0][1] = mat[1][0];
  mat[1][0] = xTemp;
  xTemp = mat[0][2];
  mat[0][2] = mat[2][0];
  mat[2][0] = xTemp;
  xTemp = mat[1][2];
  mat[1][2] = mat[2][1];
  mat[2][1] = xTemp;
}


Vector3& Matrix34::transformPos( Vector3 &point ) const
{
  if (flags == 0)
    return point;

  if (flags & rotateFlag)
  {
    float h = point.x();
    float v = point.y();
    point.x(mat[0][0] * h + mat[0][1] * v + mat[0][2] * point.z());
    point.y(mat[1][0] * h + mat[1][1] * v + mat[1][2] * point.z());
    point.z(mat[2][0] * h + mat[2][1] * v + mat[2][2] * point.z());
  }
  else
  {
    if (flags & scaleFlag)
    {
      point.x(point.x() * mat[0][0]);
      point.y(point.y() * mat[1][1]);
      point.z(point.z() * mat[2][2]);
    }
  }

  if (flags & transFlag)
  {
    point.x(point.x() + mat[0][3]);
    point.y(point.y() + mat[1][3]);
    point.z(point.z() + mat[2][3]);
  }

  return point;
}

Vector3& Matrix34::transformPos2d( Vector3 &point ) const
{
  if (flags == 0) 
    return point;

  if (flags & rotateFlag)
  {
    float h = point.x();
    point.x(mat[0][0] * h + mat[0][1] * point.y() + mat[0][2] * point.z());
    point.y(mat[1][0] * h + mat[1][1] * point.y() + mat[1][2] * point.z());
  }
  else
  {
    if (flags & scaleFlag)
    {
      point.x( point.x() * mat[0][0]);
      point.y( point.y() * mat[1][1]);
    }
  }

  if (flags & transFlag)
  {
    point.x(point.x() + mat[0][3]);
    point.y( point.y() + mat[1][3]);
  }

  return point;
}

Vector3& Matrix34::transformNorm( Vector3 &point ) const
{
  if (flags & rotateFlag)
  {
    float h = point.x();
    float v = point.y();
    point.x(mat[0][0] * h + mat[0][1] * v + mat[0][2] * point.z());
    point.y(mat[1][0] * h + mat[1][1] * v + mat[1][2] * point.z());
    point.z(mat[2][0] * h + mat[2][1] * v + mat[2][2] * point.z());
  }
  else
  {
    if (flags & scaleFlag)
    {
      point.x(point.x() * mat[0][0]);
      point.y(point.y() * mat[1][1]);
      point.z(point.z() * mat[2][2]);
    }
  }
  return point;
}
Vector3& Matrix34::transformNorm2d( Vector3 &point ) const
{
  if (flags & rotateFlag)
  {
    float h = point.x();
    float v = point.y();
    point.x(mat[0][0] * h + mat[0][1] * v + mat[0][2] * point.z());
    point.y(mat[1][0] * h + mat[1][1] * v + mat[1][2] * point.z());
  }
  else
  {
    if (flags & scaleFlag)
    {
      point.x(point.x() * mat[0][0]);
      point.y(point.y() * mat[1][1]);
    }
  }
  return point;
}

bool Matrix34::samePlane ( const Matrix34& matrix, bool *reversed )  const
{
  Vector3 rNorm1(0.0f, 0.0f, 1.0f);
  Vector3 rNorm2(0.0f, 0.0f, 1.0f);
  Vector3 rNorm3;

  /* returns whether the two matrices have the same planar orientation... */

  transformNorm(rNorm1);
  matrix.transformNorm(rNorm2);

  if (reversed)
    *reversed = (rNorm1.dot(rNorm2) < 0.0f);
  rNorm3.cross(rNorm1, rNorm2);
  return (rNorm3.dot(rNorm3) < LIN_TOL * LIN_TOL);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8