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


// BZFlag common header
#include "common.h"

// System headers
#include <math.h>
#include <list>

// Interface header
#include "TrackMarks.h"

// Common interface headers
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "CollisionManager.h"
#include "PhysicsDriver.h"
#include "Ray.h"
#include "bzfgl.h"
#include "TextureManager.h"
#include "SceneDatabase.h"
#include "SceneRenderer.h"
#include "SceneNode.h"
#include "OpenGLGState.h"
#include "OpenGLMaterial.h"


using namespace TrackMarks;


enum TrackType {
  TreadsTrack = 0,
  PuddleTrack = 1,
  SmokeTrack  = 2
};

enum TrackSides {
  LeftTread =  (1 << 0),
  RightTread = (1 << 1),
  BothTreads = (LeftTread | RightTread)
};


class TrackEntry {
  public:
    ~TrackEntry();
      
  public:
    float pos[3];
    float angle;
    float scale;
    char sides;
    int phydrv;
    float lifeTime;
    class TrackSceneNode* sceneNode;
};

class TrackRenderNode : public RenderNode {
  public:
    TrackRenderNode(const TrackEntry* te, TrackType type);
    ~TrackRenderNode();
    void render();
    void renderShadow() { return; }
    const GLfloat* getPosition() const { return te->pos; }

  private:
    TrackType type;
    const TrackEntry* te;
};

class TrackSceneNode : public SceneNode {
  public:
    TrackSceneNode(const TrackEntry*, TrackType, const OpenGLGState*);
    ~TrackSceneNode();
    bool isTranslucent() const { return false; }
    void addRenderNodes(SceneRenderer&);
    void update(); // set the sphere properties

  private:
    TrackType type;
    const TrackEntry* te;
    const OpenGLGState* gstate;
    TrackRenderNode renderNode;
};


// Local Variables
//////////////////

static std::list<TrackEntry> SmokeList;
static std::list<TrackEntry> PuddleList;
static std::list<TrackEntry> TreadsGroundList;
static std::list<TrackEntry> TreadsObstacleList;
static float TrackFadeTime = 5.0f;
static float UserFadeScale = 1.0f;
static AirCullStyle AirCull = FullAirCull;

// FIXME - get these from AnimatedTreads
static const float TreadOutside = 1.4f;
static const float TreadInside = 0.875f;

static const float TreadMiddle = 0.5f * (TreadOutside + TreadInside);
static const float TreadMarkWidth = 0.2f;

static OpenGLGState smokeGState;
static const char smokeTexture[] = "smoke";
static OpenGLGState puddleGState;
static const char puddleTexture[] = "puddle";
static OpenGLGState treadsGState;

static float TextureHeightOffset = 0.05f;


// Function Prototypes
//////////////////////

static void setup();
static void initContext(void* data);
static void drawSmoke(const TrackEntry& te);
static void drawPuddle(const TrackEntry& te);
static void drawTreads(const TrackEntry& te);
static bool onBuilding(const float pos[3]);
static void updateList(std::list<TrackEntry>& list, float dt);
static void addEntryToList(std::list<TrackEntry>& list,
                           TrackEntry& te, TrackType type);


//
// TrackMarks
//

void TrackMarks::init()
{
  clear();
  setup();
  setUserFade(BZDB.eval("userTrackFade"));
  OpenGLGState::registerContextInitializer(initContext, NULL);

  return;
}


void TrackMarks::kill()
{
  clear();
  OpenGLGState::unregisterContextInitializer(initContext, NULL);
  
  return;
}


void TrackMarks::clear()
{
  SmokeList.clear();
  PuddleList.clear();
  TreadsGroundList.clear();
  TreadsObstacleList.clear();
  return;
}


void TrackMarks::setUserFade(float value)
{
  if (value < 0.0f) {
    value = 0.0f;
  }
  else if (value > 1.0f) {
    value = 1.0f;
  }

  UserFadeScale = value;
  BZDB.setFloat("userTrackFade", value);

  if (UserFadeScale < 1.0f) { // FIXME
    AirCull = NoAirCull;
  } else {
    AirCull = FullAirCull;
  }

  return;
}


