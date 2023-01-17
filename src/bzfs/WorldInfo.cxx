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

/* interface header */
#include "WorldInfo.h"

/* system headers */
#include <ctype.h>

/* common implementation headers */
#include "global.h"
#include "Pack.h"
#include "Protocol.h"
#include "Extents.h"
#include "Intersect.h"
#include "CollisionManager.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "BzMaterial.h"
#include "PhysicsDriver.h"
#include "MeshTransform.h"
#include "MeshDrawInfo.h"
#include "TimeKeeper.h"

/* obstacle implementation headers */
#include "ObstacleMgr.h"
#include "Obstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "TetraBuilding.h"
#include "Teleporter.h"
#include "WallObstacle.h"
#include "MeshObstacle.h"
#include "ArcObstacle.h"
#include "ConeObstacle.h"
#include "SphereObstacle.h"

/* local implementation headers */
#include "FlagInfo.h"
#include "PlayerInfo.h"
#include "CustomZone.h"

/* compression library header */
#include <zlib.h>


WorldInfo::WorldInfo() :
    maxHeight(0.0f),
    database(NULL),
    databaseSize(0),
    uncompressedSize(0)
{
    size[0] = 400.0f;
    size[1] = 400.0f;
    gravity = -9.81f;
    waterLevel = -1.0f;
    waterMatRef = NULL;
    finished = false;
}

WorldInfo::~WorldInfo()
{
    delete[] database;
    links.clear();
    OBSTACLEMGR.clear();
    finished = false;
}


void WorldInfo::addWall(float x, float y, float z, float r, float w, float h)
{
    const float pos[3] = {x, y, z};
    WallObstacle* wall = new WallObstacle(pos, r, w, h, false);
    OBSTACLEMGR.addWorldObstacle(wall);
}


void WorldInfo::addLink(int src, int dst)
{
    links.addLink(src, dst);
    return;
}

void WorldInfo::addLink(const std::string& src, const std::string& dst)
{
    links.addLink(src, dst);
    return;
}


void WorldInfo::addZone(const CustomZone *zone)
{
    entryZones.addZone( zone );
}

void WorldInfo::addWeapon(const FlagType::Ptr type, const float *origin,
                          float direction, float tilt, TeamColor teamColor,
                          float initdelay, const std::vector<float> &delay,
                          TimeKeeper &sync)
{
    worldWeapons.add(type, origin, direction, tilt,
                     teamColor, initdelay, delay, sync);
}

void WorldInfo::addWaterLevel (float level, const BzMaterial* matref)
{
    waterLevel = level;
    waterMatRef = matref;
}

void WorldInfo::addBox(float x, float y, float z, float r,
                       float w, float d, float h, bool drive, bool shoot, bool rico)
{
    const float pos[3] = {x, y, z};
    BoxBuilding* box = new BoxBuilding(pos, r, w, d, h, drive, shoot, rico, false);
    OBSTACLEMGR.addWorldObstacle(box);
}

void WorldInfo::addPyramid(float x, float y, float z, float r,
                           float w, float d, float h,
                           bool drive, bool shoot, bool rico, bool flipZ)
{
    const float pos[3] = {x, y, z};
    PyramidBuilding* pyr = new PyramidBuilding(pos, r, w, d, h, drive, shoot, rico);
    if (flipZ)
        pyr->setZFlip();
    OBSTACLEMGR.addWorldObstacle(pyr);
}

void WorldInfo::addTeleporter(float x, float y, float z, float r,
                              float w, float d, float h, float b,
                              bool horizontal, bool drive, bool shoot, bool rico)
{
    const float pos[3] = {x, y, z};
    Teleporter* tele = new Teleporter(pos, r, w, d, h, b, horizontal, drive, shoot, rico);
    OBSTACLEMGR.addWorldObstacle(tele);
}

void WorldInfo::addBase(const float pos[3], float r,
                        const float _size[3], int color, bool, bool, bool rico)
{
    BaseBuilding* base = new BaseBuilding(pos, r, _size, color, rico);
    OBSTACLEMGR.addWorldObstacle(base);
}


