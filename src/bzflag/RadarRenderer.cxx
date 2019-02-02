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

// interface header
#include "RadarRenderer.h"

// System headers
#include <glm/gtc/type_ptr.hpp>

// common implementation headers
#include "SceneRenderer.h"
#include "MainWindow.h"
#include "OpenGLGState.h"
#include "BZDBCache.h"
#include "TextureManager.h"
#include "PhysicsDriver.h"
#include "ObstacleMgr.h"
#include "MeshSceneNode.h"
#include "ObstacleList.h"
#include "WallObstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "MeshObstacle.h"
#include "VBO_Drawing.h"

// local implementation headers
#include "LocalPlayer.h"
#include "World.h"
#include "FlashClock.h"
#include "ShotPath.h"


static FlashClock flashTank;
static bool toggleTank = false;

const float RadarRenderer::colorFactor = 40.0f;

RadarRenderer::RadarRenderer(const SceneRenderer&, World* _world)
    : world(_world),
      x(0),
      y(0),
      w(0),
      h(0),
      dimming(0.0f),
      ps(),
      range(),
      decay(0.01f),
      smooth(false),
      jammed(false),
      useTankModels(false),
      triangleCount()
{

    setControlColor();
    frameVBOindex   = -1;
    markersVBOindex = -1;
    tankVBOindex    = -1;
    flagVBOindex    = -1;
    wallVBOindex    = -1;
    vboManager.registerClient(this);
}

RadarRenderer::~RadarRenderer()
{
    vboV.vboFree(frameVBOindex);
    vboV.vboFree(markersVBOindex);
    vboV.vboFree(tankVBOindex);
    vboV.vboFree(flagVBOindex);
    vboV.vboFree(wallVBOindex);
    vboManager.unregisterClient(this);
}

void RadarRenderer::initVBO()
{
    frameVBOindex   = vboV.vboAlloc(4);
    markersVBOindex = vboV.vboAlloc(9);
    tankVBOindex    = vboV.vboAlloc(5);
    flagVBOindex    = vboV.vboAlloc(8);

    glm::vec3 vertex[9];

    // view frustum edges
    vertex[0] = glm::vec3(-1.0f, 1.0f, 0.0f);
    vertex[1] = glm::vec3( 0.0f, 0.0f, 0.0f);
    vertex[2] = glm::vec3(+1.0f, 1.0f, 0.0f);

    // north marker
    vertex[3] = glm::vec3(-1.0f, -1.0f, 0.0f);
    vertex[4] = glm::vec3(-1.0f, +1.0f, 0.0f);
    vertex[5] = glm::vec3(+1.0f, -1.0f, 0.0f);
    vertex[6] = glm::vec3(+1.0f, +1.0f, 0.0f);

    // forward tick
    vertex[7] = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex[8] = glm::vec3(0.0f, 4.0f, 0.0f);

    vboV.vertexData(markersVBOindex, 9, vertex);

    vertex[0] = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex[1] = glm::vec3(1.0f, 1.0f, 0.0f);
    vertex[2] = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex[3] = glm::vec3(0.0f, 0.0f, 0.0f);
    vboV.vertexData(frameVBOindex, 4, vertex);

    // draw the height box
    vertex[0] = glm::vec3(-1.0f,  0.0f, 0.0f);
    vertex[1] = glm::vec3( 0.0f, -1.0f, 0.0f);
    vertex[2] = glm::vec3(+1.0f,  0.0f, 0.0f);
    vertex[3] = glm::vec3( 0.0f, +1.0f, 0.0f);
    vertex[4] = glm::vec3(-1.0f,  0.0f, 0.0f);
    vboV.vertexData(tankVBOindex, 5, vertex);

    vertex[0] = glm::vec3(-1, 0, 0);
    vertex[1] = glm::vec3(+1, 0, 0);
    vertex[2] = glm::vec3(+1, 0, 0);
    vertex[3] = glm::vec3(-1, 0, 0);
    vertex[4] = glm::vec3(0, -1, 0);
    vertex[5] = glm::vec3(0, +1, 0);
    vertex[6] = glm::vec3(0, +1, 0);
    vertex[7] = glm::vec3(0, -1, 0);
    vboV.vertexData(flagVBOindex, 8, vertex);

    // prepare the walls
    wallVBOindex = -1;
    renderWallsPrepare();
}

void RadarRenderer::setWorld(World* _world)
{
    world = _world;

    // prepare the walls
    vboV.vboFree(wallVBOindex);
    wallVBOindex = -1;
    renderWallsPrepare();
}


void RadarRenderer::setControlColor(const GLfloat *color)
{
    if (color)
        memcpy(teamColor, color, 3 * sizeof(float));
    else
        memset(teamColor, 0, 3 * sizeof(float));
}


void RadarRenderer::setShape(int _x, int _y, int _w, int _h)
{
    x = _x;
    y = _y;
    w = _w;
    h = _h;
}


void RadarRenderer::setJammed(bool _jammed)
{
    jammed = _jammed;
    decay = 0.01;
}


void RadarRenderer::setDimming(float newDimming)
{
    dimming = (1.0f - newDimming > 1.0f) ? 1.0f : (1.0f - newDimming < 0.0f) ? 0.0f : 1.0f - newDimming;
}