float TrackMarks::getUserFade()
{
  return BZDB.eval("userTrackFade");
}


void TrackMarks::setAirCulling(AirCullStyle style)
{
  AirCull = style;
  return;
}


AirCullStyle TrackMarks::getAirCulling()
{
  return AirCull;
}


static void addEntryToList(std::list<TrackEntry>& list,
                           TrackEntry& te, TrackType type)
{
  // push the entry
  list.push_back(te);

  // make a sceneNode for the BSP rendering, if not on the ground
  if (!BZDBCache::zbuffer && (te.pos[2] != TextureHeightOffset)) {
    const OpenGLGState* gstate = NULL;
    if (type == TreadsTrack) {
      gstate = &treadsGState;
    } else if (type == PuddleTrack) {
      gstate = &puddleGState;
    } else if (type == SmokeTrack) {
      gstate = &smokeGState;
    } else {
      return;
    }
    TrackEntry& ptr = *list.rbegin();
    ptr.sceneNode = new TrackSceneNode(&ptr, type, gstate);
  }
  return;
}


bool TrackMarks::addMark(const float pos[3], float scale, float angle,
			 int phydrv)
{
  TrackEntry te;
  TrackType type;
  te.lifeTime = 0.0f;
  te.sceneNode = NULL;

  // determine the track mark type
  if ((pos[2] <= 0.1f) && BZDB.get(StateDatabase::BZDB_MIRROR) != "none") {
    type = PuddleTrack;
    if (pos[2] < 0.0f) {
      scale = 0.0f; // single puddle, like Narrow tanks
    }
  } else {
    type = TreadsTrack;
    if (scale < 0.01f) {
      return false; // Narrow tanks don't draw tread marks
    }
    if (pos[2] < 0.0f) {
      return false; // Burrowed tanks don't draw tread marks
    }
  }

  // copy some parameters
  te.pos[0] = pos[0];
  te.pos[1] = pos[1];
  if (pos[2] < 0.0f) {
    te.pos[2] = TextureHeightOffset;
  } else {
    te.pos[2] = pos[2] + TextureHeightOffset;
  }
  te.scale = scale;
  te.angle = angle * (180.0f / M_PI); // in degress, for glRotatef()
  
  // only use the physics driver if it matters
  const PhysicsDriver* driver = PHYDRVMGR.getDriver(phydrv);
  if (driver == NULL) {
    te.phydrv = -1;
  } else {
    const float* v = driver->getVelocity();
    const float av = driver->getAngularVel();
    if ((v[0] == 0.0f) && (v[1] == 0.0f) && (av == 0.0f)) {
      te.phydrv = -1;
    } else {
      te.phydrv = phydrv;
    }
  }

  if (type == PuddleTrack) {
    // Puddle track marks
    addEntryToList(PuddleList, te, type);
  }
  else {
    // Treads track marks
    if (pos[2] == 0.0f) {
      // no culling required
      te.sides = BothTreads;
      addEntryToList(TreadsGroundList, te, type);
    }
    else if ((AirCull & InitAirCull) == 0) {
      // do not cull the air marks
      te.sides = BothTreads;
      addEntryToList(TreadsObstacleList, te, type);
    }
    else {
      // cull based on track mark support
      te.sides = 0;
      float markPos[3];
      markPos[2] = pos[2];
      const float dx = -sinf(angle) * TreadMiddle;
      const float dy = +cosf(angle) * TreadMiddle;
      // left tread
      markPos[0] = pos[0] + dx;
      markPos[1] = pos[1] + dy;
      if (onBuilding(markPos)) {
        te.sides |= LeftTread;
      }
      // right tread
      markPos[0] = pos[0] - dx;
      markPos[1] = pos[1] - dy;
      if (onBuilding(markPos)) {
        te.sides |= RightTread;
      }
      // add if required
      if (te.sides != 0) {
        addEntryToList(TreadsObstacleList, te, type);
      } else {
        return false;
      }
    }
  }
  
  return true;
}


