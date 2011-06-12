
// interface header
#include "WorldManager.h"

//#include "bzfgl.h"
//#include "BZDBCache.h"

#include "TextureManager.h"
#include "CollisionManager.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "PhysicsDriver.h"
#include "WorldText.h"
#include "LinkManager.h"
#include "WallObstacle.h"
#include "MeshObstacle.h"
//#include "MeshTransform.h"


WorldManager WorldManager::instance;


//============================================================================//

void WorldManager::addMesh(MeshObstacle* mesh) {
  meshes.push_back(mesh);
  if (mesh->getHasSpecialFaces()) {
    const int faceCount = mesh->getFaceCount();
    for (int f = 0; f < faceCount; f++) {
      MeshFace* face = mesh->getFace(f);
      const MeshFace::SpecialData* sd = face->getSpecialData();
      if (face->isSpecial()) {
        if (face->isBaseFace()) {
          bases.push_back(face);
        }
        if (face->isWallFace()) {
          walls.push_back(face);
        }
        if (!sd->zoneParams.empty()) {
          zones.push_back(face);
        }
      }
    }
  }
}

//============================================================================//

void WorldManager::addText(WorldText* text) {
  texts.push_back(text);
}

//============================================================================//

void WorldManager::clear() {
  for (size_t i = 0; i < meshes.size(); i++) {
    delete meshes[i];
  }
  for (size_t i = 0; i < texts.size(); i++) {
    delete texts[i];
  }

  meshes.clear();
  texts.clear();
  bases.clear();
  walls.clear();
  zones.clear();
  weapons.clear();

  // clear the managers
  DYNCOLORMGR.clear();
  TEXMATRIXMGR.clear();
  MATERIALMGR.clear();
  PHYDRVMGR.clear();
  TRANSFORMMGR.clear();
  COLLISIONMGR.clear();
  linkManager.clear();
//  TRANSFORMMGR.clear();
}

//============================================================================//
