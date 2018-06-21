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

// common-interface headers
#include "global.h"

// implementation wrappers for all the bzf_ API functions
#include "bzfsAPIWorld.h"

#include "Obstacle.h"
#include "ObstacleMgr.h"
//-----------------------------
#include "BaseBuilding.h"

BZF_API unsigned int bz_getWorldObjectCount(void)
{
    return (unsigned int)(OBSTACLEMGR.getWalls().size() +
        OBSTACLEMGR.getBoxes().size() +
        OBSTACLEMGR.getPyrs().size() +
        OBSTACLEMGR.getBases().size() +
        OBSTACLEMGR.getMeshes().size() +
        OBSTACLEMGR.getArcs().size() +
        OBSTACLEMGR.getCones().size() +
        OBSTACLEMGR.getSpheres().size());
}

void setSolidObjectFromObstacle(bz_APISolidWorldObject_V1* object, const Obstacle* obstacle, bz_eSolidWorldObjectType objecType)
{
    object->solidType = objecType;
    object->id = obstacle->GetGUID();

    memcpy(object->center, obstacle->getPosition(), sizeof(float) * 3);

    object->rotation[0] = object->rotation[1] = 0;
    object->rotation[2] = obstacle->getRotation();

    const Extents& extents = obstacle->getExtents();

    memcpy(object->maxAABBox, extents.maxs, sizeof(float) * 3);
    memcpy(object->minAABBox, extents.mins, sizeof(float) * 3);
}

//-------------------------------------------------------------------------

void addObjectsToListsFromObstacleList(bz_APIWorldObjectList* solidList, const ObstacleList& list, bz_eSolidWorldObjectType objecType)
{
    for (unsigned int i = 0; i < list.size(); i++)
    {
        bz_APISolidWorldObject_V1* solid = new bz_APISolidWorldObject_V1(list[i]);
        setSolidObjectFromObstacle(solid, list[i], objecType);
        solidList->push_back(solid);
    }
}

void addCTFBasesToListsFromObstacleList(bz_APIWorldObjectList* solidList, const ObstacleList& list, bz_eSolidWorldObjectType objecType)
{
    for (unsigned int i = 0; i < list.size(); i++)
    {
        bz_CTFBaseWorldObject_V1* solid = new bz_CTFBaseWorldObject_V1(list[i]);;

        setSolidObjectFromObstacle(solid, list[i], objecType);
        const BaseBuilding* base = (const BaseBuilding*)list[i];
        solid->team = convertTeam(base->getTeam());
        solidList->push_back(solid);
    }
}

bz_APISolidWorldObject_V1* createAPIObstacle(const Obstacle* obstacle, bz_eSolidWorldObjectType objecType)
{
    bz_APISolidWorldObject_V1* obj = new bz_APISolidWorldObject_V1((void*)obstacle);
    setSolidObjectFromObstacle(obj, obstacle, objecType);
    return obj;

}

BZF_API bz_APIWorldObjectList* bz_getWorldObjectList(void)
{
    bz_APIWorldObjectList* worldList = new bz_APIWorldObjectList;

    addObjectsToListsFromObstacleList(worldList, OBSTACLEMGR.getWalls(), bz_eSolidWorldObjectType::Wall);
    addObjectsToListsFromObstacleList(worldList, OBSTACLEMGR.getBoxes(), bz_eSolidWorldObjectType::Box);
    addObjectsToListsFromObstacleList(worldList, OBSTACLEMGR.getPyrs(), bz_eSolidWorldObjectType::Pyramid);
    addCTFBasesToListsFromObstacleList(worldList, OBSTACLEMGR.getBases(), bz_eSolidWorldObjectType::Base);
    addObjectsToListsFromObstacleList(worldList, OBSTACLEMGR.getMeshes(), bz_eSolidWorldObjectType::Mesh);
    addObjectsToListsFromObstacleList(worldList, OBSTACLEMGR.getArcs(), bz_eSolidWorldObjectType::Arc);
    addObjectsToListsFromObstacleList(worldList, OBSTACLEMGR.getCones(), bz_eSolidWorldObjectType::Cone);
    addObjectsToListsFromObstacleList(worldList, OBSTACLEMGR.getSpheres(), bz_eSolidWorldObjectType::Sphere);

    return worldList;
}

class bz_APIBaseWorldObject::Imple
{
public:
    Obstacle * RootObject = nullptr;
};

bz_APIBaseWorldObject::bz_APIBaseWorldObject(void *ptr)
{
    pImpl = new Imple();
    type = bz_eWorldObjectType::NullObject;

    pImpl->RootObject = (Obstacle *)ptr;
}

bz_APIBaseWorldObject::~bz_APIBaseWorldObject()
{
    free(pImpl);
}

//-----------------------bz_APISolidWorldObject-----------------