void WorldInfo::makeWaterMaterial()
{
    // the texture matrix
    TextureMatrix* texmat = new TextureMatrix;
    texmat->setName("WaterMaterial");
    texmat->setDynamicShift(0.05f, 0.0f);
    texmat->finalize();
    int texmatIndex = TEXMATRIXMGR.addMatrix(texmat);

    // the material
    BzMaterial material;
    const float diffuse[4] = {0.65f, 1.0f, 0.5f, 0.9f};
    material.reset();
    material.setName("WaterMaterial");
    material.setTexture("water");
    material.setTextureMatrix(texmatIndex); // generate a default later
    material.setDiffuse(diffuse);
    material.setUseTextureAlpha(true); // make sure that alpha is enabled
    material.setUseColorOnTexture(false); // only use the color as a backup
    material.setUseSphereMap(false);
    material.setNoRadar(true);
    material.setNoShadow(true);
    waterMatRef = MATERIALMGR.addMaterial(&material);

    return;
}

float WorldInfo::getWaterLevel() const
{
    return waterLevel;
}

float WorldInfo::getMaxWorldHeight() const
{
    return maxHeight;
}

WorldWeapons& WorldInfo::getWorldWeapons()
{
    return worldWeapons;
}

EntryZones& WorldInfo::getEntryZones()
{
    return entryZones;
}


void            WorldInfo::loadCollisionManager()
{
    COLLISIONMGR.load();
    return;
}

void            WorldInfo::checkCollisionManager()
{
    if (COLLISIONMGR.needReload())
    {
        // reload the collision grid
        COLLISIONMGR.load();
    }
    return;
}

bool WorldInfo::rectHitCirc(float dx, float dy, const float *p, float r) const
{
    // Algorithm from Graphics Gems, pp51-53.
    const float rr = r * r, rx = -p[0], ry = -p[1];
    if (rx + dx < 0.0f) // west of rect
        if (ry + dy < 0.0f) //  sw corner
            return (rx + dx) * (rx + dx) + (ry + dy) * (ry + dy) < rr;
        else if (ry - dy > 0.0f) //  nw corner
            return (rx + dx) * (rx + dx) + (ry - dy) * (ry - dy) < rr;
        else //  due west
            return rx + dx > -r;

    else if (rx - dx > 0.0f) // east of rect
        if (ry + dy < 0.0f) //  se corner
            return (rx - dx) * (rx - dx) + (ry + dy) * (ry + dy) < rr;
        else if (ry - dy > 0.0f) //  ne corner
            return (rx - dx) * (rx - dx) + (ry - dy) * (ry - dy) < rr;
        else //  due east
            return rx - dx < r;

    else if (ry + dy < 0.0f) // due south
        return ry + dy > -r;

    else if (ry - dy > 0.0f) // due north
        return ry - dy < r;

    // circle origin in rect
    return true;
}

bool WorldInfo::inRect(const float *p1, float angle, const float *_size,
                       float x, float y, float r) const
{
    // translate origin
    float pa[2];
    pa[0] = x - p1[0];
    pa[1] = y - p1[1];

    // rotate
    float pb[2];
    const float c = cosf(-angle), s = sinf(-angle);
    pb[0] = c * pa[0] - s * pa[1];
    pb[1] = c * pa[1] + s * pa[0];

    // do test
    return rectHitCirc(_size[0], _size[1], pb, r);
}


InBuildingType WorldInfo::inCylinderNoOctree(Obstacle **location,
        float x, float y, float z,
        float radius, float height) const
{
    if (height < Epsilon)
        height = Epsilon;

    float pos[3] = {x, y, z};

    for (int type = 0; type < GroupDefinition::ObstacleTypeCount; type++)
    {
        const ObstacleList& list = OBSTACLEMGR.getWorld()->getList(type);
        for (unsigned int i = 0; i < list.size(); i++)
        {
            Obstacle* obs = list[i];
            if (obs->inCylinder(pos, radius, height))
            {
                if (location != NULL)
                    *location = obs;
                return classifyHit(obs);
            }
        }
    }

    if (location != NULL)
        *location = (Obstacle *)NULL;

    return NOT_IN_BUILDING;
}


