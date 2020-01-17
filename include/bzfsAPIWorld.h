/* bzflag
* Copyright (c) 1993-2018 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#pragma once
#include "bzfsAPI.h"

// System headers
#include <glm/vec3.hpp>

// map object API
enum class bz_eWorldObjectType
{
    NullObject,
    SolidObject,
    TeleporterField,
    WorldWeapon
};

class BZF_API bz_APIBaseWorldObject
{
public:
    bz_APIBaseWorldObject(const void* objPtr);
    virtual ~bz_APIBaseWorldObject();
    bz_eWorldObjectType type;
    bz_ApiString name;
    unsigned int id;

protected:
    class Imple;
    Imple* pImpl;
};

enum class bz_eSolidWorldObjectType
{
    Wall,
    Box,
    Base,
    Pyramid,
    Mesh,
    Arc,
    Cone,
    Sphere,
    Teleporter,
    Unknown
};

class BZF_API bz_APISolidWorldObject_V1 : public bz_APIBaseWorldObject
{
public:
    bz_APISolidWorldObject_V1(const void* objPtr);
    virtual ~bz_APISolidWorldObject_V1() {};

    bz_eSolidWorldObjectType  solidType;
    int         subID;

    float center[3];
    glm::vec3 maxAABBox;
    glm::vec3 minAABBox;
    float rotation[3];
    float maxBBox[3];
    float minBBox[3];
};

class BZF_API bz_CTFBaseWorldObject_V1 : public bz_APISolidWorldObject_V1
{
public:
    bz_CTFBaseWorldObject_V1(const void* objPtr);
    virtual ~bz_CTFBaseWorldObject_V1();

    bz_eTeamType team;
};

class BZF_API bz_APITeleporterField_V1 : public bz_APISolidWorldObject_V1
{
public:
    bz_APITeleporterField_V1(const void* objPtr) : bz_APISolidWorldObject_V1(objPtr) {}
    virtual ~bz_APITeleporterField_V1() {}

    bz_ApiString name;
    bz_APIStringList targets[2];
};

class BZF_API bz_APIWorldObjectList
{
public:
    bz_APIWorldObjectList();
    bz_APIWorldObjectList(const bz_APIWorldObjectList& r);
    ~bz_APIWorldObjectList();

    void push_back(bz_APIBaseWorldObject* value);
    bz_APIBaseWorldObject* get(unsigned int i) const;
    const bz_APIBaseWorldObject* operator[](unsigned int i) const;
    bz_APIWorldObjectList& operator = (const bz_APIWorldObjectList& r);
    unsigned int size(void) const;
    void clear(void);

protected:
    class dataBlob;
    dataBlob* data;
};

BZF_API unsigned int bz_getWorldObjectCount(void);

BZF_API bz_APIWorldObjectList* bz_getWorldObjectList(void);
BZF_API bz_APIWorldObjectList* bz_getWorldBases(void);

BZF_API void bz_releaseWorldObjectList(bz_APIWorldObjectList* list);

BZF_API bz_APIBaseWorldObject* bz_getWorldObjectByID(int id);

BZF_API void bz_releaseWorldObject(bz_APIBaseWorldObject* object);


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