static bool onBuilding(const float pos[3])
{
  const float dir[3] = {0.0f, 0.0f, -1.0f};
  const float org[3] = {pos[0], pos[1], pos[2] + 0.1f};
  Ray ray(org, dir);
  const ObsList* olist = COLLISIONMGR.rayTest (&ray, 0.5f);
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    const float top = obs->getPosition()[2] + obs->getHeight();
    if (fabsf(top - pos[2]) < 0.2f) {
      const float hitTime = obs->intersect(ray);
      if (hitTime >= 0.0f) {
        return true;
      }
    }
  }
  return false;
}


static void updateList(std::list<TrackEntry>& list, float dt)
{
  std::list<TrackEntry>::iterator it = list.begin();
  while (it != list.end()) {
    TrackEntry& te = *it;

    // increase the lifeTime
    te.lifeTime += dt;

    // see if this mark has expired
    std::list<TrackEntry>::iterator next = it;
    next++;
    if (te.lifeTime > TrackFadeTime) {
      list.erase(it);
      it = next;
      continue;
    }
    it = next;

    // update for the Physics Driver
    const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(te.phydrv);
    if (phydrv != NULL) {
    
      const float* v = phydrv->getVelocity();
      te.pos[0] += (v[0] * dt);
      te.pos[1] += (v[1] * dt);

      const float av = phydrv->getAngularVel();
      if (av != 0.0f) {
        const float* ap = phydrv->getAngularPos();
        const float da = (av * dt);
        const float cos_val = cosf(da);
        const float sin_val = sinf(da);
        const float dx = te.pos[0] - ap[0];
        const float dy = te.pos[1] - ap[1];
        te.pos[0] = ap[0] + ((cos_val * dx) - (sin_val * dy));
        te.pos[1] = ap[1] + ((cos_val * dy) + (sin_val * dx));
        te.angle += da * (180.0f / M_PI);
      }

      if ((AirCull & PhyDrvAirCull) != 0) {
        // no need to cull ground marks
        if (te.pos[2] == 0.0f) {
          continue;
        }
        // cull the track marks if they aren't supported
        float markPos[3];
        markPos[2] = te.pos[2] - TextureHeightOffset;
        const float radians = te.angle * (M_PI / 180.0f);
        const float dx = -sinf(radians) * TreadMiddle;
        const float dy = +cosf(radians) * TreadMiddle;
        // left tread
        if ((te.sides & LeftTread) != 0) {
          markPos[0] = te.pos[0] + dx;
          markPos[1] = te.pos[1] + dy;
          if (!onBuilding(markPos)) {
            te.sides &= ~LeftTread;
          }
        }
        // right tread
        if ((te.sides & RightTread) != 0) {
          markPos[0] = te.pos[0] - dx;
          markPos[1] = te.pos[1] - dy;
          if (!onBuilding(markPos)) {
            te.sides &= ~RightTread;
          }
        }
        // cull this node
        if (te.sides == 0) {
          te.lifeTime = MAXFLOAT;
        }
      }
    }
  }

  return;
}


void TrackMarks::update(float dt)
{
  TrackFadeTime = BZDB.eval(StateDatabase::BZDB_TRACKFADE);
  TrackFadeTime = TrackFadeTime * UserFadeScale;
  
  updateList(SmokeList, dt);
  updateList(PuddleList, dt);
  updateList(TreadsGroundList, dt);
  updateList(TreadsObstacleList, dt);
  
  return;
}