InBuildingType WorldInfo::cylinderInBuilding(const Obstacle **location,
        const float* pos, float radius,
        float height) const
{
    if (height < Epsilon)
        height = Epsilon;

    *location = NULL;

    // check everything but walls
    const ObsList* olist = COLLISIONMGR.cylinderTest (pos, radius, height);
    for (int i = 0; i < olist->count; i++)
    {
        const Obstacle* obs = olist->list[i];
        if (obs->inCylinder(pos, radius, height))
        {
            *location = obs;
            break;
        }
    }

    return classifyHit (*location);
}


InBuildingType WorldInfo::cylinderInBuilding(const Obstacle **location,
        float x, float y, float z, float radius,
        float height) const
{
    const float pos[3] = {x, y, z};
    return cylinderInBuilding (location, pos, radius, height);
}


InBuildingType WorldInfo::boxInBuilding(const Obstacle **location,
                                        const float* pos, float angle,
                                        float width, float breadth, float height) const
{
    if (height < Epsilon)
        height = Epsilon;

    *location = NULL;

    // check everything but walls
    const ObsList* olist =
        COLLISIONMGR.boxTest (pos, angle, width, breadth, height);
    for (int i = 0; i < olist->count; i++)
    {
        const Obstacle* obs = olist->list[i];
        if (obs->inBox(pos, angle, width, breadth, height))
        {
            *location = obs;
            break;
        }
    }

    return classifyHit (*location);
}


InBuildingType WorldInfo::classifyHit (const Obstacle* obstacle) const
{
    if (obstacle == NULL)
        return NOT_IN_BUILDING;
    else if (obstacle->getType() == BoxBuilding::getClassName())
    {
        if (obstacle->isDriveThrough())
            return IN_BOX_DRIVETHROUGH;
        else
            return IN_BOX_NOTDRIVETHROUGH;
    }
    else if (obstacle->getType() == PyramidBuilding::getClassName())
        return IN_PYRAMID;
    else if (obstacle->getType() == TetraBuilding::getClassName())
        return IN_TETRA;
    else if (obstacle->getType() == MeshObstacle::getClassName())
        return IN_MESH;
    else if (obstacle->getType() == MeshFace::getClassName())
        return IN_MESHFACE;
    else if (obstacle->getType() == BaseBuilding::getClassName())
        return IN_BASE;
    else if (obstacle->getType() == Teleporter::getClassName())
        return IN_TELEPORTER;
    else
    {
        // FIXME - choke here?
        printf ("*** Unknown obstacle type in WorldInfo::classifyHit()\n");
        return IN_BASE;
    }
}

const Obstacle*     WorldInfo::hitBuilding(const float* pos, float angle, float dx, float dy, float dz) const
{
    // check walls
    const ObstacleList& walls = OBSTACLEMGR.getWalls();
    for (unsigned int w = 0; w < walls.size(); w++)
    {
        const WallObstacle* wall = (const WallObstacle*)walls[w];
        if (wall->inBox(pos, angle, dx, dy, dz))
            return wall;
    }

    // check everything else
    const ObsList* olist = COLLISIONMGR.boxTest(pos, angle, dx, dy, dz);

    for (int i = 0; i < olist->count; i++)
    {
        const Obstacle* obs = olist->list[i];
        if (!obs->isDriveThrough() && obs->inBox(pos, angle, dx, dy, dz))
            return obs;
    }

    return NULL;
}

bool WorldInfo::crossingTeleporter(const float* pos, float angle, float dx, float dy, float dz,  float* plane) const
{
    const ObstacleList& teleporters = OBSTACLEMGR.getTeles();
    for (unsigned int i = 0; i < teleporters.size(); i++)
    {
        const Teleporter* teleporter = (const Teleporter*)teleporters[i];
        if (teleporter->isCrossing(pos, angle, dx, dy, dz, plane))
            return true;
    }
    return false;
}