glm::vec3 RadarRenderer::getTankColor(const Player* player)
{
    //The height box also uses the tank color

    const LocalPlayer *myTank = LocalPlayer::getMyTank();

    //my tank
    if (player->getId() == myTank->getId() )
        return glm::vec3(1.0f);

    glm::vec3 tankColor;
    //remote player
    if (player->isPaused() || player->isNotResponding())
    {
        const float dimfactor = 0.4f;

        const float *color;
        if (myTank->getFlag() == Flags::Colorblindness)
            color = Team::getRadarColor(RogueTeam);
        else
            color = Team::getRadarColor(player->getTeam());

        tankColor = glm::make_vec3(color) * dimfactor;
    }
    else
    {
        const GLfloat *col = Team::getRadarColor(myTank->getFlag() ==
                             Flags::Colorblindness ? RogueTeam : player->getTeam());
        tankColor = glm::make_vec3(col);
    }
    // If this tank is hunted flash it on the radar
    if (player->isHunted() && myTank->getFlag() != Flags::Colorblindness)
    {
        if (flashTank.isOn())
        {
            if (!toggleTank)
                tankColor = glm::vec3(0.0f, 0.8f, 0.9f);
        }
        else
        {
            toggleTank = !toggleTank;
            flashTank.setClock(0.2f);
        }
    }
    return tankColor;
}

void RadarRenderer::drawTank(const float pos[3], const Player* player, bool useSquares)
{
    glPushMatrix();

    // 'ps' is pixel scale, setup in render()
    const float tankRadius = BZDBCache::tankRadius;
    float minSize = 1.5f + (ps * BZDBCache::radarTankPixels);
    GLfloat size;
    if (tankRadius < minSize)
        size = minSize;
    else
        size = tankRadius;
    if (pos[2] < 0.0f)
        size = 0.5f;

    // NOTE: myTank was checked in render()
    const float myAngle = LocalPlayer::getMyTank()->getAngle();

    // transform to the tanks location
    glTranslatef(pos[0], pos[1], 0.0f);

    glm::vec3 tankColor(getTankColor(player));

    // draw the tank
    if (useSquares || !useTankModels)
    {
        glColor4f(tankColor.r, tankColor.g, tankColor.b, 1.0f);
        // align to the screen axes
        glRotatef(float(myAngle * 180.0 / M_PI), 0.0f, 0.0f, 1.0f);
        glPushMatrix();
        glScalef(size, size, 0.0f);
        DRAWER.simmetricRect();
        glPopMatrix();
    }
    else
    {
        const float tankAngle = player->getAngle();
        glPushMatrix();
        glRotatef(float(tankAngle * 180.0 / M_PI), 0.0f, 0.0f, 1.0f);
        drawFancyTank(player);
        glColor4f(tankColor.r, tankColor.g, tankColor.b, 1.0f);
        glPopMatrix();

        // align to the screen axes
        glRotatef(float(myAngle * 180.0 / M_PI), 0.0f, 0.0f, 1.0f);
    }

    // adjust with height box size
    const float boxHeight = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
    size = size * (1.0f + (0.5f * (pos[2] / boxHeight)));

    glScalef(size, size, size);
    // draw the height box
    vboV.enableArrays();
    glDrawArrays(GL_LINE_STRIP, tankVBOindex, 5);

    glPopMatrix();
}


void RadarRenderer::drawFancyTank(const Player* player)
{
    if (smooth)
        glDisable(GL_BLEND);

    // we use the depth buffer so that the treads look ok
    if (BZDBCache::zbuffer)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
    }

    OpenGLGState::resetState();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RENDERER.enableSun(true);

    player->renderRadar(); // draws at (0,0,0)

    RENDERER.enableSun(false);
    OpenGLGState::resetState();

    if (BZDBCache::zbuffer)
        glDisable(GL_DEPTH_TEST);

    if (smooth)
    {
        glEnable(GL_BLEND);
        glEnable(GL_POINT_SMOOTH);
    }

    return;
}


void RadarRenderer::drawFlag(const float pos[3])
{
    GLfloat s = BZDBCache::flagRadius > 3.0f * ps ? BZDBCache::flagRadius : 3.0f * ps;
    glPushMatrix();
    glTranslatef(pos[0], pos[1], 0);
    glScalef(s, s, 0);
    vboV.enableArrays();
    glDrawArrays(GL_LINES, flagVBOindex, 8);
    glPopMatrix();
}

void RadarRenderer::drawFlagOnTank(const float pos[3])
{
    glPushMatrix();

    // align it to the screen axes
    const float angle = LocalPlayer::getMyTank()->getAngle();
    glTranslatef(pos[0], pos[1], 0.0f);
    glRotatef(float(angle * 180.0 / M_PI), 0.0f, 0.0f, 1.0f);

    float tankRadius = BZDBCache::tankRadius;
    GLfloat s = 2.5f * tankRadius > 4.0f * ps ? 2.5f * tankRadius : 4.0f * ps;
    glScalef(s, s, 0);
    vboV.enableArrays();
    glDrawArrays(GL_LINES, flagVBOindex, 8);

    glPopMatrix();
}


