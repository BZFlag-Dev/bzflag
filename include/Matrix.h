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

#ifndef BZF_MATRIX_H
#define BZF_MATRIX_H

class Matrix {
public:
	Matrix();
	~Matrix();

	void				set(const float*);
	void				set(const Matrix&);
	void				setIdentity();
	void				setRotate(float x, float y, float z, float a);
	void				setScale(float x, float y, float z);
	void				setTranslate(float x, float y, float z);

	void				mult(const float*);
	void				mult(const Matrix&);
	void				transpose();
	void				inverse();

	// transform3 is like transform4 except assuming inVec[3] would be 1
	void				transform3(float* outVec3, const float* inVec3) const;
	void				transform4(float* outVec4, const float* inVec4) const;

	float*				get() { return m; }
	const float*		get() const
							{ return m; }

	float&				operator[](unsigned int index)
							{ return m[index]; }
	float				operator[](unsigned int index) const
							{ return m[index]; }

private:
	float				m[16];
};

#endif