const Teleporter* WorldInfo::crossesTeleporter(const float* oldPos, const float* newPos, int& face) const
{
    // check teleporters
    const ObstacleList& teleporters = OBSTACLEMGR.getTeles();
    for (unsigned int i = 0; i < teleporters.size(); i++)
    {
        const Teleporter* teleporter = (const Teleporter*)teleporters[i];
        if (teleporter->hasCrossed(oldPos, newPos, face))
            return teleporter;
    }

    // didn't cross
    return NULL;
}

const Teleporter* WorldInfo::crossesTeleporter(const Ray& r, int& face) const
{
    // check teleporters
    const ObstacleList& teleporters = OBSTACLEMGR.getTeles();
    for (unsigned int i = 0; i < teleporters.size(); i++)
    {
        const Teleporter* teleporter = (const Teleporter*)teleporters[i];
        if (teleporter->isTeleported(r, face) > Epsilon)
            return teleporter;
    }

    // didn't cross
    return NULL;
}

static inline int compareHeights(const Obstacle*& obsA, const Obstacle* obsB)
{
    const Extents& eA = obsA->getExtents();
    const Extents& eB = obsB->getExtents();
    if (eA.maxs[2] > eB.maxs[2])
        return -1;
    else
        return +1;
}

static int compareObstacles(const void* a, const void* b)
{
    // - normal object come first (from lowest to highest)
    // - then come the mesh face (highest to lowest)
    // - and finally, the mesh objects (checkpoints really)
    const Obstacle* obsA = *((const Obstacle* const *)a);
    const Obstacle* obsB = *((const Obstacle* const *)b);
    const char* typeA = obsA->getType();
    const char* typeB = obsB->getType();

    bool isMeshA = (typeA == MeshObstacle::getClassName());
    bool isMeshB = (typeB == MeshObstacle::getClassName());

    if (isMeshA)
    {
        if (!isMeshB)
            return +1;
        else
            return compareHeights(obsA, obsB);
    }

    if (isMeshB)
    {
        if (!isMeshA)
            return -1;
        else
            return compareHeights(obsA, obsB);
    }

    bool isFaceA = (typeA == MeshFace::getClassName());
    bool isFaceB = (typeB == MeshFace::getClassName());

    if (isFaceA)
    {
        if (!isFaceB)
            return +1;
        else
            return compareHeights(obsA, obsB);
    }

    if (isFaceB)
    {
        if (!isFaceA)
            return -1;
        else
            return compareHeights(obsA, obsB);
    }

    return compareHeights(obsB, obsA); // reversed
}

static int compareHitNormal(const void* a, const void* b)
{
    const MeshFace* faceA = *((const MeshFace* const *)a);
    const MeshFace* faceB = *((const MeshFace* const *)b);

    // Up Planes come first
    if (faceA->isUpPlane() && !faceB->isUpPlane())
        return -1;
    if (faceB->isUpPlane() && !faceA->isUpPlane())
        return +1;

    // highest Up Plane comes first
    if (faceA->isUpPlane() && faceB->isUpPlane())
    {
        if (faceA->getPosition()[2] > faceB->getPosition()[2])
            return -1;
        else
            return +1;
    }

    // compare the dot products
    if (faceA->scratchPad < faceB->scratchPad)
        return -1;
    else
        return +1;
}

