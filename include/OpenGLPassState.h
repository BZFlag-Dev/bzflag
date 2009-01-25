////////////////////////////////////////////////////////////////////////////////
//
// OpenGLPassState.h: interface for the OpenGLPassState class.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef OPENGL_PASS_STATE_H
#define OPENGL_PASS_STATE_H

#include "common.h"

// system headers
#include <set>

// common headers
#include "bzfgl.h"


class OpenGLPassState {
  public:
    enum DrawMode {
      DRAW_NONE         = -1,
      DRAW_GENESIS      = 0,
      DRAW_WORLD_START  = 1,
      DRAW_WORLD        = 2,
      DRAW_WORLD_ALPHA  = 3,
      DRAW_WORLD_SHADOW = 4,
      DRAW_SCREEN_START = 5,
      DRAW_SCREEN       = 6,
      DRAW_RADAR        = 7,
      DRAW_MODE_COUNT   = 8,
    };

  public:

  public:
    static inline int GetAttribStackDepth()    { return attribStackDepth;    }
    static inline int GetMinAttribStackDepth() { return minAttribStackDepth; }
    static inline int GetMaxAttribStackDepth() { return maxAttribStackDepth; }
    static bool TryAttribStackChange(int change); 
    static bool TestAttribStackChange(int change); 
    static bool PushAttrib(GLbitfield bits); // GLbitfield
    static bool PopAttrib();
    static bool TryAttribStackPush();
    static bool TryAttribStackPop();
    static bool TryAttribStackRangeChange(int endChange,  // for display lists
                                          int minChange,  // negative, or zero
                                          int maxChange); // positive, or zero
    static bool NewList();
    static bool EndList();
    static bool CreatingList();

    static const char* GetDrawModeName(DrawMode);

  public:
    static void Init();
    static void Free();

    static void ConfigScreen(float width, float dist);

    static inline bool IsDrawingEnabled() { return drawingEnabled; }
    static inline void SetDrawingEnabled(bool value) { drawingEnabled = value; }

    static void ResetState();
    static void ResetMatrices();

    static void EnableDrawGenesis();
    static void ResetDrawGenesis();
    static void DisableDrawGenesis();

    static void EnableDrawWorldStart();
    static void ResetDrawWorldStart();
    static void DisableDrawWorldStart();

    static void EnableDrawWorld();
    static void ResetDrawWorld();
    static void DisableDrawWorld();

    static void EnableDrawWorldAlpha();
    static void ResetDrawWorldAlpha();
    static void DisableDrawWorldAlpha();

    static void EnableDrawWorldShadow();
    static void ResetDrawWorldShadow();
    static void DisableDrawWorldShadow();

    static void EnableDrawScreenStart();
    static void ResetDrawScreenStart();
    static void DisableDrawScreenStart();

    static void EnableDrawScreen();
    static void ResetDrawScreen();
    static void DisableDrawScreen();

    static void EnableDrawRadar();
    static void ResetDrawRadar();
    static void DisableDrawRadar();

  private:
    static void EnableCommon(DrawMode);
    static void ResetCommon(DrawMode);
    static void DisableCommon(DrawMode);

    static void ResetModeState(DrawMode);

    static void ClearMatrixStack(int);
    static void ExecuteAttribStack(int depth);

    static void ResetDrawGenesisState();
    static void ResetDrawWorldStartState();
    static void ResetDrawWorldState();
    static void ResetDrawWorldAlphaState();
    static void ResetDrawWorldShadowState();
    static void ResetDrawScreenStartState();
    static void ResetDrawScreenState();
    static void ResetDrawRadarState();

    static void ResetIdentityMatrices();
    static void ResetWorldMatrices();
    static void ResetWorldShadowMatrices();
    static void ResetScreenMatrices();
    static void ResetRadarMatrices();

    static void SetupScreenMatrices();
    static void SetupScreenLighting();
    static void RevertScreenMatrices();
    static void RevertScreenLighting();

  private:
    static void InitContext(void* data);
    static void FreeContext(void* data);
    static void BZDBCallback(const std::string& name, void* data);

  private:

    static DrawMode drawMode;
    static bool drawingEnabled;

    static bool creatingList;

    static float screenWidth;
    static float screenDistance;

    static void (*resetStateFunc)(void);
    static void (*resetMatrixFunc)(void);
    static GLuint stateLists[DRAW_MODE_COUNT];
    typedef void (*ResetFunc)(void);
    static ResetFunc resetStateFuncs[DRAW_MODE_COUNT];
    static ResetFunc resetMatrixFuncs[DRAW_MODE_COUNT];

    static int attribStackDepth;
    static int minAttribStackDepth;
    static int maxAttribStackDepth;
};


#endif // OPENGL_PASS_STATE_H

