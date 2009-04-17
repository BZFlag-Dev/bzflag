/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Teleporter:
 *	Encapsulates a box in the game environment.
 */

#ifndef	BZF_TELEPORTER_H
#define	BZF_TELEPORTER_H

#include "common.h"
#include <string>
#include "Obstacle.h"
#include "MeshFace.h"

class Teleporter : public Obstacle {
  public:
			Teleporter();
			Teleporter(const fvec3& pos, float rotation,
			           float width, float breadth, float height,
			           float borderSize,
			           unsigned char drive, unsigned char shoot,
			           bool ricochet);
			~Teleporter();

    Obstacle*	copyWithTransform(const MeshTransform&) const;

    void		setName(const std::string& name);
    const std::string&	getName() const;

    const char*	getType() const;
    static const char*	getClassName(); // const

    float		getBorder() const;
    bool		isValid() const;

    float		intersect(const Ray&) const;
    void		getNormal(const fvec3& p, fvec3& n) const;

    bool		inCylinder(const fvec3& p, float radius, float height) const;
    bool		inBox(const fvec3& p, float angle,
			      float halfWidth, float halfBreadth, float height) const;
    bool		inMovingBox(const fvec3& oldP, float oldAngle,
				    const fvec3& newP, float newAngle,
				    float halfWidth, float halfBreadth, float height) const;
    bool		isCrossing(const fvec3& p, float angle,
				   float halfWidth, float halfBreadth, float height,
				   fvec4* plane) const;

    bool		getHitNormal(
				const fvec3& pos1, float azimuth1,
				const fvec3& pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float height,
				fvec3& normal) const;

    float		isTeleported(const Ray&, int& face) const;
    float		getProximity(const fvec3& p, float radius) const;
    bool		hasCrossed(const fvec3& p1, const fvec3& p2,
							int& face) const;
    void		getPointWRT(const Teleporter& t2, int face1, int face2,
                                    const fvec3& pIn, const fvec3* dIn, float aIn,
                                    fvec3& pOut, fvec3* dOut, float* aOut) const;

    void makeLinks();
    const MeshFace* getBackLink() const;
    const MeshFace* getFrontLink() const;

    int packSize() const;
    void *pack(void*) const;
    void *unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;
    void printOBJ(std::ostream& out, const std::string& indent) const;
    virtual int getTypeID() const {return teleType;}

    std::string		userTextures[1];

  private:
    void finalize();

  private:
    static const char*	typeName;

    std::string name;

    float border;
    fvec3 origSize;

    MeshFace* backLink;
    MeshFace* frontLink;
    fvec3 fvertices[4]; // front vertices
    fvec3 bvertices[4]; // back vertices
    fvec2 texcoords[4]; // shared texture coordinates
};

//
// Teleporter
//

inline float Teleporter::getBorder() const
{
  return border;
}

inline const MeshFace* Teleporter::getBackLink() const
{
  return backLink;
}

inline const MeshFace* Teleporter::getFrontLink() const
{
  return frontLink;
}

inline const std::string& Teleporter::getName() const
{
  return name;
}

inline void Teleporter::setName(const std::string& _name)
{
  name = _name;
  return;
}


#endif // BZF_TELEPORTER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
