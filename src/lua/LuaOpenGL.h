/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LUA_OPENGL_H
#define LUA_OPENGL_H


struct lua_State;


class LuaOpenGL {
  public:
    static const float defaultTextSize;

  public:
    static void Init();
    static void Free();

    static bool PushEntries(lua_State* L);

    static void CheckDrawingEnabled(lua_State* L, const char* caller);

  private:
    static void StaticInitContext(void* data);
    static void StaticFreeContext(void* data);

  private:
    static int HasExtension(lua_State* L);
    static int GetNumber(lua_State* L);
    static int GetString(lua_State* L);
    static int GetError(lua_State* L);

    static int ConfigScreen(lua_State* L);

    static int ResetState(lua_State* L);
    static int ResetMatrices(lua_State* L);
    static int Clear(lua_State* L);

    static int Hint(lua_State* L);
    static int Lighting(lua_State* L);
    static int LightModel(lua_State* L);
    static int ShadeModel(lua_State* L);
    static int Scissor(lua_State* L);
    static int Viewport(lua_State* L);
    static int ColorMask(lua_State* L);
    static int DepthMask(lua_State* L);
    static int DepthTest(lua_State* L);
    static int DepthClamp(lua_State* L);
    static int Culling(lua_State* L);
    static int FrontFace(lua_State* L);
    static int LogicOp(lua_State* L);
    static int Fog(lua_State* L);
    static int Smoothing(lua_State* L);
    static int AlphaTest(lua_State* L);
    static int LineStipple(lua_State* L);
    static int PolygonStipple(lua_State* L);
    static int Normalize(lua_State* L);

    static int Blending(lua_State* L);
    static int BlendEquation(lua_State* L);
    static int BlendFunc(lua_State* L);
    static int BlendEquationSeparate(lua_State* L);
    static int BlendFuncSeparate(lua_State* L);

    static int Color(lua_State* L);
    static int Material(lua_State* L);
    static int Ambient(lua_State* L);
    static int Diffuse(lua_State* L);
    static int Emission(lua_State* L);
    static int Specular(lua_State* L);
    static int Shininess(lua_State* L);

    static int PolygonMode(lua_State* L);
    static int PolygonOffset(lua_State* L);

    static int StencilTest(lua_State* L);
    static int StencilMask(lua_State* L);
    static int StencilFunc(lua_State* L);
    static int StencilOp(lua_State* L);
    static int StencilMaskSeparate(lua_State* L);
    static int StencilFuncSeparate(lua_State* L);
    static int StencilOpSeparate(lua_State* L);

    static int LineWidth(lua_State* L);
    static int PointSize(lua_State* L);
    static int PointSprite(lua_State* L);
    static int PointParameter(lua_State* L);

    static int BeginEnd(lua_State* L);
    static int Vertex(lua_State* L);
    static int Normal(lua_State* L);
    static int TexCoord(lua_State* L);
    static int MultiTexCoord(lua_State* L);
    static int SecondaryColor(lua_State* L);
    static int FogCoord(lua_State* L);
    static int EdgeFlag(lua_State* L);
    static int VertexAttrib(lua_State* L);

    static int Rect(lua_State* L);
    static int TexRect(lua_State* L);

    static int Text(lua_State* L);
    static int GetTextWidth(lua_State* L);
    static int GetTextHeight(lua_State* L);

    static int Map1(lua_State* L);
    static int Map2(lua_State* L);
    static int MapGrid1(lua_State* L);
    static int MapGrid2(lua_State* L);
    static int Eval(lua_State* L);
    static int EvalEnable(lua_State* L);
    static int EvalDisable(lua_State* L);
    static int EvalMesh1(lua_State* L);
    static int EvalMesh2(lua_State* L);
    static int EvalCoord1(lua_State* L);
    static int EvalCoord2(lua_State* L);
    static int EvalPoint1(lua_State* L);
    static int EvalPoint2(lua_State* L);

    static int Light(lua_State* L);
    static int ClipPlane(lua_State* L);

    static int MatrixMode(lua_State* L);
    static int LoadIdentity(lua_State* L);
    static int LoadMatrix(lua_State* L);
    static int MultMatrix(lua_State* L);
    static int Translate(lua_State* L);
    static int Scale(lua_State* L);
    static int Rotate(lua_State* L);
    static int Ortho(lua_State* L);
    static int Frustum(lua_State* L);
    static int Billboard(lua_State* L);
    static int PushMatrix(lua_State* L);
    static int PopMatrix(lua_State* L);
    static int PushPopMatrix(lua_State* L);
    static int GetMatrixData(lua_State* L);

    static int PushAttrib(lua_State* L);
    static int PopAttrib(lua_State* L);
    static int PushPopAttrib(lua_State* L);
    static int UnsafeState(lua_State* L);

    static int Flush(lua_State* L);
    static int Finish(lua_State* L);

    static int ReadPixels(lua_State* L);

    static int RenderMode(lua_State* L);
    static int SelectBuffer(lua_State* L);
    static int SelectBufferData(lua_State* L);
    static int InitNames(lua_State* L);
    static int LoadName(lua_State* L);
    static int PushName(lua_State* L);
    static int PopName(lua_State* L);
    static int PushPopName(lua_State* L);
};


#endif // LUA_OPENGL_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
