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

#include "TransformedShape.h"

//
// TransformedShape
//

TransformedShape::TransformedShape(Shape* adopted, const Matrix& xform_) :
								shape(adopted)
{
	setTransform(xform_);
}

TransformedShape::~TransformedShape()
{
	delete shape;
}

void					TransformedShape::setTransform(const Matrix& xform_)
{
	xform     = xform_;
	xformInv  = xform;
	xformInv.invert();
	xformT    = xform;
	xformInvT = xformInv;
	xformT.transpose();
	xformInvT.transpose();
}

void					TransformedShape::setTransform(
								const Matrix& xform_,
								const Matrix& xformInv_)
{
	xform     = xform_;
	xformInv  = xformInv_;
	xformT    = xform;
	xformInvT = xformInv;
	xformT.transpose();
	xformInvT.transpose();
}

const Matrix&			TransformedShape::getTransform() const
{
	return xform;
}

const Matrix&			TransformedShape::getTransposeTransform() const
{
	return xformT;
}

const Matrix&			TransformedShape::getInverseTransform() const
{
	return xformInv;
}

const Matrix&			TransformedShape::getInverseTransposeTransform() const
{
	return xformInvT;
}

Shape*					TransformedShape::getShape() const
{
	return shape;
}