void RadarRenderer::renderFrame(SceneRenderer& renderer)
{
    const MainWindow& window = renderer.getWindow();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    window.setProjectionPlay();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    OpenGLGState::resetState();

    const int ox = window.getOriginX();
    const int oy = window.getOriginY();

    glScissor(ox + x - 1, oy + y - 1, w + 2, h + 2);

    const float left = float(ox + x) - 0.5f;
    const float right = float(ox + x + w) + 0.5f;
    const float top = float(oy + y) - 0.5f;
    const float bottom = float(oy + y + h) + 0.5f;

    float outlineOpacity = RENDERER.getRadarOpacity();
    float fudgeFactor = BZDBCache::hudGUIBorderOpacityFactor; // bzdb cache this maybe?
    if ( outlineOpacity < 1.0f )
        outlineOpacity = (outlineOpacity*fudgeFactor) + (1.0f - fudgeFactor);

    glEnable(GL_BLEND);

    glColor4f(teamColor[0],teamColor[1],teamColor[2],outlineOpacity);

    glPushMatrix();
    glTranslatef(left, bottom, 0);
    glScalef(right - left, top - bottom, 0);
    vboV.enableArrays();
    glDrawArrays(GL_LINE_LOOP, frameVBOindex, 4);
    glPopMatrix();

    glDisable(GL_BLEND);

    glColor4f(teamColor[0],teamColor[1],teamColor[2],1.0f);

    const float opacity = renderer.getRadarOpacity();
    if ((opacity < 1.0f) && (opacity > 0.0f))
    {
        glScissor(ox + x - 2, oy + y - 2, w + 4, h + 4);
        // draw nice blended background
        if (opacity < 1.0f)
            glEnable(GL_BLEND);
        glColor4f(0.0f, 0.0f, 0.0f, opacity);
        glPushMatrix();
        glTranslatef((float)x, (float)y, 0.0f);
        glScalef((float)w, (float)h, 0.0f);
        DRAWER.asimmetricRect();
        glPopMatrix();
        if (opacity < 1.0f)
            glDisable(GL_BLEND);
    }

    // note that this scissor setup is used for the reset of the rendering
    glScissor(ox + x, oy + y, w, h);

    if (opacity == 1.0f)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    return;
}


