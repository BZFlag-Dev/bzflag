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

#include "math3D.h"
#include <string.h>

//
// Vec3
//

Vec3&					Vec3::xformPoint(const Matrix& m)
{
	const Real* mv = m.get();
	Real x = v[0] * mv[0] + v[1] * mv[4] + v[2] * mv[8]  + m[12];
	Real y = v[0] * mv[1] + v[1] * mv[5] + v[2] * mv[9]  + m[13];
	v[2]   = v[0] * mv[2] + v[1] * mv[6] + v[2] * mv[10] + m[14];
	v[0]   = x;
	v[1]   = y;
	return *this;
}

Vec3&					Vec3::xformPoint(const Matrix& m, Real w_)
{
	const Real* mv = m.get();
	Real x = v[0] * mv[0]  + v[1] * mv[4]  + v[2] * mv[8]  + w_ * mv[12];
	Real y = v[0] * mv[1]  + v[1] * mv[5]  + v[2] * mv[9]  + w_ * mv[13];
	Real z = v[0] * mv[2]  + v[1] * mv[6]  + v[2] * mv[10] + w_ * mv[14];
	Real w = v[0] * mv[3]  + v[1] * mv[7]  + v[2] * mv[11] + w_ * mv[15];
	w      = 1.0f / w;
	v[0]   = x * w;
	v[1]   = y * w;
	v[2]   = z * w;
	return *this;
}


//
// matrix utilities
//

static void				getMinor4(Real b[9], const Real a[16],
								unsigned int i, unsigned int j)
{
	for (unsigned int k = 0, l = 0; k < 3; ++k, ++l) {
		if (i == l)
			l++;
		for(unsigned int n = 0, m = 0; n < 3; ++n, ++m) {
			if (j == m)
				m++;
			b[3 * k + n] = a[4 * l + m];
		}
	}
}

static Real				getDeterminant3(Real a[9])
{
	Real d;
	d  = a[0] * (a[4] * a[8] - a[7] * a[5]);
	d -= a[1] * (a[3] * a[8] - a[6] * a[5]);
	d += a[2] * (a[3] * a[7] - a[6] * a[4]);
	return d;
}

static Real				getDeterminant4(Real a[16])
{
	Real b[9];
	Real d = R_(0.0);
	for(unsigned int i = 0; i < 4; ++i) {
		if (a[4 * i + 3] != R_(0.0)) {
			getMinor4(b, a, i, 3);
			const Real i0Determinant = getDeterminant3(b);
			const Real cofactor = (i & 1) ? i0Determinant : -i0Determinant;
			d += a[4 * i + 3] * cofactor;
		}
	}
	return d;
}

/*
static Real				getDeterminant2(Real a[4])
{
	return a[0] * a[3] - a[1] * a[2];
}

static void				getMinor3(Real b[4], const Real a[9],
								unsigned int i, unsigned int j)
{
	for (unsigned int k = 0, l = 0; k < 2; ++k, ++l) {
		if (i == l)
			l++;
		for(unsigned int n = 0, m = 0; n < 2; ++n, ++m) {
			if (j == m)
				m++;
			b[2 * k + n] = a[3 * l + m];
		}
	}
}
*/


//
// Matrix
//

static const Real		zeroMatrix4x4[16] = {
								R_(0.0), R_(0.0), R_(0.0), R_(0.0),
								R_(0.0), R_(0.0), R_(0.0), R_(0.0),
								R_(0.0), R_(0.0), R_(0.0), R_(0.0),
								R_(0.0), R_(0.0), R_(0.0), R_(0.0)
						};
static const Real		identityMatrix4x4[16] = {
								R_(1.0), R_(0.0), R_(0.0), R_(0.0),
								R_(0.0), R_(1.0), R_(0.0), R_(0.0),
								R_(0.0), R_(0.0), R_(1.0), R_(0.0),
								R_(0.0), R_(0.0), R_(0.0), R_(1.0)
						};

