// OpenGLPassState.h: interface for the OpenGLPassState class.
//
//////////////////////////////////////////////////////////////////////

#ifndef OPENGL_PASS_STATE_H
#define OPENGL_PASS_STATE_H

#include "common.h"

#include <set>

class OpenGLPassState {
	public:
		enum DrawMode {
			DRAW_NONE             = 0,
			DRAW_GENESIS          = 1,
			DRAW_WORLD_START      = 2,
			DRAW_WORLD            = 3,
			DRAW_WORLD_ALPHA      = 4,
			DRAW_WORLD_SHADOW     = 5,
			DRAW_SCREEN_START     = 6,
			DRAW_SCREEN           = 7,
			DRAW_RADAR            = 8,
			DRAW_LAST_MODE        = DRAW_RADAR
		};

	public:
		static void Init();
		static void Free();

		static void ConfigScreen(float width, float dist);

		static inline bool IsDrawingEnabled() {
			return drawingEnabled;
		}
		static inline void SetDrawingEnabled(bool value) {
			drawingEnabled = value;
		}

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

	protected:
		static void EnableCommon(DrawMode);
		static void ResetCommon(DrawMode);
		static void DisableCommon(DrawMode);

		static void ResetGLState();

		static void ClearMatrixStack(int);

		static void ResetWorldMatrices();
		static void ResetWorldShadowMatrices();
		static void ResetScreenMatrices();
		static void ResetRadarMatrices();

		static void SetupScreenMatrices();
		static void RevertScreenMatrices();

		static void SetupScreenLighting();
		static void RevertScreenLighting();
		static void SetupWorldLighting();
		static void RevertWorldLighting();

	private:
		static DrawMode drawMode;
		static DrawMode prevDrawMode;
		static bool drawingEnabled;
		static float screenWidth;
		static float screenDistance;
		static void (*resetMatrixFunc)(void);
		static unsigned int resetStateList;
		static std::set<unsigned int> occlusionQueries;
};


#endif // OPENGL_PASS_STATE_H