const Obstacle* WorldInfo::hitBuilding(const float* oldPos, float oldAngle, const float* pos, float angle, float dx,
                                       float dy, float dz,  bool directional) const
{
    // check walls
    const ObstacleList& walls = OBSTACLEMGR.getWalls();
    for (unsigned int w = 0; w < walls.size(); w++)
    {
        const WallObstacle* wall = (const WallObstacle*)walls[w];
        if (wall->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz))
            return wall;
    }

    // get the list of potential hits from the collision manager
    const ObsList* olist =
        COLLISIONMGR.movingBoxTest(oldPos, oldAngle, pos, angle, dx, dy, dz);

    // sort the list by type and height
    qsort(olist->list, olist->count, sizeof(Obstacle*), compareObstacles);


    int i;

    // check non-mesh obstacles
    for (i = 0; i < olist->count; i++)
    {
        const Obstacle* obs = olist->list[i];
        const char* type = obs->getType();
        if ((type == MeshFace::getClassName()) ||
                (type == MeshObstacle::getClassName()))
            break;
        if (!obs->isDriveThrough() &&
                obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz))
            return obs;
    }
    if (i == olist->count)
    {
        return NULL; // no more obstacles, we are done
    }

    // do some prep work for mesh faces
    int hitCount = 0;
    float vel[3];
    vel[0] = pos[0] - oldPos[0];
    vel[1] = pos[1] - oldPos[1];
    vel[2] = pos[2] - oldPos[2];
    bool goingDown = (vel[2] <= 0.0f);

    // check mesh faces
    for (/* do nothing */; i < olist->count; i++)
    {
        Obstacle* obs = olist->list[i];
        const char* type = obs->getType();
        if (type == MeshObstacle::getClassName())
            break;
        if (!obs->isDriveThrough() &&
                obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz))
        {
            const MeshFace* face = (const MeshFace*)obs;
            const float facePos2 = face->getPosition()[2];
            if (face->isUpPlane() &&
                    (!goingDown || (oldPos[2] < (facePos2 - 1.0e-3f))))
                continue;
            else if (face->isDownPlane() && ((oldPos[2] >= facePos2) || goingDown))
                continue;
            else
            {
                // add the face to the hitlist
                olist->list[hitCount] = obs;
                hitCount++;
                // compute its dot product and stick it in the scratchPad
                const glm::vec3 p = face->getPlane();
                const float dot = (vel[0] * p[0]) + (vel[1] * p[1]) + (vel[2] * p[2]);
                face->scratchPad = dot;
            }
        }
    }
    // sort the list by dot product (this sort will be replaced with a running tab
    qsort(olist->list, hitCount, sizeof(Obstacle*), compareHitNormal);

    // see if there as a valid meshface hit
    if (hitCount > 0)
    {
        const MeshFace* face = (const MeshFace*)olist->list[0];
        if (face->isUpPlane() || (face->scratchPad < 0.0f) || !directional)
            return face;
    }
    if (i == olist->count)
    {
        return NULL; // no more obstacles, we are done
    }

    // check mesh obstacles
    for (/* do nothing */; i < olist->count; i++)
    {
        const Obstacle* obs = olist->list[i];
        if (!obs->isDriveThrough() &&
                obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz))
            return obs;
    }

    return NULL; // no more obstacles, we are done
}

bool WorldInfo::getFlagDropPoint(const FlagInfo* fi, const float* pos,
                                 float* pt) const
{
    FlagType::Ptr flagType = fi->flag.type;
    const int team = (int)flagType->flagTeam;
    const bool teamFlag = (team != NoTeam);

    if (teamFlag)
    {
        const std::string& safetyQual =
            CustomZone::getFlagSafetyQualifier(team);
        if (entryZones.getClosePoint(safetyQual, pos, pt))
            return true;
    }
    else
    {
        const std::string& idQual =
            CustomZone::getFlagIdQualifier(fi->getIndex());
        if (entryZones.getClosePoint(idQual, pos, pt))
            return true;
        const std::string& typeQual =
            CustomZone::getFlagTypeQualifier(flagType);
        if (entryZones.getClosePoint(typeQual, pos, pt))
            return true;
    }
    return false;
}


bool WorldInfo::getFlagSpawnPoint(const FlagInfo* fi, float* pt) const
{
    FlagType::Ptr flagType = fi->flag.type;
    const int team = (int)flagType->flagTeam;
    const bool teamFlag = (team != NoTeam);

    const std::string& idQual =
        CustomZone::getFlagIdQualifier(fi->getIndex());
    if (entryZones.getRandomPoint(idQual, pt))
        return true;

    if (!teamFlag)
    {
        const std::string& typeQual =
            CustomZone::getFlagTypeQualifier(flagType);
        if (entryZones.getRandomPoint(typeQual, pt))
            return true;
    }
    return false;
}