static void setup()
{
  OpenGLGStateBuilder gb;
  
  int puddleTexId = -1;
  if (BZDBCache::texture) {
    TextureManager &tm = TextureManager::instance();
    puddleTexId = tm.getTextureID(puddleTexture, false);
  }
  gb.reset();
  gb.setShading(GL_FLAT);
  gb.setAlphaFunc(GL_GEQUAL, 0.1f);
  gb.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gb.enableMaterial(false); // no lighting
  if (puddleTexId >= 0) {
    gb.setTexture(puddleTexId);
  }
  puddleGState = gb.getState();
  
  int smokeTexId = -1;
  if (BZDBCache::texture) {
    TextureManager &tm = TextureManager::instance();
    smokeTexId = tm.getTextureID(smokeTexture, false);
  }
  gb.reset();
  gb.setShading(GL_FLAT);
  gb.setAlphaFunc(GL_GEQUAL, 0.1f);
  gb.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gb.enableMaterial(false); // no lighting
  if (smokeTexId >= 0) {
    gb.setTexture(smokeTexId);
  }
  smokeGState = gb.getState();
  
  gb.reset();
  gb.setShading(GL_FLAT);
  gb.setAlphaFunc(GL_GEQUAL, 0.1f);
  gb.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gb.enableMaterial(false); // no lighting
  treadsGState = gb.getState();
  
  return;
}
  

static void initContext(void* /*data*/)
{
  setup();
  return;
}


void TrackMarks::notifyStyleChange()
{
  setup();
  return;
}


void TrackMarks::renderGroundTracks()
{
  if (TrackFadeTime <= 0.0f) {
    clear();
    return;
  }

  std::list<TrackEntry>::iterator it;

  // disable the zbuffer for drawing on the ground
  if (BZDBCache::zbuffer) {
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
  }
  
  // draw ground treads
  treadsGState.setState();
  for (it = TreadsGroundList.begin(); it != TreadsGroundList.end(); it++) {
    drawTreads(*it);
  }

  // draw puddles
  puddleGState.setState();
  for (it = PuddleList.begin(); it != PuddleList.end(); it++) {
    drawPuddle(*it);
  }
  
  // re-enable the zbuffer
  if (BZDBCache::zbuffer) {
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
  }

  return;
}


void TrackMarks::renderObstacleTracks()
{
  if (!BZDBCache::zbuffer) {
    return; // this is not for the BSP rendering
  }

  if (TrackFadeTime <= 0.0f) {
    clear();
    return;
  }

  std::list<TrackEntry>::iterator it;

  // disable the zbuffer writing (these are the last things drawn)
  // this helps to avoid the zbuffer fighting/flickering effect
  glDepthMask(GL_FALSE);
  
  // draw treads
  treadsGState.setState();
  for (it = TreadsObstacleList.begin(); it != TreadsObstacleList.end(); it++) {
    drawTreads(*it);
  }

  // draw smoke
  smokeGState.setState();
  for (it = SmokeList.begin(); it != SmokeList.end(); it++) {
    drawSmoke(*it);
  }

  // re-enable the zbuffer writing
  glDepthMask(GL_TRUE);
  
  return;
}


