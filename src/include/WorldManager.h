
#ifndef WORLD_OBJECTS_H
#define WORLD_OBJECTS_H


#include "common.h"

// system headers
#include <vector>


class MeshObstacle;
class MeshFace;
class WorldText;


class WorldManager {
    friend class WorldScript;
  public:
    void clear();
    const std::vector<MeshObstacle*>& getMeshes()  const { return meshes;   }
    const std::vector<MeshObstacle*>& getWeapans() const { return weapons;  }
    const std::vector<WorldText*>&    getTexts()   const { return texts;    }
    const std::vector<MeshFace*>&     getBases()   const { return bases;    }
    const std::vector<MeshFace*>&     getWalls()   const { return walls;    }
    const std::vector<MeshFace*>&     getZones()   const { return zones;    }
  private:
    void addMesh(MeshObstacle* mesh);
    void addText(WorldText* text);
  private:
    WorldManager() {}
    WorldManager(const WorldManager&);
    WorldManager& operator=(const WorldManager&);
    ~WorldManager() { clear(); }
  private:
    std::vector<MeshObstacle*> meshes;
    std::vector<MeshObstacle*> weapons;
    std::vector<WorldText*>    texts;
    std::vector<MeshFace*>     bases;
    std::vector<MeshFace*>     walls;
    std::vector<MeshFace*>     zones;
  public:
    static WorldManager instance;
};


#define WORLDMGR (WorldManager::instance)


#endif // WORLD_OBJECTS_H
