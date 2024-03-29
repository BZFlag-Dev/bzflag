/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* ViewFrustum
 *  Encapsulates a camera.
 */

#ifndef BZF_VIEW_FRUSTUM_H
#define BZF_VIEW_FRUSTUM_H

// inherits from
#include "Frustum.h"

// Common headers
#include "bzfgl.h"

// FIXME -- will need a means for off center projections for
//  looking through teleporters

class ViewFrustum : public Frustum
{
public:
    ViewFrustum();
    ~ViewFrustum();
    void        executeProjection() const;
    void        executeDeepProjection() const;
    void        executeView() const;
    void        executeOrientation() const;
    void        executePosition() const;
    void        executeBillboard() const;
};

#endif // BZF_VIEW_FRUSTUM_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