bz_APISolidWorldObject_V1::bz_APISolidWorldObject_V1(void* objPtr) : bz_APIBaseWorldObject(objPtr)
{
    type = bz_eWorldObjectType::SolidObject;

    memset(center, 0, sizeof(float) * 3);
    memset(maxAABBox, 0, sizeof(float) * 3);
    memset(minAABBox, 0, sizeof(float) * 3);
    memset(rotation, 0, sizeof(float) * 3);
    memset(maxBBox, 0, sizeof(float) * 3);
    memset(minBBox, 0, sizeof(float) * 3);
}

bool bz_APISolidWorldObject_V1::collide(float /*pos*/[3], float /*rad*/, float* /*hit*/)
{
    return false;
}


bz_CTFBaseWorldObject_V1::bz_CTFBaseWorldObject_V1(void* objPtr) : bz_APISolidWorldObject_V1(objPtr)
{
}

bz_CTFBaseWorldObject_V1::~bz_CTFBaseWorldObject_V1()
{
}

const Obstacle* FindByID(int id, const ObstacleList& list)
{
    for (unsigned int i = 0; i < list.size(); i++)
    {
        if (list[i]->GetGUID() == id)
            return list[i];
    }
    return nullptr;
}

BZF_API bz_APIBaseWorldObject* bz_getWorldObjectByID(int id)
{
    const Obstacle* obs = FindByID(id, OBSTACLEMGR.getWalls());
    if (obs != nullptr)
        return createAPIObstacle(obs, bz_eSolidWorldObjectType::Wall);

    obs = FindByID(id, OBSTACLEMGR.getBoxes());
    if (obs != nullptr)
        return createAPIObstacle(obs, bz_eSolidWorldObjectType::Box);

    obs = FindByID(id, OBSTACLEMGR.getPyrs());
    if (obs != nullptr)
        return createAPIObstacle(obs, bz_eSolidWorldObjectType::Pyramid);

    obs = FindByID(id, OBSTACLEMGR.getBases());
    if (obs != nullptr)
        return createAPIObstacle(obs, bz_eSolidWorldObjectType::Base);

    obs = FindByID(id, OBSTACLEMGR.getArcs());
    if (obs != nullptr)
        return createAPIObstacle(obs, bz_eSolidWorldObjectType::Arc);

    obs = FindByID(id, OBSTACLEMGR.getMeshes());
    if (obs != nullptr)
        return createAPIObstacle(obs, bz_eSolidWorldObjectType::Mesh);

    obs = FindByID(id, OBSTACLEMGR.getCones());
    if (obs != nullptr)
        return createAPIObstacle(obs, bz_eSolidWorldObjectType::Cone);

    obs = FindByID(id, OBSTACLEMGR.getSpheres());
    if (obs != nullptr)
        return createAPIObstacle(obs, bz_eSolidWorldObjectType::Sphere);

    return nullptr;
}

BZF_API bz_APIWorldObjectList* bz_getWorldBases(void)
{
    return new bz_APIWorldObjectList();
}

BZF_API bz_APIWorldObjectList* bz_getWorldBases(bz_eTeamType team)
{
    return new bz_APIWorldObjectList();
}

BZF_API bz_APIWorldObjectList* bz_getWorldCustomObjects(const char* name)
{
    return new bz_APIWorldObjectList();
}

BZF_API void bz_releaseWorldObjectList(bz_APIWorldObjectList* list)
{
    if (list != nullptr)
        free(list);
}

BZF_API void bz_releaseWorldObject(bz_APIBaseWorldObject* object)
{
    if (object != nullptr)
        free(object);
}

//******************************bz_APIWorldObjectList********************************************
class bz_APIWorldObjectList::dataBlob
{
public:
    std::vector<bz_APIBaseWorldObject*> list;
};


bz_APIWorldObjectList::bz_APIWorldObjectList()
{
    data = new dataBlob;
}

bz_APIWorldObjectList::bz_APIWorldObjectList(const bz_APIWorldObjectList   &r)
{
    data = new dataBlob;
    data->list = r.data->list;
}

bz_APIWorldObjectList::~bz_APIWorldObjectList()
{
    clear();
    delete(data);
}

void bz_APIWorldObjectList::push_back(bz_APIBaseWorldObject* value)
{
    data->list.push_back(value);
}

bz_APIBaseWorldObject* bz_APIWorldObjectList::get(unsigned int i) const
{
    if (i >= data->list.size())
        return nullptr;

    return data->list[i];
}

const bz_APIBaseWorldObject* bz_APIWorldObjectList::operator[](unsigned int i) const
{
    return data->list[i];
}

bz_APIWorldObjectList& bz_APIWorldObjectList::operator=(const bz_APIWorldObjectList& r)
{
    data->list = r.data->list;
    return *this;
}

unsigned int bz_APIWorldObjectList::size(void) const
{
    return data->list.size();
}

void bz_APIWorldObjectList::clear(void)
{
    for (auto p : data->list)
        free(p);
    data->list.clear();
}