bool WorldInfo::getPlayerSpawnPoint(const PlayerInfo* pi, float* pt) const
{
    const std::string& teamQual =
        CustomZone::getPlayerTeamQualifier((int)pi->getTeam());
    if (entryZones.getRandomPoint(teamQual, pt))
        return true;
    return false;
}


const Obstacle*     WorldInfo::inBuilding(const float* pos, float radius, float height) const
{
    // check everything but walls
    const ObsList* olist = COLLISIONMGR.cylinderTest(pos, radius, height);
    for (int i = 0; i < olist->count; i++)
    {
        const Obstacle* obs = olist->list[i];
        if (obs->inCylinder(pos, radius, height))
            return obs;
    }

    return NULL;
}

const Obstacle*     WorldInfo::inBuilding(const float* pos, float angle, float dx, float dy, float dz) const
{
    // check everything but the walls
    const ObsList* olist = COLLISIONMGR.boxTest(pos, angle, dx, dy, dz);

    for (int i = 0; i < olist->count; i++)
    {
        const Obstacle* obs = olist->list[i];
        if (obs->inBox(pos, angle, dx, dy, dz))
            return obs;
    }

    return NULL;
}

void WorldInfo::finishWorld()
{
    entryZones.calculateQualifierLists();

    loadCollisionManager();

    links.doLinking();

    maxHeight = COLLISIONMGR.getWorldExtents().maxs[2];
    const float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
    if (maxHeight < wallHeight)
        maxHeight = wallHeight;
    if (maxHeight < 0.0f)
        maxHeight = 0.0f;

    finished = true;

    return;
}


int WorldInfo::packDatabase()
{
    // make default water material. we wait to make the default material
    // to avoid messing up any user indexing. this has to be done before
    // the texture matrices and materials are packed.
    if ((waterLevel >= 0.0f) && (waterMatRef == NULL))
        makeWaterMaterial();

    // compute the database size
    databaseSize =
        DYNCOLORMGR.packSize() + TEXMATRIXMGR.packSize() +
        MATERIALMGR.packSize() + PHYDRVMGR.packSize() +
        TRANSFORMMGR.packSize() + OBSTACLEMGR.packSize() + links.packSize() +
        worldWeapons.packSize() + entryZones.packSize();
    // add water level size
    databaseSize += sizeof(float);
    if (waterLevel >= 0.0f)
        databaseSize += sizeof(int32_t);


    // allocate the buffer
    database = new char[databaseSize];
    void *databasePtr = database;


    // pack dynamic colors
    databasePtr = DYNCOLORMGR.pack(databasePtr);

    // pack texture matrices
    databasePtr = TEXMATRIXMGR.pack(databasePtr);

    // pack materials
    databasePtr = MATERIALMGR.pack(databasePtr);

    // pack physics drivers
    databasePtr = PHYDRVMGR.pack(databasePtr);

    // pack obstacle transforms
    databasePtr = TRANSFORMMGR.pack(databasePtr);

    // pack obstacles
    databasePtr = OBSTACLEMGR.pack(databasePtr);

    // pack teleporter links
    databasePtr = links.pack(databasePtr);

    // pack water level
    databasePtr = nboPackFloat(databasePtr, waterLevel);
    if (waterLevel >= 0.0f)
    {
        int matindex = MATERIALMGR.getIndex(waterMatRef);
        databasePtr = nboPackInt(databasePtr, matindex);
    }

    // pack weapons
    databasePtr = worldWeapons.pack(databasePtr);

    // pack entry zones
    databasePtr = entryZones.pack(databasePtr);


    // compress the map database
    TimeKeeper startTime = TimeKeeper::getCurrent();
    uLongf gzDBlen = databaseSize + (databaseSize/512) + 12;
    char* gzDB = new char[gzDBlen];
    int code = compress2 ((Bytef*)gzDB, &gzDBlen,
                          (Bytef*)database, databaseSize, 9);
    if (code != Z_OK)
    {
        printf ("Could not create compressed world database: %i\n", code);
        exit (1);
    }
    TimeKeeper endTime = TimeKeeper::getCurrent();

    // switch to the compressed map database
    uncompressedSize = databaseSize;
    databaseSize = gzDBlen;
    char *oldDB = database;
    database = gzDB;
    delete[] oldDB;

    logDebugMessage(1,"Map size: uncompressed = %i, compressed = %i\n",
                    uncompressedSize, databaseSize);

    logDebugMessage(3,"Compression: %.3f seconds\n", endTime - startTime);

    return 1;
}