Matrix&					Matrix::operator*=(const Matrix& n)
{
	Real t[16];
	const Real* x = n.get();

	t[0]  = m[0]  * x[0]  + m[4]  * x[1]  + m[8]  * x[2]  + m[12] * x[3];
	t[4]  = m[0]  * x[4]  + m[4]  * x[5]  + m[8]  * x[6]  + m[12] * x[7];
	t[8]  = m[0]  * x[8]  + m[4]  * x[9]  + m[8]  * x[10] + m[12] * x[11];
	t[12] = m[0]  * x[12] + m[4]  * x[13] + m[8]  * x[14] + m[12] * x[15];

	t[1]  = m[1]  * x[0]  + m[5]  * x[1]  + m[9]  * x[2]  + m[13] * x[3];
	t[5]  = m[1]  * x[4]  + m[5]  * x[5]  + m[9]  * x[6]  + m[13] * x[7];
	t[9]  = m[1]  * x[8]  + m[5]  * x[9]  + m[9]  * x[10] + m[13] * x[11];
	t[13] = m[1]  * x[12] + m[5]  * x[13] + m[9]  * x[14] + m[13] * x[15];

	t[2]  = m[2]  * x[0]  + m[6]  * x[1]  + m[10] * x[2]  + m[14] * x[3];
	t[6]  = m[2]  * x[4]  + m[6]  * x[5]  + m[10] * x[6]  + m[14] * x[7];
	t[10] = m[2]  * x[8]  + m[6]  * x[9]  + m[10] * x[10] + m[14] * x[11];
	t[14] = m[2]  * x[12] + m[6]  * x[13] + m[10] * x[14] + m[14] * x[15];

	t[3]  = m[3]  * x[0]  + m[7]  * x[1]  + m[11] * x[2]  + m[15] * x[3];
	t[7]  = m[3]  * x[4]  + m[7]  * x[5]  + m[11] * x[6]  + m[15] * x[7];
	t[11] = m[3]  * x[8]  + m[7]  * x[9]  + m[11] * x[10] + m[15] * x[11];
	t[15] = m[3]  * x[12] + m[7]  * x[13] + m[11] * x[14] + m[15] * x[15];

	memcpy(m, t, sizeof(m));
	return *this;
}

Matrix&					Matrix::operator*=(Real x)
{
	m[0]  *= x;
	m[1]  *= x;
	m[2]  *= x;
	m[3]  *= x;
	m[4]  *= x;
	m[5]  *= x;
	m[6]  *= x;
	m[7]  *= x;
	m[8]  *= x;
	m[9]  *= x;
	m[10] *= x;
	m[11] *= x;
	m[12] *= x;
	m[13] *= x;
	m[14] *= x;
	m[15] *= x;
	return *this;
}

void					Matrix::xformPoint(float* d, const float* s) const
{
	float x = static_cast<float>(s[0] * m[0]  + s[1] * m[4]  + s[2] * m[8]  + s[3] * m[12]);
	float y = static_cast<float>(s[0] * m[1]  + s[1] * m[5]  + s[2] * m[9]  + s[3] * m[13]);
	float z = static_cast<float>(s[0] * m[2]  + s[1] * m[6]  + s[2] * m[10] + s[3] * m[14]);
	d[3]    = static_cast<float>(s[0] * m[3]  + s[1] * m[7]  + s[2] * m[11] + s[3] * m[15]);
	d[0]    = x;
	d[1]    = y;
	d[2]    = z;
}

void					Matrix::xformPoint(double* d, const double* s) const
{
	double x = static_cast<double>(s[0] * m[0]  + s[1] * m[4]  + s[2] * m[8]  + s[3] * m[12]);
	double y = static_cast<double>(s[0] * m[1]  + s[1] * m[5]  + s[2] * m[9]  + s[3] * m[13]);
	double z = static_cast<double>(s[0] * m[2]  + s[1] * m[6]  + s[2] * m[10] + s[3] * m[14]);
	d[3]     = static_cast<double>(s[0] * m[3]  + s[1] * m[7]  + s[2] * m[11] + s[3] * m[15]);
	d[0]     = x;
	d[1]     = y;
	d[2]     = z;
}

Matrix&					Matrix::transpose()
{
	Real tmp;
	tmp = m[1];  m[1]  = m[4];  m[4]  = tmp;
	tmp = m[2];  m[2]  = m[8];  m[8]  = tmp;
	tmp = m[3];  m[3]  = m[12]; m[12] = tmp;
	tmp = m[6];  m[6]  = m[9];  m[9]  = tmp;
	tmp = m[7];  m[7]  = m[13]; m[13] = tmp;
	tmp = m[11]; m[11] = m[14]; m[14] = tmp;
	return *this;
}

Matrix&					Matrix::zero()
{
	memcpy(m, zeroMatrix4x4, sizeof(m));
	return *this;
}

Matrix&					Matrix::identity()
{
	memcpy(m, identityMatrix4x4, sizeof(m));
	return *this;
}

Matrix&					Matrix::setRotate(Real x, Real y, Real z, Real a)
{
	identity();

	// get handy constants
	Real d = R_(1.0) / sqrtr(x * x + y * y + z * z);
	x *= d;
	y *= d;
	z *= d;
	Real c = cosr(deg2rad(a));
	Real s = sinr(deg2rad(a));

	// construct matrix
	m[0]  = x * x * (R_(1.0) - c) + c;
	m[1]  = x * y * (R_(1.0) - c) + z * s;
	m[2]  = x * z * (R_(1.0) - c) - y * s;

	m[4]  = y * x * (R_(1.0) - c) - z * s;
	m[5]  = y * y * (R_(1.0) - c) + c;
	m[6]  = y * z * (R_(1.0) - c) + x * s;

	m[8]  = z * x * (R_(1.0) - c) + y * s;
	m[9]  = z * y * (R_(1.0) - c) - x * s;
	m[10] = z * z * (R_(1.0) - c) + c;

	return *this;
}