void RadarRenderer::render(SceneRenderer& renderer, bool blank, bool observer)
{
    RenderNode::resetTriangleCount();

    const float radarLimit = BZDBCache::radarLimit;
    if (!BZDB.isTrue("displayRadar") || (radarLimit <= 0.0f))
    {
        triangleCount = 0;
        return;
    }

    // render the frame
    renderFrame(renderer);

    if (blank)
        return;

    if (!world)
        return;

    smooth = BZDBCache::smooth;
    const LocalPlayer *myTank = LocalPlayer::getMyTank();

    // setup the radar range
    float radarRange = BZDB.eval("displayRadarRange") * radarLimit;
    float maxRange = radarLimit;
    // when burrowed, limit radar range
    if (myTank && (myTank->getFlag() == Flags::Burrow) &&
            (myTank->getPosition()[2] < 0.0f))
        maxRange = radarLimit / 4.0f;
    if (radarRange > maxRange)
    {
        radarRange = maxRange;
        // only clamp the user's desired range if it's actually
        // greater then 1. otherwise, we may be resetting it due
        // to burrow radar limiting.
        if (BZDB.eval("displayRadarRange") > 1.0f)
            BZDB.set("displayRadarRange", "1.0");
    }

    // prepare projection matrix
    glMatrixMode(GL_PROJECTION);
    const MainWindow& window = renderer.getWindow();
    // NOTE: the visual extents include passable objects
    double maxHeight = 0.0;
    const Extents* visExts = renderer.getVisualExtents();
    if (visExts)
        maxHeight = (double)visExts->maxs[2];
    window.setProjectionRadar(x, y, w, h, radarRange, (float)(maxHeight + 10.0));

    // prepare modelview matrix
    glMatrixMode(GL_MODELVIEW);

    OpenGLGState::resetState();


    // if jammed then draw white noise.  occasionally draw a good frame.
    if (jammed && (bzfrand() > decay))
    {
        glPushMatrix();
        glLoadIdentity();
        glm::vec3 vertex[4];
        glm::vec2 textur[4];

        vertex[0] = glm::vec3(-radarRange,-radarRange, 0);
        vertex[1] = glm::vec3( radarRange,-radarRange, 0);
        vertex[2] = glm::vec3(-radarRange, radarRange, 0);
        vertex[3] = glm::vec3( radarRange, radarRange, 0);

        TextureManager &tm = TextureManager::instance();
        int noiseTexture = tm.getTextureID( "noise" );

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        if (noiseTexture >= 0)
        {

            const int sequences = 10;

            static float np[] =
            {
                0, 0, 1, 1,
                1, 1, 0, 0,
                0.5f, 0.5f, 1.5f, 1.5f,
                1.5f, 1.5f, 0.5f, 0.5f,
                0.25f, 0.25f, 1.25f, 1.25f,
                1.25f, 1.25f, 0.25f, 0.25f,
                0, 0.5f, 1, 1.5f,
                1, 1.5f, 0, 0.5f,
                0.5f, 0, 1.5f, 1,
                1.4f, 1, 0.5f, 0,
                0.75f, 0.75f, 1.75f, 1.75f,
                1.75f, 1.75f, 0.75f, 0.75f,
            };

            int noisePattern = 4 * int(floor(sequences * bzfrand()));

            textur[0] = glm::vec2(np[noisePattern+0],np[noisePattern+1]);
            textur[1] = glm::vec2(np[noisePattern+2],np[noisePattern+1]);
            textur[2] = glm::vec2(np[noisePattern+0],np[noisePattern+3]);
            textur[3] = glm::vec2(np[noisePattern+2],np[noisePattern+3]);

            int vboIndex = vboVT.vboAlloc(4);
            vboVT.textureData(vboIndex, 4, textur);
            vboVT.vertexData(vboIndex, 4, vertex);

            glEnable(GL_TEXTURE_2D);
            tm.bind(noiseTexture);

            vboVT.enableArrays();
            glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);
            vboVT.vboFree(vboIndex);

            glDisable(GL_TEXTURE_2D);
        }
        if (decay > 0.015f) decay *= 0.5f;

        glPopMatrix();
    }

    // only draw if there's a local player and a world
    else if (myTank)
    {
        glPushMatrix();
        glLoadIdentity();

        // if decay is sufficiently small then boost it so it's more
        // likely a jammed radar will get a few good frames closely
        // spaced in time.  value of 1 guarantees at least two good
        // frames in a row.
        if (decay <= 0.015f) decay = 1.0f;
        else decay *= 0.5f;


        // get size of pixel in model space (assumes radar is square)
        ps = 2.0f * (radarRange / GLfloat(w));
        MeshSceneNode::setRadarLodScale(ps);

        float tankWidth = BZDBCache::tankWidth;
        float tankLength = BZDBCache::tankLength;
        const float testMin = 8.0f * ps;
        // maintain the aspect ratio if it isn't square
        if ((tankWidth > testMin) &&  (tankLength > testMin))
            useTankModels = true;
        else
            useTankModels = false;

        // relative to my tank
        const float* myPos = myTank->getPosition();
        const float myAngle = myTank->getAngle();

        // draw the view angle below stuff
        // view frustum edges
        if (!BZDB.isTrue("hideRadarViewLines"))
        {
            glColor4f(1.0f, 0.625f, 0.125f, 1.0f);
            const float fovx = renderer.getViewFrustum().getFOVx();
            const float viewWidth = radarRange * tanf(0.5f * fovx);
            vboV.enableArrays();
            glPushMatrix();
            glScalef(viewWidth, radarRange, 0.0f);
            glDrawArrays(GL_LINE_STRIP, markersVBOindex, 3);
            glPopMatrix();
        }

        // transform to the observer's viewpoint
        glPushMatrix();
        glRotatef((float)(90.0 - myAngle * 180.0 / M_PI), 0.0f, 0.0f, 1.0f);
        glPushMatrix();
        glTranslatef(-myPos[0], -myPos[1], 0.0f);

        if (useTankModels)
        {
            // new modelview transform requires repositioning
            renderer.setupSun();
        }

        // setup the blending function
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // draw the buildings
        renderObstacles();

        if (smooth)
        {
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_POINT_SMOOTH);
        }

        // draw my shots
        int maxShots = world->getMaxShots();
        int i;
        float muzzleHeight = BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);
        for (i = 0; i < maxShots; i++)
        {
            const ShotPath* shot = myTank->getShot(i);
            if (shot)
            {
                const float cs = colorScale(shot->getPosition()[2], muzzleHeight);
                glColor4f(1.0f * cs, 1.0f * cs, 1.0f * cs, 1.0f);
                shot->radarRender();
            }
        }

        //draw world weapon shots
        WorldPlayer *worldWeapons = World::getWorld()->getWorldWeapons();
        maxShots = worldWeapons->getMaxShots();
        for (i = 0; i < maxShots; i++)
        {
            const ShotPath* shot = worldWeapons->getShot(i);
            if (shot)
            {
                const float cs = colorScale(shot->getPosition()[2], muzzleHeight);
                glColor4f(1.0f * cs, 1.0f * cs, 1.0f * cs, 1.0f);
                shot->radarRender();
            }
        }

        // draw other tanks (and any flags on them)
        // note about flag drawing.  each line segment is drawn twice
        // (once in each direction);  this degrades the antialiasing
        // but on systems that don't do correct filtering of endpoints
        // not doing it makes (half) the endpoints jump wildly.
        const int curMaxPlayers = world->getCurMaxPlayers();
        for (i = 0; i < curMaxPlayers; i++)
        {
            RemotePlayer* player = world->getPlayer(i);
            if (!player)
                continue;
            if (!player->isAlive() &&
                    (!useTankModels || !observer || !player->isExploding()))
                continue;
            if ((player->getFlag() == Flags::Stealth) &&
                    (myTank->getFlag() != Flags::Seer))
                continue;

            const float* position = player->getPosition();

            if (player->getFlag() != Flags::Null)
            {
                const GLfloat *c = player->getFlag()->getRadarColor();
                glColor4f(c[0], c[1], c[2], 1.0f);
                drawFlagOnTank(position);
            }

            if (!observer)
                drawTank(position, player, true);
            else
                drawTank(position, player, false);
        }

        bool coloredShot = BZDB.isTrue("coloredradarshots");
        // draw other tanks' shells
        bool iSeeAll = myTank && (myTank->getFlag() == Flags::Seer);
        maxShots = World::getWorld()->getMaxShots();
        for (i = 0; i < curMaxPlayers; i++)
        {
            RemotePlayer* player = world->getPlayer(i);
            if (!player) continue;
            for (int j = 0; j < maxShots; j++)
            {
                const ShotPath* shot = player->getShot(j);
                if (shot && (shot->getFlag() != Flags::InvisibleBullet || iSeeAll))
                {
                    const float cs = colorScale(shot->getPosition()[2], muzzleHeight);
                    if (coloredShot)
                    {
                        const float *shotcolor;
                        if (myTank->getFlag() == Flags::Colorblindness)
                            shotcolor = Team::getRadarColor(RogueTeam);
                        else
                            shotcolor = Team::getRadarColor(player->getTeam());
                        glColor4f(shotcolor[0] * cs, shotcolor[1] * cs, shotcolor[2] * cs, 1.0f);
                    }
                    else
                        glColor4f(cs, cs, cs, 1.0f);
                    shot->radarRender();
                }
            }
        }

        // draw flags not on tanks.
        // draw them in reverse order so that the team flags
        // (which come first), are drawn on top of the normal flags.
        const int maxFlags = world->getMaxFlags();
        const bool drawNormalFlags = BZDB.isTrue("displayRadarFlags");
        const bool hideTeamFlagsOnRadar = BZDB.isTrue(StateDatabase::BZDB_HIDETEAMFLAGSONRADAR);
        const bool hideFlagsOnRadar = BZDB.isTrue(StateDatabase::BZDB_HIDEFLAGSONRADAR);
        for (i = (maxFlags - 1); i >= 0; i--)
        {
            const Flag& flag = world->getFlag(i);
            // don't draw flags that don't exist or are on a tank
            if (flag.status == FlagNoExist || flag.status == FlagOnTank)
                continue;
            // don't draw normal flags if we aren't supposed to
            if (flag.type->flagTeam == NoTeam && !drawNormalFlags)
                continue;
            if (hideTeamFlagsOnRadar)
            {
                if (flag.type->flagTeam != ::NoTeam)
                    continue;
            }
            if (hideFlagsOnRadar)
            {
                if (flag.type)
                    continue;
            }
            // Flags change color by height
            const float cs = colorScale(flag.position[2], muzzleHeight);
            const float *flagcolor = flag.type->getRadarColor();
            glColor4f(flagcolor[0] * cs, flagcolor[1] * cs, flagcolor[2] * cs, 1.0f);
            drawFlag(flag.position);
        }
        // draw antidote flag
        const float* antidotePos =
            LocalPlayer::getMyTank()->getAntidoteLocation();
        if (antidotePos)
        {
            glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
            drawFlag(antidotePos);
        }

        // draw these markers above all others always centered
        glPopMatrix();

        // north marker
        GLfloat ns = 0.05f * radarRange, ny = 0.9f * radarRange;
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        vboV.enableArrays();
        glTranslatef(0.0f, ny, 0.0f);
        glScalef(ns, ns, 0);
        glDrawArrays(GL_LINE_STRIP, markersVBOindex + 3, 4);

        // always up
        glPopMatrix();

        // forward tick
        glPushMatrix();
        glTranslatef(0.0f, radarRange, 0.0f);
        glScalef(0.0f, -ps, 0.0f);
        glDrawArrays(GL_LINES, markersVBOindex + 7, 2);
        glPopMatrix();

        if (!observer)
        {
            // revert to the centered transformation
            glRotatef((float)(90.0 - myAngle * 180.0 / M_PI), 0.0f, 0.0f, 1.0f);
            glTranslatef(-myPos[0], -myPos[1], 0.0f);

            // my flag
            if (myTank->getFlag() != Flags::Null)
            {
                const GLfloat *c = myTank->getFlag()->getRadarColor();
                glColor4f(c[0], c[1], c[2], 1.0f);
                drawFlagOnTank(myPos);
            }

            // my tank
            drawTank(myPos, myTank, false);

            // re-setup the blending function
            // (was changed by drawing jump jets)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        glPopMatrix();

        if (dimming > 0.0f)
        {
            if (!smooth)
                glEnable(GL_BLEND);
            // darken the entire radar if we're dimmed
            // we're drawing positively, so dimming is actually an opacity
            glColor4f(0.0f, 0.0f, 0.0f, 1.0f - dimming);
            glPushMatrix();
            glScalef(radarRange, radarRange, 0.0f);
            DRAWER.simmetricRect();
            glPopMatrix();
        }
        glDisable(GL_BLEND);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_POINT_SMOOTH);
    }

    triangleCount = RenderNode::getTriangleCount();
}


