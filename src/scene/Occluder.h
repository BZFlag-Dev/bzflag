
#include "SceneNode.h"
#include "Frustum.h"
#include "Intersect.h"

class Occluder {
  public:
    Occluder(SceneNode *node);
    ~Occluder();
    bool makePlanes(const Frustum* frustum);
    IntersectLevel doCullAxisBox(const float* mins, const float* maxs);
    bool doCullSceneNode(SceneNode* node);
    void addScore(unsigned int score);
    void divScore();
    int getScore() const;
    int getVertexCount() const;
    SceneNode* getSceneNode() const;
    void draw() const;
    void print(const char* string) const; // for debugging

  private:
    SceneNode* sceneNode;
    unsigned int cullScore;
    int planeCount;  // one more then the vertex count
    int vertexCount; // vertex count of the occluding plane
    float (*planes)[4];
    float (*vertices)[3];
    static const bool DrawEdges;
    static const bool DrawNormals;
    static const bool DrawVertices;
};

#define MAX_OCCLUDERS 64

class OccluderManager {

  public:
    OccluderManager();
    ~OccluderManager();

    void clear();
    void update(const Frustum* frustum);
    void select(SceneNode** list, int listCount);

    IntersectLevel occlude(const float* mins, const float* maxs,
			   unsigned int score);
    bool occludePeek(const float* mins, const float* maxs);

    int getOccluderCount () const;
    
    void enable() { enabled = true; }
    void disable() { enabled = false; }
    
    void draw() const;

  private:
    bool enabled;
    void setMaxOccluders(int size);
    void sort();
    int activeOccluders;
    int allowedOccluders;
    static const int MaxOccluders;
    Occluder* occluders[MAX_OCCLUDERS];
};

inline void Occluder::addScore(unsigned int score)
{
  unsigned int tmp = cullScore + score;
  if (tmp > cullScore) {
    cullScore = tmp;
  }
  return;
}

inline void Occluder::divScore()
{
  cullScore = cullScore >> 1;
  return;
}

inline int Occluder::getScore() const
{
  return cullScore;
}

inline SceneNode* Occluder::getSceneNode()const
{
  return sceneNode;
}

inline int Occluder::getVertexCount() const
{
  return vertexCount;
}

inline int OccluderManager::getOccluderCount () const
{
  return activeOccluders;
}

