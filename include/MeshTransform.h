/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_


#include <string>
#include <vector>
#include <iostream>


class MeshTransform {
  public:
    MeshTransform();
    ~MeshTransform();

    MeshTransform& operator=(const MeshTransform& transform);
    void append(const MeshTransform& transform);
    void prepend(const MeshTransform& transform);

    bool setName(const std::string& name);
    void addShift(const float shift[3]);
    void addScale(const float scale[3]);
    void addShear(const float shear[3]);
    void addSpin(const float degrees, const float normal[3]);
    void addReference(int transform);

    bool isValid();
    void finalize();

    const std::string& getName() const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;
    void printTransforms(std::ostream& out, const std::string& indent) const;

  public:
    static void setGlobalTransform(const MeshTransform& transform);
    static void resetGlobalTransform();
  
  private:
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
      float data[4];
    } TransformData;

    std::string name;
    std::vector<TransformData> transforms;
    
  private:
    static MeshTransform globalTransform;
    
  public:
    class Tool {
      public:
        Tool(const MeshTransform& transform);
        ~Tool();
        
        bool getIsInverted() const;
        void modifyVertex(float vertex[3]) const;
        void modifyNormal(float normal[3]) const;

      private:
        void processTransforms(const std::vector<TransformData>& transforms);
      
        bool inverted;
        float vertexMatrix[4][4];
        float normalMatrix[3][3];
    };
  friend class MeshTransform::Tool;
};

inline bool MeshTransform::Tool::getIsInverted() const
{
  return inverted;
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