float RadarRenderer::colorScale(const float z, const float h)
{
    float scaleColor;
    {
        const LocalPlayer* myTank = LocalPlayer::getMyTank();

        // Scale color so that objects that are close to tank's level are opaque
        const float zTank = myTank->getPosition()[2];

        if (zTank > (z + h))
            scaleColor = 1.0f - (zTank - (z + h)) / colorFactor;
        else if (zTank < z)
            scaleColor = 1.0f - (z - zTank) / colorFactor;
        else
            scaleColor = 1.0f;

        // Don't fade all the way
        if (scaleColor < 0.35f)
            scaleColor = 0.35f;
    }

    return scaleColor;
}


float RadarRenderer::transScale(const float z, const float h)
{
    float scaleColor;
    const LocalPlayer* myTank = LocalPlayer::getMyTank();

    // Scale color so that objects that are close to tank's level are opaque
    const float zTank = myTank->getPosition()[2];
    if (zTank > (z + h))
        scaleColor = 1.0f - (zTank - (z + h)) / colorFactor;
    else if (zTank < z)
        scaleColor = 1.0f - (z - zTank) / colorFactor;
    else
        scaleColor = 1.0f;

    if (scaleColor < 0.5f)
        scaleColor = 0.5f;

    return scaleColor;
}


