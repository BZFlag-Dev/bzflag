/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "common.h"

#include <string>
#include <vector>
#include <iostream>

#include "vectors.h"


enum TransformType {
  ShiftTransform = 0,
  ScaleTransform = 1,
  ShearTransform = 2,
  SpinTransform  = 3,
  IndexTransform = 4,
  LastTransform
};

typedef struct {
  TransformType type;
  int index;
  fvec4 data;
} TransformData;

class MeshTransform {
  public:
    MeshTransform();
    ~MeshTransform();

    MeshTransform& operator=(const MeshTransform& transform);
    void append(const MeshTransform& transform);
    void prepend(const MeshTransform& transform);

    bool setName(const std::string& name);
    void addShift(const fvec3& shift);
    void addScale(const fvec3& scale);
    void addShear(const fvec3& shear);
    void addSpin(const float degrees, const fvec3& normal);
    void addReference(int transform);

    bool isEmpty() const { return (transforms.size() <= 0); }

    bool isValid();
    void finalize();

    const std::string& getName() const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;
    void printTransforms(std::ostream& out, const std::string& indent) const;

  private:

    std::string name;
    std::vector<TransformData> transforms;

  public:
    class Tool {
      public:
	Tool(const MeshTransform& transform);
	Tool(const Tool&);
	~Tool();

	bool isInverted() const;
	bool isSkewed() const; // scaled or sheared
	void modifyVertex(fvec3& vertex) const;
	void modifyNormal(fvec3& normal) const;
	void modifyOldStyle(fvec3& pos, fvec3& size,
			    float& angle, bool& flipz) const;
	const float* getMatrix() const;

	bool operator<(const Tool&) const;

      private:
	void processTransforms(const std::vector<TransformData>& transforms);

	bool empty;
	bool inverted;
	bool skewed;
	fvec4 vertexMatrix[4];
	fvec3 normalMatrix[3];
    };

  friend class MeshTransform::Tool;
};

inline bool MeshTransform::Tool::isInverted() const
{
  return inverted;
}

inline bool MeshTransform::Tool::isSkewed() const
{
  return skewed;
}

inline const float* MeshTransform::Tool::getMatrix() const
{
  return (const float*)vertexMatrix;
}


class MeshTransformManager {
  public:
    MeshTransformManager();
    ~MeshTransformManager();
    void update();
    void clear();
    int addTransform(MeshTransform* driver);
    int findTransform(const std::string& name) const;
    const MeshTransform* getTransform(int id) const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    std::vector<MeshTransform*> transforms;
};

inline const MeshTransform* MeshTransformManager::getTransform(int id) const
{
  if ((id >= 0) && (id < (int)transforms.size())) {
    return transforms[id];
  } else {
    return NULL;
  }
}


extern MeshTransformManager TRANSFORMMGR;


#endif //_TRANSFORM_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
