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

#ifndef BZF_TRANSFORMED_SHAPE_H
#define BZF_TRANSFORMED_SHAPE_H

#include "TransformableShape.h"

class Matrix;

class TransformedShape : public TransformableShape {
public:
	TransformedShape(Shape* adopted, const Matrix& xform_);
	virtual ~TransformedShape();

	// set the transform.  if the inverse is supplied it must be
	// the inverse of the transformation.
	void				setTransform(const Matrix&);
	void				setTransform(const Matrix& xform,
								const Matrix& inverse);

	// TransformableShape overrides
	virtual const Matrix&	getTransform() const;
	virtual const Matrix&	getTransposeTransform() const;
	virtual const Matrix&	getInverseTransform() const;
	virtual const Matrix&	getInverseTransposeTransform() const;

protected:
	virtual Shape*		getShape() const;

private:
	Shape*				shape;
	Matrix				xform;		// local-to-world for points
	Matrix				xformInv;	// world-to-local for points
	Matrix				xformInvT;	// local-to-world for normals
	Matrix				xformT;		// world-to-local for normals
};

#endif