void RadarRenderer::renderObstacles()
{
    vertices.clear();
    colors.clear();

    // prepare the boxes, pyramids, and meshes
    renderBoxPyrMeshPrepare();
    // prepare the team bases and teleporters
    renderBasesAndTelesPrepare();

    int vboIndexVC = vboVC.vboAlloc(vertices.size());
    vboVC.vertexData(vboIndexVC, vertices);
    vboVC.colorData(vboIndexVC, colors);


    curVBOIndex = vboIndexVC;

    if (smooth)
    {
        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
    }

    // draw the walls
    renderWalls();

    // draw the boxes, pyramids, and meshes
    renderBoxPyrMesh();

    // draw the team bases and teleporters
    renderBasesAndTeles();

    vboVC.vboFree(vboIndexVC);

    if (smooth)
    {
        glDisable(GL_BLEND);
        glDisable(GL_LINE_SMOOTH);
    }

    return;
}


void RadarRenderer::renderWalls()
{
    const ObstacleList& walls = OBSTACLEMGR.getWalls();
    int count = walls.size();
    if (!count)
        return;

    glColor4f(0.25f, 0.5f, 0.5f, 1.0f);
    vboV.enableArrays();
    glDrawArrays(GL_LINES, wallVBOindex, count * 2);

    return;
}


void RadarRenderer::renderWallsPrepare()
{
    const ObstacleList& walls = OBSTACLEMGR.getWalls();
    int count = walls.size();
    std::vector<glm::vec3> vertex;

    if (!count)
        return;

    for (int i = 0; i < count; i++)
    {
        const WallObstacle& wall = *((const WallObstacle*) walls[i]);
        const float wid = wall.getBreadth();
        const float c   = wid * cosf(wall.getRotation());
        const float s   = wid * sinf(wall.getRotation());
        const float* pos = wall.getPosition();
        vertex.push_back(glm::vec3(pos[0] - s, pos[1] + c, 0));
        vertex.push_back(glm::vec3(pos[0] + s, pos[1] - c, 0));
    }

    wallVBOindex = vboV.vboAlloc(vertex.size());
    vboV.vertexData(wallVBOindex, vertex);

    return;
}


void RadarRenderer::renderBoxPyrMesh()
{
    int i;

    if (!smooth)
        // smoothing has blending disabled
        glEnable(GL_BLEND);
    else
        // smoothing has blending enabled
        glDisable(GL_BLEND);

    vboVC.enableArrays();
    // draw box buildings.
    // draw pyramid buildings
    for (i = 0; i < boxPyrCount; i++)
    {
        glDrawArrays(GL_TRIANGLE_STRIP, curVBOIndex, 4);
        curVBOIndex += 4;
    }

    // draw mesh obstacles
    const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
    int count = meshes.size();
    if (smooth)
        glEnable(GL_POLYGON_SMOOTH);
    for (i = 0; i < count; i++)
    {
        const MeshObstacle* mesh = (const MeshObstacle*) meshes[i];
        int faces = mesh->getFaceCount();

        for (int f = 0; f < faces; f++)
        {
            const MeshFace* face = mesh->getFace(f);
            {
                if (face->getPlane()[2] <= 0.0f)
                    continue;
                const BzMaterial* bzmat = face->getMaterial();
                if ((bzmat != NULL) && bzmat->getNoRadar())
                    continue;
            }
            // draw the face as a triangle fan
            int vertexCount = face->getVertexCount();
            glDrawArrays(GL_TRIANGLE_FAN, curVBOIndex, vertexCount);
            curVBOIndex += vertexCount;
        }
    }
    if (smooth)
        glDisable(GL_POLYGON_SMOOTH);

    if (!smooth)
        glDisable(GL_BLEND);

    // now draw antialiased outlines around the polygons
    if (smooth)
    {
        glEnable(GL_BLEND);

        for (i = 0; i < boxPyrCount; i++)
        {
            glDrawArrays(GL_LINE_LOOP, curVBOIndex, 4);
            curVBOIndex += 4;
        }
    }

    return;
}