static void drawPuddle(const TrackEntry& te)
{
  const float ratio = (te.lifeTime / TrackFadeTime);
  const float scale = 2.0f * ratio;
  const float offset = te.scale * TreadMiddle;

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f - ratio);

  glPushMatrix();
  {
    glTranslatef(te.pos[0], te.pos[1], te.pos[2]);
    glRotatef(te.angle, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.0f, +offset, 0.0f);
    glScalef(scale, scale, 1.0f);
    glBegin(GL_QUADS);
    {
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(-1.0f, -1.0f, 0.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(+1.0f, -1.0f, 0.0f);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(+1.0f, +1.0f, 0.0f);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(-1.0f, +1.0f, 0.0f);
    }
    glEnd();
  }
  glPopMatrix();

  // Narrow tanks only need 1 puddle
  if (offset > 0.01f) {
    glPushMatrix();
    {
      glTranslatef(te.pos[0], te.pos[1], te.pos[2]);
      glRotatef(te.angle, 0.0f, 0.0f, 1.0f);
      glTranslatef(0.0f, -offset, 0.0f);
      glScalef(scale, scale, 1.0f);

      glBegin(GL_QUADS);
      {
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(+1.0f, -1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(+1.0f, +1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, +1.0f, 0.0f);
      }
      glEnd();
    }
    glPopMatrix();
  }

  return;
}


static void drawTreads(const TrackEntry& te)
{
  const float ratio = (te.lifeTime / TrackFadeTime);

  glColor4f(0.0f, 0.0f, 0.0f, 1.0f - ratio);

  glPushMatrix();
  {
    glTranslatef(te.pos[0], te.pos[1], te.pos[2]);
    glRotatef(te.angle, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, te.scale, 1.0f);

    const float halfWidth = 0.5f * TreadMarkWidth;

    if ((te.sides & LeftTread) != 0) {
      glRectf(-halfWidth, +TreadInside, +halfWidth, +TreadOutside);
    }
    if ((te.sides & RightTread) != 0) {
      glRectf(-halfWidth, -TreadOutside, +halfWidth, -TreadInside);
    }
  }
  glPopMatrix();

  return;
}


static void drawSmoke(const TrackEntry& te)
{
  const float ratio = (te.lifeTime / TrackFadeTime);

  glColor4f(0.0f, 0.0f, 0.0f, 1.0f - ratio);

  glPushMatrix();
  {
    glTranslatef(te.pos[0], te.pos[1], te.pos[2]);
    glRotatef(te.angle, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, te.scale, 1.0f);

    const float halfWidth = 0.5f * TreadMarkWidth;

    if ((te.sides & LeftTread) != 0) {
      glRectf(-halfWidth, +TreadInside, +halfWidth, +TreadOutside);
    }
    if ((te.sides & RightTread) != 0) {
      glRectf(-halfWidth, -TreadOutside, +halfWidth, -TreadInside);
    }
  }
  glPopMatrix();

  return;
}


void TrackMarks::addSceneNodes(SceneDatabase* scene)
{
  // Depth Buffer does not need to use SceneNodes
  if (BZDBCache::zbuffer) {
    return;
  }
  
  // do not add track marks that are drawn by renderGroundTracks
  std::list<TrackEntry>::iterator it;

  // tread track marks on obstacles
  for (it = TreadsObstacleList.begin(); it != TreadsObstacleList.end(); it++) {
    const TrackEntry& te = *it;
    if (te.sceneNode != NULL) {
      te.sceneNode->update();
      scene->addDynamicNode(te.sceneNode);
    }
  }
      
  // smoke track marks in the air
  for (it = SmokeList.begin(); it != SmokeList.end(); it++) {
    const TrackEntry& te = *it;
    if (te.sceneNode != NULL) {
      te.sceneNode->update();
      scene->addDynamicNode(te.sceneNode);
    }
  }
      
  return;
}


//
// TrackEntry
//

TrackEntry::~TrackEntry()
{
  delete sceneNode;
  return;
}


//
// TrackRenderNode
//

TrackRenderNode::TrackRenderNode(const TrackEntry* _te, TrackType _type)
{
  te = _te;
  type = _type;
  return;
}
  

TrackRenderNode::~TrackRenderNode()
{
  return;
}


void TrackRenderNode::render()
{ 
  if (type == TreadsTrack) {
    drawTreads(*te);
  } else if (type == PuddleTrack) {
    drawPuddle(*te);
  } else if (type == SmokeTrack) {
    drawSmoke(*te);
  }
  return;
};


//
// TrackSceneNode
//

TrackSceneNode::TrackSceneNode(const TrackEntry* _te, TrackType _type,
                               const OpenGLGState* _gstate) :
                                 renderNode(_te, _type)
{
  te = _te;
  type = _type;
  gstate = _gstate;
  return;
}
                               
TrackSceneNode::~TrackSceneNode()
{
  return;
}

void TrackSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, gstate);
  return;
}

void TrackSceneNode::update()
{
  // update the position
  setCenter(te->pos);

  // update the radius squared (for culling)
  float radius;
  if (type == TreadsTrack) {
    radius = (te->scale * TreadOutside);
  } else if (type == PuddleTrack) {
    radius = (te->scale * (TreadMiddle + 1.0f));
  } else if (type == SmokeTrack) {
    radius = (te->scale * TreadOutside);
  }
  setRadius(radius * radius);
  
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