void *WorldInfo::getDatabase() const
{
    return database;
}

int WorldInfo::getDatabaseSize() const
{
    return databaseSize;
}

int WorldInfo::getUncompressedSize() const
{
    return uncompressedSize;
}

int WorldInfo::getTeleportTarget(int source) const
{
    return links.getTeleportTarget(source);
}


int WorldInfo::getTeleportTarget(int source, unsigned int seed) const
{
    return links.getTeleportTarget(source, seed);
}


bool WorldInfo::getGround(const Ray& r, float min, float &t)
{
    if (r.getDirection()[2] >= 0.0f)
        return false;

    float groundT = r.getOrigin()[2] / -r.getDirection()[2];
    if ((groundT > min) && (groundT < t))
    {
        t = groundT;
        return true;
    }
    return false;
}

const Obstacle* WorldInfo::getFirstBuilding(const Ray& ray, float min, float& t)
{
    const Obstacle* closestObstacle = NULL;
    unsigned int i = 0;

    // check walls
    const ObstacleList& walls = OBSTACLEMGR.getWalls();
    for (i = 0; i < walls.size(); i++)
    {
        const WallObstacle* wall = (const WallObstacle*)walls[i];
        if (!wall->isShootThrough())
        {
            const float wallt = wall->intersect(ray);
            if (wallt > min && wallt < t)
            {
                t = wallt;
                closestObstacle = wall;
            }
        }
    }

    //check everything else
    const ObsList* olist = COLLISIONMGR.rayTest(&ray, t);

    for (i = 0; i < (unsigned int)olist->count; i++)
    {
        const Obstacle* obs = olist->list[i];
        if (!obs->isShootThrough())
        {
            const float timet = obs->intersect(ray);
            if (obs->getType() == Teleporter::getClassName())
            {
                const Teleporter* tele = (const Teleporter*)obs;
                int face;
                if ((timet > min) && (timet < t) &&
                        (tele->isTeleported(ray, face) < 0.0f))
                {
                    t = timet;
                    closestObstacle = obs;
                }
            }
            else
            {
                if ((timet > min) && (timet < t))
                {
                    t = timet;
                    closestObstacle = obs;
                }
            }
        }
    }

    return closestObstacle;
}

const Teleporter* WorldInfo::getFirstTeleporter(const Ray& ray, float min, float& t, int& f)
{
    const Teleporter* closestTeleporter = NULL;
    int face;

    const ObstacleList& teles = OBSTACLEMGR.getTeles();

    for (unsigned int i = 0; i < teles.size(); i++)
    {
        const Teleporter& tele = *((const Teleporter*)teles[i]);
        const float telet = tele.isTeleported(ray, face);
        if (telet > min && telet < t)
        {
            t = telet;
            f = face;
            closestTeleporter = &tele;
        }
    }

    return closestTeleporter;
}

int WorldInfo::getTeleporter(const Teleporter* teleporter, int face)
{
    // search for teleporter
    const ObstacleList& teleporters = OBSTACLEMGR.getTeles();
    const int count = teleporters.size();
    for (int i = 0; i < count; i++)
    {
        if (teleporter == (const Teleporter*)teleporters[i])
            return ((2 * i) + face);
    }

    return 0;
}


const Teleporter* WorldInfo::getTeleporter(int source, int& face)
{
    const ObstacleList& teleporters = OBSTACLEMGR.getTeles();
    if (source >= 0 && source < (int)(2 * teleporters.size()))
    {
        face = (source & 1);
        return ((const Teleporter*)teleporters[source / 2]);
    }
    return nullptr;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