void RadarRenderer::renderBoxPyrMeshPrepare()
{
    int i;

    boxPyrCount = 0;

    // draw box buildings.
    const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
    int count = boxes.size();
    for (i = 0; i < count; i++)
    {
        const BoxBuilding& box = *((const BoxBuilding*) boxes[i]);
        if (box.isInvisible())
            continue;
        const float z = box.getPosition()[2];
        const float bh = box.getHeight();
        const float cs = colorScale(z, bh);
        glm::vec4 color(0.25f * cs, 0.5f * cs, 0.5f * cs, transScale(z, bh));
        const float c = cosf(box.getRotation());
        const float s = sinf(box.getRotation());
        const float wx = c * box.getWidth(), wy = s * box.getWidth();
        const float hx = -s * box.getBreadth(), hy = c * box.getBreadth();
        const float* pos = box.getPosition();
        vertices.push_back(glm::vec3(pos[0] - wx - hx, pos[1] - wy - hy, 0));
        vertices.push_back(glm::vec3(pos[0] + wx - hx, pos[1] + wy - hy, 0));
        vertices.push_back(glm::vec3(pos[0] - wx + hx, pos[1] - wy + hy, 0));
        vertices.push_back(glm::vec3(pos[0] + wx + hx, pos[1] + wy + hy, 0));
        for (int j = 4; j > 0; j--)
            colors.push_back(color);
        boxPyrCount++;
    }

    // draw pyramid buildings
    const ObstacleList& pyramids = OBSTACLEMGR.getPyrs();
    count = pyramids.size();
    for (i = 0; i < count; i++)
    {
        const PyramidBuilding& pyr = *((const PyramidBuilding*) pyramids[i]);
        const float z = pyr.getPosition()[2];
        const float bh = pyr.getHeight();
        const float cs = colorScale(z, bh);
        glm::vec4 color(0.25f * cs, 0.5f * cs, 0.5f * cs, transScale(z, bh));
        const float c = cosf(pyr.getRotation());
        const float s = sinf(pyr.getRotation());
        const float wx = c * pyr.getWidth(), wy = s * pyr.getWidth();
        const float hx = -s * pyr.getBreadth(), hy = c * pyr.getBreadth();
        const float* pos = pyr.getPosition();
        vertices.push_back(glm::vec3(pos[0] - wx - hx, pos[1] - wy - hy, 0));
        vertices.push_back(glm::vec3(pos[0] + wx - hx, pos[1] + wy - hy, 0));
        vertices.push_back(glm::vec3(pos[0] - wx + hx, pos[1] - wy + hy, 0));
        vertices.push_back(glm::vec3(pos[0] + wx + hx, pos[1] + wy + hy, 0));
        for (int j = 4; j > 0; j--)
            colors.push_back(color);
        boxPyrCount++;
    }

    // draw mesh obstacles
    const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
    count = meshes.size();
    for (i = 0; i < count; i++)
    {
        const MeshObstacle* mesh = (const MeshObstacle*) meshes[i];
        int faces = mesh->getFaceCount();

        for (int f = 0; f < faces; f++)
        {
            const MeshFace* face = mesh->getFace(f);
            {
                if (face->getPlane()[2] <= 0.0f)
                    continue;
                const BzMaterial* bzmat = face->getMaterial();
                if ((bzmat != NULL) && bzmat->getNoRadar())
                    continue;
            }

            glm::vec4 color;

            float z = face->getPosition()[2];
            float bh = face->getSize()[2];

            if (BZDBCache::useMeshForRadar)
            {
                z = mesh->getPosition()[2];
                bh = mesh->getSize()[2];
            }

            const float cs = colorScale(z, bh);
            // draw death faces with a soupcon of red
            const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(face->getPhysicsDriver());
            if ((phydrv != NULL) && phydrv->getIsDeath())
                color = glm::vec4(0.75f * cs, 0.25f * cs, 0.25f * cs, transScale(z, bh));
            else
                color = glm::vec4(0.25f * cs, 0.5f * cs, 0.5f * cs, transScale(z, bh));

            // draw the face as a triangle fan
            int vertexCount = face->getVertexCount();
            for (int v = 0; v < vertexCount; v++)
            {
                const float* pos = face->getVertex(v);
                vertices.push_back(glm::vec3(pos[0], pos[1], 0));
                colors.push_back(color);
            }
        }
    }

    // now draw antialiased outlines around the polygons
    if (smooth)
    {
        count = boxes.size();
        for (i = 0; i < count; i++)
        {
            const BoxBuilding& box = *((const BoxBuilding*) boxes[i]);
            if (box.isInvisible())
                continue;
            const float z = box.getPosition()[2];
            const float bh = box.getHeight();
            const float cs = colorScale(z, bh);
            glm::vec4 color(0.25f * cs, 0.5f * cs, 0.5f * cs, transScale(z, bh));
            const float c = cosf(box.getRotation());
            const float s = sinf(box.getRotation());
            const float wx = c * box.getWidth(), wy = s * box.getWidth();
            const float hx = -s * box.getBreadth(), hy = c * box.getBreadth();
            const float* pos = box.getPosition();

            vertices.push_back(glm::vec3(pos[0] - wx - hx, pos[1] - wy - hy, 0));
            vertices.push_back(glm::vec3(pos[0] + wx - hx, pos[1] + wy - hy, 0));
            vertices.push_back(glm::vec3(pos[0] + wx + hx, pos[1] + wy + hy, 0));
            vertices.push_back(glm::vec3(pos[0] - wx + hx, pos[1] - wy + hy, 0));
            for (int j = 4; j > 0; j--)
                colors.push_back(color);
        }

        count = pyramids.size();
        for (i = 0; i < count; i++)
        {
            const PyramidBuilding& pyr = *((const PyramidBuilding*) pyramids[i]);
            const float z = pyr.getPosition()[2];
            const float bh = pyr.getHeight();
            const float cs = colorScale(z, bh);
            glm::vec4 color(0.25f * cs, 0.5f * cs, 0.5f * cs, transScale(z, bh));
            const float c = cosf(pyr.getRotation());
            const float s = sinf(pyr.getRotation());
            const float wx = c * pyr.getWidth(), wy = s * pyr.getWidth();
            const float hx = -s * pyr.getBreadth(), hy = c * pyr.getBreadth();
            const float* pos = pyr.getPosition();

            vertices.push_back(glm::vec3(pos[0] - wx - hx, pos[1] - wy - hy, 0));
            vertices.push_back(glm::vec3(pos[0] + wx - hx, pos[1] + wy - hy, 0));
            vertices.push_back(glm::vec3(pos[0] + wx + hx, pos[1] + wy + hy, 0));
            vertices.push_back(glm::vec3(pos[0] - wx + hx, pos[1] - wy + hy, 0));
            for (int j = 4; j > 0; j--)
                colors.push_back(color);
        }
    }

    return;
}