Matrix&					Matrix::invert()
{
	Real b[9];
	Real minv[16];

	// get determinant
	const Real determinant = getDeterminant4(m);

	// bail if there's no inverse
	if (determinant == R_(0.0)) {
		// FIXME -- throw?  maybe use NaN.
		return *this;
	}

	// do inverse
	const Real determinantInv = R_(1.0) / determinant;
	for (unsigned int i = 0; i < 4; ++i) {
		for (unsigned int j = 0; j < 4; ++j) {
			getMinor4(b, m, i, j);
			const Real ijDeterminant = getDeterminant3(b);
			const Real cofactor = ((i + j) & 1) ?
										-ijDeterminant : ijDeterminant;
			minv[4 * j + i] = cofactor * determinantInv;
		}
	}

	// save inverse
	memcpy(m, minv, sizeof(m));
	return *this;
}

float*					Matrix::get(float* buffer) const
{
	buffer[0]  = static_cast<float>(m[0]);
	buffer[1]  = static_cast<float>(m[1]);
	buffer[2]  = static_cast<float>(m[2]);
	buffer[3]  = static_cast<float>(m[3]);
	buffer[4]  = static_cast<float>(m[4]);
	buffer[5]  = static_cast<float>(m[5]);
	buffer[6]  = static_cast<float>(m[6]);
	buffer[7]  = static_cast<float>(m[7]);
	buffer[8]  = static_cast<float>(m[8]);
	buffer[9]  = static_cast<float>(m[9]);
	buffer[10] = static_cast<float>(m[10]);
	buffer[11] = static_cast<float>(m[11]);
	buffer[12] = static_cast<float>(m[12]);
	buffer[13] = static_cast<float>(m[13]);
	buffer[14] = static_cast<float>(m[14]);
	buffer[15] = static_cast<float>(m[15]);
	return buffer;
}

double*					Matrix::get(double* buffer) const
{
	buffer[0]  = static_cast<double>(m[0]);
	buffer[1]  = static_cast<double>(m[1]);
	buffer[2]  = static_cast<double>(m[2]);
	buffer[3]  = static_cast<double>(m[3]);
	buffer[4]  = static_cast<double>(m[4]);
	buffer[5]  = static_cast<double>(m[5]);
	buffer[6]  = static_cast<double>(m[6]);
	buffer[7]  = static_cast<double>(m[7]);
	buffer[8]  = static_cast<double>(m[8]);
	buffer[9]  = static_cast<double>(m[9]);
	buffer[10] = static_cast<double>(m[10]);
	buffer[11] = static_cast<double>(m[11]);
	buffer[12] = static_cast<double>(m[12]);
	buffer[13] = static_cast<double>(m[13]);
	buffer[14] = static_cast<double>(m[14]);
	buffer[15] = static_cast<double>(m[15]);
	return buffer;
}


//
// Quaternion
//

Quaternion::Quaternion(const Vec3& v_, Real angle)
{
	const Real* vv = v_.get();
	const Real len = sinr(R_(0.5) * deg2rad(angle)) / v_.length();
	v[0] = cosf(R_(0.5) * deg2rad(angle));
	v[1] = len * vv[0];
	v[2] = len * vv[1];
	v[3] = len * vv[2];
}

Quaternion&				Quaternion::operator*=(const Quaternion& q)
{
	Real t[3];
	t[0] = v[0] * q.v[0] -   v[1] * q.v[1] - v[2] * q.v[2] - v[3] * q.v[3];
	t[1] = v[0] * q.v[1] + q.v[0] *   v[1] + v[2] * q.v[3] - v[3] * q.v[2];
	t[2] = v[0] * q.v[2] + q.v[0] *   v[2] + v[3] * q.v[1] - v[1] * q.v[3];
	v[3] = v[0] * q.v[3] + q.v[0] *   v[3] + v[1] * q.v[2] - v[2] * q.v[1];
	v[0] = t[0];
	v[1] = t[1];
	v[2] = t[2];
	return *this;
}

Quaternion&				Quaternion::zero()
{
	v[0] = v[1] = v[2] = v[3] = R_(0.0);
	return *this;
}

Quaternion&				Quaternion::identity()
{
	v[0] = R_(1.0);
	v[1] = v[2] = v[3] = R_(0.0);
	return *this;
}
// ex: shiftwidth=4 tabstop=4
