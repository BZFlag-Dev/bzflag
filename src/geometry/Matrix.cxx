/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "Matrix.h"
#include <math.h>
#include <string.h>

//
// Matrix
//

static const float		identityMatrix[16] = {
								1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f
						};

Matrix::Matrix()
{
	setIdentity();
}

Matrix::~Matrix()
{
	// do nothing
}

void					Matrix::set(const float* _matrix)
{
	memcpy(m, _matrix, sizeof(m));
}

void					Matrix::set(const Matrix& _matrix)
{
	memcpy(m, _matrix.m, sizeof(m));
}

    void			set(unsigned int, float);

void					Matrix::setIdentity()
{
	memcpy(m, identityMatrix, sizeof(m));
}

void					Matrix::setRotate(float x, float y, float z, float a)
{
	// get handy constants
	float d = 1.0f / hypotf(x, hypotf(y, z));
	x *= d;
	y *= d;
	z *= d;
	float c = cosf(a * M_PI / 180.0f);
	float s = sinf(a * M_PI / 180.0f);

	// construct matrix
	memcpy(m, identityMatrix, sizeof(m));
	m[0]  = x * x * (1 - c) + c;
	m[1]  = x * y * (1 - c) + z * s;
	m[2]  = x * z * (1 - c) - y * s;

	m[4]  = y * x * (1 - c) - z * s;
	m[5]  = y * y * (1 - c) + c;
	m[6]  = y * z * (1 - c) + x * s;

	m[8]  = z * x * (1 - c) + y * s;
	m[9]  = z * y * (1 - c) - x * s;
	m[10] = z * z * (1 - c) + c;
}

void					Matrix::setScale(float x, float y, float z)
{
	memcpy(m, identityMatrix, sizeof(m));
	m[0]  = x;
	m[5]  = y;
	m[10] = z;
}

void					Matrix::setTranslate(float x, float y, float z)
{
	memcpy(m, identityMatrix, sizeof(m));
	m[12] = x;
	m[13] = y;
	m[14] = z;
}

void					Matrix::mult(const float* x)
{
	float t[16];

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
}

void					Matrix::mult(const Matrix& matrix)
{
	mult(matrix.m);
}

void					Matrix::transpose()
{
	float tmp;
	tmp = m[1];  m[1]  = m[4];  m[4]  = tmp;
	tmp = m[2];  m[2]  = m[8];  m[8]  = tmp;
	tmp = m[3];  m[3]  = m[12]; m[12] = tmp;
	tmp = m[6];  m[6]  = m[9];  m[9]  = tmp;
	tmp = m[7];  m[7]  = m[13]; m[13] = tmp;
	tmp = m[11]; m[11] = m[14]; m[14] = tmp;
}

void					Matrix::transform3(float* b, const float* a) const
{
	float v[2];
	v[0] = a[0] * m[0]  + a[1] * m[4]  + a[2] * m[8]  + m[12];
	v[1] = a[0] * m[1]  + a[1] * m[5]  + a[2] * m[9]  + m[13];
	b[2] = a[0] * m[2]  + a[1] * m[6]  + a[2] * m[10] + m[14];
	b[0] = v[0];
	b[1] = v[1];
}

void					Matrix::transform4(float* b, const float* a) const
{
	float v[3];
	v[0] = a[0] * m[0]  + a[1] * m[4]  + a[2] * m[8]  + a[3] * m[12];
	v[1] = a[0] * m[1]  + a[1] * m[5]  + a[2] * m[9]  + a[3] * m[13];
	v[2] = a[0] * m[2]  + a[1] * m[6]  + a[2] * m[10] + a[3] * m[14];
	b[3] = a[0] * m[3]  + a[1] * m[7]  + a[2] * m[11] + a[3] * m[15];
	b[0] = v[0];
	b[1] = v[1];
	b[2] = v[2];
}

static void				getMinor(float b[3], const float a[16],
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

static float				getDeterminant3(float a[9])
{
	float	d;
	d  = a[0] * (a[4] * a[8] - a[7] * a[5]);
	d -= a[1] * (a[3] * a[8] - a[6] * a[5]);
	d += a[2] * (a[3] * a[7] - a[6] * a[4]);
	return d;
}

static float				getDeterminant(float a[16])
{
	float b[9];
	float d = 0.0f;
	for(unsigned int i = 0; i < 4; ++i) {
		if (a[4 * i] != 0.0f) {
			getMinor(b, a, i, 0);
			const float i0Determinant = getDeterminant3(b);
			const float cofactor = (i & 1) ? -i0Determinant : i0Determinant;
			d += a[4 * i] * cofactor;
		}
	}
	return d;
}

void					Matrix::inverse()
{
	float b[9];
	float minv[16];

	// get determinant
	const float determinant = getDeterminant(m);

	// bail if there's no inverse
	if (determinant == 0.0f)
		return;

	// do inverse
	const float determinantInv = 1.0f / determinant;
	for (unsigned int i = 0; i < 4; ++i) {
		for (unsigned int j = 0; j < 4; ++j) {
			getMinor(b, m, i, j);
			const float ijDeterminant = getDeterminant3(b);
			const float cofactor = ((i + j) & 1) ? -ijDeterminant : ijDeterminant;
			minv[4 * j + i] = cofactor * determinantInv;
		}
	}

	// save inverse
	memcpy(m, minv, sizeof(m));
}