void RadarRenderer::renderBasesAndTeles()
{
    vboVC.enableArrays();
    // draw team bases
    for (int i = baseCount; i > 0; i--)
    {
        glDrawArrays(GL_LINE_LOOP, curVBOIndex, 4);
        curVBOIndex += 4;
    }
    // draw teleporters.
    glDrawArrays(GL_LINES, curVBOIndex, teleportLineCount);
    curVBOIndex += teleportLineCount;

    return;
}


void RadarRenderer::renderBasesAndTelesPrepare()
{
    int i;
    baseCount = 0;
    teleportLineCount = 0;

    // draw team bases
    if (world->allowTeamFlags())
    {
        for (i = 1; i < NumTeams; i++)
        {
            for (int j = 0;; j++)
            {
                const float *base = world->getBase(i, j);
                if (base == NULL)
                    break;
                glm::vec4 color = glm::vec4(glm::make_vec3(Team::getRadarColor(TeamColor(i))), 1.0f);
                const float beta = atan2f(base[5], base[4]);
                const float r = hypotf(base[4], base[5]);
                vertices.push_back(glm::vec3(base[0] + r * cosf(base[3] + beta),
                                             base[1] + r * sinf(base[3] + beta), 0));
                vertices.push_back(glm::vec3(base[0] + r * cosf((float)(base[3] - beta + M_PI)),
                                             base[1] + r * sinf((float)(base[3] - beta + M_PI)), 0));
                vertices.push_back(glm::vec3(base[0] + r * cosf((float)(base[3] + beta + M_PI)),
                                             base[1] + r * sinf((float)(base[3] + beta + M_PI)), 0));
                vertices.push_back(glm::vec3(base[0] + r * cosf(base[3] - beta),
                                             base[1] + r * sinf(base[3] - beta), 0));
                for (int k = 4; k > 0; k--)
                    colors.push_back(color);
                baseCount++;
            }
        }
    }

    // draw teleporters.  teleporters are pretty thin so use lines
    // (which, if longer than a pixel, are guaranteed to draw something;
    // not so for a polygon).  just in case the system doesn't correctly
    // filter the ends of line segments, we'll draw the line in each
    // direction (which degrades the antialiasing).  Newport graphics
    // is one system that doesn't do correct filtering.
    const ObstacleList& teleporters = OBSTACLEMGR.getTeles();
    int count = teleporters.size();
    for (i = 0; i < count; i++)
    {
        const Teleporter & tele = *((const Teleporter *) teleporters[i]);
        if (tele.isHorizontal ())
        {
            const float z = tele.getPosition ()[2];
            const float bh = tele.getHeight ();
            const float cs = colorScale (z, bh);
            glm::vec4 color(1.0f * cs, 1.0f * cs, 0.25f * cs, transScale (z, bh));
            const float c = cosf (tele.getRotation ());
            const float s = sinf (tele.getRotation ());
            const float wx = c * tele.getWidth (), wy = s * tele.getWidth ();
            const float hx = -s * tele.getBreadth (), hy = c * tele.getBreadth ();
            const float *pos = tele.getPosition ();
            vertices.push_back(glm::vec3 (pos[0] - wx - hx, pos[1] - wy - hy, 0));
            vertices.push_back(glm::vec3 (pos[0] + wx - hx, pos[1] + wy - hy, 0));

            vertices.push_back(glm::vec3 (pos[0] + wx - hx, pos[1] + wy - hy, 0));
            vertices.push_back(glm::vec3 (pos[0] + wx + hx, pos[1] + wy + hy, 0));

            vertices.push_back(glm::vec3 (pos[0] + wx + hx, pos[1] + wy + hy, 0));
            vertices.push_back(glm::vec3 (pos[0] - wx + hx, pos[1] - wy + hy, 0));

            vertices.push_back(glm::vec3 (pos[0] - wx + hx, pos[1] - wy + hy, 0));
            vertices.push_back(glm::vec3 (pos[0] - wx - hx, pos[1] - wy - hy, 0));

            vertices.push_back(glm::vec3 (pos[0] - wx - hx, pos[1] - wy - hy, 0));
            vertices.push_back(glm::vec3 (pos[0] - wx - hx, pos[1] - wy - hy, 0));
            for (int j = 10; j > 0; j--)
                colors.push_back(color);
            teleportLineCount += 10;
        }
        else
        {
            const float z = tele.getPosition ()[2];
            const float bh = tele.getHeight ();
            const float cs = colorScale (z, bh);
            glm::vec4 color(1.0f * cs, 1.0f * cs, 0.25f * cs, transScale (z, bh));
            const float tw = tele.getBreadth ();
            const float c = tw * cosf (tele.getRotation ());
            const float s = tw * sinf (tele.getRotation ());
            const float *pos = tele.getPosition ();
            vertices.push_back(glm::vec3 (pos[0] - s, pos[1] + c, 0));
            vertices.push_back(glm::vec3 (pos[0] + s, pos[1] - c, 0));
            vertices.push_back(glm::vec3 (pos[0] + s, pos[1] - c, 0));
            vertices.push_back(glm::vec3 (pos[0] - s, pos[1] + c, 0));
            for (int j = 4; j > 0; j--)
                colors.push_back(color);
            teleportLineCount += 4;
        }
    }

    return;
}


int RadarRenderer::getFrameTriangleCount() const
{
    return triangleCount;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
